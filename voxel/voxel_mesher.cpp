#include "voxel_mesher.h"

#include "../model/mesh/tetra/array_tetra_mesh_4d.h"

// The threshold between well-constrained and weakly-constrained directions of
// a vertex's quadratic error function: eigenvalues above it are inverted
// exactly, snapping the vertex onto sharp features, while eigenvalues below
// it are damped, keeping the vertex near the average of its surface
// crossings where the crossings do not constrain it.
static constexpr double SMOOTHNESS = 0.25;

// Cyclic Jacobi rotations. The tolerances are chosen so that the error
// contributed to a vertex position stays around 1e-2 or less: the damping
// function applied to the decomposition is (1 / SMOOTHNESS^2)-Lipschitz and
// the residual vector it is applied to has magnitude at most a few dozen, so
// an off-diagonal residue of about 1e-5 is small enough.
void VoxelMesher::eigen_decompose_symmetric_4(const Basis4D &p_matrix, Vector4 &r_values, Basis4D &r_vectors) {
	Basis4D a = p_matrix;
	r_vectors = Basis4D();
	for (int sweep = 0; sweep < 16; sweep++) {
		real_t off_diagonal = 0.0f;
		for (int i = 0; i < 4; i++) {
			for (int j = i + 1; j < 4; j++) {
				off_diagonal += a[i][j] * a[i][j];
			}
		}
		if (off_diagonal < 1e-10f) {
			break;
		}
		for (int p = 0; p < 4; p++) {
			for (int q = p + 1; q < 4; q++) {
				if (Math::abs(a[p][q]) < (real_t)1e-6) {
					continue;
				}
				const real_t theta = 0.5f * Math::atan2(2.0f * a[p][q], a[q][q] - a[p][p]);
				const real_t c = Math::cos(theta);
				const real_t s = Math::sin(theta);
				const Vector4 column_p = a[p];
				const Vector4 column_q = a[q];
				a[p] = column_p * c - column_q * s;
				a[q] = column_p * s + column_q * c;
				for (int k = 0; k < 4; k++) {
					const real_t akp = a[k][p];
					const real_t akq = a[k][q];
					a[k][p] = c * akp - s * akq;
					a[k][q] = s * akp + c * akq;
				}
				const Vector4 vectors_p = r_vectors[p];
				const Vector4 vectors_q = r_vectors[q];
				r_vectors[p] = vectors_p * c - vectors_q * s;
				r_vectors[q] = vectors_p * s + vectors_q * c;
			}
		}
	}
	r_values = Vector4(a[0][0], a[1][1], a[2][2], a[3][3]);
}

// A cell's 32 primal edges are identified by the edge's axis and the position
// of its lower voxel among the cell's 16 voxels, packed into the slot's low
// bits in ascending axis order, skipping the edge's own axis.
static int _edge_slot(const int p_axis, const Vector4i &p_cell_local_lower) {
	int block = 0;
	int bit = 0;
	for (int i = 0; i < 4; i++) {
		if (i == p_axis) {
			continue;
		}
		block |= p_cell_local_lower[i] << bit;
		bit++;
	}
	return p_axis * 8 + block;
}

// Minimizes the quadratic error of a group of surface crossings. Solves
// relative to the average crossing point, so that directions the crossings do
// not constrain keep the vertex there instead of pulling it elsewhere.
static Vector4 _solve_vertex(const Basis4D &p_ata, const Vector4 &p_atb, const Vector4 &p_point_sum, const int p_crossing_count) {
	const Vector4 masspoint = p_point_sum / (real_t)p_crossing_count;
	const Vector4 residual = p_atb - p_ata.xform(masspoint);
	Vector4 values;
	Basis4D vectors;
	VoxelMesher::eigen_decompose_symmetric_4(p_ata, values, vectors);
	// The pseudo-inverse applied to the residual: V * diag(inverted) * V^T * residual.
	Vector4 rotated = vectors.xform_transposed(residual);
	for (int i = 0; i < 4; i++) {
		rotated[i] *= Math::abs(values[i]) > (real_t)SMOOTHNESS ? 1.0f / values[i] : values[i] / (real_t)(SMOOTHNESS * SMOOTHNESS);
	}
	return masspoint + vectors.xform(rotated);
}

// How far a couple's two crossings are from lying along each other's surfaces.
static real_t _couple_mismatch(const Vector4 &p_point_1, const Vector4 &p_normal_1, const Vector4 &p_point_2, const Vector4 &p_normal_2) {
	const Vector4 offset = p_point_2 - p_point_1;
	const real_t offset_1 = p_normal_1.dot(offset);
	const real_t offset_2 = p_normal_2.dot(offset);
	return offset_1 * offset_1 + offset_2 * offset_2;
}

