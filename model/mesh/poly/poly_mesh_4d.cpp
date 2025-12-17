#include "poly_mesh_4d.h"

#include "../../../math/math_4d.h"
#include "../../../math/vector_4d.h"
#include "../material_4d.h"
#include "array_poly_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#elif GODOT_MODULE
#include "scene/resources/material.h"
#include "scene/resources/surface_tool.h"
#endif

bool PolyMesh4D::is_poly_mesh_data_valid() {
	if (likely(_is_poly_mesh_data_valid)) {
		return true;
	}
	_is_poly_mesh_data_valid = validate_mesh_data();
	if (!_is_poly_mesh_data_valid) {
		ERR_PRINT("Mesh4D: Mesh data is invalid on mesh '" + get_name() + "'.");
	}
	return _is_poly_mesh_data_valid;
}

void PolyMesh4D::reset_poly_mesh_data_validation() {
	_is_poly_mesh_data_valid = false;
	reset_mesh_data_validation();
}

bool PolyMesh4D::validate_mesh_data() {
	if (_validate_poly_mesh_data_only()) {
		_is_poly_mesh_data_valid = true;
	} else {
		_is_poly_mesh_data_valid = false;
		return false;
	}
	// Also check that the result of converting the poly data into tetra data is valid.
	// This function is used to validate if a mesh is good for rendering, so we need to check this.
	return TetraMesh4D::validate_mesh_data();
}

bool PolyMesh4D::_validate_poly_mesh_data_only() {
	const Vector<Vector<PackedInt32Array>> poly_cell_indices = get_poly_cell_indices();
	const PackedInt32Array edge_indices = get_edge_indices();
	const PackedVector4Array poly_cell_vertices = get_poly_cell_vertices();
	const int64_t edge_index_count = edge_indices.size();
	if (edge_index_count % 2 != 0) {
		return false;
	}
	const int64_t vertex_count = poly_cell_vertices.size();
	for (int64_t i = 0; i < edge_index_count; i++) {
		if (edge_indices[i] < 0 || edge_indices[i] >= vertex_count) {
			return false;
		}
	}
	const int64_t poly_cell_dims = poly_cell_indices.size();
	const int64_t edge_count = edge_index_count / 2;
	if (poly_cell_dims != 0) {
		Vector<PackedInt32Array> cells_of_prev_dim;
		int64_t prev_dim_count = edge_count;
		for (int64_t dim = 0; dim < poly_cell_dims; dim++) {
			const Vector<PackedInt32Array> &cells_of_dim = poly_cell_indices[dim];
			for (int64_t cell_idx = 0; cell_idx < cells_of_dim.size(); cell_idx++) {
				const PackedInt32Array &cell = cells_of_dim[cell_idx];
				if (cell.size() < dim + 2) {
					return false;
				}
				for (int64_t i = 0; i < cell.size(); i++) {
					if (cell[i] < 0 || cell[i] >= prev_dim_count) {
						return false;
					}
				}
				bool is_common = false;
				if (dim == 0) {
					is_common = _do_edges_have_common_vertex(edge_indices[cell[0] * 2], edge_indices[cell[0] * 2 + 1], edge_indices[cell[1] * 2], edge_indices[cell[1] * 2 + 1]);
				} else {
					int64_t common_in_first = 0;
					int64_t common_in_second = 0;
					int32_t common_edge = Math4D::find_common_int32(cells_of_prev_dim[cell[0]], cells_of_prev_dim[cell[1]], common_in_first, common_in_second);
					is_common = common_edge != INT32_MIN;
				}
				if (!is_common) {
					// This problem may be common so let's give a descriptive error message.
					const String cell_str = String(Variant(cell));
					if (dim == 0) {
						ERR_PRINT("The first two edges of face " + cell_str + " do not share a common vertex, therefore orientation is not determinable and the face is invalid.");
					} else if (dim == 1) {
						ERR_PRINT("The first two faces of cell " + cell_str + " do not share a common edge, therefore orientation is not determinable and the cell is invalid.");
					} else if (dim == 2) {
						ERR_PRINT("The first two 3D cells of 4D cell " + cell_str + " do not share a common face, therefore orientation is not determinable and the 4D cell is invalid.");
					} else {
						ERR_PRINT("The first two " + itos(dim + 1) + "D cells of " + itos(dim + 2) + "D cell " + cell_str + " do not share a common " + itos(dim) + "D cell, therefore orientation is not determinable and the " + itos(dim + 2) + "D cell is invalid.");
					}
					return false;
				}
			}
			cells_of_prev_dim = cells_of_dim;
			prev_dim_count = cells_of_dim.size();
		}
	}
	const PackedVector4Array poly_cell_normals = get_poly_cell_boundary_normals();
	const int64_t poly_cell_normals_count = poly_cell_normals.size();
	if (poly_cell_normals_count != 0) {
		if (poly_cell_dims < 2) {
			return false;
		}
		const Vector<PackedInt32Array> &cells_3d = poly_cell_indices[1];
		if (cells_3d.size() != poly_cell_normals_count) {
			return false;
		}
	}
	const Vector<PackedVector3Array> poly_cell_uvw_map = get_poly_cell_texture_map();
	const int64_t poly_cell_uvw_map_count = poly_cell_uvw_map.size();
	if (poly_cell_uvw_map_count != 0) {
		if (poly_cell_dims < 2) {
			return false; // Invalid to have UVW texture map data without any 3D cells to map to.
		}
		const Vector<PackedInt32Array> &cells_3d = poly_cell_indices[1];
		if (cells_3d.size() != poly_cell_uvw_map_count) {
			return false; // Invalid number of UVW texture maps, should match number of 3D cells.
		}
		const Vector<PackedInt32Array> cell_vert = _get_vertex_indices_of_boundary_cells(poly_cell_indices, edge_indices, false);
		for (int64_t i = 0; i < poly_cell_uvw_map_count; i++) {
			if (poly_cell_uvw_map[i].is_empty()) {
				continue; // Allow unmapped 3D cells.
			}
			if (poly_cell_uvw_map[i].size() != cell_vert[i].size()) {
				return false; // Each UVW texture map entry corresponds to a vertex instance in the 3D cell.
			}
		}
	}
	return true;
}

