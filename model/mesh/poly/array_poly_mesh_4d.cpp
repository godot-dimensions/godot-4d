#include "array_poly_mesh_4d.h"

#include "../../../math/math_4d.h"
#include "../../../math/vector_4d.h"

// Append and delete functions.

int64_t ArrayPolyMesh4D::append_edge_points(const Vector4 &p_point_a, const Vector4 &p_point_b, const bool p_deduplicate) {
	const int32_t index_a = append_vertex(p_point_a, p_deduplicate);
	const int32_t index_b = append_vertex(p_point_b, p_deduplicate);
	return append_edge_indices(index_a, index_b, p_deduplicate);
}

int64_t ArrayPolyMesh4D::append_edge_indices(int32_t p_index_a, int32_t p_index_b, const bool p_deduplicate) {
	const int64_t old_edge_count = _edge_vertex_indices.size() / 2;
	if (p_index_a > p_index_b) {
		SWAP(p_index_a, p_index_b);
	}
	if (p_deduplicate) {
		for (int64_t i = 0; i < old_edge_count; i++) {
			if (_edge_vertex_indices[i * 2] == p_index_a && _edge_vertex_indices[i * 2 + 1] == p_index_b) {
				return i;
			}
		}
	}
	// Append a new edge. We don't need to clear the poly mesh cache, but TetraMesh4D does cache edges.
	_edge_vertex_indices.append(p_index_a);
	_edge_vertex_indices.append(p_index_b);
	tetra_mesh_clear_cache();
	reset_poly_mesh_data_validation();
	return old_edge_count;
}

int64_t ArrayPolyMesh4D::append_poly_cell(const int32_t p_dimension, const PackedInt32Array &p_cell, const bool p_deduplicate) {
	ERR_FAIL_COND_V_MSG(_poly_cell_vertices.is_empty() || _edge_vertex_indices.is_empty(), -1, "This ArrayPolyMesh4D lacks any 0D vertices or 1D edges, so cannot append a poly cell.");
	ERR_FAIL_COND_V_MSG(p_dimension < 2, -1, "ArrayPolyMesh4D: Cannot append " + itos(p_dimension) + "D poly cell. For 0D vertices and 1D edges, use the special functions for those.");
	const int64_t old_mesh_dim = _poly_cell_indices.size() + 1;
	ERR_FAIL_COND_V_MSG(p_dimension > old_mesh_dim + 1, -1, "ArrayPolyMesh4D: Cannot append a " + itos(p_dimension) + "D poly cell because the mesh currently only has cells up to " + itos(old_mesh_dim) + "D. Cells must be appended in order of dimension, so append the missing " + itos(old_mesh_dim + 1) + "D cell(s) first.");
	// Check if the elements of the previous dimension referenced by this new cell actually exist.
	// If inserting a 2D face (`p_dimension == 2`), check edges. If inserting a 3D cell (`p_dimension == 3`), check faces (`_poly_cell_indices[0]`). Etc.
	const int64_t prev_dim_cell_count = p_dimension == 2 ? _edge_vertex_indices.size() / 2 : _poly_cell_indices[p_dimension - 3].size();
	for (const int32_t cell_element : p_cell) {
		ERR_FAIL_INDEX_V_MSG(cell_element, prev_dim_cell_count, -1, "ArrayPolyMesh4D: Cannot append poly cell because it references non-existent elements of the previous dimension.");
	}
	// Append the new cell to the poly cell indices, deduplicating if desired.
	if (p_dimension == old_mesh_dim + 1) {
		Vector<PackedInt32Array> new_dim;
		new_dim.append(p_cell);
		_poly_cell_indices.append(new_dim);
		return 0;
	}
	const int64_t poly_cell_dim_index = p_dimension - 2;
	Vector<PackedInt32Array> existing_dim = _poly_cell_indices[poly_cell_dim_index];
	const int64_t existing_cell_count = existing_dim.size();
	if (p_deduplicate) {
		// Check for cells made of the same elements, regardless of order.
		// When appending, we need to preserve the order given, but for deduplication,
		// it is fine if the order is different, as long as the same elements are present.
		PackedInt32Array sorted_new_cell = p_cell;
		sorted_new_cell.sort();
		for (int64_t existing_cell_index = 0; existing_cell_index < existing_cell_count; existing_cell_index++) {
			PackedInt32Array existing_cell_copy = PackedInt32Array(existing_dim[existing_cell_index]);
			if (existing_cell_copy.size() != sorted_new_cell.size()) {
				continue;
			}
			existing_cell_copy.sort();
			if (existing_cell_copy == sorted_new_cell) {
				// This existing cell is made of the same elements as the new cell, so consider them duplicates.
				return existing_cell_index;
			}
		}
	}
	existing_dim.append(p_cell);
	_poly_cell_indices.set(poly_cell_dim_index, existing_dim);
	reset_poly_mesh_data_validation();
	return existing_cell_count;
}

int ArrayPolyMesh4D::append_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices) {
	const int vertex_count = _poly_cell_vertices.size();
	if (p_deduplicate_vertices) {
		for (int i = 0; i < vertex_count; i++) {
			if (_poly_cell_vertices[i] == p_vertex) {
				return i;
			}
		}
	}
	ERR_FAIL_COND_V(_poly_cell_vertices.size() > MAX_POLY_VERTICES, 2147483647);
	_poly_cell_vertices.push_back(p_vertex);
	reset_poly_mesh_data_validation();
	return vertex_count;
}

PackedInt32Array ArrayPolyMesh4D::append_vertices(const PackedVector4Array &p_vertices, const bool p_deduplicate_vertices) {
	PackedInt32Array indices;
	for (int i = 0; i < p_vertices.size(); i++) {
		indices.append(append_vertex(p_vertices[i], p_deduplicate_vertices));
	}
	reset_poly_mesh_data_validation();
	return indices;
}

void ArrayPolyMesh4D::_delete_edge_internal(const int32_t p_index) {
	const int32_t edge_count = _edge_vertex_indices.size() / 2;
	ERR_FAIL_COND_MSG(p_index < 0 || p_index >= edge_count, "ArrayPolyMesh4D: Edge index is out of range.");
	// Before deleting this edge, we need to delete any poly cells in higher dimensions that reference it.
	if (!_poly_cell_indices.is_empty()) {
		Vector<int32_t> faces_to_delete;
		const Vector<PackedInt32Array> &face_edge_indices = _poly_cell_indices[0];
		for (int32_t face_index = 0; face_index < face_edge_indices.size(); face_index++) {
			if (face_edge_indices[face_index].has(p_index)) {
				faces_to_delete.push_back(face_index);
			}
		}
		for (int32_t i = faces_to_delete.size() - 1; i >= 0; i--) {
			_delete_poly_cell_element_internal(0, faces_to_delete[i]);
		}
	}
	// Delete the edge's two vertex index entries from the flat edge array.
	const int32_t edge_vertex_start = p_index * 2;
	_edge_vertex_indices.remove_at(edge_vertex_start + 1);
	_edge_vertex_indices.remove_at(edge_vertex_start);
	// Shift remaining face edge references down to preserve index semantics.
	if (!_poly_cell_indices.is_empty()) {
		Vector<PackedInt32Array> face_edge_indices = _poly_cell_indices[0];
		for (int32_t face_index = 0; face_index < face_edge_indices.size(); face_index++) {
			PackedInt32Array face = face_edge_indices[face_index];
			bool changed = false;
			for (int32_t edge_index_in_face = 0; edge_index_in_face < face.size(); edge_index_in_face++) {
				if (face[edge_index_in_face] > p_index) {
					face.set(edge_index_in_face, face[edge_index_in_face] - 1);
					changed = true;
				}
			}
			if (changed) {
				face_edge_indices.set(face_index, face);
			}
		}
		_poly_cell_indices.set(0, face_edge_indices);
	}
}

void ArrayPolyMesh4D::_delete_vertex_internal(const int32_t p_index) {
	const int64_t vertex_count = _poly_cell_vertices.size();
	ERR_FAIL_COND_MSG(p_index < 0 || p_index >= vertex_count, "ArrayPolyMesh4D: Vertex index is out of range.");
	// Before deleting this vertex, we need to delete any edges that reference it,
	// and any poly cells in higher dimensions that reference those edges.
	const int32_t edge_count = _edge_vertex_indices.size() / 2;
	Vector<int32_t> edges_to_delete;
	for (int32_t edge_index = 0; edge_index < edge_count; edge_index++) {
		if (_edge_vertex_indices[edge_index * 2] == p_index || _edge_vertex_indices[edge_index * 2 + 1] == p_index) {
			edges_to_delete.push_back(edge_index);
		}
	}
	for (int32_t i = edges_to_delete.size() - 1; i >= 0; i--) {
		_delete_edge_internal(edges_to_delete[i]);
	}
	// Delete the vertex itself now that all dependent edges (and higher dimensions) are gone.
	_poly_cell_vertices.remove_at(p_index);
	// Shift remaining edge vertex references down to preserve index semantics.
	for (int64_t edge_vertex_index = 0; edge_vertex_index < _edge_vertex_indices.size(); edge_vertex_index++) {
		if (_edge_vertex_indices[edge_vertex_index] > p_index) {
			_edge_vertex_indices.set(edge_vertex_index, _edge_vertex_indices[edge_vertex_index] - 1);
		}
	}
}

void ArrayPolyMesh4D::_delete_poly_cell_element_internal(const int32_t p_poly_cell_index, const int32_t p_index) {
	ERR_FAIL_COND_MSG(p_poly_cell_index < 0 || p_poly_cell_index >= _poly_cell_indices.size(), "ArrayPolyMesh4D: Dimension is out of range.");
	ERR_FAIL_COND_MSG(p_index < 0 || p_index >= _poly_cell_indices[p_poly_cell_index].size(), "ArrayPolyMesh4D: Index is out of range.");
	// Before deleting this poly cell element, we need to delete anything in higher dimensions that reference it.
	const int32_t next_dim_poly_index = p_poly_cell_index + 1;
	if (next_dim_poly_index < _poly_cell_indices.size()) {
		// Collect indices in next_dim_poly_index whose elements reference p_index.
		Vector<int32_t> to_delete;
		const Vector<PackedInt32Array> &next_level = _poly_cell_indices[next_dim_poly_index];
		for (int32_t j = 0; j < next_level.size(); j++) {
			const PackedInt32Array &refs = next_level[j];
			for (int32_t k = 0; k < refs.size(); k++) {
				if (refs[k] == p_index) {
					to_delete.push_back(j);
					break;
				}
			}
		}
		// Delete in reverse order so that earlier indices are not shifted by later removals.
		for (int32_t i = to_delete.size() - 1; i >= 0; i--) {
			_delete_poly_cell_element_internal(next_dim_poly_index, to_delete[i]);
		}
	}
	// Delete any corresponding elements in the associated arrays for this poly cell dimension.
	if (p_poly_cell_index == 0) {
		// For border faces (poly cell index 0), delete from the seam faces.
		if (!_seam_face_indices.is_empty()) {
			HashSet<int32_t> adjusted_seam_face_indices;
			for (const int32_t face_index : _seam_face_indices) {
				if (face_index == p_index) {
					continue;
				}
				adjusted_seam_face_indices.insert(face_index > p_index ? face_index - 1 : face_index);
			}
			_seam_face_indices = adjusted_seam_face_indices;
		}
	}
	const int geom_dim = p_poly_cell_index + 2;
	// Delete from the normals, including boundary and vertex.
	for (KeyValue<Vector2i, Vector<PackedVector4Array>> &normals_iterator : _all_poly_cell_normals) {
		const Vector2i key = normals_iterator.key;
		if (key.x == geom_dim) {
			Vector<PackedVector4Array> &normals_for_dim = normals_iterator.value;
			if (key.y == key.x) {
				// Non-decomposed case. Only one array, with one normal per cell.
				normals_for_dim.ptrw()[0].remove_at(p_index);
			} else {
				// Decomposed case, such as vertex normals for each cell.
				normals_for_dim.remove_at(p_index);
			}
		}
	}
	// Delete from the texture map.
	for (KeyValue<Vector2i, Vector<PackedVector3Array>> &texture_map_iterator : _all_poly_cell_texture_maps) {
		const Vector2i key = texture_map_iterator.key;
		if (key.x == geom_dim) {
			Vector<PackedVector3Array> &texture_map_for_dim = texture_map_iterator.value;
			if (key.y == key.x) {
				// Non-decomposed case. Only one array, with one texture coordinate (UVW) per cell.
				texture_map_for_dim.ptrw()[0].remove_at(p_index);
			} else {
				// Decomposed case, such as vertex texture coordinates (UVWs) for each cell.
				texture_map_for_dim.remove_at(p_index);
			}
		}
	}
	// Remove the element at p_index from _poly_cell_indices[p_dimension].
	_poly_cell_indices.ptrw()[p_poly_cell_index].remove_at(p_index);
	// Fix up references in next_dim_poly_index by decrementing any index greater than p_index.
	if (next_dim_poly_index < _poly_cell_indices.size()) {
		Vector<PackedInt32Array> next_dim_data = _poly_cell_indices[next_dim_poly_index];
		for (int32_t j = 0; j < next_dim_data.size(); j++) {
			PackedInt32Array refs = next_dim_data[j];
			bool changed = false;
			for (int32_t k = 0; k < refs.size(); k++) {
				if (refs[k] > p_index) {
					refs.set(k, refs[k] - 1);
					changed = true;
				}
			}
			if (changed) {
				next_dim_data.set(j, refs);
			}
		}
		_poly_cell_indices.set(next_dim_poly_index, next_dim_data);
	}
	// Keep dimensions normalized by trimming from the first empty dimension onward.
	// In a valid poly mesh, once a dimension is empty, all higher dimensions must also be empty.
	for (int32_t dim_index = 0; dim_index < _poly_cell_indices.size(); dim_index++) {
		if (_poly_cell_indices[dim_index].is_empty()) {
			_poly_cell_indices.resize(dim_index);
			break;
		}
	}
}

void ArrayPolyMesh4D::delete_poly_element(const int32_t p_dimension, const int32_t p_index) {
	if (p_dimension < 0) {
		ERR_FAIL_MSG("ArrayPolyMesh4D: Cannot delete from negative dimension.");
	} else if (p_dimension == 0) {
		_delete_vertex_internal(p_index);
	} else if (p_dimension == 1) {
		_delete_edge_internal(p_index);
	} else {
		const int64_t poly_cell_index = p_dimension - 2;
		if (poly_cell_index >= _poly_cell_indices.size()) {
			ERR_FAIL_MSG("ArrayPolyMesh4D: Cannot delete from dimension higher than the highest poly cell dimension.");
		}
		_delete_poly_cell_element_internal(poly_cell_index, p_index);
	}
	poly_mesh_clear_cache();
	reset_poly_mesh_data_validation();
}

// Normal calculation functions.