static int _find_surface_root(int *p_parent, int p_slot) {
	while (p_parent[p_slot] != p_slot) {
		p_parent[p_slot] = p_parent[p_parent[p_slot]];
		p_slot = p_parent[p_slot];
	}
	return p_slot;
}

// Connects the surfaces containing the two crossings.
static void _union_surfaces(int *p_parent, const int p_slot_a, const int p_slot_b) {
	p_parent[_find_surface_root(p_parent, p_slot_a)] = _find_surface_root(p_parent, p_slot_b);
}

// Connects the surfaces of the crossings on one square's edges, given in
// cyclic order. 2 or 3 crossings all belong to one surface. 4 crossings pair
// into two surfaces as the cyclically adjacent couples with the smaller total
// mismatch, or, when the normals are all too close to perpendicular to the
// square to tell the pairings apart, as the couples of nearest points.
// p_points and p_normals are indexed by edge slot, like p_active.
static void _pair_square_crossings(int *p_union_set, const bool *p_active, const int p_slots[4], const Vector4 *p_points, const Vector4 *p_normals) {
	int active_count = 0;
	for (int k = 0; k < 4; k++) {
		if (p_active[p_slots[k]]) {
			active_count++;
		}
	}
	if (active_count < 2) {
		return;
	}
	if (active_count < 4) {
		int first = -1;
		for (int k = 0; k < 4; k++) {
			if (!p_active[p_slots[k]]) {
				continue;
			}
			if (first < 0) {
				first = k;
				continue;
			}
			_union_surfaces(p_union_set, p_slots[k], p_slots[first]);
		}
		return;
	}
	// The crossings all lie in the square's plane, so the full 4D points and
	// normals give the same mismatches as their in-square projections would.
	const Vector4 &point_0 = p_points[p_slots[0]];
	const Vector4 &point_1 = p_points[p_slots[1]];
	const Vector4 &point_2 = p_points[p_slots[2]];
	const Vector4 &point_3 = p_points[p_slots[3]];
	real_t total_adjacent = _couple_mismatch(point_0, p_normals[p_slots[0]], point_1, p_normals[p_slots[1]]) + _couple_mismatch(point_2, p_normals[p_slots[2]], point_3, p_normals[p_slots[3]]);
	real_t total_opposite = _couple_mismatch(point_0, p_normals[p_slots[0]], point_3, p_normals[p_slots[3]]) + _couple_mismatch(point_1, p_normals[p_slots[1]], point_2, p_normals[p_slots[2]]);
	if (total_adjacent == total_opposite) {
		total_adjacent = point_0.distance_squared_to(point_1) + point_2.distance_squared_to(point_3);
		total_opposite = point_0.distance_squared_to(point_3) + point_1.distance_squared_to(point_2);
	}
	if (total_adjacent <= total_opposite) {
		_union_surfaces(p_union_set, p_slots[1], p_slots[0]);
		_union_surfaces(p_union_set, p_slots[3], p_slots[2]);
	} else {
		_union_surfaces(p_union_set, p_slots[3], p_slots[0]);
		_union_surfaces(p_union_set, p_slots[2], p_slots[1]);
	}
}

struct CellSurfaces {
	// The surface each of the cell's 32 primal edge crossings belongs to, -1
	// where the edge has no crossing.
	int8_t edge_surfaces[32];
	// The mesh vertex placed for each surface.
	LocalVector<int32_t> vertex_indices;
};