Vector3 PolyMesh4D::_average_vector3(const PackedVector3Array &p_vector3_array) {
	const int64_t count = p_vector3_array.size();
	if (count == 0) {
		return Vector3();
	}
	Vector3 sum = Vector3();
	for (int64_t i = 0; i < count; i++) {
		sum += p_vector3_array[i];
	}
	return sum / (real_t)count;
}

int64_t PolyMesh4D::_append_vertex_internal(PackedVector4Array &r_vertices, const Vector4 &p_vertex, const bool p_deduplicate) {
	const int64_t vertex_count = r_vertices.size();
	if (p_deduplicate) {
		for (int64_t i = 0; i < vertex_count; i++) {
			if (r_vertices[i] == p_vertex) {
				return i;
			}
		}
	}
	r_vertices.append(p_vertex);
	return vertex_count;
}

int32_t PolyMesh4D::_compare_triangulation_alignment(const PackedInt32Array &p_a, const PackedInt32Array &p_b) {
	const int64_t size = p_a.size();
	ERR_FAIL_COND_V(size != p_b.size() || size < 3 || (size % 3) != 0, -1000000000);
	if (size == 3) {
		return size; // All triangulations of a single triangle line up perfectly.
	}
	if (size == 6) {
		// Special case: Triangulating a quadrilateral is aligned if starting from the opposite vertex.
		if (p_a[0] == p_b[2] && p_a[1] == p_b[5] && p_a[2] == p_b[0] && p_a[3] == p_b[4] && p_a[4] == p_b[3] && p_a[5] == p_b[1]) {
			return size;
		}
	}
	// General case: Triangulating is only aligned when starting from the same vertex.
	if (p_a[0] == p_b[0] && p_a[1] == p_b[1] && p_a[2] == p_b[2]) {
		return size;
	}
	// Be harsh on misalignment to strongly discourage it.
	return -1000 * size;
}

PackedInt32Array PolyMesh4D::_get_canonical_span_vertex_index_sequence(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const PackedInt32Array &p_all_edge_indices, const int64_t p_indices_dim_index, const int64_t p_which_cell) {
	const PackedInt32Array &indices = p_poly_cell_indices[p_indices_dim_index][p_which_cell];
	if (p_indices_dim_index == 0) {
		// Given a 2D face (dim index 0) made of edges, get 3 vertex indices from the first 2 edges in the face's winding order.
		return _get_face_edge_3_vertex_index_sequence(
				p_all_edge_indices[indices[0] * 2], p_all_edge_indices[indices[0] * 2 + 1],
				p_all_edge_indices[indices[1] * 2], p_all_edge_indices[indices[1] * 2 + 1]);
	}
	const int64_t prev_dim_index = p_indices_dim_index - 1;
	const Vector<PackedInt32Array> &low_dim = p_poly_cell_indices[prev_dim_index];
	if (p_indices_dim_index == 1) {
		// Given a 3D cell (dim index 1) made of faces of dim 0, get 4 vertex indices from 3 edges from the first 2 faces.
		return _get_cell_face_4_vertex_index_sequence(p_all_edge_indices, low_dim[indices[0]], low_dim[indices[1]]);
	}
	// General recursive case: Given a 4+ dimensional cell (dim index N>1) made of cells of dim N-1, get the vertex indices that make up those cells.
	const PackedInt32Array first_edges = _get_edges_of_cell(p_poly_cell_indices, prev_dim_index, indices[0]);
	const PackedInt32Array second_edges = _get_edges_of_cell(p_poly_cell_indices, prev_dim_index, indices[1]);
	PackedInt32Array ret = _get_canonical_span_vertex_index_sequence(p_poly_cell_indices, p_all_edge_indices, prev_dim_index, indices[0]);
	// We need to add one vertex only from the second cell that extends the span.
	for (const int32_t second_edge : second_edges) {
		if (first_edges.has(second_edge)) {
			continue;
		}
		const int32_t edge_start_vertex = p_all_edge_indices[second_edge * 2];
		const int32_t edge_end_vertex = p_all_edge_indices[second_edge * 2 + 1];
		if (ret.has(edge_start_vertex)) {
			ret.append(edge_end_vertex);
			return ret;
		}
		if (ret.has(edge_end_vertex)) {
			ret.append(edge_start_vertex);
			return ret;
		}
	}
	ERR_FAIL_V_MSG(ret, "PolyMesh4D: Could not find a vertex in the second cell that extends the span of the first cell in a way connected to the existing canonical span vertices.");
}

// These internal functions can be fast and exclude most ERR_FAIL_* checks because
// they are only called after `validate_poly_mesh_data_only()` has returned true.
// If something can crash these functions, it should be caught by that validation first.
PackedInt32Array PolyMesh4D::_get_cell_face_4_vertex_index_sequence(const PackedInt32Array &p_all_edge_indices, const PackedInt32Array &p_first_face, const PackedInt32Array &p_second_face) {
	int64_t common_in_first = 0;
	int64_t common_in_second = 0;
	int32_t common_edge = Math4D::find_common_int32(p_first_face, p_second_face, common_in_first, common_in_second);
	CRASH_COND_MSG(common_edge == INT32_MIN, "PolyMesh4D: Cell faces do not share a common item, this cell's initial 2 faces are invalid.");
	const int64_t first_next_index = (common_in_first + 1) % p_first_face.size();
	const int64_t second_next_index = (common_in_second + 1) % p_second_face.size();
	// Use these 3 edges to get 4 vertex indices in a consistent "winding" order.
	const int32_t common_vertex_start = p_all_edge_indices[common_edge * 2];
	const int32_t common_vertex_end = p_all_edge_indices[common_edge * 2 + 1];
	int32_t first_next_vertex = p_all_edge_indices[p_first_face[first_next_index] * 2];
	if (first_next_vertex == common_vertex_start || first_next_vertex == common_vertex_end) {
		first_next_vertex = p_all_edge_indices[p_first_face[first_next_index] * 2 + 1];
	}
	int32_t second_next_vertex = p_all_edge_indices[p_second_face[second_next_index] * 2];
	if (second_next_vertex == common_vertex_start || second_next_vertex == common_vertex_end) {
		second_next_vertex = p_all_edge_indices[p_second_face[second_next_index] * 2 + 1];
	}
	return PackedInt32Array{ first_next_vertex, common_vertex_start, common_vertex_end, second_next_vertex };
}