void ArrayPolyMesh4D::calculate_boundary_normals(const ComputeNormalsMode p_mode, const bool p_keep_existing) {
	ERR_FAIL_COND_MSG(_poly_cell_indices.size() < 2, "ArrayPolyMesh4D: Cannot calculate boundary normals because there are no boundary cells.");
	ERR_FAIL_COND_MSG(!is_poly_mesh_data_valid(), "ArrayPolyMesh4D: Cannot calculate boundary normals for invalid poly mesh data.");
	const PackedVector4Array &vertices = get_poly_cell_vertices();
	ERR_FAIL_COND_MSG(vertices.is_empty(), "ArrayPolyMesh4D: Cannot calculate boundary normals because there are no vertices.");
	const Vector<PackedInt32Array> cell_vertex_indices = _get_vertex_indices_of_boundary_cells(_poly_cell_indices, _edge_vertex_indices, true);
	if (cell_vertex_indices.is_empty()) {
		return;
	}
	PackedVector4Array poly_cell_boundary_normals = _compute_boundary_normals_based_on_cell_orientation(cell_vertex_indices, p_keep_existing);
	CRASH_COND(poly_cell_boundary_normals.size() != cell_vertex_indices.size());
	if (p_mode == COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY) {
		_all_poly_cell_normals.insert(PER_CELL_KEY, Vector<PackedVector4Array>{ poly_cell_boundary_normals });
		poly_mesh_clear_cache();
		return;
	}
	// Ensure normals point outward from the mesh.
	Vector<PackedInt32Array> boundary_cell_indices = _poly_cell_indices[1];
	for (int64_t cell_index = 0; cell_index < cell_vertex_indices.size(); cell_index++) {
		const PackedInt32Array &vertex_indices = cell_vertex_indices[cell_index];
		Vector4 average = Vector4();
		for (int64_t vertex_index : vertex_indices) {
			ERR_FAIL_COND(vertex_index < 0 || vertex_index >= _poly_cell_vertices.size());
			average += _poly_cell_vertices[vertex_index];
		}
		average /= (real_t)vertex_indices.size();
		if (average.dot(poly_cell_boundary_normals[cell_index]) < 0) {
			// Normal points inward, so flip it, and optionally correct the cell orientation.
			poly_cell_boundary_normals.set(cell_index, -poly_cell_boundary_normals[cell_index]);
			if (p_mode == COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION) {
				// Reverse the order of the first two faces in this cell to flip its orientation.
				PackedInt32Array cell_face_indices = boundary_cell_indices[cell_index];
				const int32_t temp = cell_face_indices[0];
				cell_face_indices.set(0, cell_face_indices[1]);
				cell_face_indices.set(1, temp);
				boundary_cell_indices.set(cell_index, cell_face_indices);
			}
		}
	}
	if (p_mode == COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION) {
		_poly_cell_indices.set(1, boundary_cell_indices);
	}
	_all_poly_cell_normals.insert(PER_CELL_KEY, Vector<PackedVector4Array>{ poly_cell_boundary_normals });
	poly_mesh_clear_cache();
}

void ArrayPolyMesh4D::set_flat_shading_normals(const ComputeNormalsMode p_mode, const bool p_recalculate_boundary_normals) {
	_all_poly_cell_normals.erase(CELL_TO_VERT_KEY);
	ERR_FAIL_COND_MSG(_poly_cell_indices.size() < 2, "ArrayPolyMesh4D: Cannot calculate boundary normals because there are no boundary cells.");
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayPolyMesh4D: Cannot calculate boundary normals for an invalid mesh.");
	if (p_recalculate_boundary_normals || !_all_poly_cell_normals.has(PER_CELL_KEY) || _all_poly_cell_normals[PER_CELL_KEY][0].size() != _poly_cell_indices[1].size()) {
		calculate_boundary_normals(p_mode);
	}
	const Vector<PackedInt32Array> cell_vertex_indices = _get_vertex_indices_of_boundary_cells(_poly_cell_indices, _edge_vertex_indices, false);
	const int64_t cell_count = cell_vertex_indices.size();
	const PackedVector4Array &poly_cell_boundary_normals = _all_poly_cell_normals[PER_CELL_KEY][0];
	CRASH_COND(poly_cell_boundary_normals.size() != cell_count);
	Vector<PackedVector4Array> poly_cell_vertex_normals;
	poly_cell_vertex_normals.resize(cell_count);
	for (int64_t cell_index = 0; cell_index < cell_count; cell_index++) {
		PackedVector4Array vertex_normals_for_cell;
		const Vector4 &cell_normal = poly_cell_boundary_normals[cell_index];
		const int64_t cell_vertex_count = cell_vertex_indices[cell_index].size();
		vertex_normals_for_cell.resize(cell_vertex_count);
		for (int64_t vertex_index = 0; vertex_index < cell_vertex_count; vertex_index++) {
			vertex_normals_for_cell.set(vertex_index, cell_normal);
		}
		poly_cell_vertex_normals.set(cell_index, vertex_normals_for_cell);
	}
	_all_poly_cell_normals.insert(CELL_TO_VERT_KEY, poly_cell_vertex_normals);
	poly_mesh_clear_cache();
}

void ArrayPolyMesh4D::set_smooth_shading_normals(const ComputeNormalsMode p_mode, const bool p_recalculate_boundary_normals) {
	_all_poly_cell_normals.erase(CELL_TO_VERT_KEY);
	ERR_FAIL_COND_MSG(_poly_cell_indices.size() < 2, "ArrayPolyMesh4D: Cannot calculate boundary normals because there are no boundary cells.");
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayPolyMesh4D: Cannot calculate boundary normals for an invalid mesh.");
	// Step 1: Prepare the data arrays which will be used by this function.
	if (p_recalculate_boundary_normals || !_all_poly_cell_normals.has(PER_CELL_KEY) || _all_poly_cell_normals[PER_CELL_KEY][0].size() != _poly_cell_indices[1].size()) {
		calculate_boundary_normals(p_mode);
	}
	const Vector<PackedInt32Array> cell_vertex_indices = _get_vertex_indices_of_boundary_cells(_poly_cell_indices, _edge_vertex_indices, false);
	const PackedVector4Array &poly_cell_boundary_normals = _all_poly_cell_normals[PER_CELL_KEY][0];
	CRASH_COND(poly_cell_boundary_normals.size() != cell_vertex_indices.size());
	Vector<PackedVector4Array> poly_cell_vertex_normals;
	poly_cell_vertex_normals.resize(poly_cell_boundary_normals.size());
	PackedVector4Array vertex_normals;
	vertex_normals.resize(_poly_cell_vertices.size());
	// Step 2: Iterate through each island separately such that seams (if they exist)
	// are respected and are treated as sharp edges that should not be smoothed across.
	const Vector<PackedInt32Array> islands = collect_all_islands();
	for (int64_t island_index = 0; island_index < islands.size(); island_index++) {
		const PackedInt32Array &cells_in_island = islands[island_index];
		// Step 3: Calculate the average normal of each vertex across each usage in cells in this island.
		for (int64_t vertex_index = 0; vertex_index < _poly_cell_vertices.size(); vertex_index++) {
			vertex_normals.set(vertex_index, Vector4(0.0, 0.0, 0.0, 0.0));
		}
		for (const int32_t cell_index : cells_in_island) {
			const PackedInt32Array &vertex_indices_for_cell = cell_vertex_indices[cell_index];
			const Vector4 &cell_normal = poly_cell_boundary_normals[cell_index];
			for (const int32_t vertex_index : vertex_indices_for_cell) {
				vertex_normals.set(vertex_index, vertex_normals[vertex_index] + cell_normal);
			}
		}
		for (int64_t vertex_index = 0; vertex_index < _poly_cell_vertices.size(); vertex_index++) {
			vertex_normals.set(vertex_index, vertex_normals[vertex_index].normalized());
		}
		// Step 4: Assign each cell's vertex normals to the average normal of the vertices that make up that cell.
		for (const int32_t cell_index : cells_in_island) {
			const PackedInt32Array &vertex_indices_for_cell = cell_vertex_indices[cell_index];
			PackedVector4Array vertex_normals_for_cell;
			const int64_t cell_vertex_count = vertex_indices_for_cell.size();
			vertex_normals_for_cell.resize(cell_vertex_count);
			for (int64_t vertex_in_cell = 0; vertex_in_cell < cell_vertex_count; vertex_in_cell++) {
				vertex_normals_for_cell.set(vertex_in_cell, vertex_normals[vertex_indices_for_cell[vertex_in_cell]]);
			}
			poly_cell_vertex_normals.set(cell_index, vertex_normals_for_cell);
		}
	}
	_all_poly_cell_normals.insert(CELL_TO_VERT_KEY, poly_cell_vertex_normals);
	poly_mesh_clear_cache();
}

void ArrayPolyMesh4D::make_double_sided(const bool p_idempotent) {
	ERR_FAIL_COND_MSG(_poly_cell_indices.size() < 2, "ArrayPolyMesh4D: Cannot make double sided because there are no boundary cells.");
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayPolyMesh4D: Cannot make double sided for an invalid mesh.");
	if (!_all_poly_cell_normals.has(PER_CELL_KEY) || _all_poly_cell_normals[PER_CELL_KEY][0].size() != _poly_cell_indices[1].size()) {
		calculate_boundary_normals(COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, false);
	}
	Vector<PackedInt32Array> cell_face_indices = Vector<PackedInt32Array>(_poly_cell_indices[1]);
	const int64_t original_cell_count = cell_face_indices.size();
	// This has to be a copy, it's set back in place at the end of the function.
	PackedVector4Array poly_cell_boundary_normals = _all_poly_cell_normals[PER_CELL_KEY][0];
	CRASH_COND(poly_cell_boundary_normals.size() != original_cell_count);
	PackedInt32Array flipped_cell_index_for_original;
	flipped_cell_index_for_original.resize(original_cell_count);
	for (int64_t i = 0; i < original_cell_count; i++) {
		flipped_cell_index_for_original.set(i, -1);
	}
	for (int64_t cell_index = 0; cell_index < original_cell_count; cell_index++) {
		PackedInt32Array flipped_cell_faces = PackedInt32Array(cell_face_indices[cell_index]);
		int32_t temp = flipped_cell_faces[0];
		flipped_cell_faces.set(0, flipped_cell_faces[1]);
		flipped_cell_faces.set(1, temp);
		int32_t existing_flipped_index = -1;
		if (p_idempotent) {
			// Check if this flipped cell already exists before adding it, whenever idempotence is requested,
			// to avoid creating duplicates if this function is called multiple times. Note: This algorithm only
			// checks the original cells, which assumes that there are no same-facing duplicate cells in the original mesh.
			bool already_exists = false;
			for (int64_t other_cell_index = 0; other_cell_index < original_cell_count; other_cell_index++) {
				if (other_cell_index == cell_index) {
					continue;
				}
				if (cell_face_indices[other_cell_index] == flipped_cell_faces) {
					already_exists = true;
					existing_flipped_index = other_cell_index;
					break;
				}
			}
			if (already_exists) {
				flipped_cell_index_for_original.set(cell_index, existing_flipped_index);
				continue;
			}
		}
		// Copy the texture map if it exists for this cell before adding the flipped cell.
		if (_all_poly_cell_texture_maps.has(CELL_TO_VERT_KEY)) {
			// HashMap's indexing operator allows getting a mutable reference, so we don't need to set it back after.
			Vector<PackedVector3Array> &poly_cell_texture_maps = _all_poly_cell_texture_maps[CELL_TO_VERT_KEY];
			ERR_FAIL_COND(poly_cell_texture_maps.size() != poly_cell_boundary_normals.size());
			if (cell_index < poly_cell_texture_maps.size()) {
				const PackedVector3Array &flipped_cell_texture_map = poly_cell_texture_maps[cell_index];
				poly_cell_texture_maps.append(flipped_cell_texture_map);
			}
		}
		// Copy and flip the vertex normals if they exist for this cell before adding the flipped cell.
		if (_all_poly_cell_normals.has(CELL_TO_VERT_KEY)) {
			// HashMap's indexing operator allows getting a mutable reference, so we don't need to set it back after.
			Vector<PackedVector4Array> &poly_cell_vertex_normals = _all_poly_cell_normals[CELL_TO_VERT_KEY];
			ERR_FAIL_COND(poly_cell_vertex_normals.size() != poly_cell_boundary_normals.size());
			if (cell_index < poly_cell_vertex_normals.size()) {
				PackedVector4Array flipped_cell_vertex_normals = PackedVector4Array(poly_cell_vertex_normals[cell_index]);
				for (int64_t vertex_in_cell = 0; vertex_in_cell < flipped_cell_vertex_normals.size(); vertex_in_cell++) {
					flipped_cell_vertex_normals.set(vertex_in_cell, -flipped_cell_vertex_normals[vertex_in_cell]);
				}
				poly_cell_vertex_normals.append(flipped_cell_vertex_normals);
			}
		}
		// Append the flipped cell, and record its index for later when we update volumetric cells.
		const int32_t new_flipped_cell_index = cell_face_indices.size();
		cell_face_indices.append(flipped_cell_faces);
		flipped_cell_index_for_original.set(cell_index, new_flipped_cell_index);
		poly_cell_boundary_normals.append(-poly_cell_boundary_normals[cell_index]);
	}
	_poly_cell_indices.set(1, cell_face_indices);
	if (_poly_cell_indices.size() > 2) {
		Vector<PackedInt32Array> volumetric_cell_indices = _poly_cell_indices[2];
		for (int64_t volumetric_cell_index = 0; volumetric_cell_index < volumetric_cell_indices.size(); volumetric_cell_index++) {
			PackedInt32Array vol_cell = volumetric_cell_indices[volumetric_cell_index];
			const int64_t original_vol_cell_size = vol_cell.size();
			for (int64_t i = 0; i < original_vol_cell_size; i++) {
				const int32_t original_boundary_cell_index = vol_cell[i];
				if (original_boundary_cell_index < 0 || original_boundary_cell_index >= original_cell_count) {
					continue;
				}
				const int32_t flipped_boundary_cell_index = flipped_cell_index_for_original[original_boundary_cell_index];
				if (flipped_boundary_cell_index == -1 || vol_cell.has(flipped_boundary_cell_index)) {
					continue;
				}
				vol_cell.append(flipped_boundary_cell_index);
			}
			volumetric_cell_indices.set(volumetric_cell_index, vol_cell);
		}
		_poly_cell_indices.set(2, volumetric_cell_indices);
	}
	_all_poly_cell_normals.insert(PER_CELL_KEY, Vector<PackedVector4Array>{ poly_cell_boundary_normals });
	poly_mesh_clear_cache();
}

PackedInt32Array ArrayPolyMesh4D::make_single_cell_from_all_faces() const {
	ERR_FAIL_COND_V_MSG(_poly_cell_indices.size() < 1, PackedInt32Array(), "ArrayPolyMesh4D: Cannot make single cell from all faces because there are no faces.");
	const Vector<PackedInt32Array> &faces = _poly_cell_indices[0];
	const int64_t face_count = faces.size();
	PackedInt32Array cell_indices;
	cell_indices.resize(face_count);
	for (int64_t face_index = 0; face_index < face_count; face_index++) {
		cell_indices.set(face_index, face_index);
	}
	// For a deterministic cell orientation normal, the first two boundary cells must share a face.
	bool success = Math4D::ensure_first_two_indices_share_common_int32(cell_indices, faces);
	if (!success) {
		ERR_PRINT("ArrayPolyMesh4D: Failed to make single cell from all faces because the first face does not share an edge with any other face.");
	}
	return cell_indices;
}

PackedInt32Array ArrayPolyMesh4D::make_single_volume_from_all_cells() const {
	ERR_FAIL_COND_V_MSG(_poly_cell_indices.size() < 2, PackedInt32Array(), "ArrayPolyMesh4D: Cannot make single volume from all cells because there are no boundary cells.");
	const Vector<PackedInt32Array> &boundary_cells = _poly_cell_indices[1];
	const int64_t boundary_cell_count = boundary_cells.size();
	PackedInt32Array volumetric_cell_indices;
	volumetric_cell_indices.resize(boundary_cell_count);
	for (int64_t cell_index = 0; cell_index < boundary_cell_count; cell_index++) {
		volumetric_cell_indices.set(cell_index, cell_index);
	}
	// For a deterministic volumetric orientation basis, the first two boundary cells must share a face.
	bool success = Math4D::ensure_first_two_indices_share_common_int32(volumetric_cell_indices, boundary_cells);
	if (!success) {
		ERR_PRINT("ArrayPolyMesh4D: Failed to make single volume from all cells because the first boundary cell does not share a face with any other cell.");
	}
	return volumetric_cell_indices;
}