// Computes the given cell's surfaces and their dual vertices, one vertex per
// surface: crossings are grouped into surfaces as connected components, where
// each of the cell's 24 squares connects its crossings as decided by
// _pair_square_crossings.
static const CellSurfaces &_get_cell_surfaces(HashMap<Vector4i, CellSurfaces> &r_cells, PackedVector4Array &r_vertices, const VoxelDataNeighbourhood &p_neighbourhood, const Vector4i &p_chunk_position, const Vector4i &p_lattice_local) {
	CellSurfaces *existing = r_cells.getptr(p_lattice_local);
	if (existing != nullptr) {
		return *existing;
	}
	CellSurfaces &cell = r_cells.insert(p_lattice_local, CellSurfaces())->value;
	for (int slot = 0; slot < 32; slot++) {
		cell.edge_surfaces[slot] = -1;
	}
	const Vector4i lattice_point = p_chunk_position + p_lattice_local;
	bool active[32] = {};
	Vector4 points[32];
	Vector4 normals[32];
	for (int axis = 0; axis < 4; axis++) {
		for (int block = 0; block < 8; block++) {
			Vector4i lower = lattice_point;
			lower[axis] -= 1;
			int bit = 0;
			for (int i = 0; i < 4; i++) {
				if (i == axis) {
					continue;
				}
				if ((block & (1 << bit)) == 0) {
					lower[i] -= 1;
				}
				bit++;
			}
			Vector4i upper = lower;
			upper[axis] += 1;
			const VoxelMaterial lower_material = p_neighbourhood.get_material(lower);
			const VoxelMaterial upper_material = p_neighbourhood.get_material(upper);
			if (lower_material == upper_material || lower_material == VoxelMaterial::UNDEFINED || upper_material == VoxelMaterial::UNDEFINED) {
				continue;
			}
			// Every active edge between defined voxels has stored data.
			const VoxelEdgeData edge_data = p_neighbourhood.get_edge_data(lower, axis);
			const int slot = axis * 8 + block;
			active[slot] = true;
			normals[slot] = edge_data.decode_normal();
			Vector4 point = Vector4(lower - lattice_point) + Vector4(0.5f, 0.5f, 0.5f, 0.5f);
			point[axis] += edge_data.decode_position();
			points[slot] = point;
		}
	}
	// Used to track connected components.
	// Godot's existing DisjointSet would be slower.
	int union_set[32];
	for (int slot = 0; slot < 32; slot++) {
		union_set[slot] = slot;
	}
	for (int axis_a = 0; axis_a < 4; axis_a++) {
		for (int axis_b = axis_a + 1; axis_b < 4; axis_b++) {
			int other_axes[2];
			int other_axis_count = 0;
			for (int i = 0; i < 4; i++) {
				if (i != axis_a && i != axis_b) {
					other_axes[other_axis_count++] = i;
				}
			}
			for (int square = 0; square < 4; square++) {
				Vector4i base = Vector4i();
				base[other_axes[0]] = square & 1;
				base[other_axes[1]] = square >> 1;
				// The square's edges in cyclic order: bottom, right, top, left
				// in the (axis_a, axis_b) plane.
				int slots[4];
				Vector4i lower = base;
				slots[0] = _edge_slot(axis_a, lower);
				lower[axis_a] = 1;
				slots[1] = _edge_slot(axis_b, lower);
				lower[axis_a] = 0;
				lower[axis_b] = 1;
				slots[2] = _edge_slot(axis_a, lower);
				lower[axis_b] = 0;
				slots[3] = _edge_slot(axis_b, lower);
				_pair_square_crossings(union_set, active, slots, points, normals);
			}
		}
	}
	int8_t root_surfaces[32];
	for (int slot = 0; slot < 32; slot++) {
		root_surfaces[slot] = -1;
	}
	int surface_count = 0;
	for (int slot = 0; slot < 32; slot++) {
		if (!active[slot]) {
			continue;
		}
		const int root = _find_surface_root(union_set, slot);
		if (root_surfaces[root] < 0) {
			root_surfaces[root] = (int8_t)surface_count;
			surface_count++;
		}
		cell.edge_surfaces[slot] = root_surfaces[root];
	}
	for (int surface = 0; surface < surface_count; surface++) {
		// The quadratic error of a position x is the sum over the surface's
		// crossings of (normal . (x - point))^2. In terms of the matrix A
		// whose rows are the normals and the vector b of each normal . point,
		// that is |A x - b|^2 = x^T (A^T A) x - 2 x . (A^T b) + constant.
		Basis4D ata = Basis4D(Vector4(), Vector4(), Vector4(), Vector4());
		Vector4 atb;
		Vector4 point_sum;
		int crossing_count = 0;
		for (int slot = 0; slot < 32; slot++) {
			if (cell.edge_surfaces[slot] != (int8_t)surface) {
				continue;
			}
			const Vector4 &normal = normals[slot];
			const Vector4 &point = points[slot];
			for (int j = 0; j < 4; j++) {
				ata[j] += normal * normal[j];
			}
			atb += normal * normal.dot(point);
			point_sum += point;
			crossing_count++;
		}
		cell.vertex_indices.push_back((int32_t)r_vertices.size());
		r_vertices.append(Vector4(p_lattice_local) + _solve_vertex(ata, atb, point_sum, crossing_count));
	}
	return cell;
}

// The two mirror-image ways to split a cube into 5 tets, with all tets having
// the same winding order.
constexpr int32_t FACE_CELLS_EVEN[5][4] = {
	{ 0, 3, 5, 6 },
	{ 0, 3, 1, 5 },
	{ 0, 3, 6, 2 },
	{ 4, 0, 5, 6 },
	{ 3, 7, 5, 6 },
};
constexpr int32_t FACE_CELLS_ODD[5][4] = {
	{ 1, 2, 7, 4 },
	{ 1, 2, 4, 0 },
	{ 1, 2, 3, 7 },
	{ 5, 1, 7, 4 },
	{ 2, 6, 7, 4 },
};