PackedInt32Array PolyMesh4D::_get_face_edge_3_vertex_index_sequence(const int32_t p_edge1_a, const int32_t p_edge1_b, const int32_t p_edge2_a, const int32_t p_edge2_b) {
	// Deduplicate the matching vertex index and place it in the middle.
	if (p_edge1_a == p_edge2_a) {
		return PackedInt32Array{ p_edge1_b, p_edge1_a, p_edge2_b };
	} else if (p_edge1_a == p_edge2_b) {
		return PackedInt32Array{ p_edge1_b, p_edge1_a, p_edge2_a };
	} else if (p_edge1_b == p_edge2_a) {
		return PackedInt32Array{ p_edge1_a, p_edge1_b, p_edge2_b };
	} else if (p_edge1_b == p_edge2_b) {
		return PackedInt32Array{ p_edge1_a, p_edge1_b, p_edge2_a };
	}
	CRASH_NOW_MSG("PolyMesh4D: Edges do not share a vertex, this face's initial 2 edges are invalid.");
	return PackedInt32Array();
}

PackedInt32Array PolyMesh4D::_get_edges_of_cell(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const int64_t p_cell_dim_index, const int64_t p_which_cell) {
	const PackedInt32Array cell_indices = p_poly_cell_indices[p_cell_dim_index][p_which_cell];
	if (p_cell_dim_index == 0) {
		// Given a 2D face (dim index 0) made of edges, it's already a list of edges.
		return cell_indices;
	}
	// Given a 3D cell (dim index 1) or higher, run this function recursively.
	PackedInt32Array ret;
	for (int64_t i = 0; i < cell_indices.size(); i++) {
		PackedInt32Array face_edges = _get_edges_of_cell(p_poly_cell_indices, p_cell_dim_index - 1, cell_indices[i]);
		if (i == 0) {
			ret = face_edges;
			continue;
		}
		for (int64_t j = 0; j < face_edges.size(); j++) {
			if (!ret.has(face_edges[j])) {
				ret.append(face_edges[j]);
			}
		}
	}
	return ret;
}

PackedInt32Array PolyMesh4D::_get_vertex_indices_of_cell(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const PackedInt32Array &p_all_edge_indices, const int64_t p_cell_dim_index, const int64_t p_which_cell, const bool p_start_with_canonical_span) {
	const PackedInt32Array cell_edges = _get_edges_of_cell(p_poly_cell_indices, p_cell_dim_index, p_which_cell);
	PackedInt32Array ret;
	if (p_start_with_canonical_span) {
		ret = _get_canonical_span_vertex_index_sequence(p_poly_cell_indices, p_all_edge_indices, p_cell_dim_index, p_which_cell);
	}
	for (int64_t i = 0; i < cell_edges.size(); i++) {
		const int32_t edge_start_vertex = p_all_edge_indices[cell_edges[i] * 2];
		const int32_t edge_end_vertex = p_all_edge_indices[cell_edges[i] * 2 + 1];
		if (!ret.has(edge_start_vertex)) {
			ret.append(edge_start_vertex);
		}
		if (!ret.has(edge_end_vertex)) {
			ret.append(edge_end_vertex);
		}
	}
	return ret;
}

PackedInt32Array PolyMesh4D::_get_vertex_indices_of_face(const PackedInt32Array &p_all_edge_indices, const PackedInt32Array &p_face_edge_indices) {
	if (p_face_edge_indices.is_empty()) {
		return PackedInt32Array();
	}
	if (p_face_edge_indices.size() == 1) {
		return PackedInt32Array{
			p_all_edge_indices[p_face_edge_indices[0] * 2],
			p_all_edge_indices[p_face_edge_indices[0] * 2 + 1]
		};
	}
	PackedInt32Array ret = _get_face_edge_3_vertex_index_sequence(
			p_all_edge_indices[p_face_edge_indices[0] * 2], p_all_edge_indices[p_face_edge_indices[0] * 2 + 1],
			p_all_edge_indices[p_face_edge_indices[1] * 2], p_all_edge_indices[p_face_edge_indices[1] * 2 + 1]);
	for (int64_t i = 2; i < p_face_edge_indices.size(); i++) {
		const int32_t edge_start_vertex = p_all_edge_indices[p_face_edge_indices[i] * 2];
		const int32_t edge_end_vertex = p_all_edge_indices[p_face_edge_indices[i] * 2 + 1];
		if (!ret.has(edge_start_vertex)) {
			ret.append(edge_start_vertex);
		}
		if (!ret.has(edge_end_vertex)) {
			ret.append(edge_end_vertex);
		}
	}
	return ret;
}

PackedInt32Array PolyMesh4D::_triangulate_face_vertex_indices(const PackedInt32Array &p_face_vertex_indices, const int32_t p_pivot_attempt) {
	PackedInt32Array face_triangles;
	ERR_FAIL_COND_V(p_face_vertex_indices.size() < 3, face_triangles);
	const int64_t vertex_count = p_face_vertex_indices.size();
	const int64_t triangle_count = vertex_count - 2;
	face_triangles.resize(triangle_count * 3);
	int64_t where_pivot = p_face_vertex_indices.find(p_pivot_attempt);
	if (where_pivot == -1) {
		where_pivot = 0;
	}
	const int32_t pivot_vertex = p_face_vertex_indices[where_pivot];
	for (int64_t i = 0; i < triangle_count; i++) {
		const int64_t b = (where_pivot + i + 1) % vertex_count;
		const int64_t c = (where_pivot + i + 2) % vertex_count;
		face_triangles.set(i * 3, pivot_vertex);
		face_triangles.set(i * 3 + 1, p_face_vertex_indices[b]);
		face_triangles.set(i * 3 + 2, p_face_vertex_indices[c]);
	}
	return face_triangles;
}