void ArrayPolyMesh4D::calculate_seam_faces(const double p_angle_threshold_radians, const bool p_discard_seams_within_islands) {
	ERR_FAIL_COND_MSG(_poly_cell_indices.size() < 2, "ArrayPolyMesh4D: Cannot calculate seam faces because there are no boundary cells.");
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayPolyMesh4D: Cannot calculate seam faces for an invalid mesh.");
	if (!_all_poly_cell_normals.has(PER_CELL_KEY) || _all_poly_cell_normals[PER_CELL_KEY][0].size() != _poly_cell_indices[1].size()) {
		calculate_boundary_normals(COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, false);
	}
	_seam_face_indices.clear();
	HashSet<int32_t> cells_between_volumetric;
	const Vector<PackedInt32Array> &cell_face_indices = _poly_cell_indices[1];
	const PackedVector4Array &poly_cell_boundary_normals = _all_poly_cell_normals[PER_CELL_KEY][0];
	CRASH_COND(poly_cell_boundary_normals.size() != cell_face_indices.size());
	if (_poly_cell_indices.size() > 2) {
		// If this is a volumetric poly mesh with 4D cells, ignore cells between volumetric cells.
		// Such cells are "virtual", only used to define geometry, and are not part of the visible surface.
		const Vector<PackedInt32Array> &volumetric_cell_indices = _poly_cell_indices[2];
		for (int64_t volumetric_cell_index = 0; volumetric_cell_index < volumetric_cell_indices.size(); volumetric_cell_index++) {
			const PackedInt32Array &vol_cells = volumetric_cell_indices[volumetric_cell_index];
			for (int64_t other_volumetric_cell_index = volumetric_cell_index + 1; other_volumetric_cell_index < volumetric_cell_indices.size(); other_volumetric_cell_index++) {
				const PackedInt32Array &other_vol_cells = volumetric_cell_indices[other_volumetric_cell_index];
				int64_t index_in_self;
				int64_t index_in_other;
				const int32_t common_cell = Math4D::find_common_int32(vol_cells, other_vol_cells, index_in_self, index_in_other);
				if (common_cell != INT32_MIN) {
					cells_between_volumetric.insert(common_cell);
				}
			}
		}
	}
	const Vector<PackedInt32Array> face_to_cell_map = _get_face_to_cell_map();
	// Actually calculate seams based on the angles between normals of adjacent cells.
	for (int64_t cell_index = 0; cell_index < cell_face_indices.size(); cell_index++) {
		if (cells_between_volumetric.has(cell_index)) {
			continue;
		}
		const PackedInt32Array &cell_faces = cell_face_indices[cell_index];
		for (int64_t face_index_in_cell = 0; face_index_in_cell < cell_faces.size(); face_index_in_cell++) {
			const int32_t face_index = cell_faces[face_index_in_cell];
			const PackedInt32Array &cells_using_face = face_to_cell_map[face_index];
			for (int64_t i = 0; i < cells_using_face.size(); i++) {
				const int32_t other_cell_index = cells_using_face[i];
				if (other_cell_index == cell_index || cells_between_volumetric.has(other_cell_index)) {
					continue;
				}
				const Vector4 &normal_a = poly_cell_boundary_normals[cell_index];
				const Vector4 &normal_b = poly_cell_boundary_normals[other_cell_index];
				if (Vector4D::angle_to(normal_a, normal_b) > p_angle_threshold_radians) {
					_seam_face_indices.insert(face_index);
				}
			}
		}
	}
	if (!p_discard_seams_within_islands) {
		return;
	}
	const Vector<PackedInt32Array> islands = collect_all_islands();
	for (int64_t island_index = 0; island_index < islands.size(); island_index++) {
		const PackedInt32Array &cells_in_island = islands[island_index];
		for (int64_t cell_index_index = 0; cell_index_index < cells_in_island.size(); cell_index_index++) {
			const int32_t cell_index = cells_in_island[cell_index_index];
			const PackedInt32Array &cell_faces = cell_face_indices[cell_index];
			for (int64_t other_cell_index_index = cell_index + 1; other_cell_index_index < cells_in_island.size(); other_cell_index_index++) {
				const int32_t other_cell_index = cells_in_island[other_cell_index_index];
				const PackedInt32Array &other_cell_faces = cell_face_indices[other_cell_index];
				int64_t index_in_self;
				int64_t index_in_other;
				const int32_t common_face = Math4D::find_common_int32(cell_faces, other_cell_faces, index_in_self, index_in_other);
				if (common_face != INT32_MIN) {
					_seam_face_indices.erase(common_face);
				}
			}
		}
	}
}

Vector<PackedInt32Array> ArrayPolyMesh4D::_get_face_to_cell_map() const {
	// For efficiency, pre-compute a mapping of face index to the cells that use it.
	const Vector<PackedInt32Array> &face_edge_indices = _poly_cell_indices[0];
	const Vector<PackedInt32Array> &cell_face_indices = _poly_cell_indices[1];
	Vector<PackedInt32Array> face_to_cells;
	face_to_cells.resize(face_edge_indices.size());
	for (int64_t cell_index = 0; cell_index < cell_face_indices.size(); cell_index++) {
		const PackedInt32Array &cell_faces = cell_face_indices[cell_index];
		for (int64_t face_index_in_cell = 0; face_index_in_cell < cell_faces.size(); face_index_in_cell++) {
			const int32_t face_index = cell_faces[face_index_in_cell];
			CRASH_COND(face_index < 0 || face_index >= face_to_cells.size());
			PackedInt32Array cells_for_face = face_to_cells[face_index];
			cells_for_face.append(int32_t(cell_index));
			face_to_cells.set(face_index, cells_for_face);
		}
	}
	return face_to_cells;
}

PackedInt32Array ArrayPolyMesh4D::_collect_cells_in_island_internal(const int64_t p_start_cell, const Vector<PackedInt32Array> &p_face_to_cell_map) {
	PackedInt32Array cells_in_island = { int32_t(p_start_cell) };
	PackedInt32Array faces_to_search = PackedInt32Array(_poly_cell_indices[1][p_start_cell]); // Copy.
	HashSet<int32_t> cells_in_island_set; // Redundant with cells_in_island, but faster to check existence for large meshes.
	HashSet<int32_t> faces_to_search_set; // Redundant with faces_to_search, but faster to check existence for large meshes.
	for (int32_t c : cells_in_island) {
		cells_in_island_set.insert(c);
	}
	for (int32_t f : faces_to_search) {
		faces_to_search_set.insert(f);
	}
	HashSet<int32_t> faces_already_searched;
	int64_t search_index = 0;
	while (search_index < faces_to_search.size()) {
		const int32_t face_index = faces_to_search[search_index];
		faces_already_searched.insert(face_index);
		if (_seam_face_indices.has(face_index)) {
			// This face is a seam, so do not cross it, as it may be another island.
			search_index++;
			continue;
		}
		const PackedInt32Array &cells_using_face = p_face_to_cell_map[face_index];
		for (int64_t i = 0; i < cells_using_face.size(); i++) {
			const int32_t cell_index = cells_using_face[i];
			if (!cells_in_island_set.has(cell_index)) {
				cells_in_island.append(cell_index);
				cells_in_island_set.insert(cell_index);
				// Add all faces of this new cell to the search list, except those already searched.
				const PackedInt32Array &new_cell_faces = _poly_cell_indices[1][cell_index];
				for (int64_t j = 0; j < new_cell_faces.size(); j++) {
					const int32_t new_face_index = new_cell_faces[j];
					if (!faces_already_searched.has(new_face_index) && !faces_to_search_set.has(new_face_index)) {
						faces_to_search.append(new_face_index);
						faces_to_search_set.insert(new_face_index);
					}
				}
			}
		}
		// If memory constraints are a concern, we could remove items from the start when this gets too big.
		// But in most cases, it's cheaper to just endlessly increase the search index.
		search_index++;
		faces_to_search_set.erase(face_index);
	}
	return cells_in_island;
}

PackedInt32Array ArrayPolyMesh4D::collect_cells_in_island(const int64_t p_start_cell) {
	// This function is exposed so start by performing validation to avoid running with garbage data.
	ERR_FAIL_COND_V_MSG(_poly_cell_indices.size() < 2, PackedInt32Array(), "ArrayPolyMesh4D: Cannot unwrap texture map for a mesh with no cells.");
	ERR_FAIL_INDEX_V_MSG(p_start_cell, _poly_cell_indices[1].size(), PackedInt32Array(), "ArrayPolyMesh4D: The island start cell is not in the mesh.");
	ERR_FAIL_COND_V_MSG(!is_poly_mesh_data_valid(), PackedInt32Array(), "ArrayPolyMesh4D: Poly mesh data is invalid, cannot unwrap.");
	const Vector<PackedInt32Array> face_to_cell_map = _get_face_to_cell_map();
	// Use the internal version internally when we know the data is valid.
	return _collect_cells_in_island_internal(p_start_cell, face_to_cell_map);
}

Vector<PackedInt32Array> ArrayPolyMesh4D::collect_all_islands() {
	ERR_FAIL_COND_V_MSG(_poly_cell_indices.size() < 2, Vector<PackedInt32Array>(), "ArrayPolyMesh4D: Cannot unwrap texture map for a mesh with no cells.");
	ERR_FAIL_COND_V_MSG(!is_poly_mesh_data_valid(), Vector<PackedInt32Array>(), "ArrayPolyMesh4D: Poly mesh data is invalid, cannot unwrap.");
	const Vector<PackedInt32Array> face_to_cell_map = _get_face_to_cell_map();
	Vector<PackedInt32Array> islands;
	for (int64_t cell_index = 0; cell_index < _poly_cell_indices[1].size(); cell_index++) {
		bool cell_already_in_island = false;
		for (int64_t island_index = 0; island_index < islands.size(); island_index++) {
			if (islands[island_index].has(cell_index)) {
				cell_already_in_island = true;
				break;
			}
		}
		if (cell_already_in_island) {
			continue;
		}
		islands.append(_collect_cells_in_island_internal(cell_index, face_to_cell_map));
	}
	return islands;
}

PackedInt32Array ArrayPolyMesh4D::_get_cell_4_vertices_starting_from_face(const int64_t p_which_cell, const int64_t p_start_face_in_cell) const {
	const PackedInt32Array &cell_faces = _poly_cell_indices[1][p_which_cell];
	ERR_FAIL_INDEX_V(p_start_face_in_cell, cell_faces.size(), PackedInt32Array());
	const int32_t first_face_index = cell_faces[p_start_face_in_cell];
	const PackedInt32Array &first_face = _poly_cell_indices[0][first_face_index];
	int64_t common_in_first = 0;
	int64_t common_in_second = 0;
	int32_t common_edge = INT32_MIN;
	for (int64_t cell_face_index = 0; cell_face_index < cell_faces.size(); cell_face_index++) {
		const int32_t face_index = cell_faces[cell_face_index];
		if (face_index == first_face_index) {
			continue;
		}
		const PackedInt32Array &second_face = _poly_cell_indices[0][face_index];
		common_edge = Math4D::find_common_int32(first_face, second_face, common_in_first, common_in_second);
		if (common_edge != INT32_MIN) {
			const int64_t first_next_edge = (common_in_first + 1) % first_face.size();
			const int64_t second_next_edge = (common_in_second + 1) % second_face.size();
			// Use these 3 edges to get 4 vertex indices in a consistent "winding" order.
			const int32_t common_vertex_start = _edge_vertex_indices[common_edge * 2];
			const int32_t common_vertex_end = _edge_vertex_indices[common_edge * 2 + 1];
			int32_t first_next_vertex = _edge_vertex_indices[first_face[first_next_edge] * 2];
			if (first_next_vertex == common_vertex_start || first_next_vertex == common_vertex_end) {
				first_next_vertex = _edge_vertex_indices[first_face[first_next_edge] * 2 + 1];
			}
			int32_t second_next_vertex = _edge_vertex_indices[second_face[second_next_edge] * 2];
			if (second_next_vertex == common_vertex_start || second_next_vertex == common_vertex_end) {
				second_next_vertex = _edge_vertex_indices[second_face[second_next_edge] * 2 + 1];
			}
			return PackedInt32Array{ first_next_vertex, common_vertex_start, common_vertex_end, second_next_vertex };
		}
	}
	ERR_FAIL_V_MSG(PackedInt32Array(), "ArrayPolyMesh4D: Cell face does not share a common edge with any other face, this cell is invalid.");
}

void ArrayPolyMesh4D::_get_cell_world_span_seed(const int64_t p_which_cell, Vector4 &r_world_x, Vector4 &r_world_y, Vector4 &r_world_z, int32_t &p_pivot) const {
	const PackedInt32Array &cell_faces = _poly_cell_indices[1][p_which_cell];
	const int32_t first_face_index = cell_faces[0];
	const int32_t second_face_index = cell_faces[1];
	const PackedInt32Array &first_face = _poly_cell_indices[0][first_face_index];
	const PackedInt32Array &second_face = _poly_cell_indices[0][second_face_index];
	int64_t common_in_first = 0;
	int64_t common_in_second = 0;
	int32_t common_edge = Math4D::find_common_int32(first_face, second_face, common_in_first, common_in_second);
	ERR_FAIL_COND_MSG(common_edge == INT32_MIN, "ArrayPolyMesh4D: First two faces of cell do not share a common edge, this cell is invalid.");
	const int64_t first_next_edge = (common_in_first + 1) % first_face.size();
	const int64_t second_next_edge = (common_in_second + 1) % second_face.size();
	// Use these 3 edges to get 4 vertex indices in a consistent "winding" order.
	int32_t common_vertex_start = _edge_vertex_indices[common_edge * 2];
	int32_t common_vertex_end = _edge_vertex_indices[common_edge * 2 + 1];
	int32_t first_next_vertex = _edge_vertex_indices[first_face[first_next_edge] * 2];
	int32_t first_common_vertex = _edge_vertex_indices[first_face[first_next_edge] * 2 + 1];
	if (first_next_vertex == common_vertex_start || first_next_vertex == common_vertex_end) {
		SWAP(first_next_vertex, first_common_vertex);
	}
	p_pivot = first_next_vertex;
	r_world_x = _poly_cell_vertices[first_next_vertex].direction_to(_poly_cell_vertices[first_common_vertex]);
	if (first_common_vertex == common_vertex_start) {
		r_world_y = _poly_cell_vertices[first_common_vertex].direction_to(_poly_cell_vertices[common_vertex_end]);
	} else {
		r_world_y = _poly_cell_vertices[first_common_vertex].direction_to(_poly_cell_vertices[common_vertex_start]);
	}
	int32_t second_next_vertex = _edge_vertex_indices[second_face[second_next_edge] * 2];
	int32_t second_common_vertex = _edge_vertex_indices[second_face[second_next_edge] * 2 + 1];
	if (second_next_vertex == common_vertex_start || second_next_vertex == common_vertex_end) {
		SWAP(second_next_vertex, second_common_vertex);
	}
	r_world_z = _poly_cell_vertices[second_common_vertex].direction_to(_poly_cell_vertices[second_next_vertex]);
}

void ArrayPolyMesh4D::_transform_cell_to_texture_space(const Transform4D &p_world_to_texcoord, const Vector<PackedInt32Array> &p_cell_vert, const int64_t p_cell_index, const int32_t p_pivot, Vector<PackedVector3Array> &r_poly_cell_texture_map) {
	const PackedInt32Array &cell_vert = p_cell_vert[p_cell_index];
	PackedVector3Array cell_texture_map;
	cell_texture_map.resize(cell_vert.size());
	for (int64_t i = 0; i < cell_vert.size(); i++) {
		// If we ever have 3.5-dimensional transforms, this would be a good place to use them.
		// But let's not make a whole new data structure for one use case, we'll use 4D transforms instead.
		const Vector4 offset = _poly_cell_vertices[cell_vert[i]] - _poly_cell_vertices[p_pivot];
		const Vector4 rel = p_world_to_texcoord.xform(offset);
		cell_texture_map.set(i, Vector3(rel.x, rel.y, rel.z));
	}
	r_poly_cell_texture_map.set(p_cell_index, cell_texture_map);
}

