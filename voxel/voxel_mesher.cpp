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

// Places the dual vertex for the given lattice point by dual contouring:
// minimizing the quadratic error of the surface crossings on the 32 primal
// edges between the 16 voxels sharing the point. Returns the vertex's offset
// from the lattice point.
static Vector4 _place_vertex(const Ref<VoxelData> &p_voxel_data, const Vector4i &p_lattice_point) {
	// The quadratic error of a position x is the sum over the surface
	// crossings of (normal . (x - point))^2. In terms of the matrix A whose
	// rows are the normals and the vector b of each normal . point, that is
	// |A x - b|^2 = x^T (A^T A) x - 2 x . (A^T b) + constant.
	Basis4D ata = Basis4D(Vector4(), Vector4(), Vector4(), Vector4());
	Vector4 atb;
	Vector4 masspoint;
	int crossing_count = 0;
	for (int axis = 0; axis < 4; axis++) {
		for (int block = 0; block < 8; block++) {
			Vector4i lower = p_lattice_point;
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
			const VoxelValue lower_value = p_voxel_data->get_value(lower);
			const VoxelValue upper_value = p_voxel_data->get_value(upper);
			if (lower_value.material == upper_value.material || lower_value.material == VoxelMaterial::UNDEFINED || upper_value.material == VoxelMaterial::UNDEFINED) {
				continue;
			}
			const Vector4 normal = p_voxel_data->get_edge_normal(lower, axis);
			if (normal == Vector4()) {
				continue;
			}
			// The surface crosses the edge a / (a + b) of the way between the
			// two voxel centers.
			const real_t a = lower_value.density;
			const real_t b = upper_value.density;
			const real_t crossing = a + b > 0.0f ? a / (a + b) : 0.5f;
			Vector4 point = Vector4(lower - p_lattice_point) + Vector4(0.5f, 0.5f, 0.5f, 0.5f);
			point[axis] += crossing;
			const real_t normal_dot_point = normal.dot(point);
			for (int j = 0; j < 4; j++) {
				ata[j] += normal * normal[j];
			}
			atb += normal * normal_dot_point;
			masspoint += point;
			crossing_count++;
		}
	}
	if (crossing_count == 0) {
		return Vector4();
	}
	// Solve relative to the average crossing point, so that directions the
	// crossings do not constrain keep the vertex there instead of pulling it
	// toward the lattice point.
	masspoint /= (real_t)crossing_count;
	const Vector4 residual = atb - ata.xform(masspoint);
	Vector4 values;
	Basis4D vectors;
	VoxelMesher::eigen_decompose_symmetric_4(ata, values, vectors);
	// The pseudo-inverse applied to the residual: V * diag(inverted) * V^T * residual.
	Vector4 rotated = vectors.xform_transposed(residual);
	for (int i = 0; i < 4; i++) {
		rotated[i] *= Math::abs(values[i]) > (real_t)SMOOTHNESS ? 1.0f / values[i] : values[i] / (real_t)(SMOOTHNESS * SMOOTHNESS);
	}
	return masspoint + vectors.xform(rotated);
}

static int32_t _get_or_add_vertex(HashMap<Vector4i, int32_t> &r_vertex_indices, PackedVector4Array &r_vertices, const Ref<VoxelData> &p_voxel_data, const Vector4i &p_chunk_position, const Vector4i &p_position) {
	HashMap<Vector4i, int32_t>::Iterator found = r_vertex_indices.find(p_position);
	if (found) {
		return found->value;
	}
	const int32_t index = (int32_t)r_vertices.size();
	r_vertices.append(Vector4(p_position) + _place_vertex(p_voxel_data, p_chunk_position + p_position));
	r_vertex_indices.insert(p_position, index);
	return index;
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

Ref<Mesh4D> VoxelMesher::generate_chunk_mesh(const Ref<VoxelData> &p_voxel_data, const Vector4i &p_chunk_position) {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	ERR_FAIL_COND_V(p_voxel_data.is_null(), mesh);
	struct FaceDirection {
		Vector4i normal;
		Vector4i tangents[3];
	};
	FaceDirection face_directions[8];
	for (int face_axis = 0; face_axis < 4; face_axis++) {
		for (int side_index = 0; side_index < 2; side_index++) {
			FaceDirection &direction = face_directions[face_axis * 2 + side_index];
			direction.normal[face_axis] = side_index == 0 ? -1 : 1;
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
			const bool want_even = side_index == 1;
			if (ascending_is_even != want_even) {
				SWAP(direction.tangents[1], direction.tangents[2]);
			}
		}
	}
	PackedVector4Array vertices;
	PackedInt32Array cell_indices;
	HashMap<Vector4i, int32_t> vertex_indices;
	Vector4i local = Vector4i();
	for (local.w = 0; local.w < VOXEL_MESH_CHUNK_SIZE; local.w++) {
		for (local.z = 0; local.z < VOXEL_MESH_CHUNK_SIZE; local.z++) {
			for (local.y = 0; local.y < VOXEL_MESH_CHUNK_SIZE; local.y++) {
				for (local.x = 0; local.x < VOXEL_MESH_CHUNK_SIZE; local.x++) {
					const Vector4i voxel = p_chunk_position + local;
					if (!p_voxel_data->get_value(voxel).is_opaque()) {
						continue;
					}
					for (int direction_index = 0; direction_index < 8; direction_index++) {
						const FaceDirection &direction = face_directions[direction_index];
						const Vector4i facing_voxel = voxel + direction.normal;
						if (p_voxel_data->get_value(facing_voxel).material != VoxelMaterial::AIR) {
							continue;
						}
						const Vector4i base_corner = local + direction.normal.maxi(0);
						int32_t corner_indices[8];
						for (int i = 0; i < 8; i++) {
							Vector4i corner = base_corner;
							for (int bit = 0; bit < 3; bit++) {
								if (i & (1 << bit)) {
									corner += direction.tangents[bit];
								}
							}
							corner_indices[i] = _get_or_add_vertex(vertex_indices, vertices, p_voxel_data, p_chunk_position, corner);
						}
						// Pick the decomposition whose central tetrahedron sits on the
						// corners with even world-coordinate parity. Every square face
						// is then cut along its even-parity diagonal, a global rule, so
						// any two cubes sharing a square triangulate it identically.
						const Vector4i world_base = p_chunk_position + base_corner;
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