void PolyMesh4D::_decompose_boundary_cells_into_simplexes(const bool p_force_align_triangulations) {
	// This function is required to make the mesh renderable, so it needs to only check the poly mesh data.
	if (!_validate_poly_mesh_data_only()) {
		return;
	}
	// Gather information needed to compute the simplex decomposition.
	_simplex_cell_vertices_cache = get_poly_cell_vertices();
	const PackedInt32Array all_edge_indices = get_edge_indices();
	const Vector<Vector<PackedInt32Array>> poly_cell_indices = get_poly_cell_indices();
	ERR_FAIL_COND_MSG(poly_cell_indices.size() < 2, "PolyMesh4D: Cannot decompose boundary cells into simplexes because there are no boundary cells.");
	const Vector<PackedInt32Array> &faces = poly_cell_indices[0];
	const Vector<PackedInt32Array> &boundary_cells = poly_cell_indices[1];
	const int64_t boundary_cell_count = boundary_cells.size();
	// First pass: Drill down into each cell's components to get the vertex indices and normals.
	// The `true` argument makes the first 4 vertices form the "canonical span" of the cell.
	Vector<PackedInt32Array> cell_vertex_indices = _get_vertex_indices_of_boundary_cells(poly_cell_indices, all_edge_indices, true);
	PackedVector4Array poly_cell_normals = get_poly_cell_boundary_normals();
	if (poly_cell_normals.size() != boundary_cell_count) {
		poly_cell_normals = _compute_boundary_normals_based_on_cell_orientation(cell_vertex_indices, false);
	}
	// Second pass: Determine which boundary cells to use for tetrahedralization.
	// Don't bother tetrahedralizing surface cells completely covered by volumetric cells (2+ uses),
	// because such cells will never be externally visible (like the face between two Minecraft blocks).
	PackedInt32Array surface_cells_to_use;
	surface_cells_to_use.resize(boundary_cell_count);
	if (poly_cell_indices.size() > 2) {
		const Vector<PackedInt32Array> &volumetric_cells = poly_cell_indices[2];
		PackedInt32Array cell_usage_counts;
		cell_usage_counts.resize(boundary_cell_count);
		for (int64_t i = 0; i < volumetric_cells.size(); i++) {
			const PackedInt32Array &volumetric_cell = volumetric_cells[i];
			for (const int32_t vol_cell_part : volumetric_cell) {
				ERR_FAIL_INDEX_MSG(vol_cell_part, boundary_cell_count, "PolyMesh4D: Volumetric cell references an invalid boundary cell index.");
				cell_usage_counts.set(vol_cell_part, cell_usage_counts[vol_cell_part] + 1);
			}
		}
		// Instead of appending, we preallocate and then resize at the end for efficiency.
		// This would be a good use case for `reserve()` if it was exposed, but it's not.
		int64_t surface_cell_count = 0;
		for (int64_t boundary_index = 0; boundary_index < boundary_cell_count; boundary_index++) {
			if (cell_usage_counts[boundary_index] < 2) {
				surface_cells_to_use.set(surface_cell_count, boundary_index);
				surface_cell_count++;
			}
		}
		surface_cells_to_use.resize(surface_cell_count);
	} else {
		for (int64_t i = 0; i < boundary_cell_count; i++) {
			surface_cells_to_use.set(i, i);
		}
	}
	// Third pass: Determine which vertex in each cell to use as the "start" vertex.
	// Note that this algorithm is O(n^2) time complexity for the number of boundary cells.
	PackedInt32Array cell_pivot_vertices;
	cell_pivot_vertices.resize(boundary_cell_count);
	// Keep track of triangulated versions of 2D faces so we can reuse them for all cells that use that face.
	// This tries to avoid this problem: https://www.youtube.com/watch?v=Ir8oft_qAMQ&t=90s
	// However this is not a complete solution to that problem, it WILL still occur in SOME cases.
	// There surely is an algorithm that can completely avoid it, but it would be even more complex.
	Vector<PackedInt32Array> face_triangulations;
	face_triangulations.resize(faces.size());
	{
		// Start this by copying of all cells we are using.
		PackedInt32Array surface_cells_to_check = PackedInt32Array(surface_cells_to_use);
		HashSet<int32_t> chosen_pivot_vertices;
		int32_t prev_vertex_index = -1;
		while (!surface_cells_to_check.is_empty()) {
			int32_t best_cell_number = -1;
			int32_t best_vertex_index = -1;
			int32_t best_score = INT32_MIN;
			// Find the cell and pivot vertex that is least disruptive to the decomposition of other cells.
			for (int64_t cell_number = 0; cell_number < surface_cells_to_check.size(); cell_number++) {
				const int32_t cell_index = surface_cells_to_check[cell_number];
				const PackedInt32Array &cell_data = boundary_cells[cell_index];
				const PackedInt32Array &cell_vertices = cell_vertex_indices[cell_index];
				const int64_t vertex_count = cell_vertices.size();
				for (int64_t vertex_number = 0; vertex_number < vertex_count; vertex_number++) {
					const int32_t vertex_index = cell_vertices[vertex_number];
					int32_t score = 0;
					if (vertex_index == prev_vertex_index) {
						// Strongly discourage using the same vertex as the previous cell.
						// to support the `"polytopeCells"` option (basically an ND triangle fan).
						score = -10000000;
					} else if (chosen_pivot_vertices.has(vertex_index)) {
						score = 1; // Slightly encourage non-sequential reuse of pivot vertices.
					}
					for (int64_t face_number = 0; face_number < cell_data.size(); face_number++) {
						const int32_t face_index = cell_data[face_number];
						ERR_FAIL_INDEX(face_index, faces.size());
						const PackedInt32Array existing_triangulation = face_triangulations[face_index];
						if (existing_triangulation.is_empty()) {
							continue; // No triangulation yet, if we apply one then it can't conflict with anything (zero score).
						}
						const PackedInt32Array face_data = faces[face_index];
						const PackedInt32Array face_vertices = _get_vertex_indices_of_face(all_edge_indices, face_data);
						if (!face_vertices.has(vertex_index)) {
							continue; // Non-adjacent faces don't affect the score because we will triangulate them later (zero score).
						}
						const PackedInt32Array candidate_triangulation = _triangulate_face_vertex_indices(face_vertices, vertex_index);
						// Score faces with compatible triangulations higher, and incompatible triangulations much lower.
						score += _compare_triangulation_alignment(existing_triangulation, candidate_triangulation);
					}
					if (score > best_score) {
						best_score = score;
						best_cell_number = cell_number;
						best_vertex_index = vertex_index;
					}
				}
			}
			const int32_t best_cell_index = surface_cells_to_check[best_cell_number];
			// Now that the best cell has been chosen, we need to either bake its adjacent face triangulations, or mark it as a centroid.
			if (p_force_align_triangulations && best_score < 0) {
				// This cell is disruptive to other cells, so we will tetrahedralize it using the centroid of its vertices.
				Vector4 centroid;
				for (const int64_t vertex_index : cell_vertex_indices[best_cell_index]) {
					centroid += _simplex_cell_vertices_cache[vertex_index];
				}
				centroid /= (real_t)cell_vertex_indices[best_cell_index].size();
				best_vertex_index = _append_vertex_internal(_simplex_cell_vertices_cache, centroid, true);
			} else {
				const PackedInt32Array &cell_data = boundary_cells[best_cell_index];
				for (int64_t face_number = 0; face_number < cell_data.size(); face_number++) {
					const int32_t face_index = cell_data[face_number];
					const PackedInt32Array face_vertices = _get_vertex_indices_of_face(all_edge_indices, faces[face_index]);
					const PackedInt32Array triangulation = _triangulate_face_vertex_indices(face_vertices, best_vertex_index);
					// It's okay if we overwrite an existing triangulation. If it's the same, there is no problem.
					// If it's different, then it's bad either way, overwriting won't make it worse, so just do it.
					face_triangulations.set(face_index, triangulation);
				}
			}
			// We found the best cell and vertex to use as the pivot for that cell.
			// Record it, and remove that cell from the list of cells to check.
			cell_pivot_vertices.set(best_cell_index, best_vertex_index);
			chosen_pivot_vertices.insert(best_vertex_index);
			prev_vertex_index = best_vertex_index;
			surface_cells_to_check.remove_at(best_cell_number);
		}
	}
	// Fourth pass: Triangulate all faces that haven't been triangulated yet.
	// Since the second pass did not determine these, any triangulation works equally well.
	for (int64_t face_index = 0; face_index < faces.size(); face_index++) {
		if (!face_triangulations[face_index].is_empty()) {
			continue; // Already triangulated during the second pass.
		}
		const PackedInt32Array &face_data = faces[face_index];
		const PackedInt32Array face_vertices = _get_vertex_indices_of_face(all_edge_indices, face_data);
		face_triangulations.set(face_index, _triangulate_face_vertex_indices(face_vertices, -1));
	}
	// Fifth and final pass: Tetrahedralize each cell by connecting each opposing face to the start vertex.
	// Determine which way is "outward" for this cell's normal vector, so we can orient the tetrahedra properly.
	_simplex_cell_indices_source_poly_cells.clear();
	for (const int32_t cell_index : surface_cells_to_use) {
		const PackedInt32Array &cell_data = boundary_cells[cell_index];
		const int32_t pivot_vertex_index = cell_pivot_vertices[cell_index];
		for (int64_t face_number = 0; face_number < cell_data.size(); face_number++) {
			const int32_t face_index = cell_data[face_number];
			ERR_FAIL_INDEX(face_index, faces.size());
			const PackedInt32Array &face_triangulation = face_triangulations[face_index];
			// If the pivot vertex is a centroid, this will be false, such that all faces are used.
			if (face_triangulation.has(pivot_vertex_index)) {
				continue;
			}
			for (int64_t tri_index = 0; tri_index < face_triangulation.size(); tri_index += 3) {
				int32_t new_tet[4] = { pivot_vertex_index, face_triangulation[tri_index], face_triangulation[tri_index + 1], face_triangulation[tri_index + 2] };
				const Vector4 tet_perp = Vector4D::perpendicular(
						_simplex_cell_vertices_cache[new_tet[0]].direction_to(_simplex_cell_vertices_cache[new_tet[1]]),
						_simplex_cell_vertices_cache[new_tet[0]].direction_to(_simplex_cell_vertices_cache[new_tet[2]]),
						_simplex_cell_vertices_cache[new_tet[0]].direction_to(_simplex_cell_vertices_cache[new_tet[3]]));
				if (tet_perp.dot(poly_cell_normals[cell_index]) < 0.0) {
					SWAP(new_tet[2], new_tet[3]);
				}
				const int64_t old_size = _simplex_cell_indices_cache.size();
				_simplex_cell_indices_cache.resize(old_size + 4);
				_simplex_cell_indices_cache.set(old_size, new_tet[0]);
				_simplex_cell_indices_cache.set(old_size + 1, new_tet[1]);
				_simplex_cell_indices_cache.set(old_size + 2, new_tet[2]);
				_simplex_cell_indices_cache.set(old_size + 3, new_tet[3]);
				_simplex_cell_indices_source_poly_cells.append(cell_index);
			}
		}
	}
}