bool ArrayPolyMesh4D::_unwrap_texture_map_island_cell(const PackedInt32Array &p_cells_in_island, const int64_t p_current_cell_index_index, const Vector<PackedInt32Array> &p_cell_vert) {
	const int32_t cell_index = p_cells_in_island[p_current_cell_index_index];
	const PackedInt32Array &cell_data = _poly_cell_indices[1][cell_index];
	if (!_all_poly_cell_texture_maps.has(CELL_TO_VERT_KEY)) {
		_all_poly_cell_texture_maps.insert(CELL_TO_VERT_KEY, Vector<PackedVector3Array>());
	}
	// HashMap's indexing operator allows getting a mutable reference, so we don't need to set it back after.
	Vector<PackedVector3Array> &poly_cell_texture_map = _all_poly_cell_texture_maps[CELL_TO_VERT_KEY];
	// For the first cell in the island, there is nothing to "build on",
	// so just start by directly projecting.
	if (p_current_cell_index_index == 0) {
		Vector4 world_x, world_y, world_z;
		int32_t pivot;
		_get_cell_world_span_seed(cell_index, world_x, world_y, world_z, pivot);
		const Basis4D world_coord = Basis4D::from_xyz(world_x, world_y, world_z).orthonormalized();
		ERR_FAIL_COND_V_MSG(Math::is_zero_approx(world_coord.determinant()), false, "ArrayPolyMesh4D: Cell is degenerate.");
		// This needs to be "flattened" into the UVW texture space.
		Basis4D tex_coord = Basis4D(world_coord);
		tex_coord.x.w = 0.0;
		tex_coord.y.w = 0.0;
		tex_coord.z.w = 0.0;
		tex_coord.orthonormalize();
		if (Math::is_zero_approx(tex_coord.determinant())) {
			tex_coord = Basis4D();
		}
		const Transform4D world_to_texcoord = Transform4D(world_coord.transform_to(tex_coord));
		_transform_cell_to_texture_space(world_to_texcoord, p_cell_vert, cell_index, pivot, poly_cell_texture_map);
		return true;
	}
	// Search for neighboring cells in this island which share a face with this cell.
	for (int64_t already_mapped_cell_index_index = 0; already_mapped_cell_index_index < p_current_cell_index_index; already_mapped_cell_index_index++) {
		const int32_t already_mapped_cell_index = p_cells_in_island[already_mapped_cell_index_index];
		const PackedInt32Array &already_mapped_cell_data = _poly_cell_indices[1][already_mapped_cell_index];
		const PackedInt32Array &already_mapped_cell_verts = p_cell_vert[already_mapped_cell_index];
		const PackedVector3Array &already_mapped_texture_map = poly_cell_texture_map[already_mapped_cell_index];
		if (already_mapped_cell_verts.is_empty() || already_mapped_texture_map.is_empty()) {
			continue;
		}
		int64_t cell_data_index;
		int64_t already_mapped_cell_data_index;
		const int32_t common_face = Math4D::find_common_int32(cell_data, already_mapped_cell_data, cell_data_index, already_mapped_cell_data_index);
		if (common_face != INT32_MIN) {
			const PackedInt32Array cell_span = _get_cell_4_vertices_starting_from_face(cell_index, cell_data_index);
			ERR_FAIL_COND_V_MSG(cell_span.size() != 4, false, "ArrayPolyMesh4D: Failed to get 4 vertex span for cell.");
			// Don't normalize X and Y because the lengths of these matter.
			const Vector4 world_x = _poly_cell_vertices[cell_span[1]] - _poly_cell_vertices[cell_span[0]];
			const Vector4 world_y = _poly_cell_vertices[cell_span[2]] - _poly_cell_vertices[cell_span[0]];
			// Z needs to be perpendicular to X and Y and the length proportion needs to be consistent between world and texcoord space.
			Vector4 world_z = _poly_cell_vertices[cell_span[3]] - _poly_cell_vertices[cell_span[0]];
			const real_t world_z_len = world_x.length() * world_y.length();
			world_z = Vector4D::orthogonal_from_two(world_z, world_x, world_y).normalized() * world_z_len;
			ERR_FAIL_COND_V_MSG(Math::is_zero_approx(world_z.length()), false, "ArrayPolyMesh4D: Cell is degenerate.");
			const Basis4D world_coord = Basis4D::from_xyz(world_x, world_y, world_z);
			// Don't use Math::is_zero_approx here because this will be very small for small cells.
			// Instead let's hand-roll our own tolerance epsilon based on the lengths of the axes.
			// world_z_len already includes X and Y, so this accounts for all axes, and therefore
			// the tolerance has an effective dimensionality of length^4.
			real_t tolerance = world_z_len * world_z_len * CMP_EPSILON;
			if (tolerance < 1e-38) {
				tolerance = 1e-38; // Lower bound based on 32-bit floats.
			}
			ERR_FAIL_COND_V_MSG(Math::abs(world_coord.determinant()) < tolerance, false, "ArrayPolyMesh4D: Cell is degenerate.");
			const int64_t texcoord_start_index = already_mapped_cell_verts.find(cell_span[0]);
			const int64_t texcoord_x_index = already_mapped_cell_verts.find(cell_span[1]);
			const int64_t texcoord_y_index = already_mapped_cell_verts.find(cell_span[2]);
			if (texcoord_start_index < 0 || texcoord_x_index < 0 || texcoord_y_index < 0) {
				continue;
			}
			const Vector3 texcoord_start = already_mapped_texture_map[texcoord_start_index];
			const Vector3 texcoord_x = already_mapped_texture_map[texcoord_x_index] - texcoord_start;
			const Vector3 texcoord_y = already_mapped_texture_map[texcoord_y_index] - texcoord_start;
			// This could be `length_squared()`, so long as the world's lengths were also changed, but then it would
			// fail for vertex separations around 10^-9 or smaller, or vertex separations around 10^9 or bigger,
			// which I think is... not entirely unreasonable of a use case, so let's just use `length()`.
			Vector3 texcoord_z = texcoord_x.cross(texcoord_y).normalized() * (texcoord_x.length() * texcoord_y.length());
			Vector3 already_mapped_average;
			for (int64_t i = 0; i < already_mapped_texture_map.size(); i++) {
				already_mapped_average += already_mapped_texture_map[i];
			}
			already_mapped_average /= (real_t)already_mapped_texture_map.size();
			const Vector3 to_already_mapped_average = texcoord_start.direction_to(already_mapped_average);
			// We want the Z axis to point "away" from the already mapped cell.
			if (to_already_mapped_average.dot(texcoord_z) > 0) {
				texcoord_z = -texcoord_z;
			}
			const Basis4D tex_coord = Basis4D::from_xyz(Vector4D::from_3d(texcoord_x), Vector4D::from_3d(texcoord_y), Vector4D::from_3d(texcoord_z));
			const Transform4D world_to_texcoord = Transform4D(world_coord.transform_to(tex_coord), Vector4D::from_3d(texcoord_start));
			_transform_cell_to_texture_space(world_to_texcoord, p_cell_vert, cell_index, cell_span[0], poly_cell_texture_map);
			return true;
		}
	}
	ERR_FAIL_V_MSG(false, "ArrayPolyMesh4D: Island cells are not contiguous, cannot unwrap.");
}

void ArrayPolyMesh4D::_unwrap_texture_map_island_internal(const PackedInt32Array &p_cells_in_island, const bool p_keep_existing) {
	const Vector<PackedVector3Array> &poly_cell_texture_map = _all_poly_cell_texture_maps[CELL_TO_VERT_KEY];
	CRASH_COND(poly_cell_texture_map.size() != _poly_cell_indices[1].size());
	const Vector<PackedInt32Array> cell_vert = _get_vertex_indices_of_boundary_cells(_poly_cell_indices, _edge_vertex_indices, false);
	for (int64_t cell_index_index = 0; cell_index_index < p_cells_in_island.size(); cell_index_index++) {
		if (p_keep_existing && !poly_cell_texture_map[p_cells_in_island[cell_index_index]].is_empty()) {
			continue;
		}
		if (!_unwrap_texture_map_island_cell(p_cells_in_island, cell_index_index, cell_vert)) {
			return;
		}
	}
}

void ArrayPolyMesh4D::unwrap_texture_map_island(const PackedInt32Array &p_cells_in_island, const bool p_keep_existing) {
	// This function is exposed so start by performing validation to avoid running with garbage data.
	ERR_FAIL_COND_MSG(_poly_cell_indices.size() < 2, "ArrayPolyMesh4D: Cannot unwrap texture map for a mesh with no cells.");
	ERR_FAIL_COND_MSG(p_cells_in_island.is_empty(), "ArrayPolyMesh4D: Cannot unwrap texture map for an empty island of cells.");
	const Vector<PackedInt32Array> cells = _poly_cell_indices[1];
	for (int64_t i = 0; i < p_cells_in_island.size(); i++) {
		ERR_FAIL_COND_MSG(p_cells_in_island[i] >= cells.size(), "ArrayPolyMesh4D: A cell in this island is not in the mesh.");
	}
	ERR_FAIL_COND_MSG(!is_poly_mesh_data_valid(), "ArrayPolyMesh4D: Poly mesh data is invalid, cannot unwrap.");
	const int64_t cell_count = cells.size();
	// HashMap's indexing operator allows getting a mutable reference, so we don't need to set it back after.
	Vector<PackedVector3Array> &poly_cell_texture_map = _all_poly_cell_texture_maps[CELL_TO_VERT_KEY];
	const int64_t existing_texture_map_count = poly_cell_texture_map.size();
	poly_cell_texture_map.resize_initialized(cell_count);
	for (int64_t i = existing_texture_map_count; i < cell_count; i++) {
		poly_cell_texture_map.set(i, PackedVector3Array());
	}
	// Use the internal version internally when we know the data is valid.
	_unwrap_texture_map_island_internal(p_cells_in_island, p_keep_existing);
	poly_mesh_clear_cache();
}

void ArrayPolyMesh4D::unwrap_texture_map(const UnwrapTextureMapMode p_mode, const double p_padding, const bool p_proportional, const bool p_keep_existing) {
	ERR_FAIL_COND_MSG(p_padding < 0.0, "ArrayPolyMesh4D: Padding must be non-negative.");
	ERR_FAIL_COND_MSG(_poly_cell_indices.size() < 2, "ArrayPolyMesh4D: Cannot unwrap texture map for a mesh with no cells.");
	ERR_FAIL_COND_MSG(!is_poly_mesh_data_valid(), "ArrayPolyMesh4D: Poly mesh data is invalid, cannot unwrap.");
	const int64_t cell_count = _poly_cell_indices[1].size();
	if (!p_keep_existing) {
		_all_poly_cell_texture_maps.erase(CELL_TO_VERT_KEY);
	}
	// HashMap's indexing operator allows getting a mutable reference, so we don't need to set it back after.
	Vector<PackedVector3Array> &poly_cell_texture_map = _all_poly_cell_texture_maps[CELL_TO_VERT_KEY];
	const int64_t existing_texture_map_count = poly_cell_texture_map.size();
	poly_cell_texture_map.resize_initialized(cell_count);
	for (int64_t i = existing_texture_map_count; i < cell_count; i++) {
		poly_cell_texture_map.set(i, PackedVector3Array());
	}
	UnwrapTextureMapMode actual_mode = p_mode;
	if (actual_mode == UNWRAP_MODE_AUTOMATIC) {
		actual_mode = _seam_face_indices.is_empty() ? UNWRAP_MODE_TILE_CELLS : UNWRAP_MODE_TILE_ISLANDS;
	}
	const double pad_offset = p_padding * 0.5 / (1.0 + p_padding);
	const double pad_size = 1.0 / (1.0 + p_padding);
	// Step 1: What is the list of islands we need to unwrap? This depends on the mode.
	Vector<PackedInt32Array> islands;
	if (actual_mode == UNWRAP_MODE_EACH_CELL_FILLS || actual_mode == UNWRAP_MODE_TILE_CELLS) {
		islands.resize(cell_count);
		for (int32_t cell_index = 0; cell_index < cell_count; cell_index++) {
			islands.set(cell_index, PackedInt32Array{ cell_index });
		}
	} else if (actual_mode == UNWRAP_MODE_EACH_ISLAND_FILLS || actual_mode == UNWRAP_MODE_TILE_ISLANDS) {
		islands = collect_all_islands();
	} else {
		ERR_FAIL_MSG("ArrayPolyMesh4D: Unknown unwrap texture map mode.");
	}
	// Step 2: Unwrap each island individually into an unbounded space.
	for (int64_t island_index = 0; island_index < islands.size(); island_index++) {
		_unwrap_texture_map_island_internal(islands[island_index], p_keep_existing);
	}
	// Step 3: Fit or tile the islands into the texture space depending on the mode.
	if (actual_mode == UNWRAP_MODE_EACH_CELL_FILLS || actual_mode == UNWRAP_MODE_EACH_ISLAND_FILLS) {
		// Fit each island into the 0-to-1 UVW texture space with padding.
		const AABB padded_full_aabb = AABB(Vector3(pad_offset, pad_offset, pad_offset), Vector3(pad_size, pad_size, pad_size));
		for (int32_t island_index = 0; island_index < islands.size(); island_index++) {
			_fit_island_texture_map_into_aabb(islands[island_index], padded_full_aabb, p_proportional);
		}
	} else if (actual_mode == UNWRAP_MODE_TILE_CELLS || actual_mode == UNWRAP_MODE_TILE_ISLANDS) {
		// Tile the islands in texture space by rescaling and offsetting them.
		const Vector3i tiles = _tiles_for_island_count(islands.size());
		const Vector3 unpadded_tile_size = Vector3(1.0 / real_t(tiles.x), 1.0 / real_t(tiles.y), 1.0 / real_t(tiles.z));
		const Vector3 padding_offset = unpadded_tile_size * pad_offset;
		const Vector3 padded_tile_size = unpadded_tile_size * pad_size;
		for (int32_t x = 0; x < tiles.x; x++) {
			for (int32_t y = 0; y < tiles.y; y++) {
				for (int32_t z = 0; z < tiles.z; z++) {
					const int32_t tile_index = x + y * tiles.x + z * tiles.x * tiles.y;
					if (tile_index >= islands.size()) {
						break;
					}
					const PackedInt32Array &cells_in_island = islands[tile_index];
					const AABB island_aabb = AABB(Vector3(x, y, z) * unpadded_tile_size + padding_offset, padded_tile_size);
					_fit_island_texture_map_into_aabb(cells_in_island, island_aabb, p_proportional);
				}
			}
		}
	} else {
		ERR_FAIL_MSG("ArrayPolyMesh4D: Unknown unwrap texture map mode.");
	}
	poly_mesh_clear_cache();
}

void ArrayPolyMesh4D::_fit_island_texture_map_into_aabb(const PackedInt32Array &p_cells_in_island, const AABB &p_target_aabb, const bool p_proportional) {
	// HashMap's indexing operator allows getting a mutable reference, so we don't need to set it back after.
	Vector<PackedVector3Array> &poly_cell_texture_map = _all_poly_cell_texture_maps[CELL_TO_VERT_KEY];
	const PackedVector3Array first_cell_texture_map = poly_cell_texture_map[p_cells_in_island[0]];
	ERR_FAIL_COND_MSG(first_cell_texture_map.is_empty(), "ArrayPolyMesh4D: Cannot fit island texture map into AABB because at least one cell in the island has an empty texture map.");
	AABB current_aabb = AABB(first_cell_texture_map[0], Vector3());
	for (int64_t cell_index_index = 0; cell_index_index < p_cells_in_island.size(); cell_index_index++) {
		const int32_t cell_index = p_cells_in_island[cell_index_index];
		const PackedVector3Array &cell_texture_map = poly_cell_texture_map[cell_index];
		for (int64_t vertex_index = 0; vertex_index < cell_texture_map.size(); vertex_index++) {
			current_aabb.expand_to(cell_texture_map[vertex_index]);
		}
	};
	Vector3 scale_vec;
	scale_vec.x = (current_aabb.size.x < CMP_EPSILON2) ? 1.0 : p_target_aabb.size.x / current_aabb.size.x;
	scale_vec.y = (current_aabb.size.y < CMP_EPSILON2) ? 1.0 : p_target_aabb.size.y / current_aabb.size.y;
	scale_vec.z = (current_aabb.size.z < CMP_EPSILON2) ? 1.0 : p_target_aabb.size.z / current_aabb.size.z;
	if (p_proportional) {
		const real_t min_scale = MIN(MIN(scale_vec.x, scale_vec.y), scale_vec.z);
		scale_vec = Vector3(min_scale, min_scale, min_scale);
	}
	const Basis scale_basis = Basis::from_scale(scale_vec);
	// CAUTION: Godot's Basis.xform_inv performs a transposed xform, not an inverse xform.
	// In this case, we explicitly want a transposed xform, so we use this badly named function.
	// See this proposal to rename it upstream: https://github.com/godotengine/godot-proposals/issues/11235
	const Transform3D to_target = Transform3D(scale_basis, p_target_aabb.position - scale_basis.xform_inv(current_aabb.position));
	for (int64_t cell_index_index = 0; cell_index_index < p_cells_in_island.size(); cell_index_index++) {
		const int32_t cell_index = p_cells_in_island[cell_index_index];
		PackedVector3Array cell_texture_map = poly_cell_texture_map[cell_index];
		for (int64_t vertex_index = 0; vertex_index < cell_texture_map.size(); vertex_index++) {
			cell_texture_map.set(vertex_index, to_target.xform(cell_texture_map[vertex_index]));
		}
		poly_cell_texture_map.set(cell_index, cell_texture_map);
	}
}