Ref<Mesh4D> VoxelMesher::generate_chunk_mesh(const VoxelDataNeighbourhood &p_neighbourhood, const Vector4i &p_chunk_position) {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	ERR_FAIL_NULL_V(p_neighbourhood.node, mesh);
	struct FaceDirection {
		Vector4i tangents[3];
	};
	FaceDirection face_directions[8];
	for (int face_axis = 0; face_axis < 4; face_axis++) {
		for (int side_index = 0; side_index < 2; side_index++) {
			FaceDirection &direction = face_directions[face_axis * 2 + side_index];
			int spanning_axis_count = 0;
			for (int i = 0; i < 4; i++) {
				if (i != face_axis) {
					Vector4i step = Vector4i();
					step[i] = 1;
					direction.tangents[spanning_axis_count++] = step;
				}
			}
			// Order the axes so that (normal, tangents[...]) is an
			// even permutation of (X, Y, Z, W) for a positive face and an odd
			// one for a negative face, making the cells' winding correct with
			// no per-cell orientation check. In ascending order the parity of
			// the permutation is the parity of the face axis's index.
			const bool ascending_is_even = (face_axis & 1) == 0;
			const bool want_even = side_index == 0;
			if (ascending_is_even != want_even) {
				SWAP(direction.tangents[1], direction.tangents[2]);
			}
		}
	}
	PackedVector4Array vertices;
	PackedInt32Array cell_indices;
	HashMap<Vector4i, CellSurfaces> cells;
	Vector4i local = Vector4i();
	for (local.w = 0; local.w < VOXEL_MESH_CHUNK_SIZE; local.w++) {
		for (local.z = 0; local.z < VOXEL_MESH_CHUNK_SIZE; local.z++) {
			for (local.y = 0; local.y < VOXEL_MESH_CHUNK_SIZE; local.y++) {
				for (local.x = 0; local.x < VOXEL_MESH_CHUNK_SIZE; local.x++) {
					const Vector4i voxel = p_chunk_position + local;
					const VoxelMaterial material = p_neighbourhood.get_material(voxel);
					if (material == VoxelMaterial::UNDEFINED) {
						continue;
					}
					for (int axis = 0; axis < 4; axis++) {
						Vector4i upper_voxel = voxel;
						upper_voxel[axis]++;
						const VoxelMaterial upper_material = p_neighbourhood.get_material(upper_voxel);
						// A face separates a solid voxel from an air voxel, and
						// belongs to the chunk containing the lower voxel of the
						// edge it crosses, like the edge's surface data.
						const bool lower_solid = is_material_opaque(material) && upper_material == VoxelMaterial::AIR;
						const bool upper_solid = is_material_opaque(upper_material) && material == VoxelMaterial::AIR;
						if (!lower_solid && !upper_solid) {
							continue;
						}
						// The face's winding must put its normal on the air side.
						const FaceDirection &direction = face_directions[axis * 2 + (lower_solid ? 0 : 1)];
						const Vector4i edge_lower = local;
						Vector4i edge_upper = local;
						edge_upper[axis]++;
						int32_t corner_indices[8];
						for (int i = 0; i < 8; i++) {
							Vector4i corner = edge_upper;
							for (int bit = 0; bit < 3; bit++) {
								if (i & (1 << bit)) {
									corner += direction.tangents[bit];
								}
							}
							// The vertex of the surface crossing this face's
							// primal edge, which is active, so it always has
							// a surface.
							const CellSurfaces &cell_surfaces = _get_cell_surfaces(cells, vertices, p_neighbourhood, p_chunk_position, corner);
							const int slot = _edge_slot(axis, edge_lower - corner + Vector4i(1, 1, 1, 1));
							corner_indices[i] = cell_surfaces.vertex_indices[cell_surfaces.edge_surfaces[slot]];
						}
						// Pick the decomposition whose central tetrahedron sits on the
						// corners with even world-coordinate parity. Every square face
						// is then cut along its even-parity diagonal, a global rule, so
						// any two cubes sharing a square triangulate it identically.
						const Vector4i world_base = p_chunk_position + edge_upper;
						const bool base_parity_odd = ((world_base.x + world_base.y + world_base.z + world_base.w) & 1) != 0;
						const int32_t (*face_cells)[4] = base_parity_odd ? FACE_CELLS_ODD : FACE_CELLS_EVEN;
						for (int cell = 0; cell < 5; cell++) {
							cell_indices.append(corner_indices[face_cells[cell][0]]);
							cell_indices.append(corner_indices[face_cells[cell][1]]);
							cell_indices.append(corner_indices[face_cells[cell][2]]);
							cell_indices.append(corner_indices[face_cells[cell][3]]);
						}
					}
				}
			}
		}
	}
	mesh->set_vertices(vertices);
	mesh->set_simplex_cell_indices(cell_indices);
	mesh->set_flat_shading_normals();
	return mesh;
}