Vector<PackedInt32Array> PolyMesh4D::_get_vertex_indices_of_boundary_cells(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const PackedInt32Array &p_all_edge_indices, const bool p_start_with_canonical_span) {
	ERR_FAIL_COND_V(p_poly_cell_indices.size() < 2, Vector<PackedInt32Array>());
	Vector<PackedInt32Array> cell_vertex_indices;
	const int64_t boundary_cell_count = p_poly_cell_indices[1].size();
	cell_vertex_indices.resize(boundary_cell_count);
	for (int64_t cell_index = 0; cell_index < boundary_cell_count; cell_index++) {
		cell_vertex_indices.set(cell_index, _get_vertex_indices_of_cell(p_poly_cell_indices, p_all_edge_indices, 1, cell_index, p_start_with_canonical_span));
	}
	return cell_vertex_indices;
}

PackedVector4Array PolyMesh4D::_compute_boundary_normals_based_on_cell_orientation(const Vector<PackedInt32Array> &p_boundary_cell_vertex_indices, const bool p_keep_existing) {
	CRASH_COND_MSG(_simplex_cell_vertices_cache.is_empty(), "PolyMesh4D: Simplex cell vertices cache needs to be populated before computing normals.");
	PackedVector4Array poly_cell_normals;
	if (p_keep_existing) {
		poly_cell_normals = get_poly_cell_boundary_normals();
	}
	const int64_t boundary_cell_count = p_boundary_cell_vertex_indices.size();
	poly_cell_normals.resize(boundary_cell_count);
	for (int64_t cell_index = 0; cell_index < boundary_cell_count; cell_index++) {
		if (p_keep_existing && poly_cell_normals[cell_index] != Vector4()) {
			continue; // Keep existing normal.
		}
		const PackedInt32Array &cell_vertices = p_boundary_cell_vertex_indices[cell_index];
		ERR_FAIL_COND_V_MSG(cell_vertices.size() < 4, poly_cell_normals, "PolyMesh4D: Cannot compute normal for boundary cell because it has fewer than 4 vertices.");
		const Vector4 cell_perp = Vector4D::perpendicular(
				_simplex_cell_vertices_cache[cell_vertices[0]].direction_to(_simplex_cell_vertices_cache[cell_vertices[1]]),
				_simplex_cell_vertices_cache[cell_vertices[0]].direction_to(_simplex_cell_vertices_cache[cell_vertices[2]]),
				_simplex_cell_vertices_cache[cell_vertices[0]].direction_to(_simplex_cell_vertices_cache[cell_vertices[3]]));
		poly_cell_normals.set(cell_index, cell_perp.normalized());
	}
	return poly_cell_normals;
}