Vector3i ArrayPolyMesh4D::_tiles_for_island_count(const int32_t p_island_count) {
	if (p_island_count < 2) {
		return Vector3i(1, 1, 1);
	}
	const int32_t cube_root = Math::ceil(std::cbrt(p_island_count));
	int32_t remaining = _ceil_div(p_island_count, cube_root);
	const int32_t square_root = Math::ceil(std::sqrt(remaining));
	remaining = _ceil_div(remaining, square_root);
	return Vector3i(remaining, square_root, cube_root);
}

void ArrayPolyMesh4D::transform_texture_map(const Transform3D &p_transform) {
	// HashMap's indexing operator allows getting a mutable reference, so we don't need to set it back after.
	Vector<PackedVector3Array> &poly_cell_texture_map = _all_poly_cell_texture_maps[CELL_TO_VERT_KEY];
	const int64_t cell_count = poly_cell_texture_map.size();
	for (int64_t cell_index = 0; cell_index < cell_count; cell_index++) {
		PackedVector3Array cell_texture_map = poly_cell_texture_map[cell_index];
		for (int64_t vertex_index = 0; vertex_index < cell_texture_map.size(); vertex_index++) {
			cell_texture_map.set(vertex_index, p_transform.xform(cell_texture_map[vertex_index]));
		}
		poly_cell_texture_map.set(cell_index, cell_texture_map);
	}
	poly_mesh_clear_cache();
}

// Misc functions.

void ArrayPolyMesh4D::deduplicate_all_elements() {
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayPolyMesh4D: Cannot deduplicate elements of an invalid mesh.");
	// We need to ensure the boundary normals stay the same before and after deduplication,
	// which means we need to start with boundary normals calculated from the original data.
	if (!_all_poly_cell_normals.has(PER_CELL_KEY) || _all_poly_cell_normals[PER_CELL_KEY].size() != _poly_cell_indices[1].size()) {
		calculate_boundary_normals();
	}
	// Snapshot the sub-element traversal order for all decomposed bindings BEFORE any deduplication.
	// Deduplication can change sub-element ordering within cells (e.g., edge deduplication can reverse
	// an edge's stored vertex order), breaking data indexed by traversal position.
	HashMap<Vector2i, Vector<PackedInt32Array>> pre_dedup_poly;
	for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &pre_kv : _all_poly_cell_normals) {
		const Vector2i pre_key = pre_kv.key;
		if (pre_key.x != pre_key.y && !pre_dedup_poly.has(pre_key)) {
			pre_dedup_poly.insert(pre_key, get_all_poly_cell_poly_indices(pre_key.x, pre_key.y));
		}
	}
	for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &pre_kv : _all_poly_cell_texture_maps) {
		const Vector2i pre_key = pre_kv.key;
		if (pre_key.x != pre_key.y && !pre_dedup_poly.has(pre_key)) {
			pre_dedup_poly.insert(pre_key, get_all_poly_cell_poly_indices(pre_key.x, pre_key.y));
		}
	}
	// Deduplicate vertices.
	PackedVector4Array output_vertices;
	HashMap<int32_t, int32_t> vertex_index_remap;
	for (int64_t input_vertex_index = 0; input_vertex_index < _poly_cell_vertices.size(); input_vertex_index++) {
		const Vector4 vertex = _poly_cell_vertices[input_vertex_index];
		bool found_duplicate = false;
		for (int64_t output_vertex_index = 0; output_vertex_index < output_vertices.size(); output_vertex_index++) {
			if (vertex.is_equal_approx(output_vertices[output_vertex_index])) {
				vertex_index_remap[input_vertex_index] = (int32_t)output_vertex_index;
				found_duplicate = true;
				break;
			}
		}
		if (!found_duplicate) {
			vertex_index_remap[input_vertex_index] = (int32_t)output_vertices.size();
			output_vertices.append(vertex);
		}
	}
	// Update edges that reference those vertices.
	for (int64_t edge_index = 0; edge_index < _edge_vertex_indices.size(); edge_index++) {
		const int64_t input_vertex_index = _edge_vertex_indices[edge_index];
		_edge_vertex_indices.set(edge_index, vertex_index_remap[input_vertex_index]);
	}
	// Deduplicate edges.
	PackedInt32Array output_edge_vertex_indices;
	HashMap<int32_t, int32_t> edge_index_remap;
	for (int64_t input_edge_index = 0; input_edge_index < _edge_vertex_indices.size(); input_edge_index += 2) {
		const int32_t vertex_index_a = _edge_vertex_indices[input_edge_index];
		const int32_t vertex_index_b = _edge_vertex_indices[input_edge_index + 1];
		bool found_duplicate = false;
		for (int64_t output_edge_index = 0; output_edge_index < output_edge_vertex_indices.size(); output_edge_index += 2) {
			const int32_t output_vertex_index_a = output_edge_vertex_indices[output_edge_index];
			const int32_t output_vertex_index_b = output_edge_vertex_indices[output_edge_index + 1];
			// Deduplicate edges in the same order and in the opposite order.
			// Both orders should be considered the same edge in the PolyMesh4D code.
			if ((vertex_index_a == output_vertex_index_a && vertex_index_b == output_vertex_index_b) ||
					(vertex_index_a == output_vertex_index_b && vertex_index_b == output_vertex_index_a)) {
				edge_index_remap[input_edge_index / 2] = output_edge_index / 2;
				found_duplicate = true;
				break;
			}
		}
		if (!found_duplicate) {
			edge_index_remap[input_edge_index / 2] = output_edge_vertex_indices.size() / 2;
			output_edge_vertex_indices.append(vertex_index_a);
			output_edge_vertex_indices.append(vertex_index_b);
		}
	}
	// Deduplicate poly cell indices.
	Vector<Vector<PackedInt32Array>> output_poly_cell_indices;
	Vector<HashMap<int32_t, int32_t>> poly_cell_index_remaps;
	for (int64_t dim_index = 0; dim_index < _poly_cell_indices.size(); dim_index++) {
		Vector<PackedInt32Array> dim_output;
		Vector<PackedInt32Array> dim_output_sorted;
		HashMap<int32_t, int32_t> dim_index_remap;
		const HashMap<int32_t, int32_t> &prev_index_remap = (dim_index == 0) ? edge_index_remap : poly_cell_index_remaps[dim_index - 1];
		Vector<PackedInt32Array> input_cells = _poly_cell_indices[dim_index];
		for (int64_t input_cell_index = 0; input_cell_index < input_cells.size(); input_cell_index++) {
			PackedInt32Array cell = input_cells[input_cell_index];
			// Remap the indices in the cell based on the previous remap.
			for (int64_t i = 0; i < cell.size(); i++) {
				cell.set(i, prev_index_remap[cell[i]]);
			}
			// Deduplicate cells regardless of the order of the indices in the cell.
			PackedInt32Array cell_sorted = PackedInt32Array(cell); // Copy.
			cell_sorted.sort();
			bool found_duplicate = false;
			for (int64_t output_cell_index = 0; output_cell_index < dim_output.size(); output_cell_index++) {
				if (cell_sorted == dim_output_sorted[output_cell_index]) {
					dim_index_remap[input_cell_index] = output_cell_index;
					found_duplicate = true;
					break;
				}
			}
			if (!found_duplicate) {
				dim_index_remap[input_cell_index] = dim_output.size();
				dim_output.append(cell);
				dim_output_sorted.append(cell_sorted);
			}
		}
		output_poly_cell_indices.append(dim_output);
		poly_cell_index_remaps.append(dim_index_remap);
	}
	// Write back deduplicated geometry now so that get_all_poly_cell_poly_indices reads the new arrays
	// when computing the post-dedup sub-element orderings for remapping data bindings.
	const int64_t input_boundary_cell_count = (_poly_cell_indices.size() > 1) ? _poly_cell_indices[1].size() : 0;
	_poly_cell_vertices = output_vertices;
	_edge_vertex_indices = output_edge_vertex_indices;
	_poly_cell_indices = output_poly_cell_indices;
	// Snapshot the sub-element traversal order for all decomposed bindings AFTER deduplication.
	HashMap<Vector2i, Vector<PackedInt32Array>> post_dedup_poly;
	for (const KeyValue<Vector2i, Vector<PackedInt32Array>> &post_kv : pre_dedup_poly) {
		post_dedup_poly.insert(post_kv.key, get_all_poly_cell_poly_indices(post_kv.key.x, post_kv.key.y));
	}
	// Update boundary pivot overrides based on boundary cell remap, then remap pivot vertices.
	PackedInt32Array output_poly_cell_boundary_pivot_overrides;
	if (!_poly_cell_boundary_pivot_overrides.is_empty() && input_boundary_cell_count > 0 && output_poly_cell_indices.size() > 1) {
		const int64_t output_boundary_cell_count = output_poly_cell_indices[1].size();
		const int64_t input_pivot_count = MIN(_poly_cell_boundary_pivot_overrides.size(), input_boundary_cell_count);
		output_poly_cell_boundary_pivot_overrides.resize(output_boundary_cell_count);
		for (int64_t i = 0; i < output_boundary_cell_count; i++) {
			output_poly_cell_boundary_pivot_overrides.set(i, -1);
		}
		const HashMap<int32_t, int32_t> &boundary_cell_index_remap = poly_cell_index_remaps[1];
		for (int64_t input_cell_index = 0; input_cell_index < input_pivot_count; input_cell_index++) {
			const int32_t input_pivot_vertex_index = _poly_cell_boundary_pivot_overrides[input_cell_index];
			if (input_pivot_vertex_index < 0) {
				continue;
			}
			const int32_t output_cell_index = boundary_cell_index_remap[input_cell_index];
			if (output_poly_cell_boundary_pivot_overrides[output_cell_index] == -1) {
				output_poly_cell_boundary_pivot_overrides.set(output_cell_index, vertex_index_remap[input_pivot_vertex_index]);
			}
		}
	}
	// Update seam face indices based on the face index remap.
	HashSet<int32_t> output_seam_face_indices;
	if (_seam_face_indices.size() > 0 && _poly_cell_indices.size() > 0) {
		const HashMap<int32_t, int32_t> &face_index_remap = poly_cell_index_remaps[0];
		for (const int32_t seam_face_index : _seam_face_indices) {
			output_seam_face_indices.insert(face_index_remap[seam_face_index]);
		}
	}
	// Update poly cell normal bindings.
	HashMap<Vector2i, Vector<PackedVector4Array>> output_poly_cell_normals;
	for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &kv : _all_poly_cell_normals) {
		const Vector2i key = kv.key;
		ERR_CONTINUE_MSG((key.x - 2) >= poly_cell_index_remaps.size(), "ArrayPolyMesh4D: Invalid normal binding for geometry dimension " + itos(key.x) + ". Skipping.");
		const HashMap<int32_t, int32_t> &index_remap = (key.x == 0) ? vertex_index_remap : ((key.x == 1) ? edge_index_remap : poly_cell_index_remaps[key.x - 2]);
		const Vector<PackedVector4Array> &input_normal_data = kv.value;
		Vector<PackedVector4Array> output_normals_data;
		if (key.y == key.x && input_normal_data.size() == 1) {
			// Non-decomposed normals use a flat structure.
			const PackedVector4Array &input_flat_normals = input_normal_data[0];
			PackedVector4Array output_flat_normals;
			for (int64_t input_index = 0; input_index < input_flat_normals.size(); input_index++) {
				const int64_t output_index = index_remap[input_index];
				// New items should be contiguously added in the same order as the input,
				// so the next output index should never be greater than the current output size.
				CRASH_COND(output_index > output_flat_normals.size());
				// When deduplicating, we only want the first copy's attached data,
				// so that it pairs with the geometry that was kept.
				if (output_index == output_flat_normals.size()) {
					output_flat_normals.resize(output_index + 1);
					output_flat_normals.set(output_index, input_flat_normals[input_index]);
				}
			}
			output_normals_data = { output_flat_normals };
		} else {
			for (int64_t input_index = 0; input_index < input_normal_data.size(); input_index++) {
				const int64_t output_index = index_remap[input_index];
				CRASH_COND(output_index > output_normals_data.size());
				if (output_index == output_normals_data.size()) {
					output_normals_data.resize(output_index + 1);
					PackedVector4Array cell_normals = input_normal_data[input_index];
					// Remap inner positions if sub-element ordering changed due to deduplication.
					// This can happen at any dimension: e.g. edge reversal changes vertex traversal order,
					// or face deduplication with different edge order changes cell-to-edge order.
					if (pre_dedup_poly.has(key) && input_index < pre_dedup_poly[key].size() && output_index < post_dedup_poly[key].size()) {
						const PackedInt32Array &old_elems = pre_dedup_poly[key][input_index];
						const PackedInt32Array &new_elems = post_dedup_poly[key][output_index];
						if (cell_normals.size() == old_elems.size() && old_elems.size() == new_elems.size()) {
							ERR_CONTINUE_MSG(key.y >= 2 && (key.y - 2) >= poly_cell_index_remaps.size(), "ArrayPolyMesh4D: Invalid normal sub-element dimension " + itos(key.y) + ". Skipping remap.");
							const HashMap<int32_t, int32_t> &subelement_remap = (key.y == 0) ? vertex_index_remap : ((key.y == 1) ? edge_index_remap : poly_cell_index_remaps[key.y - 2]);
							PackedVector4Array remapped_normals;
							remapped_normals.resize(new_elems.size());
							for (int64_t new_pos = 0; new_pos < new_elems.size(); new_pos++) {
								const int32_t new_elem = new_elems[new_pos];
								bool found = false;
								for (int64_t old_pos = 0; old_pos < old_elems.size(); old_pos++) {
									if (subelement_remap[old_elems[old_pos]] == new_elem) {
										remapped_normals.set(new_pos, cell_normals[old_pos]);
										found = true;
										break;
									}
								}
								ERR_FAIL_COND_MSG(!found, vformat("ArrayPolyMesh4D::deduplicate_all_elements: Failed to remap normal for cell %d (new sub-element %d not found in pre-dedup traversal).", input_index, new_elem));
							}
							cell_normals = remapped_normals;
						}
					}
					output_normals_data.set(output_index, cell_normals);
				}
			}
		}
		output_poly_cell_normals.insert(key, output_normals_data);
	}
	// Update poly cell texture map bindings.
	HashMap<Vector2i, Vector<PackedVector3Array>> output_poly_cell_texture_maps;
	for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &kv : _all_poly_cell_texture_maps) {
		const Vector2i key = kv.key;
		ERR_CONTINUE_MSG((key.x - 2) >= poly_cell_index_remaps.size(), "ArrayPolyMesh4D: Invalid texture map binding for geometry dimension " + itos(key.x) + ". Skipping.");
		const HashMap<int32_t, int32_t> &index_remap = (key.x == 0) ? vertex_index_remap : ((key.x == 1) ? edge_index_remap : poly_cell_index_remaps[key.x - 2]);
		const Vector<PackedVector3Array> &input_texture_map_data = kv.value;
		Vector<PackedVector3Array> output_texture_map_data;
		if (key.y == key.x && input_texture_map_data.size() == 1) {
			// Non-decomposed texture maps use a flat structure.
			const PackedVector3Array &input_flat_texture_map = input_texture_map_data[0];
			PackedVector3Array output_flat_texture_map;
			for (int64_t input_index = 0; input_index < input_flat_texture_map.size(); input_index++) {
				const int64_t output_index = index_remap[input_index];
				CRASH_COND(output_index > output_flat_texture_map.size());
				if (output_index == output_flat_texture_map.size()) {
					output_flat_texture_map.resize(output_index + 1);
					output_flat_texture_map.set(output_index, input_flat_texture_map[input_index]);
				}
			}
			output_texture_map_data = { output_flat_texture_map };
		} else {
			for (int64_t input_index = 0; input_index < input_texture_map_data.size(); input_index++) {
				const int64_t output_index = index_remap[input_index];
				CRASH_COND(output_index > output_texture_map_data.size());
				if (output_index == output_texture_map_data.size()) {
					output_texture_map_data.resize(output_index + 1);
					PackedVector3Array cell_texture_map = input_texture_map_data[input_index];
					// Remap inner positions if sub-element ordering changed due to deduplication.
					// This can happen at any dimension: e.g. edge reversal changes vertex traversal order,
					// or face deduplication with different edge order changes cell-to-edge order.
					if (pre_dedup_poly.has(key) && input_index < pre_dedup_poly[key].size() && output_index < post_dedup_poly[key].size()) {
						const PackedInt32Array &old_elems = pre_dedup_poly[key][input_index];
						const PackedInt32Array &new_elems = post_dedup_poly[key][output_index];
						if (cell_texture_map.size() == old_elems.size() && old_elems.size() == new_elems.size()) {
							ERR_CONTINUE_MSG(key.y >= 2 && (key.y - 2) >= poly_cell_index_remaps.size(), "ArrayPolyMesh4D: Invalid texture map sub-element dimension " + itos(key.y) + ". Skipping remap.");
							const HashMap<int32_t, int32_t> &subelement_remap = (key.y == 0) ? vertex_index_remap : ((key.y == 1) ? edge_index_remap : poly_cell_index_remaps[key.y - 2]);
							PackedVector3Array remapped_texture_map;
							remapped_texture_map.resize(new_elems.size());
							for (int64_t new_pos = 0; new_pos < new_elems.size(); new_pos++) {
								const int32_t new_elem = new_elems[new_pos];
								bool found = false;
								for (int64_t old_pos = 0; old_pos < old_elems.size(); old_pos++) {
									if (subelement_remap[old_elems[old_pos]] == new_elem) {
										remapped_texture_map.set(new_pos, cell_texture_map[old_pos]);
										found = true;
										break;
									}
								}
								ERR_FAIL_COND_MSG(!found, vformat("ArrayPolyMesh4D::deduplicate_all_elements: Failed to remap texture map for cell %d (new sub-element %d not found in pre-dedup traversal).", input_index, new_elem));
							}
							cell_texture_map = remapped_texture_map;
						}
					}
					output_texture_map_data.set(output_index, cell_texture_map);
				}
			}
		}
		output_poly_cell_texture_maps.insert(key, output_texture_map_data);
	}
	// Write back the remaining deduplicated data (geometry was already written back earlier).
	_poly_cell_boundary_pivot_overrides = output_poly_cell_boundary_pivot_overrides;
	_seam_face_indices = output_seam_face_indices;
	_all_poly_cell_normals = output_poly_cell_normals;
	_all_poly_cell_texture_maps = output_poly_cell_texture_maps;
	if (output_poly_cell_indices.size() > 1) {
		// Ensure boundary normals stay consistent with the geometry after deduplication,
		// since the reordering can cause them to become flipped. This is done by swapping
		// the first two faces in any cell whose normal was flipped. This may also affect
		// the binding of other decomposed elements, so those need to be resampled as well.
		HashMap<Vector2i, Vector<PackedInt32Array>> remapped_poly_poly;
		for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &kv : output_poly_cell_normals) {
			const Vector2i key = kv.key;
			if (key.x < 3 || key.y >= 3) {
				continue; // Swapping won't affect the binding of these elements, so we can skip remapping them.
			}
			remapped_poly_poly.insert(key, get_all_poly_cell_poly_indices(key.x, key.y));
		}
		for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &kv : output_poly_cell_texture_maps) {
			const Vector2i key = kv.key;
			if (remapped_poly_poly.has(key)) {
				continue; // Already tracked.
			}
			if (key.x < 3 || key.y >= 3) {
				continue; // Swapping won't affect the binding of these elements, so we can skip remapping them.
			}
			remapped_poly_poly.insert(key, get_all_poly_cell_poly_indices(key.x, key.y));
		}
		// Now actually check on the boundary cells and see if their first two elements need swapping.
		const PackedVector4Array &remapped_boundary_normals = output_poly_cell_normals[PER_CELL_KEY][0];
		calculate_boundary_normals();
		const PackedVector4Array &recalculated_boundary_normals = _all_poly_cell_normals[PER_CELL_KEY][0];
		Vector<PackedInt32Array> all_cell_face_indices = output_poly_cell_indices[1];
		for (int64_t cell_index = 0; cell_index < recalculated_boundary_normals.size(); cell_index++) {
			if (remapped_boundary_normals[cell_index].dot(recalculated_boundary_normals[cell_index]) < 0) {
				// Swap the first two faces in the cell to flip the normal.
				PackedInt32Array cell_face_indices = all_cell_face_indices[cell_index];
				CRASH_COND(cell_face_indices.size() < 2);
				const int32_t temp = cell_face_indices[0];
				cell_face_indices.set(0, cell_face_indices[1]);
				cell_face_indices.set(1, temp);
				all_cell_face_indices.set(cell_index, cell_face_indices);
			}
		}
		output_poly_cell_indices.set(1, all_cell_face_indices);
		_poly_cell_indices = output_poly_cell_indices;
		calculate_boundary_normals();
		// Resample the bindings of any decomposed elements that were affected by the swap.
		for (const KeyValue<Vector2i, Vector<PackedInt32Array>> &kv : remapped_poly_poly) {
			const Vector2i key = kv.key;
			const Vector<PackedInt32Array> &remapped_poly = kv.value;
			const Vector<PackedInt32Array> recalculated_poly = get_all_poly_cell_poly_indices(key.x, key.y);
			CRASH_COND(remapped_poly.size() != recalculated_poly.size());
			if (_all_poly_cell_normals.has(key)) {
				Vector<PackedVector4Array> normal_bindings = _all_poly_cell_normals[key];
				CRASH_COND(normal_bindings.size() != remapped_poly.size());
				for (int64_t cell_index = 0; cell_index < remapped_poly.size(); cell_index++) {
					const PackedInt32Array &remapped_cell_poly = remapped_poly[cell_index];
					const PackedInt32Array &recalculated_cell_poly = recalculated_poly[cell_index];
					if (remapped_cell_poly == recalculated_cell_poly) {
						continue; // This cell's poly didn't change, so its binding is still correct.
					}
					CRASH_COND(remapped_cell_poly.size() != recalculated_cell_poly.size());
					// This cell's poly changed, so we need to resample normals.
					const PackedVector4Array &old_cell_normals = normal_bindings[cell_index];
					PackedVector4Array new_cell_normals;
					new_cell_normals.resize(old_cell_normals.size());
					for (int64_t elem_index = 0; elem_index < remapped_cell_poly.size(); elem_index++) {
						const int64_t search_element = remapped_cell_poly[elem_index];
						const int64_t dest_index = recalculated_cell_poly.find(search_element);
						new_cell_normals.set(dest_index, old_cell_normals[elem_index]);
					}
					normal_bindings.set(cell_index, new_cell_normals);
				}
				_all_poly_cell_normals.insert(key, normal_bindings);
			}
			if (_all_poly_cell_texture_maps.has(key)) {
				Vector<PackedVector3Array> texture_map_bindings = _all_poly_cell_texture_maps[key];
				CRASH_COND(texture_map_bindings.size() != remapped_poly.size());
				for (int64_t cell_index = 0; cell_index < remapped_poly.size(); cell_index++) {
					const PackedInt32Array &remapped_cell_poly = remapped_poly[cell_index];
					const PackedInt32Array &recalculated_cell_poly = recalculated_poly[cell_index];
					if (remapped_cell_poly == recalculated_cell_poly) {
						continue; // This cell's poly didn't change, so its binding is still correct.
					}
					CRASH_COND(remapped_cell_poly.size() != recalculated_cell_poly.size());
					// This cell's poly changed, so we need to resample texture maps.
					const PackedVector3Array &old_cell_texture_maps = texture_map_bindings[cell_index];
					PackedVector3Array new_cell_texture_maps;
					new_cell_texture_maps.resize(old_cell_texture_maps.size());
					for (int64_t elem_index = 0; elem_index < remapped_cell_poly.size(); elem_index++) {
						const int64_t search_element = remapped_cell_poly[elem_index];
						const int64_t dest_index = recalculated_cell_poly.find(search_element);
						new_cell_texture_maps.set(dest_index, old_cell_texture_maps[elem_index]);
					}
					texture_map_bindings.set(cell_index, new_cell_texture_maps);
				}
				_all_poly_cell_texture_maps.insert(key, texture_map_bindings);
			}
		}
	}
	poly_mesh_clear_cache(false);
}

void ArrayPolyMesh4D::transform_vertices(const Transform4D &p_transform) {
	const int64_t vertex_count = _poly_cell_vertices.size();
	for (int64_t vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
		_poly_cell_vertices.set(vertex_index, p_transform.xform(_poly_cell_vertices[vertex_index]));
	}
	poly_mesh_clear_cache();
}

void ArrayPolyMesh4D::transform_vertices_bind(const Vector4 &p_offset, const Projection &p_basis) {
	transform_vertices(Transform4D(p_basis, p_offset));
}

void ArrayPolyMesh4D::merge_with(const Ref<PolyMesh4D> &p_other, const Transform4D &p_transform) {
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayPolyMesh4D: This mesh is invalid, cannot merge another mesh into it.");
	ERR_FAIL_COND_MSG(p_other.is_null() || !p_other->is_mesh_data_valid(), "ArrayPolyMesh4D: Cannot merge an invalid PolyMesh4D into this mesh.");
	const Vector<Vector<PackedInt32Array>> &other_poly_cell_indices = p_other->get_poly_cell_indices();
	const PackedVector4Array &other_poly_cell_vertices = p_other->get_poly_cell_vertices();
	const PackedInt32Array &other_poly_cell_boundary_pivot_overrides = p_other->get_poly_cell_boundary_pivot_overrides();
	const PackedInt32Array &other_edge_indices = p_other->get_edge_indices();
	const HashSet<int32_t> &other_seam_face_indices = p_other->get_seam_face_indices();
	const int64_t start_vertex_count = _poly_cell_vertices.size();
	const int64_t start_edge_index_count = _edge_vertex_indices.size();
	const int64_t other_vertex_count = other_poly_cell_vertices.size();
	const int64_t other_edge_index_count = other_edge_indices.size();
	const int64_t end_vertex_count = start_vertex_count + other_vertex_count;
	const int64_t end_edge_index_count = start_edge_index_count + other_edge_index_count;
	// Expand the poly cell indices dimensions if needed and count how many entries are in each dimension.
	const int64_t poly_cell_indices_dims = MAX(_poly_cell_indices.size(), other_poly_cell_indices.size());
	if (poly_cell_indices_dims > _poly_cell_indices.size()) {
		_poly_cell_indices.resize(poly_cell_indices_dims);
	}
	PackedInt32Array start_poly_cell_indices_counts;
	start_poly_cell_indices_counts.resize(poly_cell_indices_dims);
	for (int64_t dim_index = 0; dim_index < poly_cell_indices_dims; dim_index++) {
		start_poly_cell_indices_counts.set(dim_index, _poly_cell_indices[dim_index].size());
	}
	PackedInt32Array other_poly_cell_indices_counts;
	other_poly_cell_indices_counts.resize(poly_cell_indices_dims);
	for (int64_t dim_index = 0; dim_index < poly_cell_indices_dims; dim_index++) {
		if (dim_index < other_poly_cell_indices.size()) {
			other_poly_cell_indices_counts.set(dim_index, other_poly_cell_indices[dim_index].size());
		} else {
			other_poly_cell_indices_counts.set(dim_index, 0);
		}
	}
	// Merge vertices.
	_poly_cell_vertices.resize(end_vertex_count);
	for (int64_t i = 0; i < other_vertex_count; i++) {
		_poly_cell_vertices.set(start_vertex_count + i, p_transform * other_poly_cell_vertices[i]);
	}
	// Merge edges.
	_edge_vertex_indices.resize(end_edge_index_count);
	for (int64_t i = 0; i < other_edge_index_count; i += 2) {
		_edge_vertex_indices.set(start_edge_index_count + i, other_edge_indices[i] + int32_t(start_vertex_count));
		_edge_vertex_indices.set(start_edge_index_count + i + 1, other_edge_indices[i + 1] + int32_t(start_vertex_count));
	}
	// Compute cell vertex instances for the original cells before merging poly cell indices.
	// This must happen before the merge so the result only spans the original cells,
	// which is required later when filling in missing boundary normals.
	Vector<PackedInt32Array> cell_vertex_instances_span_first;
	if (poly_cell_indices_dims > 1) {
		cell_vertex_instances_span_first = _get_vertex_indices_of_boundary_cells(_poly_cell_indices, _edge_vertex_indices, true);
	}
	// Merge poly cell indices.
	for (int64_t dim_index = 0; dim_index < poly_cell_indices_dims; dim_index++) {
		Vector<PackedInt32Array> this_dim = _poly_cell_indices[dim_index];
		const Vector<PackedInt32Array> &other_dim = other_poly_cell_indices[dim_index];
		const int64_t start_count = start_poly_cell_indices_counts[dim_index];
		const int64_t other_count = other_poly_cell_indices_counts[dim_index];
		const int64_t end_count = start_count + other_count;
		const int32_t adjust_items = (dim_index == 0) ? (start_edge_index_count / 2) : start_poly_cell_indices_counts[dim_index - 1];
		this_dim.resize(end_count);
		for (int64_t other_cell_index = 0; other_cell_index < other_count; other_cell_index++) {
			PackedInt32Array other_cell_copy = PackedInt32Array(other_dim[other_cell_index]); // Copy.
			for (int64_t i = 0; i < other_cell_copy.size(); i++) {
				other_cell_copy.set(i, other_cell_copy[i] + adjust_items);
			}
			this_dim.set(start_count + other_cell_index, other_cell_copy);
		}
		_poly_cell_indices.set(dim_index, this_dim);
	}
	// Merge pivot overrides.
	if (!_poly_cell_boundary_pivot_overrides.is_empty() || !other_poly_cell_boundary_pivot_overrides.is_empty()) {
		const int64_t start_required_amount = start_poly_cell_indices_counts[1];
		const int64_t other_required_amount = other_poly_cell_indices_counts[1];
		const int64_t start_current_amount = _poly_cell_boundary_pivot_overrides.size();
		_poly_cell_boundary_pivot_overrides.resize(start_required_amount + other_required_amount);
		for (int64_t i = start_current_amount; i < start_required_amount; i++) {
			_poly_cell_boundary_pivot_overrides.set(i, -1);
		}
		for (int64_t i = 0; i < other_poly_cell_boundary_pivot_overrides.size(); i++) {
			const int32_t new_pivot = other_poly_cell_boundary_pivot_overrides[i] + int32_t(start_vertex_count);
			_poly_cell_boundary_pivot_overrides.set(start_required_amount + i, new_pivot);
		}
		for (int64_t i = start_required_amount + other_poly_cell_boundary_pivot_overrides.size(); i < _poly_cell_boundary_pivot_overrides.size(); i++) {
			_poly_cell_boundary_pivot_overrides.set(i, -1);
		}
	}
	// Merge seams.
	if (poly_cell_indices_dims > 0) {
		const int32_t adjust_seam_face = start_poly_cell_indices_counts[0];
		for (const int32_t seam_face_index : other_seam_face_indices) {
			_seam_face_indices.insert(seam_face_index + adjust_seam_face);
		}
	} else if (!other_seam_face_indices.is_empty()) {
		WARN_PRINT("ArrayPolyMesh4D: Ignoring seam face indices while merging because there is no face dimension in the merged poly cell indices.");
	}
	// Merge all normals and texture maps from the HashMaps.
	// This requires the other mesh be ArrayPolyMesh4D and the cell vertex instances be calculated.
	Ref<ArrayPolyMesh4D> other_array_mesh = p_other;
	if (other_array_mesh.is_null()) {
		other_array_mesh = p_other->to_array_poly_mesh();
	}
	const HashMap<Vector2i, Vector<PackedVector4Array>> &other_poly_cell_normals = other_array_mesh->_all_poly_cell_normals;
	PackedVector4Array boundary_normals_cache;
	// Merge all normals.
	if (!_all_poly_cell_normals.is_empty() || !other_poly_cell_normals.is_empty()) {
		// Merge all normals from other mesh, iterating through all keys.
		for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &other_normals_kv : other_poly_cell_normals) {
			const Vector2i key = other_normals_kv.key;
			const Vector<PackedVector4Array> &other_normals_data = other_normals_kv.value;
			if (other_normals_data.is_empty()) {
				continue;
			}
			// This usage of HashMap's indexing operator writes to the map if missing.
			Vector<PackedVector4Array> &merged_normals = _all_poly_cell_normals[key];
			if (key.y == key.x) {
				if (other_normals_data[0].is_empty()) {
					continue;
				}
				// Non-decomposed normals use a flat structure, which we need to ensure is populated.
				if (merged_normals.is_empty()) {
					merged_normals.append(PackedVector4Array());
				}
			}
			// Figure out how many entries are needed in this dimension based on the key.
			// Use pre-merge counts to check if the original mesh was missing entries.
			int64_t start_cell_count_for_geom_dim = 0;
			if (key.x == 0) {
				start_cell_count_for_geom_dim = start_vertex_count;
			} else if (key.x == 1) {
				start_cell_count_for_geom_dim = start_edge_index_count / 2;
			} else if (key.x > 1 && (key.x - 2) < poly_cell_indices_dims) {
				start_cell_count_for_geom_dim = start_poly_cell_indices_counts[key.x - 2];
			}
			// Use post-merge counts for padding/filling operations.
			int64_t cell_count_for_geom_dim = 0;
			if (key.x == 0) {
				cell_count_for_geom_dim = _poly_cell_vertices.size();
			} else if (key.x == 1) {
				cell_count_for_geom_dim = _edge_vertex_indices.size() / 2;
			} else if (key.x > 1 && (key.x - 2) < poly_cell_indices_dims) {
				cell_count_for_geom_dim = _poly_cell_indices[key.x - 2].size();
			}
			bool missing_entries = false;
			if (key.y == key.x) {
				// Non-decomposed normals use a flat structure, so read the count from the first array.
				missing_entries = merged_normals[0].size() < start_cell_count_for_geom_dim;
			} else {
				// Decomposed normals use a nested structure, so read the count from the outer array.
				missing_entries = merged_normals.size() < start_cell_count_for_geom_dim;
			}
			if (missing_entries) {
				if (poly_cell_indices_dims > 1 && key == PER_CELL_KEY) {
					// Special case: Generate missing boundary normals if needed.
					if (merged_normals[0].size() < start_poly_cell_indices_counts[1]) {
						if (boundary_normals_cache.is_empty()) {
							boundary_normals_cache = _compute_boundary_normals_based_on_cell_orientation(cell_vertex_instances_span_first, true);
							CRASH_COND(boundary_normals_cache.size() != start_poly_cell_indices_counts[1]);
						}
						merged_normals.set(0, boundary_normals_cache);
					}
				} else if (poly_cell_indices_dims > 1 && key == CELL_TO_VERT_KEY) {
					// Special case: Generate missing vertex normals if needed.
					if (merged_normals.size() < start_poly_cell_indices_counts[1]) {
						if (boundary_normals_cache.is_empty()) {
							boundary_normals_cache = _compute_boundary_normals_based_on_cell_orientation(cell_vertex_instances_span_first, true);
							CRASH_COND(boundary_normals_cache.size() != start_poly_cell_indices_counts[1]);
						}
						// Set flat shading normals for all cells without vertex normals.
						const int64_t start_count = merged_normals.size();
						merged_normals.resize(start_poly_cell_indices_counts[1]);
						for (int64_t cell_index = start_count; cell_index < start_poly_cell_indices_counts[1]; cell_index++) {
							PackedVector4Array vertex_normals_for_cell;
							vertex_normals_for_cell.resize(cell_vertex_instances_span_first[cell_index].size());
							const Vector4 cell_boundary_normal = boundary_normals_cache[cell_index];
							for (int64_t vert_inst = 0; vert_inst < vertex_normals_for_cell.size(); vert_inst++) {
								vertex_normals_for_cell.set(vert_inst, cell_boundary_normal);
							}
							merged_normals.set(cell_index, vertex_normals_for_cell);
						}
					}
				} else {
					WARN_PRINT("ArrayPolyMesh4D: The original mesh was missing normal entries for geometry dimension " + itos(key.x) + " and decomposition dimension " + itos(key.y) + ", but the other mesh has entries for this key. Filling missing entries with empty data while merging. Consider updating the original mesh with this data before merging to avoid this warning in the future.");
					if (key.y == key.x) {
						// Fill missing non-decomposed normals with zeroes.
						PackedVector4Array flat_merged_normals;
						flat_merged_normals.resize(cell_count_for_geom_dim);
						merged_normals.set(0, flat_merged_normals);
					} else {
						// Fill missing decomposed normals with empty arrays.
						merged_normals.resize(cell_count_for_geom_dim);
					}
				}
			}
			// Merge normals with transformation if needed, and insert into the map.
			if (key.y == key.x) {
				// Non-decomposed normals use a flat structure, so we need to merge them into one array.
				PackedVector4Array flat_merged_normals = merged_normals[0];
				const PackedVector4Array &other_flat_normals = other_normals_data.size() > 0 ? other_normals_data[0] : PackedVector4Array();
				const int64_t start_count = flat_merged_normals.size();
				flat_merged_normals.resize(start_count + other_flat_normals.size());
				for (int64_t cell_index = 0; cell_index < other_flat_normals.size(); cell_index++) {
					flat_merged_normals.set(start_count + cell_index, p_transform.basis.xform(other_flat_normals[cell_index]));
				}
				merged_normals.set(0, flat_merged_normals);
			} else {
				// Decomposed normals use a nested structure, so we just append the arrays for each cell.
				const int64_t start_count = merged_normals.size();
				merged_normals.resize(start_count + other_normals_data.size());
				for (int64_t cell_index = 0; cell_index < other_normals_data.size(); cell_index++) {
					PackedVector4Array transformed_cell_normals = other_normals_data[cell_index];
					for (int64_t vert_inst = 0; vert_inst < transformed_cell_normals.size(); vert_inst++) {
						transformed_cell_normals.set(vert_inst, p_transform.basis.xform(transformed_cell_normals[vert_inst]));
					}
					merged_normals.set(start_count + cell_index, transformed_cell_normals);
				}
			}
		}
	}
	// Merge all texture maps.
	const HashMap<Vector2i, Vector<PackedVector3Array>> &other_poly_cell_texture_maps = other_array_mesh->_all_poly_cell_texture_maps;
	if (!_all_poly_cell_texture_maps.is_empty() || !other_poly_cell_texture_maps.is_empty()) {
		for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &other_tex_map_kv : other_poly_cell_texture_maps) {
			const Vector2i key = other_tex_map_kv.key;
			const Vector<PackedVector3Array> &other_texture_map_data = other_tex_map_kv.value;
			if (other_texture_map_data.is_empty()) {
				continue;
			}
			// This usage of HashMap's indexing operator writes to the map if missing.
			Vector<PackedVector3Array> &merged_texture_map = _all_poly_cell_texture_maps[key];
			if (key.y == key.x) {
				if (other_texture_map_data[0].is_empty()) {
					continue;
				}
				// Non-decomposed normals use a flat structure, which we need to ensure is populated.
				if (merged_texture_map.is_empty()) {
					merged_texture_map.append(PackedVector3Array());
				}
			}
			// Figure out how many entries are needed in this dimension based on the key.
			// Use pre-merge counts to check if the original mesh was missing entries.
			int64_t start_cell_count_for_geom_dim = 0;
			if (key.x == 0) {
				start_cell_count_for_geom_dim = start_vertex_count;
			} else if (key.x == 1) {
				start_cell_count_for_geom_dim = start_edge_index_count / 2;
			} else if (key.x > 1 && (key.x - 2) < poly_cell_indices_dims) {
				start_cell_count_for_geom_dim = start_poly_cell_indices_counts[key.x - 2];
			}
			// Use post-merge counts for padding/filling operations.
			int64_t cell_count_for_geom_dim = 0;
			if (key.x == 0) {
				cell_count_for_geom_dim = _poly_cell_vertices.size();
			} else if (key.x == 1) {
				cell_count_for_geom_dim = _edge_vertex_indices.size() / 2;
			} else if (key.x > 1 && (key.x - 2) < poly_cell_indices_dims) {
				cell_count_for_geom_dim = _poly_cell_indices[key.x - 2].size();
			}
			bool missing_entries = false;
			if (key.y == key.x) {
				// Non-decomposed texture maps use a flat structure, so read the count from the first array.
				missing_entries = merged_texture_map[0].size() < start_cell_count_for_geom_dim;
			} else {
				// Decomposed texture maps use a nested structure, so read the count from the outer array.
				missing_entries = merged_texture_map.size() < start_cell_count_for_geom_dim;
			}
			if (missing_entries) {
				// For texture maps, we just pad with empty arrays if needed, no generation logic.
				WARN_PRINT("ArrayPolyMesh4D: The original mesh was missing texture map entries for geometry dimension " + itos(key.x) + " and decomposition dimension " + itos(key.y) + ", but the other mesh has entries for this key. Filling missing entries with empty data while merging. Consider updating the original mesh with this data before merging to avoid this warning in the future.");
				if (key.y == key.x) {
					// Fill missing non-decomposed texture maps with zeroes.
					if (merged_texture_map.is_empty()) {
						merged_texture_map.append(PackedVector3Array());
					}
				} else {
					// Fill missing decomposed texture maps with empty arrays.
					merged_texture_map.resize(cell_count_for_geom_dim);
				}
			}
			// Merge texture maps with transformation if needed, and insert into the map.
			if (key.y == key.x) {
				// Non-decomposed texture maps use a flat structure, so we need to merge them into one array.
				PackedVector3Array flat_merged_texture_map = merged_texture_map[0];
				const PackedVector3Array &other_flat_texture_map = other_texture_map_data.size() > 0 ? other_texture_map_data[0] : PackedVector3Array();
				const int64_t start_count = flat_merged_texture_map.size();
				flat_merged_texture_map.resize(start_count + other_flat_texture_map.size());
				for (int64_t cell_index = 0; cell_index < other_flat_texture_map.size(); cell_index++) {
					flat_merged_texture_map.set(start_count + cell_index, other_flat_texture_map[cell_index]);
				}
				merged_texture_map.set(0, flat_merged_texture_map);
			} else {
				// Decomposed texture maps use a nested structure, so we just append the arrays for each cell.
				const int64_t start_count = merged_texture_map.size();
				merged_texture_map.resize(start_count + other_texture_map_data.size());
				for (int64_t cell_index = 0; cell_index < other_texture_map_data.size(); cell_index++) {
					merged_texture_map.set(start_count + cell_index, other_texture_map_data[cell_index]);
				}
			}
		}
	}
	// Merge materials.
	Ref<Material4D> other_material = p_other->get_material();
	if (other_material.is_valid()) {
		Ref<Material4D> self_material = get_material();
		if (self_material.is_valid()) {
			self_material->merge_with(other_material, start_vertex_count, other_vertex_count);
		} else if (other_material->get_albedo_color_array().size() > 0) {
			self_material.instantiate();
			self_material->merge_with(other_material, start_vertex_count, other_vertex_count);
			set_material(self_material);
		} else {
			set_material(other_material);
		}
	}
	reset_poly_mesh_data_validation();
}