Vector<PackedInt32Array> PolyMesh4D::get_all_face_vertex_indices() {
	ERR_FAIL_COND_V(!is_mesh_data_valid(), Vector<PackedInt32Array>());
	const Vector<Vector<PackedInt32Array>> poly_cell_indices = get_poly_cell_indices();
	ERR_FAIL_COND_V(poly_cell_indices.is_empty(), Vector<PackedInt32Array>());
	const Vector<PackedInt32Array> face_edge_indices = poly_cell_indices[0];
	const PackedInt32Array all_edge_indices = get_edge_indices();
	ERR_FAIL_COND_V(all_edge_indices.is_empty(), Vector<PackedInt32Array>());
	Vector<PackedInt32Array> ret;
	ret.resize(face_edge_indices.size());
	for (int64_t face_index = 0; face_index < face_edge_indices.size(); face_index++) {
		ret.set(face_index, _get_vertex_indices_of_face(all_edge_indices, face_edge_indices[face_index]));
	}
	return ret;
}

TypedArray<PackedInt32Array> PolyMesh4D::get_all_face_vertex_indices_bind() {
	ERR_FAIL_COND_V(!is_mesh_data_valid(), TypedArray<PackedInt32Array>());
	const Vector<Vector<PackedInt32Array>> poly_cell_indices = get_poly_cell_indices();
	ERR_FAIL_COND_V(poly_cell_indices.is_empty(), TypedArray<PackedInt32Array>());
	const Vector<PackedInt32Array> face_edge_indices = poly_cell_indices[0];
	const PackedInt32Array all_edge_indices = get_edge_indices();
	ERR_FAIL_COND_V(all_edge_indices.is_empty(), TypedArray<PackedInt32Array>());
	TypedArray<PackedInt32Array> ret;
	ret.resize(face_edge_indices.size());
	for (int64_t face_index = 0; face_index < face_edge_indices.size(); face_index++) {
		ret[face_index] = _get_vertex_indices_of_face(all_edge_indices, face_edge_indices[face_index]);
	}
	return ret;
}

TypedArray<PackedInt32Array> PolyMesh4D::get_all_cell_vertex_indices_bind(const bool p_start_with_canonical_span) {
	ERR_FAIL_COND_V(!is_mesh_data_valid(), TypedArray<PackedInt32Array>());
	const Vector<Vector<PackedInt32Array>> poly_cell_indices = get_poly_cell_indices();
	ERR_FAIL_COND_V(poly_cell_indices.size() < 2, TypedArray<PackedInt32Array>());
	const PackedInt32Array all_edge_indices = get_edge_indices();
	ERR_FAIL_COND_V(all_edge_indices.is_empty(), TypedArray<PackedInt32Array>());
	Vector<PackedInt32Array> vec = _get_vertex_indices_of_boundary_cells(poly_cell_indices, all_edge_indices, p_start_with_canonical_span);
	TypedArray<PackedInt32Array> ret;
	ret.resize(vec.size());
	for (int64_t i = 0; i < vec.size(); i++) {
		ret[i] = vec[i];
	}
	return ret;
}

void PolyMesh4D::poly_mesh_clear_cache(const bool p_normals_only) {
	_simplex_cell_normals_cache.clear();
	reset_mesh_data_validation();
	// Normals can be computed separately from the rest, so allow resetting just them (and mark cross section dirty).
	if (p_normals_only) {
		mark_cross_section_mesh_dirty();
		return;
	}
	_simplex_cell_indices_cache.clear();
	_simplex_cell_uvw_texture_map_cache.clear();
	_simplex_cell_vertices_cache.clear();
	tetra_mesh_clear_cache();
}

Ref<ArrayPolyMesh4D> PolyMesh4D::to_array_poly_mesh() {
	// Copy all of this data to a new ArrayPolyMesh4D.
	Ref<ArrayPolyMesh4D> array_poly_mesh;
	array_poly_mesh.instantiate();
	array_poly_mesh->set_edge_vertex_indices(get_edge_indices());
	array_poly_mesh->set_poly_cell_indices(get_poly_cell_indices());
	array_poly_mesh->set_poly_cell_boundary_normals(get_poly_cell_boundary_normals());
	array_poly_mesh->set_poly_cell_vertex_normals(get_poly_cell_vertex_normals());
	array_poly_mesh->set_poly_cell_texture_map(get_poly_cell_texture_map());
	array_poly_mesh->set_poly_cell_vertices(get_poly_cell_vertices());
	array_poly_mesh->set_material(get_material());
	return array_poly_mesh;
}