void ArrayPolyMesh4D::merge_with_bind(const Ref<PolyMesh4D> &p_other, const Vector4 &p_offset, const Projection &p_basis) {
	merge_with(p_other, Transform4D(p_basis, p_offset));
}

// Getters and setters.

void ArrayPolyMesh4D::set_all_poly_cell_normals(const HashMap<Vector2i, Vector<PackedVector4Array>> &p_all_poly_cell_normals) {
	_all_poly_cell_normals = HashMap<Vector2i, Vector<PackedVector4Array>>(p_all_poly_cell_normals);
	poly_mesh_clear_cache(true);
}

void ArrayPolyMesh4D::set_all_poly_cell_texture_maps(const HashMap<Vector2i, Vector<PackedVector3Array>> &p_all_poly_cell_texture_maps) {
	_all_poly_cell_texture_maps = HashMap<Vector2i, Vector<PackedVector3Array>>(p_all_poly_cell_texture_maps);
	poly_mesh_clear_cache(false);
}

#if GODOT_HAS_TYPED_DICTIONARY
TypedDictionary<Vector2i, Array> ArrayPolyMesh4D::get_all_poly_cell_normals_bind() const {
	TypedDictionary<Vector2i, Array> result;
	for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &kv : _all_poly_cell_normals) {
		const Vector2i &key = kv.key;
		const Vector<PackedVector4Array> &normals_data = kv.value;
		Array normals_array;
		normals_array.resize(normals_data.size());
		for (int64_t i = 0; i < normals_data.size(); i++) {
			normals_array.set(i, normals_data[i]);
		}
		result[key] = normals_array;
	}
	return result;
}

void ArrayPolyMesh4D::set_all_poly_cell_normals_bind(const TypedDictionary<Vector2i, Array> &p_all_poly_cell_normals) {
	HashMap<Vector2i, Vector<PackedVector4Array>> normals_hashmap;
	for (const KeyValue<Variant, Variant> &kv : p_all_poly_cell_normals) {
		const Vector2i key = kv.key;
		const Array normals_array = kv.value;
		Vector<PackedVector4Array> normals_data;
		normals_data.resize(normals_array.size());
		for (int64_t i = 0; i < normals_array.size(); i++) {
			const PackedVector4Array vec4_array = normals_array[i];
			normals_data.set(i, vec4_array);
		}
		normals_hashmap.insert(key, normals_data);
	}
	set_all_poly_cell_normals(normals_hashmap);
}

TypedDictionary<Vector2i, Array> ArrayPolyMesh4D::get_all_poly_cell_texture_maps_bind() const {
	TypedDictionary<Vector2i, Array> result;
	for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &kv : _all_poly_cell_texture_maps) {
		const Vector2i &key = kv.key;
		const Vector<PackedVector3Array> &texture_map_data = kv.value;
		Array texture_map_array;
		texture_map_array.resize(texture_map_data.size());
		for (int64_t i = 0; i < texture_map_data.size(); i++) {
			texture_map_array.set(i, texture_map_data[i]);
		}
		result[key] = texture_map_array;
	}
	return result;
}

void ArrayPolyMesh4D::set_all_poly_cell_texture_maps_bind(const TypedDictionary<Vector2i, Array> &p_all_poly_cell_texture_maps) {
	HashMap<Vector2i, Vector<PackedVector3Array>> texture_maps_hashmap;
	for (const KeyValue<Variant, Variant> &kv : p_all_poly_cell_texture_maps) {
		const Vector2i key = kv.key;
		const Array texture_map_array = kv.value;
		Vector<PackedVector3Array> texture_map_data;
		texture_map_data.resize(texture_map_array.size());
		for (int64_t i = 0; i < texture_map_array.size(); i++) {
			const PackedVector3Array vec3_array = texture_map_array[i];
			texture_map_data.set(i, vec3_array);
		}
		texture_maps_hashmap.insert(key, texture_map_data);
	}
	set_all_poly_cell_texture_maps(texture_maps_hashmap);
}
#endif // GODOT_HAS_TYPED_DICTIONARY

void ArrayPolyMesh4D::set_edge_vertex_indices(const PackedInt32Array &p_edge_indices) {
	_edge_vertex_indices = PackedInt32Array(p_edge_indices);
	poly_mesh_clear_cache();
	reset_poly_mesh_data_validation();
}

void ArrayPolyMesh4D::set_poly_cell_indices(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices) {
	_poly_cell_indices = p_poly_cell_indices;
	poly_mesh_clear_cache();
	reset_poly_mesh_data_validation();
}

#define IS_NOT_INDICES_ARRAY(m_var) (m_var.get_type() != Variant::PACKED_INT32_ARRAY && m_var.get_type() != Variant::ARRAY)

void ArrayPolyMesh4D::set_poly_cell_indices_bind(const TypedArray<Array> &p_poly_cell_indices) {
	Vector<Vector<PackedInt32Array>> indices;
	indices.resize(p_poly_cell_indices.size());
	for (int i = 0; i < p_poly_cell_indices.size(); i++) {
		const Array &dim_array = p_poly_cell_indices[i];
		Vector<PackedInt32Array> dim_vector;
		dim_vector.resize(dim_array.size());
		for (int j = 0; j < dim_array.size(); j++) {
			const Variant &cell_indices_variant = dim_array[j];
			ERR_FAIL_COND_MSG(IS_NOT_INDICES_ARRAY(cell_indices_variant),
					"ArrayPolyMesh4D: Poly cell indices must be an array of polytope dimensions, "
					"containing an array of cells, each of which contains indices. For example, "
					"`[[[0, 1, 2]]]` encodes a single triangle made of edges 0, 1, and 2.");
			const PackedInt32Array cell_indices = cell_indices_variant;
			dim_vector.set(j, cell_indices);
		}
		indices.set(i, dim_vector);
	}
	set_poly_cell_indices(indices);
}

PackedVector4Array ArrayPolyMesh4D::get_poly_cell_boundary_normals() {
	if (!_all_poly_cell_normals.has(PER_CELL_KEY)) {
		return PackedVector4Array();
	}
	return PackedVector4Array(_all_poly_cell_normals[PER_CELL_KEY][0]);
}

void ArrayPolyMesh4D::set_poly_cell_boundary_normals(const PackedVector4Array &p_poly_cell_boundary_normals) {
	if (p_poly_cell_boundary_normals.is_empty()) {
		_all_poly_cell_normals.erase(PER_CELL_KEY);
	} else {
		_all_poly_cell_normals.insert(PER_CELL_KEY, Vector<PackedVector4Array>{ p_poly_cell_boundary_normals });
	}
	poly_mesh_clear_cache(true);
}

PackedInt32Array ArrayPolyMesh4D::get_poly_cell_boundary_pivot_overrides() {
	return _poly_cell_boundary_pivot_overrides;
}

void ArrayPolyMesh4D::set_poly_cell_boundary_pivot_overrides(const PackedInt32Array &p_poly_cell_boundary_pivot_overrides) {
	_poly_cell_boundary_pivot_overrides = p_poly_cell_boundary_pivot_overrides;
	poly_mesh_clear_cache(false);
}

Vector<PackedVector4Array> ArrayPolyMesh4D::get_poly_cell_vertex_normals() {
	if (!_all_poly_cell_normals.has(CELL_TO_VERT_KEY)) {
		return Vector<PackedVector4Array>();
	}
	return Vector<PackedVector4Array>(_all_poly_cell_normals[CELL_TO_VERT_KEY]);
}

void ArrayPolyMesh4D::set_poly_cell_vertex_normals(const Vector<PackedVector4Array> &p_poly_cell_vertex_normals) {
	if (p_poly_cell_vertex_normals.is_empty()) {
		_all_poly_cell_normals.erase(CELL_TO_VERT_KEY);
	} else {
		_all_poly_cell_normals.insert(CELL_TO_VERT_KEY, p_poly_cell_vertex_normals);
	}
	poly_mesh_clear_cache(true);
}

void ArrayPolyMesh4D::set_poly_cell_vertex_normals_bind(const TypedArray<PackedVector4Array> &p_poly_cell_vertex_normals) {
	Vector<PackedVector4Array> normals;
	normals.resize(p_poly_cell_vertex_normals.size());
	for (int i = 0; i < p_poly_cell_vertex_normals.size(); i++) {
		normals.set(i, p_poly_cell_vertex_normals[i]);
	}
	set_poly_cell_vertex_normals(normals);
}

Vector<PackedVector3Array> ArrayPolyMesh4D::get_poly_cell_texture_map() {
	if (!_all_poly_cell_texture_maps.has(CELL_TO_VERT_KEY)) {
		return Vector<PackedVector3Array>();
	}
	return Vector<PackedVector3Array>(_all_poly_cell_texture_maps[CELL_TO_VERT_KEY]);
}

void ArrayPolyMesh4D::set_poly_cell_texture_map(const Vector<PackedVector3Array> &p_poly_cell_texture_map) {
	if (p_poly_cell_texture_map.is_empty()) {
		_all_poly_cell_texture_maps.erase(CELL_TO_VERT_KEY);
	} else {
		_all_poly_cell_texture_maps.insert(CELL_TO_VERT_KEY, p_poly_cell_texture_map);
	}
	poly_mesh_clear_cache(false);
}

void ArrayPolyMesh4D::set_poly_cell_texture_map_bind(const TypedArray<PackedVector3Array> &p_poly_cell_texture_map) {
	Vector<PackedVector3Array> uvw_map;
	uvw_map.resize(p_poly_cell_texture_map.size());
	for (int i = 0; i < p_poly_cell_texture_map.size(); i++) {
		uvw_map.set(i, p_poly_cell_texture_map[i]);
	}
	set_poly_cell_texture_map(uvw_map);
}

PackedInt32Array ArrayPolyMesh4D::get_seam_face_indices_bind() const {
	PackedInt32Array seam_face_indices;
	seam_face_indices.resize(_seam_face_indices.size());
	int64_t seam_index = 0;
	for (const int32_t face_index : _seam_face_indices) {
		seam_face_indices.set(seam_index, face_index);
		seam_index++;
	}
	// The order does not matter, but sorting makes it stable and easier to read and compare.
	seam_face_indices.sort();
	return seam_face_indices;
}

void ArrayPolyMesh4D::set_seam_face_indices(const HashSet<int32_t> &p_seam_face_indices) {
	_seam_face_indices = HashSet<int32_t>(p_seam_face_indices);
	// No need to reset validation, even invalid seams do not affect mesh operations.
}

void ArrayPolyMesh4D::set_seam_face_indices_bind(const PackedInt32Array &p_seam_face_indices) {
	_seam_face_indices.clear();
	for (int64_t i = 0; i < p_seam_face_indices.size(); i++) {
		_seam_face_indices.insert(p_seam_face_indices[i]);
	}
}

PackedVector4Array ArrayPolyMesh4D::get_poly_cell_vertices() {
	return _poly_cell_vertices;
}

void ArrayPolyMesh4D::set_poly_cell_vertices(const PackedVector4Array &p_poly_cell_vertices) {
	ERR_FAIL_COND(p_poly_cell_vertices.size() > MAX_POLY_VERTICES); // Prevent overflow.
	_poly_cell_vertices = p_poly_cell_vertices;
	poly_mesh_clear_cache();
	reset_poly_mesh_data_validation();
}

void ArrayPolyMesh4D::_bind_methods() {
	// Append and delete functions.
	ClassDB::bind_method(D_METHOD("append_edge_points", "point_a", "point_b", "deduplicate"), &ArrayPolyMesh4D::append_edge_points, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_edge_indices", "index_a", "index_b", "deduplicate"), &ArrayPolyMesh4D::append_edge_indices, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_poly_cell", "dimension", "cell", "deduplicate"), &ArrayPolyMesh4D::append_poly_cell, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_vertex", "vertex", "deduplicate_vertices"), &ArrayPolyMesh4D::append_vertex, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_vertices", "vertices", "deduplicate_vertices"), &ArrayPolyMesh4D::append_vertices, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("delete_poly_element", "dimension", "index"), &ArrayPolyMesh4D::delete_poly_element);

	// Normal calculation functions.
	ClassDB::bind_method(D_METHOD("calculate_boundary_normals", "normals_mode", "keep_existing"), &ArrayPolyMesh4D::calculate_boundary_normals, DEFVAL(COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("set_flat_shading_normals", "normals_mode", "recalculate_boundary_normals"), &ArrayPolyMesh4D::set_flat_shading_normals, DEFVAL(COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("set_smooth_shading_normals", "normals_mode", "recalculate_boundary_normals"), &ArrayPolyMesh4D::set_smooth_shading_normals, DEFVAL(COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("make_double_sided", "idempotent"), &ArrayPolyMesh4D::make_double_sided, DEFVAL(true));

	// Texture map and seam functions.
	ClassDB::bind_method(D_METHOD("calculate_seam_faces", "angle_threshold_radians", "discard_seams_within_islands"), &ArrayPolyMesh4D::calculate_seam_faces, DEFVAL(Math_TAU / 8.0), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("collect_cells_in_island", "start_cell"), &ArrayPolyMesh4D::collect_cells_in_island);
	ClassDB::bind_method(D_METHOD("unwrap_texture_map_island", "cells_in_island", "keep_existing"), &ArrayPolyMesh4D::unwrap_texture_map_island, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("unwrap_texture_map", "mode", "padding", "proportional", "keep_existing"), &ArrayPolyMesh4D::unwrap_texture_map, DEFVAL(UNWRAP_MODE_TILE_ISLANDS), DEFVAL(0.0), DEFVAL(true), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("transform_texture_map", "transform"), &ArrayPolyMesh4D::transform_texture_map);

	// Misc functions.
	ClassDB::bind_method(D_METHOD("deduplicate_all_elements"), &ArrayPolyMesh4D::deduplicate_all_elements);
	ClassDB::bind_method(D_METHOD("transform_vertices", "offset", "basis"), &ArrayPolyMesh4D::transform_vertices_bind, DEFVAL(Projection()));
	ClassDB::bind_method(D_METHOD("merge_with", "other", "offset", "basis"), &ArrayPolyMesh4D::merge_with_bind, DEFVAL(Vector4()), DEFVAL(Projection()));
	ClassDB::bind_method(D_METHOD("make_single_volume_from_all_cells"), &ArrayPolyMesh4D::make_single_volume_from_all_cells);

	// Properties. Only bind the setters here because the getters are already bound in PolyMesh4D.
	ClassDB::bind_method(D_METHOD("set_poly_cell_indices", "poly_cell_indices"), &ArrayPolyMesh4D::set_poly_cell_indices_bind);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "poly_cell_indices"), "set_poly_cell_indices", "get_poly_cell_indices");

	ClassDB::bind_method(D_METHOD("set_poly_cell_vertices", "poly_cell_vertices"), &ArrayPolyMesh4D::set_poly_cell_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "poly_cell_vertices"), "set_poly_cell_vertices", "get_poly_cell_vertices");

	ClassDB::bind_method(D_METHOD("set_poly_cell_boundary_pivot_overrides", "poly_cell_boundary_pivot_overrides"), &ArrayPolyMesh4D::set_poly_cell_boundary_pivot_overrides);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "poly_cell_boundary_pivot_overrides"), "set_poly_cell_boundary_pivot_overrides", "get_poly_cell_boundary_pivot_overrides");

	ClassDB::bind_method(D_METHOD("get_seam_face_indices"), &ArrayPolyMesh4D::get_seam_face_indices_bind);
	ClassDB::bind_method(D_METHOD("set_seam_face_indices", "seam_face_indices"), &ArrayPolyMesh4D::set_seam_face_indices_bind);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "seam_face_indices"), "set_seam_face_indices", "get_seam_face_indices");

	ClassDB::bind_method(D_METHOD("set_edge_indices", "edge_indices"), &ArrayPolyMesh4D::set_edge_vertex_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "edge_indices"), "set_edge_indices", "get_edge_indices");

	// Normals and texture maps. The "all" ones need the getters bound here.
#if GODOT_HAS_TYPED_DICTIONARY
	ClassDB::bind_method(D_METHOD("get_all_poly_cell_normals"), &ArrayPolyMesh4D::get_all_poly_cell_normals_bind);
	ClassDB::bind_method(D_METHOD("set_all_poly_cell_normals", "all_poly_cell_normals"), &ArrayPolyMesh4D::set_all_poly_cell_normals_bind);
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "all_poly_cell_normals", PROPERTY_HINT_TYPE_STRING, "Vector2i:Array"), "set_all_poly_cell_normals", "get_all_poly_cell_normals");

	ClassDB::bind_method(D_METHOD("get_all_poly_cell_texture_maps"), &ArrayPolyMesh4D::get_all_poly_cell_texture_maps_bind);
	ClassDB::bind_method(D_METHOD("set_all_poly_cell_texture_maps", "all_poly_cell_texture_maps"), &ArrayPolyMesh4D::set_all_poly_cell_texture_maps_bind);
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "all_poly_cell_texture_maps", PROPERTY_HINT_TYPE_STRING, "Vector2i:Array"), "set_all_poly_cell_texture_maps", "get_all_poly_cell_texture_maps");

	constexpr PropertyUsageFlags PROPERTY_USAGE_HIGH_LEVEL_NORMALS_TEXMAPS = PROPERTY_USAGE_EDITOR;
#else
	// When not saving the "all" ones, save the high-level properties (default = storage + editor).
	constexpr PropertyUsageFlags PROPERTY_USAGE_HIGH_LEVEL_NORMALS_TEXMAPS = PROPERTY_USAGE_DEFAULT;
#endif // GODOT_HAS_TYPED_DICTIONARY

	ClassDB::bind_method(D_METHOD("set_poly_cell_boundary_normals", "poly_cell_boundary_normals"), &ArrayPolyMesh4D::set_poly_cell_boundary_normals);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "poly_cell_boundary_normals", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_HIGH_LEVEL_NORMALS_TEXMAPS), "set_poly_cell_boundary_normals", "get_poly_cell_boundary_normals");

	ClassDB::bind_method(D_METHOD("set_poly_cell_vertex_normals", "poly_cell_vertex_normals"), &ArrayPolyMesh4D::set_poly_cell_vertex_normals_bind);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "poly_cell_vertex_normals", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_HIGH_LEVEL_NORMALS_TEXMAPS), "set_poly_cell_vertex_normals", "get_poly_cell_vertex_normals");

	ClassDB::bind_method(D_METHOD("set_poly_cell_texture_map", "poly_cell_texture_map"), &ArrayPolyMesh4D::set_poly_cell_texture_map_bind);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "poly_cell_texture_map", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_HIGH_LEVEL_NORMALS_TEXMAPS), "set_poly_cell_texture_map", "get_poly_cell_texture_map");

	// Enums.
	BIND_ENUM_CONSTANT(COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
	BIND_ENUM_CONSTANT(COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION);
	BIND_ENUM_CONSTANT(COMPUTE_NORMALS_MODE_FORCE_OUTWARD_OVERRIDE_CELL_ORIENTATION);

	BIND_ENUM_CONSTANT(UNWRAP_MODE_AUTOMATIC);
	BIND_ENUM_CONSTANT(UNWRAP_MODE_EACH_CELL_FILLS);
	BIND_ENUM_CONSTANT(UNWRAP_MODE_TILE_CELLS);
	BIND_ENUM_CONSTANT(UNWRAP_MODE_EACH_ISLAND_FILLS);
	BIND_ENUM_CONSTANT(UNWRAP_MODE_TILE_ISLANDS);
}