Vector<Vector<PackedInt32Array>> PolyMesh4D::get_poly_cell_indices() {
	TypedArray<Array> indices_bind;
	GDVIRTUAL_CALL(_get_poly_cell_indices, indices_bind);
	Vector<Vector<PackedInt32Array>> indices;
	indices.resize(indices_bind.size());
	for (int i = 0; i < indices_bind.size(); i++) {
		Array dim_array = indices_bind[i];
		Vector<PackedInt32Array> dim_vector;
		dim_vector.resize(dim_array.size());
		for (int j = 0; j < dim_array.size(); j++) {
			const PackedInt32Array cell_indices = dim_array[j];
			dim_vector.set(j, cell_indices);
		}
		indices.set(i, dim_vector);
	}
	return indices;
}

PackedVector4Array PolyMesh4D::get_poly_cell_vertices() {
	PackedVector4Array vertices;
	GDVIRTUAL_CALL(_get_poly_cell_vertices, vertices);
	return vertices;
}

PackedVector4Array PolyMesh4D::get_poly_cell_boundary_normals() {
	PackedVector4Array normals;
	GDVIRTUAL_CALL(_get_poly_cell_boundary_normals, normals);
	return normals;
}

Vector<PackedVector4Array> PolyMesh4D::get_poly_cell_vertex_normals() {
	TypedArray<PackedVector4Array> vertex_normals_bind;
	GDVIRTUAL_CALL(_get_poly_cell_vertex_normals, vertex_normals_bind);
	Vector<PackedVector4Array> vertex_normals;
	vertex_normals.resize(vertex_normals_bind.size());
	for (int i = 0; i < vertex_normals_bind.size(); i++) {
		const PackedVector4Array cell_vertex_normals = vertex_normals_bind[i];
		vertex_normals.set(i, cell_vertex_normals);
	}
	return vertex_normals;
}

Vector<PackedVector3Array> PolyMesh4D::get_poly_cell_texture_map() {
	TypedArray<PackedVector3Array> uvw_texture_map_bind;
	GDVIRTUAL_CALL(_get_poly_cell_texture_map, uvw_texture_map_bind);
	Vector<PackedVector3Array> uvw_texture_map;
	uvw_texture_map.resize(uvw_texture_map_bind.size());
	for (int i = 0; i < uvw_texture_map_bind.size(); i++) {
		const PackedVector3Array cell_uvw_map = uvw_texture_map_bind[i];
		uvw_texture_map.set(i, cell_uvw_map);
	}
	return uvw_texture_map;
}

TypedArray<Array> PolyMesh4D::get_poly_cell_indices_bind() {
	TypedArray<Array> indices_bind;
	GDVIRTUAL_CALL(_get_poly_cell_indices, indices_bind);
	if (!indices_bind.is_empty()) {
		return indices_bind;
	}
	const Vector<Vector<PackedInt32Array>> indices = get_poly_cell_indices();
	indices_bind.resize(indices.size());
	for (int i = 0; i < indices.size(); i++) {
		const Vector<PackedInt32Array> &dim_vector = indices[i];
		Array dim_array;
		dim_array.resize(dim_vector.size());
		for (int j = 0; j < dim_vector.size(); j++) {
			const PackedInt32Array &cell_indices = dim_vector[j];
			dim_array[j] = cell_indices;
		}
		indices_bind[i] = dim_array;
	}
	return indices_bind;
}

TypedArray<PackedVector4Array> PolyMesh4D::get_poly_cell_vertex_normals_bind() {
	TypedArray<PackedVector4Array> vertex_normals_bind;
	GDVIRTUAL_CALL(_get_poly_cell_vertex_normals, vertex_normals_bind);
	if (!vertex_normals_bind.is_empty()) {
		return vertex_normals_bind;
	}
	const Vector<PackedVector4Array> vertex_normals = get_poly_cell_vertex_normals();
	vertex_normals_bind.resize(vertex_normals.size());
	for (int i = 0; i < vertex_normals.size(); i++) {
		const PackedVector4Array &cell_vertex_normals = vertex_normals[i];
		vertex_normals_bind[i] = cell_vertex_normals;
	}
	return vertex_normals_bind;
}

TypedArray<PackedVector3Array> PolyMesh4D::get_poly_cell_texture_map_bind() {
	TypedArray<PackedVector3Array> uvw_texture_map_bind;
	GDVIRTUAL_CALL(_get_poly_cell_texture_map, uvw_texture_map_bind);
	if (!uvw_texture_map_bind.is_empty()) {
		return uvw_texture_map_bind;
	}
	const Vector<PackedVector3Array> uvw_map = get_poly_cell_texture_map();
	uvw_texture_map_bind.resize(uvw_map.size());
	for (int i = 0; i < uvw_map.size(); i++) {
		const PackedVector3Array &cell_uvw_map = uvw_map[i];
		uvw_texture_map_bind[i] = cell_uvw_map;
	}
	return uvw_texture_map_bind;
}

PackedInt32Array PolyMesh4D::get_cell_indices() {
	if (_simplex_cell_indices_cache.is_empty()) {
		_decompose_boundary_cells_into_simplexes(true);
	}
	return _simplex_cell_indices_cache;
}

PackedVector4Array PolyMesh4D::get_cell_boundary_normals() {
	if (_simplex_cell_normals_cache.is_empty()) {
		if (_simplex_cell_indices_cache.is_empty()) {
			_decompose_boundary_cells_into_simplexes(true);
		}
		const int64_t simplex_count = _simplex_cell_indices_cache.size() / 4;
		_simplex_cell_normals_cache.resize(simplex_count);
		for (int64_t i = 0; i < simplex_count; i++) {
			const int64_t offset = i * 4;
			const int32_t vert0 = _simplex_cell_indices_cache[offset];
			const int32_t vert1 = _simplex_cell_indices_cache[offset + 1];
			const int32_t vert2 = _simplex_cell_indices_cache[offset + 2];
			const int32_t vert3 = _simplex_cell_indices_cache[offset + 3];
			const Vector4 perp = Vector4D::perpendicular(
					_simplex_cell_vertices_cache[vert0].direction_to(_simplex_cell_vertices_cache[vert1]),
					_simplex_cell_vertices_cache[vert0].direction_to(_simplex_cell_vertices_cache[vert2]),
					_simplex_cell_vertices_cache[vert0].direction_to(_simplex_cell_vertices_cache[vert3]));
			_simplex_cell_normals_cache.set(i, perp.normalized());
		}
	}
	return _simplex_cell_normals_cache;
}

PackedVector3Array PolyMesh4D::get_cell_uvw_map() {
	if (_simplex_cell_uvw_texture_map_cache.is_empty()) {
		const Vector<PackedVector3Array> poly_cell_texture_map = get_poly_cell_texture_map();
		if (poly_cell_texture_map.is_empty()) {
			return PackedVector3Array(); // No texture map data available.
		}
		int64_t simplex_count = _simplex_cell_indices_source_poly_cells.size();
		if (simplex_count * 4 != _simplex_cell_indices_cache.size()) {
			_decompose_boundary_cells_into_simplexes(true);
			simplex_count = _simplex_cell_indices_source_poly_cells.size();
			CRASH_COND_MSG(simplex_count * 4 != _simplex_cell_indices_cache.size(), "PolyMesh4D: Simplex cell indices cache is corrupt.");
		}
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = get_poly_cell_indices();
		ERR_FAIL_COND_V_MSG(poly_cell_indices.size() < 2, PackedVector3Array(), "PolyMesh4D: No boundary cells available, cannot compute simplex UVW map.");
		const PackedInt32Array all_edge_indices = get_edge_indices();
		const Vector<PackedInt32Array> cell_vert = _get_vertex_indices_of_boundary_cells(poly_cell_indices, all_edge_indices, false);
		_simplex_cell_uvw_texture_map_cache.resize(simplex_count * 4);
		bool has_some_texture_map = false;
		bool missing_some_texture_map = false;
		for (int64_t simplex_index = 0; simplex_index < simplex_count; simplex_index++) {
			const int32_t source_cell = _simplex_cell_indices_source_poly_cells[simplex_index];
			const PackedVector3Array &source_poly_texture_map = poly_cell_texture_map[source_cell];
			if (source_poly_texture_map.is_empty()) {
				missing_some_texture_map = true;
				continue;
			}
			has_some_texture_map = true;
			const PackedInt32Array &source_cell_vertices = cell_vert[source_cell];
			CRASH_COND_MSG(source_poly_texture_map.size() != source_cell_vertices.size(), "PolyMesh4D: Source polytope cell texture map size does not match cell vertex count.");
			const int64_t offset = simplex_index * 4;
			for (int64_t vertex_in_simplex = 0; vertex_in_simplex < 4; vertex_in_simplex++) {
				const int32_t vertex_index = _simplex_cell_indices_cache[offset + vertex_in_simplex];
				const int64_t vertex_in_source_poly = source_cell_vertices.find(vertex_index);
				Vector3 texcoord;
				if (vertex_in_source_poly == -1) {
					texcoord = _average_vector3(source_poly_texture_map);
				} else {
					texcoord = source_poly_texture_map[vertex_in_source_poly];
				}
				_simplex_cell_uvw_texture_map_cache.set(offset + vertex_in_simplex, texcoord);
			}
		}
		if (missing_some_texture_map) {
			if (has_some_texture_map) {
				WARN_PRINT("PolyMesh4D: Some polytope cells are missing UVW texture map data, the texture mapping will be missing for the corresponding simplexes.");
			} else {
				_simplex_cell_uvw_texture_map_cache.clear(); // No texture map data available.
			}
		}
	}
	return _simplex_cell_uvw_texture_map_cache;
}

PackedVector4Array PolyMesh4D::get_vertices() {
	if (_simplex_cell_vertices_cache.is_empty()) {
		// This will populate the vertex cache as a side effect.
		_decompose_boundary_cells_into_simplexes(true);
	}
	return _simplex_cell_vertices_cache;
}

void PolyMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_all_face_vertex_indices"), &PolyMesh4D::get_all_face_vertex_indices_bind);
	ClassDB::bind_method(D_METHOD("get_all_cell_vertex_indices", "start_with_canonical_span"), &PolyMesh4D::get_all_cell_vertex_indices_bind);
	ClassDB::bind_method(D_METHOD("poly_mesh_clear_cache", "normals_only"), &PolyMesh4D::poly_mesh_clear_cache, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("to_array_poly_mesh"), &PolyMesh4D::to_array_poly_mesh);

	ClassDB::bind_method(D_METHOD("get_poly_cell_indices"), &PolyMesh4D::get_poly_cell_indices_bind);
	ClassDB::bind_method(D_METHOD("get_poly_cell_vertices"), &PolyMesh4D::get_poly_cell_vertices);
	ClassDB::bind_method(D_METHOD("get_poly_cell_boundary_normals"), &PolyMesh4D::get_poly_cell_boundary_normals);
	ClassDB::bind_method(D_METHOD("get_poly_cell_vertex_normals"), &PolyMesh4D::get_poly_cell_vertex_normals_bind);
	ClassDB::bind_method(D_METHOD("get_poly_cell_texture_map"), &PolyMesh4D::get_poly_cell_texture_map_bind);

	GDVIRTUAL_BIND(_get_poly_cell_indices);
	GDVIRTUAL_BIND(_get_poly_cell_vertices);
	GDVIRTUAL_BIND(_get_poly_cell_boundary_normals);
	GDVIRTUAL_BIND(_get_poly_cell_vertex_normals);
	GDVIRTUAL_BIND(_get_poly_cell_texture_map);
}
