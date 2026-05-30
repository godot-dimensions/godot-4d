#include "poly_mesh_builder_4d.h"

#include "../../../math/math_4d.h"

Ref<ArrayPolyMesh4D> PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(const Ref<ArrayMesh> &p_mesh_3d, const int p_which_surface, const bool p_deduplicate) {
	Ref<ArrayPolyMesh4D> ret;
	ret.instantiate();
	ERR_FAIL_COND_V_MSG(p_mesh_3d.is_null(), ret, "Input mesh is null.");
	const int surface_count = p_mesh_3d->get_surface_count();
	ERR_FAIL_COND_V_MSG(surface_count == 0, ret, "Input mesh has no surfaces.");
	if (p_which_surface != -1) {
		ERR_FAIL_INDEX_V_MSG(p_which_surface, surface_count, ret, "Invalid surface index.");
	}
	int start_surface = p_which_surface == -1 ? 0 : p_which_surface;
	int end_surface = p_which_surface == -1 ? surface_count : p_which_surface + 1;
	PackedVector4Array output_vertices;
	PackedVector4Array output_face_boundary_normals;
	Vector<PackedVector4Array> output_face_vertex_normals;
	Vector<PackedVector3Array> output_face_texture_maps;
	Vector<PackedInt32Array> output_face_indices;
	for (int surface_index = start_surface; surface_index < end_surface; surface_index++) {
		const Array surface_arrays = p_mesh_3d->surface_get_arrays(surface_index);
		CRASH_COND(surface_arrays.size() < Mesh::ARRAY_MAX); // ArrayMesh should always return surfaces arrays of length Mesh::ARRAY_MAX, even if some of them are empty.
		const PackedVector3Array surface_vertices = PackedVector3Array(surface_arrays[Mesh::ARRAY_VERTEX]);
		const PackedVector3Array surface_normals = PackedVector3Array(surface_arrays[Mesh::ARRAY_NORMAL]);
		const PackedVector2Array surface_uvs = PackedVector2Array(surface_arrays[Mesh::ARRAY_TEX_UV]);
		ERR_FAIL_COND_V_MSG(surface_normals.size() > 0 && surface_normals.size() != surface_vertices.size(), ret, "Surface normals array size does not match vertices array size.");
		ERR_FAIL_COND_V_MSG(surface_uvs.size() > 0 && surface_uvs.size() != surface_vertices.size(), ret, "Surface texture map UVs array size does not match vertices array size.");
		PackedInt32Array surface_indices = surface_arrays.size() > Mesh::ARRAY_INDEX ? PackedInt32Array(surface_arrays[Mesh::ARRAY_INDEX]) : PackedInt32Array();
		bool is_indexed = surface_indices.size() > 0;
		if (!is_indexed) {
			// Standardize everything to indexed format for easier processing.
			surface_indices.resize(surface_vertices.size());
			for (int32_t i = 0; i < (int32_t)surface_indices.size(); i++) {
				surface_indices.set(i, i);
			}
		}
		// Append vertices, deduplicating with existing vertices along the way.
		PackedInt32Array surface_verts_to_inserted;
		surface_verts_to_inserted.resize(surface_vertices.size());
		for (int64_t vertex_index = 0; vertex_index < surface_vertices.size(); vertex_index++) {
			const Vector4 vert_4d = Vector4D::from_3d(surface_vertices[vertex_index]);
			if (p_deduplicate) {
				const int existing_index = (int)output_vertices.find(vert_4d);
				if (existing_index != -1) {
					surface_verts_to_inserted.set(vertex_index, existing_index);
					continue;
				}
			}
			surface_verts_to_inserted.set(vertex_index, (int32_t)output_vertices.size());
			output_vertices.append(vert_4d);
		}
		// Read the triangle vertex indices from the index array.
		ERR_FAIL_COND_V_MSG(surface_indices.size() % 3 != 0, ret, "Indexed surface index count is not a multiple of 3, so it cannot be converted to faces.");
		for (int64_t vertex_index = 0; vertex_index < surface_indices.size(); vertex_index += 3) {
			const int32_t orig_v0 = surface_indices[vertex_index + 0];
			const int32_t orig_v1 = surface_indices[vertex_index + 2];
			const int32_t orig_v2 = surface_indices[vertex_index + 1];
			// Always deduplicate edges for indexed 3D mesh inputs.
			// Indexed 3D meshes have control over edge merging via the vertex indexing already.
			const bool deduplicate_edges = is_indexed || p_deduplicate;
			const int32_t e0 = ret->append_edge_indices(surface_verts_to_inserted[orig_v0], surface_verts_to_inserted[orig_v1], deduplicate_edges);
			const int32_t e1 = ret->append_edge_indices(surface_verts_to_inserted[orig_v1], surface_verts_to_inserted[orig_v2], deduplicate_edges);
			const int32_t e2 = ret->append_edge_indices(surface_verts_to_inserted[orig_v0], surface_verts_to_inserted[orig_v2], deduplicate_edges);
			output_face_indices.append(PackedInt32Array{ e0, e1, e2 });
			// Append face boundary normal for this face.
			const Vector3 face_a = surface_vertices[orig_v1] - surface_vertices[orig_v0];
			const Vector3 face_b = surface_vertices[orig_v2] - surface_vertices[orig_v0];
			const Vector3 face_boundary_normal = face_a.cross(face_b).normalized();
			output_face_boundary_normals.append(Vector4D::from_3d(face_boundary_normal));
			// Append face vertex normals for this face if they exist.
			if (!surface_normals.is_empty()) {
				PackedVector4Array face_vertex_normals = {
					Vector4D::from_3d(surface_normals[orig_v0]),
					Vector4D::from_3d(surface_normals[orig_v1]),
					Vector4D::from_3d(surface_normals[orig_v2]),
				};
				output_face_vertex_normals.append(face_vertex_normals);
			}
			// Append face texture maps if they exist.
			if (!surface_uvs.is_empty()) {
				PackedVector3Array face_texture_maps = {
					Vector3(surface_uvs[orig_v0].x, surface_uvs[orig_v0].y, 0.0f),
					Vector3(surface_uvs[orig_v1].x, surface_uvs[orig_v1].y, 0.0f),
					Vector3(surface_uvs[orig_v2].x, surface_uvs[orig_v2].y, 0.0f),
				};
				output_face_texture_maps.append(face_texture_maps);
			}
		}
	}
	ret->set_poly_cell_vertices(output_vertices);
	ret->set_poly_cell_indices(Vector<Vector<PackedInt32Array>>{ output_face_indices });
	if (!output_face_boundary_normals.is_empty() || !output_face_vertex_normals.is_empty()) {
		HashMap<Vector2i, Vector<PackedVector4Array>> all_poly_cell_normals;
		if (!output_face_boundary_normals.is_empty()) {
			all_poly_cell_normals.insert(PolyMesh4D::PER_FACE_KEY, Vector<PackedVector4Array>{ output_face_boundary_normals });
		}
		if (!output_face_vertex_normals.is_empty()) {
			all_poly_cell_normals.insert(PolyMesh4D::FACE_TO_VERT_KEY, output_face_vertex_normals);
		}
		ret->set_all_poly_cell_normals(all_poly_cell_normals);
	}
	if (!output_face_texture_maps.is_empty()) {
		HashMap<Vector2i, Vector<PackedVector3Array>> all_poly_cell_texture_maps;
		all_poly_cell_texture_maps.insert(PolyMesh4D::FACE_TO_VERT_KEY, output_face_texture_maps);
		ret->set_all_poly_cell_texture_maps(all_poly_cell_texture_maps);
	}
	return ret;
}

Ref<ArrayPolyMesh4D> PolyMeshBuilder4D::extrude_linear(const Ref<ArrayPolyMesh4D> &p_input_mesh, const Vector4 &p_extrusion_vector) {
	Ref<ArrayPolyMesh4D> ret;
	ret.instantiate();
	ERR_FAIL_COND_V_MSG(p_input_mesh.is_null() || !p_input_mesh->is_mesh_data_valid(), ret, "Input mesh is not valid, so extrusion cannot be performed.");
	// Extract and copy a bunch of data from the input mesh.
	// Start by copying the input mesh's data into the output mesh twice,
	// offset by the extrusion vector in both negative and positive directions.
	ret = p_input_mesh->duplicate();
	ret->transform_vertices(Transform4D(Basis4D(), -p_extrusion_vector));
	ret->merge_with(p_input_mesh, Transform4D(Basis4D(), p_extrusion_vector));
	// The two copies aren't connected yet, so it's safe to blindly force their normals outward.
	ret->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION);
	// Mark all existing faces as seams since they will become sharp borders (usually 90 degrees).
	Vector<Vector<PackedInt32Array>> poly_cell_indices = ret->get_poly_cell_indices();
	if (poly_cell_indices.size() > 0) {
		// Use the faces retrieved from the output mesh since this includes the copying.
		const Vector<PackedInt32Array> &faces = poly_cell_indices[0];
		HashSet<int32_t> seam_face_indices;
		for (int face_index = 0; face_index < faces.size(); face_index++) {
			seam_face_indices.insert(face_index);
		}
		ret->set_seam_face_indices(seam_face_indices);
	}
	// Now connect everything. Work with the raw data to avoid costly safety checks
	// present inside of the ArrayPolyMesh4D's functions. All of this should be valid
	// as long as the input mesh is valid, which is checked at the start of this function.
	// Form new edges between the vertices of the two copies of the input mesh.
	const PackedInt32Array &input_edge_indices = p_input_mesh->get_edge_indices();
	const PackedVector4Array &input_vertices = p_input_mesh->get_poly_cell_vertices();
	const int32_t input_edge_count = (int32_t)(input_edge_indices.size() / 2);
	const int32_t input_vertex_count = (int32_t)input_vertices.size();
	PackedInt32Array edge_indices = ret->get_edge_indices();
	PackedInt32Array vertex_to_extruded_edge;
	vertex_to_extruded_edge.resize(input_vertex_count);
	for (int input_vertex_index = 0; input_vertex_index < input_vertex_count; input_vertex_index++) {
		vertex_to_extruded_edge.set(input_vertex_index, edge_indices.size() / 2);
		edge_indices.append(input_vertex_index);
		edge_indices.append(input_vertex_index + input_vertex_count);
	}
	ret->set_edge_vertex_indices(edge_indices);
	// Form new faces between the edges of the two copies of the input mesh.
	PackedInt32Array edge_to_extruded_face;
	edge_to_extruded_face.resize(input_edge_count);
	const Vector<Vector<PackedInt32Array>> &input_poly_cell_indices = p_input_mesh->get_poly_cell_indices();
	if (input_poly_cell_indices.size() > 0) {
		Vector<PackedInt32Array> face_indices = poly_cell_indices[0];
		for (int32_t input_edge_index = 0; input_edge_index < input_edge_count; input_edge_index++) {
			const int32_t vertex_index_a = input_edge_indices[input_edge_index * 2];
			const int32_t vertex_index_b = input_edge_indices[input_edge_index * 2 + 1];
			// Create a directed loop of edges:
			const PackedInt32Array new_face = {
				input_edge_index, // First copy.
				vertex_to_extruded_edge[vertex_index_a], // Extruded from vertex A.
				input_edge_index + input_edge_count, // Second copy.
				vertex_to_extruded_edge[vertex_index_b], // Extruded from vertex B.
			};
			const int32_t new_face_index = face_indices.size();
			face_indices.append(new_face);
			edge_to_extruded_face.set(input_edge_index, new_face_index);
		}
		poly_cell_indices.set(0, face_indices);
		// 0: Edges to extruded faces. 1: Faces to extruded cells. 2: Cells to extruded volumes. And so on.
		Vector<PackedInt32Array> all_cell_to_extruded_cell;
		all_cell_to_extruded_cell.append(edge_to_extruded_face);
		// Form new cells between the faces of the two copies of the input mesh, and so on.
		for (int input_poly_index = 0; input_poly_index < input_poly_cell_indices.size(); input_poly_index++) {
			// This code operates between dimensions, so "prev" and "next" are adjacent dimensions.
			// Also, in terms of dimensional index, "prev" equals "input", and "next" equals "output".
			const Vector<PackedInt32Array> &input_cells = input_poly_cell_indices[input_poly_index];
			const PackedInt32Array &prev_dim_cell_to_extruded_cell = all_cell_to_extruded_cell[input_poly_index];
			const int32_t next_dim_poly_index = input_poly_index + 1;
			if (next_dim_poly_index >= poly_cell_indices.size()) {
				// This may only happen once per call to `extrude_linear`.
				poly_cell_indices.resize_initialized(next_dim_poly_index + 1);
			}
			Vector<PackedInt32Array> next_dim_cell_indices = poly_cell_indices[next_dim_poly_index];
			PackedInt32Array next_dim_cell_to_extruded_cell;
			for (int cell_index = 0; cell_index < input_cells.size(); cell_index++) {
				const PackedInt32Array cell_to_lower_dim_indices = input_cells[cell_index];
				PackedInt32Array new_cell = { cell_index }; // First copy.
				for (int elem_index = 0; elem_index < cell_to_lower_dim_indices.size(); elem_index++) {
					// Extruded from lower dimension element. For example, when extruding from faces
					// to cells, prev_dim_cell_to_extruded_cell holds the mapping from edges to extruded
					// faces, and cell_to_lower_dim_indices holds the edges that make up this face,
					// therefore this lets us get the faces created from extruding the edges of this face,
					// which are the sideways faces needed to make the new cell.
					new_cell.append(prev_dim_cell_to_extruded_cell[cell_to_lower_dim_indices[elem_index]]);
				}
				new_cell.append(cell_index + input_cells.size()); // Second copy.
				const int32_t new_cell_index = next_dim_cell_indices.size();
				next_dim_cell_indices.append(new_cell);
				next_dim_cell_to_extruded_cell.append(new_cell_index);
			}
			poly_cell_indices.set(next_dim_poly_index, next_dim_cell_indices);
			all_cell_to_extruded_cell.append(next_dim_cell_to_extruded_cell);
		}
		ret->set_poly_cell_indices(poly_cell_indices);
		// Figure out the boundary normals for the extruded faces. Start by calculating them as-is.
		// Then, depending on the input mesh's data, these will be rectified in some way.
		ret->set_poly_cell_boundary_normals(PackedVector4Array());
		ret->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
		CRASH_COND(!ret->is_mesh_data_valid());
		const PackedInt32Array &face_to_extruded_cell = all_cell_to_extruded_cell[1];
		// Copy over the normals from the original 2D faces, if that data is present.
		HashMap<Vector2i, Vector<PackedVector4Array>> all_poly_cell_normals = ret->get_all_poly_cell_normals();
		if (all_poly_cell_normals.has(PolyMesh4D::PER_FACE_KEY) && all_poly_cell_normals[PolyMesh4D::PER_FACE_KEY].size() == 1) {
			const PackedVector4Array &per_face_normals = all_poly_cell_normals[PolyMesh4D::PER_FACE_KEY][0];
			CRASH_COND(per_face_normals.size() < face_to_extruded_cell.size());
			Vector<PackedInt32Array> boundary_cells = poly_cell_indices[1];
			PackedVector4Array per_cell_normals = ret->get_poly_cell_boundary_normals();
			for (int32_t face_index = 0; face_index < face_to_extruded_cell.size(); face_index++) {
				const Vector4 face_normal = per_face_normals[face_index];
				const int32_t extruded_cell_index = face_to_extruded_cell[face_index];
				const Vector4 cell_normal = per_cell_normals[extruded_cell_index];
				// The exact boundary cell normal needs to depend on the orientation of the cell,
				// but we can flip it the other way if it's backwards compared to the face normal.
				if (face_normal.dot(cell_normal) < 0.0) {
					per_cell_normals.set(extruded_cell_index, -cell_normal);
					// Also swap the cell's first two elements to flip its orientation.
					PackedInt32Array boundary_cell = boundary_cells[extruded_cell_index];
					int32_t temp = boundary_cell[0];
					boundary_cell.set(0, boundary_cell[1]);
					boundary_cell.set(1, temp);
					boundary_cells.set(extruded_cell_index, boundary_cell);
				}
			}
			// Write the corrected boundary cell data.
			poly_cell_indices.set(1, boundary_cells);
			ret->set_poly_cell_indices(poly_cell_indices);
			// The boundary normals themselves will be recalculated at the end of this function,
			// but write the result back anyway for internal consistency.
			all_poly_cell_normals.insert(PolyMesh4D::PER_CELL_KEY, Vector<PackedVector4Array>{ per_cell_normals });
			ret->set_all_poly_cell_normals(all_poly_cell_normals);
		} else {
			// Otherwise, if there is no data to copy over, calculate new boundary normals for the extruded faces.
			// 4D-specific code: Ensure boundary cells are correctly oriented with outward facing normals.
			// This only works when we have 4D volumes (poly index 2) to indicate which side of a boundary
			// cell is the inside vs the outside, which came from the input mesh's 3D cells.
			if (poly_cell_indices.size() > 2) {
				const PackedVector4Array &output_vertices = ret->get_poly_cell_vertices();
				const Vector<PackedInt32Array> volume_vert = ret->get_all_poly_cell_vertex_indices(4, false);
				const Vector<PackedInt32Array> volume_cells = poly_cell_indices[2];
				const Vector<PackedInt32Array> boundary_vert = ret->get_all_poly_cell_vertex_indices(3, false);
				const PackedVector4Array boundary_normals = ret->get_poly_cell_boundary_normals();
				Vector<PackedInt32Array> boundary_cells = poly_cell_indices[1];
				CRASH_COND(volume_vert.size() != volume_cells.size() || boundary_vert.size() != boundary_cells.size() || boundary_normals.size() != boundary_cells.size());
				// Compute the average position of each boundary cell.
				PackedVector4Array boundary_average_pos;
				boundary_average_pos.resize(boundary_cells.size());
				for (int64_t boundary_cell_index = 0; boundary_cell_index < boundary_cells.size(); boundary_cell_index++) {
					const PackedInt32Array &boundary_cell_vert_indices = boundary_vert[boundary_cell_index];
					Vector4 average_pos = Vector4(0.0, 0.0, 0.0, 0.0);
					for (int64_t i = 0; i < boundary_cell_vert_indices.size(); i++) {
						average_pos += output_vertices[boundary_cell_vert_indices[i]];
					}
					average_pos /= boundary_cell_vert_indices.size();
					boundary_average_pos.set(boundary_cell_index, average_pos);
				}
				// Iterate over each volume.
				for (int64_t volume_index = 0; volume_index < volume_cells.size(); volume_index++) {
					// Compute the average position of this volume.
					const PackedInt32Array &volume_cell_vert_indices = volume_vert[volume_index];
					Vector4 volume_average_pos = Vector4(0.0, 0.0, 0.0, 0.0);
					for (int64_t i = 0; i < volume_cell_vert_indices.size(); i++) {
						volume_average_pos += output_vertices[volume_cell_vert_indices[i]];
					}
					volume_average_pos /= volume_cell_vert_indices.size();
					// Ensure each boundary cell of this volume is oriented with its normal facing outward from the volume.
					const PackedInt32Array &volume_cell_boundary_indices = volume_cells[volume_index];
					for (int64_t i = 0; i < volume_cell_boundary_indices.size(); i++) {
						const int64_t boundary_cell_index = volume_cell_boundary_indices[i];
						const Vector4 out = boundary_average_pos[boundary_cell_index] - volume_average_pos;
						if (boundary_normals[boundary_cell_index].dot(out) < 0.0) {
							// This boundary cell is facing inward, so swap the first two faces to flip the normal to face outward.
							PackedInt32Array boundary_cell = boundary_cells[boundary_cell_index];
							int32_t temp = boundary_cell[0];
							boundary_cell.set(0, boundary_cell[1]);
							boundary_cell.set(1, temp);
							boundary_cells.set(boundary_cell_index, boundary_cell);
						}
					}
				}
				poly_cell_indices.set(1, boundary_cells);
				ret->set_poly_cell_indices(poly_cell_indices);
			}
			// The new boundary cells created from the extrusion may have inconsistent normal directions.
			// Fix them, using the two copies of the input mesh's boundary cells as authoritative references.
			if (input_poly_cell_indices.size() > 1) {
				const int32_t input_boundary_cell_count = (int32_t)input_poly_cell_indices[1].size();
				if (input_boundary_cell_count > 0) {
					PackedInt32Array authoritative_boundary_cells;
					authoritative_boundary_cells.resize(input_boundary_cell_count * 2);
					for (int32_t i = 0; i < input_boundary_cell_count * 2; i++) {
						authoritative_boundary_cells.set(i, i);
					}
					make_boundary_normals_topologically_consistent(ret, authoritative_boundary_cells);
				}
			}
		}
		// Copy over the vertex normals from the original 2D faces, if that data is present.
		// Unlike per-face normals, there is no need to rectify or generate fallback data when missing.
		if (all_poly_cell_normals.has(PolyMesh4D::FACE_TO_VERT_KEY)) {
			Vector<PackedVector4Array> face_to_vert_normals = all_poly_cell_normals[PolyMesh4D::FACE_TO_VERT_KEY];
			// This data currently only contains one copy of the input face vertex normals, copy it again.
			const Vector<PackedInt32Array> &input_faces = input_poly_cell_indices[0];
			const int64_t input_face_count = input_faces.size();
			face_to_vert_normals.resize(input_face_count); // Just in case the original size was smaller due to missing data. Empty entries are fine.
			face_to_vert_normals.append_array(face_to_vert_normals); // New size will be 2x the input face count.
			all_poly_cell_normals.insert(PolyMesh4D::FACE_TO_VERT_KEY, face_to_vert_normals);
			// Now transfer the face vertex normals to the extruded cell vertex normals.
			const PackedVector4Array &per_cell_normals = ret->get_poly_cell_boundary_normals();
			const Vector<PackedInt32Array> all_face_vert = ret->get_all_poly_cell_vertex_indices(2, false);
			const Vector<PackedInt32Array> all_cell_vert = ret->get_all_poly_cell_vertex_indices(3, false);
			Vector<PackedVector4Array> cell_to_vert_normals = all_poly_cell_normals.has(PolyMesh4D::CELL_TO_VERT_KEY) ? all_poly_cell_normals[PolyMesh4D::CELL_TO_VERT_KEY] : Vector<PackedVector4Array>();
			cell_to_vert_normals.resize(all_cell_vert.size());
			for (int face_index = 0; face_index < input_face_count; face_index++) {
				const PackedVector4Array &face_vert_normals = face_to_vert_normals[face_index];
				const PackedInt32Array &first_copy_face_vert = all_face_vert[face_index];
				// The second copy being offset by input_face_count is guaranteed because we start with `merge_with`.
				const PackedInt32Array &second_copy_face_vert = all_face_vert[face_index + input_face_count];
				const int64_t cell_index = face_to_extruded_cell[face_index];
				const PackedInt32Array &this_cell_vert = all_cell_vert[cell_index];
				PackedVector4Array cell_vert_normals;
				cell_vert_normals.resize(this_cell_vert.size());
				for (int64_t vert_in_cell = 0; vert_in_cell < this_cell_vert.size(); vert_in_cell++) {
					const int32_t vert_index = this_cell_vert[vert_in_cell];
					const int64_t vert_in_first_copy = first_copy_face_vert.find(vert_index);
					const int64_t vert_in_second_copy = second_copy_face_vert.find(vert_index);
					if (vert_in_first_copy != -1 && vert_in_first_copy < face_vert_normals.size()) {
						cell_vert_normals.set(vert_in_cell, face_vert_normals[vert_in_first_copy]);
					} else if (vert_in_second_copy != -1 && vert_in_second_copy < face_vert_normals.size()) {
						cell_vert_normals.set(vert_in_cell, face_vert_normals[vert_in_second_copy]);
					} else {
						// Neither face has vertex normal data for this vertex in this cell, so just use the cell's boundary normal.
						cell_vert_normals.set(vert_in_cell, per_cell_normals[cell_index]);
					}
				}
				cell_to_vert_normals.set(cell_index, cell_vert_normals);
			}
			all_poly_cell_normals.insert(PolyMesh4D::CELL_TO_VERT_KEY, cell_to_vert_normals);
			ret->set_all_poly_cell_normals(all_poly_cell_normals);
		}
		// Copy over the vertex texture maps from the original 2D faces, if that data is present.
		HashMap<Vector2i, Vector<PackedVector3Array>> all_poly_cell_texture_maps = ret->get_all_poly_cell_texture_maps();
		if (all_poly_cell_texture_maps.has(PolyMesh4D::FACE_TO_VERT_KEY)) {
			Vector<PackedVector3Array> face_to_vert_texture_maps = all_poly_cell_texture_maps[PolyMesh4D::FACE_TO_VERT_KEY];
			// This data currently only contains one copy of the input face vertex texture maps, copy it again.
			const Vector<PackedInt32Array> &input_faces = input_poly_cell_indices[0];
			const int64_t input_face_count = input_faces.size();
			// Special case for texture maps: The second copy should be offset in the Z direction.
			face_to_vert_texture_maps.resize(input_face_count * 2);
			for (int64_t face_index = 0; face_index < input_face_count; face_index++) {
				PackedVector3Array face_vert_texture_map = face_to_vert_texture_maps[face_index];
				for (int64_t vert_index = 0; vert_index < face_vert_texture_map.size(); vert_index++) {
					Vector3 tex_coord = face_vert_texture_map[vert_index];
					tex_coord.z += 1.0f; // Setting equal to 1.0f would also work but this handles the case of existing non-zero Z values in the input texture maps.
					face_vert_texture_map.set(vert_index, tex_coord);
				}
				face_to_vert_texture_maps.set(face_index + input_face_count, face_vert_texture_map);
			}
			all_poly_cell_texture_maps.insert(PolyMesh4D::FACE_TO_VERT_KEY, face_to_vert_texture_maps);
			// Now transfer the face vertex texture maps to the extruded cell vertex texture maps.
			const Vector<PackedInt32Array> all_face_vert = ret->get_all_poly_cell_vertex_indices(2, false);
			const Vector<PackedInt32Array> all_cell_vert = ret->get_all_poly_cell_vertex_indices(3, false);
			Vector<PackedVector3Array> cell_to_vert_texture_maps = all_poly_cell_texture_maps.has(PolyMesh4D::CELL_TO_VERT_KEY) ? all_poly_cell_texture_maps[PolyMesh4D::CELL_TO_VERT_KEY] : Vector<PackedVector3Array>();
			cell_to_vert_texture_maps.resize(all_cell_vert.size());
			for (int face_index = 0; face_index < input_face_count; face_index++) {
				const PackedVector3Array &first_copy_face_vert_texture_maps = face_to_vert_texture_maps[face_index];
				const PackedInt32Array &first_copy_face_vert_ind = all_face_vert[face_index];
				// The second copy being offset by input_face_count is guaranteed because we start with `merge_with`.
				const PackedVector3Array &second_copy_face_vert_texture_maps = face_to_vert_texture_maps[face_index + input_face_count];
				const PackedInt32Array &second_copy_face_vert_ind = all_face_vert[face_index + input_face_count];
				const int64_t cell_index = face_to_extruded_cell[face_index];
				const PackedInt32Array &this_cell_vert = all_cell_vert[cell_index];
				PackedVector3Array cell_vert_texture_maps;
				cell_vert_texture_maps.resize(this_cell_vert.size());
				for (int64_t vert_in_cell = 0; vert_in_cell < this_cell_vert.size(); vert_in_cell++) {
					const int32_t vert_index = this_cell_vert[vert_in_cell];
					const int64_t vert_in_first_copy = first_copy_face_vert_ind.find(vert_index);
					const int64_t vert_in_second_copy = second_copy_face_vert_ind.find(vert_index);
					if (vert_in_first_copy != -1 && vert_in_first_copy < first_copy_face_vert_texture_maps.size()) {
						cell_vert_texture_maps.set(vert_in_cell, first_copy_face_vert_texture_maps[vert_in_first_copy]);
					} else if (vert_in_second_copy != -1 && vert_in_second_copy < second_copy_face_vert_texture_maps.size()) {
						cell_vert_texture_maps.set(vert_in_cell, second_copy_face_vert_texture_maps[vert_in_second_copy]);
					} else {
						// Neither face has vertex texture map data for this vertex in this cell, so just use a default value.
						cell_vert_texture_maps.set(vert_in_cell, Vector3(0.0f, 0.0f, 0.0f));
					}
				}
				cell_to_vert_texture_maps.set(cell_index, cell_vert_texture_maps);
			}
			all_poly_cell_texture_maps.insert(PolyMesh4D::CELL_TO_VERT_KEY, cell_to_vert_texture_maps);
			ret->set_all_poly_cell_texture_maps(all_poly_cell_texture_maps);
		}
	}
	// Overwrite the cells and recalculate the normals again to ensure data consistency.
	ret->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
	CRASH_COND(!ret->is_mesh_data_valid());
	return ret;
}

Ref<ArrayPolyMesh4D> PolyMeshBuilder4D::extrude_spin_from_faces_xw(const Ref<ArrayPolyMesh4D> &p_input_mesh, const int p_steps) {
	Ref<ArrayPolyMesh4D> ret;
	ret.instantiate();
	// Step 1: Validate the input mesh is in the correct format for this function.
	ERR_FAIL_COND_V(p_input_mesh.is_null(), ret);
	ERR_FAIL_COND_V_MSG(p_steps < 3, ret, "At least 3 steps are required for spin extrusion to form a closed loop. Multiples of 4 are recommended for axis alignment.");
	const Vector<Vector<PackedInt32Array>> &input_poly_cell_indices = p_input_mesh->get_poly_cell_indices();
	ERR_FAIL_COND_V_MSG(input_poly_cell_indices.size() == 0 || input_poly_cell_indices.size() > 2, ret, "Input mesh must have 2D faces and optionally 3D cells (poly cell indices of size 1 or 2), with no higher order elements like 4D volumes.");
	ERR_FAIL_COND_V_MSG(!p_input_mesh->is_mesh_data_valid(), ret, "Input mesh is not valid, so extrusion cannot be performed.");
	const PackedVector4Array &input_vertices = p_input_mesh->get_poly_cell_vertices();
	const PackedInt32Array &input_edge_indices = p_input_mesh->get_edge_indices();
	const Vector<PackedInt32Array> &input_face_indices = input_poly_cell_indices[0];
	const Vector<PackedInt32Array> &input_cell_indices = input_poly_cell_indices.size() > 1 ? input_poly_cell_indices[1] : Vector<PackedInt32Array>();
	const int32_t input_vertex_count = input_vertices.size();
	const int64_t input_edge_count = input_edge_indices.size() / 2;
	const int32_t input_face_count = input_face_indices.size();
	const int32_t input_cell_count = input_cell_indices.size();
	ERR_FAIL_COND_V_MSG(input_vertex_count <= 0 || input_edge_count <= 0 || input_face_count <= 0, ret, "Input mesh must contain vertices, edges, and faces.");
	const double radians_per_step = Math_TAU / p_steps;
	// Step 2: Start with a copy of the input mesh's vertices, edges, and faces.
	// Work with raw arrays to avoid safety checks and other overhead of the ArrayPolyMesh4D's functions.
	PackedVector4Array output_vertices = PackedVector4Array(input_vertices);
	PackedInt32Array output_edge_indices = PackedInt32Array(input_edge_indices);
	Vector<PackedInt32Array> output_face_indices = Vector<PackedInt32Array>(input_face_indices);
	Vector<PackedInt32Array> output_cell_indices = Vector<PackedInt32Array>(input_cell_indices);
	Vector<PackedInt32Array> output_volume_indices; // Will be filled with only new volumes formed between the input cells, if any.
	// Step 3: Account for floating-point error by aligning vertices nearly zero on the X axis.
	for (int vertex_index = 0; vertex_index < output_vertices.size(); vertex_index++) {
		Vector4 vertex = output_vertices[vertex_index];
		if (Math::is_zero_approx(vertex.x)) {
			vertex.x = 0.0;
			output_vertices.set(vertex_index, vertex);
		}
	}
	// Step 4: Keep track of the mapping between input elements and output elements.
	PackedInt32Array original_to_rotated_vertices;
	PackedInt32Array original_to_rotated_edges;
	PackedInt32Array original_to_rotated_faces;
	PackedInt32Array original_to_rotated_cells;
	original_to_rotated_vertices.resize(input_vertex_count * p_steps);
	original_to_rotated_edges.resize(input_edge_count * p_steps);
	original_to_rotated_faces.resize(input_face_count * p_steps);
	original_to_rotated_cells.resize(input_cell_count * p_steps);
	for (int32_t vertex_index = 0; vertex_index < input_vertex_count; vertex_index++) {
		original_to_rotated_vertices.set(vertex_index * p_steps, vertex_index);
	}
	for (int64_t edge_index = 0; edge_index < input_edge_count; edge_index++) {
		original_to_rotated_edges.set(edge_index * p_steps, edge_index);
	}
	for (int32_t face_index = 0; face_index < input_face_count; face_index++) {
		original_to_rotated_faces.set(face_index * p_steps, face_index);
	}
	for (int32_t cell_index = 0; cell_index < input_cell_count; cell_index++) {
		original_to_rotated_cells.set(cell_index * p_steps, cell_index);
	}
	// Step 5: Rotate and merge copies of the input mesh's vertices for each step.
	for (int step = 1; step < p_steps; step++) {
		const double angle = step * radians_per_step;
		const Basis4D rotation = Basis4D::from_xw(angle);
		for (int32_t vertex_index = 0; vertex_index < input_vertex_count; vertex_index++) {
			const int32_t map_index = vertex_index * p_steps + step;
			Vector4 vertex = input_vertices[vertex_index];
			if (vertex.x == 0.0) {
				// Rotating would have no effect here, so deduplicate this vertex.
				original_to_rotated_vertices.set(map_index, original_to_rotated_vertices[vertex_index * p_steps]);
			} else {
				const Vector4 rotated_vertex = rotation.xform(input_vertices[vertex_index]);
				original_to_rotated_vertices.set(map_index, output_vertices.size());
				output_vertices.append(rotated_vertex);
			}
		}
	}
	// Step 6: Merge copies of the edges. Check for edges aligned with X=0 through
	// both vertices being deduplicated, and if so, deduplicate the edge as well.
	for (int step = 1; step < p_steps; step++) {
		for (int64_t edge_index = 0; edge_index < input_edge_count; edge_index++) {
			const int32_t vertex_index_a = input_edge_indices[edge_index * 2];
			const int32_t vertex_index_b = input_edge_indices[edge_index * 2 + 1];
			const int32_t vertex_a_base = vertex_index_a * p_steps;
			const int32_t vertex_b_base = vertex_index_b * p_steps;
			const bool vertex_a_deduplicated = original_to_rotated_vertices[vertex_a_base] == original_to_rotated_vertices[vertex_a_base + 1];
			const bool vertex_b_deduplicated = original_to_rotated_vertices[vertex_b_base] == original_to_rotated_vertices[vertex_b_base + 1];
			const int32_t edge_map_index = edge_index * p_steps + step;
			if (vertex_a_deduplicated && vertex_b_deduplicated) {
				// Both vertices of this edge are aligned with X=0 and were deduplicated, so deduplicate this edge as well.
				original_to_rotated_edges.set(edge_map_index, original_to_rotated_edges[edge_index * p_steps]);
			} else {
				original_to_rotated_edges.set(edge_map_index, output_edge_indices.size() / 2);
				int32_t rotated_a = original_to_rotated_vertices[vertex_a_base + step];
				int32_t rotated_b = original_to_rotated_vertices[vertex_b_base + step];
				if (rotated_a > rotated_b) {
					SWAP(rotated_a, rotated_b);
				}
				output_edge_indices.append(rotated_a);
				output_edge_indices.append(rotated_b);
			}
		}
	}
	// Step 7: Merge copies of the faces. Assume that faces aren't aligned with X=0, don't deduplicate.
	// Input meshes with faces aligned with X=0 are not valid for this function anyway.
	for (int step = 1; step < p_steps; step++) {
		for (int face_index = 0; face_index < input_face_count; face_index++) {
			const PackedInt32Array input_face_edge_indices = input_face_indices[face_index];
			PackedInt32Array rotated_face_edges;
			for (int i = 0; i < input_face_edge_indices.size(); i++) {
				const int32_t edge_index = input_face_edge_indices[i];
				rotated_face_edges.append(original_to_rotated_edges[edge_index * p_steps + step]);
			}
			original_to_rotated_faces.set(face_index * p_steps + step, output_face_indices.size());
			output_face_indices.append(rotated_face_edges);
		}
	}
	// Step 8: Merge copies of the cells. Assume that cells aren't aligned with X=0, don't deduplicate.
	for (int step = 1; step < p_steps; step++) {
		for (int cell_index = 0; cell_index < input_cell_count; cell_index++) {
			const PackedInt32Array input_cell_face_indices = input_cell_indices[cell_index];
			PackedInt32Array rotated_cell_faces;
			for (int i = 0; i < input_cell_face_indices.size(); i++) {
				const int32_t face_index = input_cell_face_indices[i];
				rotated_cell_faces.append(original_to_rotated_faces[face_index * p_steps + step]);
			}
			original_to_rotated_cells.set(cell_index * p_steps + step, output_cell_indices.size());
			output_cell_indices.append(rotated_cell_faces);
		}
	}
	Vector<PackedInt32Array> input_cells_to_output_volumes;
	Vector<PackedInt32Array> input_faces_to_output_cells;
	Vector<PackedInt32Array> input_edges_to_output_faces;
	Vector<PackedInt32Array> input_vertices_to_output_edges;
	if (input_cell_count > 0) {
		input_cells_to_output_volumes.resize(p_steps);
	}
	input_faces_to_output_cells.resize(p_steps);
	input_edges_to_output_faces.resize(p_steps);
	input_vertices_to_output_edges.resize(p_steps);
	for (int this_step = 0; this_step < p_steps; this_step++) {
		const int next_step = (this_step + 1) % p_steps;
		// Step 9: Form new edges between the rotated vertices of the current step and the next step.
		PackedInt32Array input_vertex_to_output_edge;
		input_vertex_to_output_edge.resize(input_vertex_count);
		for (int32_t vertex_index = 0; vertex_index < input_vertex_count; vertex_index++) {
			const int32_t vertex_base = vertex_index * p_steps;
			int32_t this_rotated_vertex_index = original_to_rotated_vertices[vertex_base + this_step];
			int32_t next_rotated_vertex_index = original_to_rotated_vertices[vertex_base + next_step];
			if (this_rotated_vertex_index == next_rotated_vertex_index) {
				// This vertex is aligned with X=0 and was deduplicated, so skip creating an edge for it.
				// Since the source vertex did not move, we can't create an edge between a vertex and itself.
				input_vertex_to_output_edge.set(vertex_index, -1);
			} else {
				input_vertex_to_output_edge.set(vertex_index, output_edge_indices.size() / 2);
				if (this_rotated_vertex_index > next_rotated_vertex_index) {
					SWAP(this_rotated_vertex_index, next_rotated_vertex_index);
				}
				output_edge_indices.append(this_rotated_vertex_index);
				output_edge_indices.append(next_rotated_vertex_index);
			}
		}
		input_vertices_to_output_edges.set(this_step, input_vertex_to_output_edge);
		// Step 10: Form new faces between the rotated edges of the current step
		// and the next step, and the new edges formed in the previous step.
		PackedInt32Array input_edge_to_output_face;
		input_edge_to_output_face.resize(input_edge_count);
		for (int edge_index = 0; edge_index < input_edge_count; edge_index++) {
			const int32_t edge_base = edge_index * p_steps;
			const int32_t this_rotated_edge_index = original_to_rotated_edges[edge_base + this_step];
			const int32_t next_rotated_edge_index = original_to_rotated_edges[edge_base + next_step];
			if (this_rotated_edge_index == next_rotated_edge_index) {
				// This edge was deduplicated, so skip creating a face for it.
				// Since the source edge did not move, we can't create a face between an edge and itself.
				input_edge_to_output_face.set(edge_index, -1);
			} else {
				input_edge_to_output_face.set(edge_index, output_face_indices.size());
				PackedInt32Array new_face_edges = { this_rotated_edge_index }; // First copy.
				const int32_t edge_from_vertex_a = input_vertex_to_output_edge[input_edge_indices[edge_index * 2]];
				if (edge_from_vertex_a != -1) {
					new_face_edges.append(edge_from_vertex_a);
				}
				new_face_edges.append(next_rotated_edge_index); // Second copy.
				const int32_t edge_from_vertex_b = input_vertex_to_output_edge[input_edge_indices[edge_index * 2 + 1]];
				if (edge_from_vertex_b != -1) {
					new_face_edges.append(edge_from_vertex_b);
				}
				output_face_indices.append(new_face_edges);
			}
		}
		input_edges_to_output_faces.set(this_step, input_edge_to_output_face);
		// Step 11: Form new cells between the rotated faces of the current step
		// and the next step, and the new faces formed in the previous step.
		PackedInt32Array input_face_to_output_cell;
		input_face_to_output_cell.resize(input_face_count);
		for (int32_t face_index = 0; face_index < input_face_count; face_index++) {
			const int32_t face_base = face_index * p_steps;
			const int32_t this_rotated_face_index = original_to_rotated_faces[face_base + this_step];
			const int32_t next_rotated_face_index = original_to_rotated_faces[face_base + next_step];
			PackedInt32Array new_cell_faces = { this_rotated_face_index }; // First copy.
			PackedInt32Array input_face_edge_indices = input_face_indices[face_index];
			for (int i = 0; i < input_face_edge_indices.size(); i++) {
				const int32_t edge_index = input_face_edge_indices[i];
				const int32_t output_face_for_edge = input_edge_to_output_face[edge_index];
				if (output_face_for_edge != -1) {
					new_cell_faces.append(output_face_for_edge);
				}
			}
			new_cell_faces.append(next_rotated_face_index); // Second copy.
			input_face_to_output_cell.set(face_index, output_cell_indices.size());
			output_cell_indices.append(new_cell_faces);
		}
		input_faces_to_output_cells.set(this_step, input_face_to_output_cell);
		// Step 12: Form new volumes between the rotated cells of the current step (if any)
		// and the next step, and the new cells formed in the previous step.
		if (input_cell_count > 0) {
			PackedInt32Array input_cell_to_output_volume;
			input_cell_to_output_volume.resize(input_cell_count);
			for (int32_t cell_index = 0; cell_index < input_cell_count; cell_index++) {
				const int32_t cell_base = cell_index * p_steps;
				const int32_t this_rotated_cell_index = original_to_rotated_cells[cell_base + this_step];
				const int32_t next_rotated_cell_index = original_to_rotated_cells[cell_base + next_step];
				PackedInt32Array new_volume_cells = { this_rotated_cell_index };
				PackedInt32Array input_cell_face_indices = input_cell_indices[cell_index];
				for (int i = 0; i < input_cell_face_indices.size(); i++) {
					const int32_t face_index = input_cell_face_indices[i];
					const int32_t output_cell_for_face = input_face_to_output_cell[face_index];
					if (output_cell_for_face != -1) {
						new_volume_cells.append(output_cell_for_face);
					}
				}
				new_volume_cells.append(next_rotated_cell_index);
				input_cell_to_output_volume.set(cell_index, output_volume_indices.size());
				output_volume_indices.append(new_volume_cells);
			}
			input_cells_to_output_volumes.set(this_step, input_cell_to_output_volume);
		}
	}
	// Step 13: Set all of this data on the output mesh.
	ret->set_poly_cell_vertices(output_vertices);
	ret->set_edge_vertex_indices(output_edge_indices);
	Vector<Vector<PackedInt32Array>> output_poly_cell_indices = {
		output_face_indices,
		output_cell_indices,
	};
	if (!output_volume_indices.is_empty()) {
		output_poly_cell_indices.append(output_volume_indices);
	}
	ret->set_poly_cell_indices(output_poly_cell_indices);
	ret->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
	const PackedVector4Array wip_boundary_normals = ret->get_poly_cell_boundary_normals();
	const Vector<PackedInt32Array> input_face_vertex_indices = p_input_mesh->get_all_face_vertex_indices();
	// Step 14: Get or compute the face boundary normals.
	const HashMap<Vector2i, Vector<PackedVector4Array>> input_all_poly_cell_normals = p_input_mesh->get_all_poly_cell_normals();
	PackedVector4Array input_face_normals;
	if (input_all_poly_cell_normals.has(PolyMesh4D::PER_FACE_KEY)) {
		input_face_normals = input_all_poly_cell_normals[PolyMesh4D::PER_FACE_KEY][0];
	} else {
		input_face_normals.resize(input_face_count);
		for (int32_t face_index = 0; face_index < input_face_count; face_index++) {
			const PackedInt32Array this_input_face_vert = input_face_vertex_indices[face_index];
			ERR_FAIL_COND_V(this_input_face_vert.size() < 3, ret);
			const Vector4 face_x = input_vertices[this_input_face_vert[1]] - input_vertices[this_input_face_vert[0]];
			const Vector4 face_y = input_vertices[this_input_face_vert[2]] - input_vertices[this_input_face_vert[0]];
			const Vector3 face_cross_3d = Vector4D::to_3d(face_x).cross(Vector4D::to_3d(face_y));
			input_face_normals.set(face_index, Vector4D::from_3d(face_cross_3d.normalized()));
		}
	}
	// Step 15: Ensure correct winding order of the new cells, make cell boundary normals match the face boundary normals.
	for (int step = 0; step < p_steps; step++) {
		const PackedInt32Array cell_indices_for_faces = input_faces_to_output_cells[step];
		const double angle = (step + 0.5) * radians_per_step;
		const Basis4D rotation = Basis4D::from_xw(angle);
		for (int32_t face_index = 0; face_index < input_face_count; face_index++) {
			const Vector4 desired_boundary_normal = rotation.xform(input_face_normals[face_index]);
			const int32_t boundary_cell_index = cell_indices_for_faces[face_index];
			if (desired_boundary_normal.dot(wip_boundary_normals[boundary_cell_index]) < 0.0) {
				// This cell is wound in the wrong direction, so swap the first two faces to flip the normal.
				PackedInt32Array cell = output_cell_indices[boundary_cell_index];
				int32_t temp = cell[0];
				cell.set(0, cell[1]);
				cell.set(1, temp);
				output_cell_indices.set(boundary_cell_index, cell);
			}
		}
	}
	// Keep mesh topology in sync with local winding fixes before deriving vertex-mapped attributes.
	output_poly_cell_indices.set(1, output_cell_indices);
	ret->set_poly_cell_indices(output_poly_cell_indices);
	// Step 16: Copy and convert the face normals if they exist, rotating them appropriately for each step.
	HashMap<Vector2i, Vector<PackedVector4Array>> output_all_poly_cell_normals = ret->get_all_poly_cell_normals();
	output_all_poly_cell_normals.insert(PolyMesh4D::PER_FACE_KEY, Vector<PackedVector4Array>{ input_face_normals });
	for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &normal_kv : input_all_poly_cell_normals) {
		const Vector2i key = normal_kv.key;
		if (key == PolyMesh4D::PER_FACE_KEY) {
			continue; // Used above to orient the output cells, but cannot be represented for all output faces.
		}
		// Step 16.1: Get and validate the normal data for this key.
		const Vector<PackedVector4Array> &normals = normal_kv.value;
		if (normals.is_empty()) {
			continue; // Ignore empty normal data.
		}
		ERR_FAIL_COND_V_MSG(key.x > 3, ret, "Input mesh has normals for poly dimension higher than cells, which is not supported for spin extrusion.");
		int32_t input_element_count = 0;
		if (key.x == 0) {
			input_element_count = input_vertex_count;
		} else if (key.x == 1) {
			input_element_count = input_edge_count;
		} else if (key.x == 2) {
			input_element_count = input_face_count;
		}
		if (key.y == key.x) {
			// Step 16.2: Copy and rotate the flat array structure of per-element normals. If so, skip step 14.3.
			ERR_FAIL_COND_V_MSG(normals.size() != 1, ret, "Expected flat array for per-element normal data.");
			PackedVector4Array flat_array = normals[0];
			PackedVector4Array output_flat_array;
			output_flat_array.resize(flat_array.size() * p_steps);
			for (int step = 0; step < p_steps; step++) {
				const double angle = step * radians_per_step;
				const Basis4D rotation = Basis4D::from_xw(angle);
				for (int32_t element_index = 0; element_index < input_element_count; element_index++) {
					const Vector4 rotated_normal = rotation.xform(flat_array[element_index]);
					output_flat_array.set(step * input_element_count + element_index, rotated_normal);
				}
			}
			output_all_poly_cell_normals.insert(key, Vector<PackedVector4Array>{ output_flat_array });
		} else {
			// Step 16.3: Copy and rotate the nested array structure of element-to-subelement normals.
			ERR_FAIL_COND_V_MSG(normals.size() != input_element_count, ret, "Expected array with one entry per element for element-to-subelement normal data.");
			Vector<PackedVector4Array> output_arrays;
			output_arrays.resize(input_element_count * p_steps);
			for (int step = 0; step < p_steps; step++) {
				const double angle = step * radians_per_step;
				const Basis4D rotation = Basis4D::from_xw(angle);
				for (int32_t element_index = 0; element_index < input_element_count; element_index++) {
					const PackedVector4Array &elem_vertex_normals = normals[element_index];
					PackedVector4Array output_elem_vertex_normals;
					for (int i = 0; i < elem_vertex_normals.size(); i++) {
						const Vector4 rotated_normal = rotation.xform(elem_vertex_normals[i]);
						output_elem_vertex_normals.append(rotated_normal);
					}
					output_arrays.set(input_element_count * step + element_index, output_elem_vertex_normals);
				}
			}
			output_all_poly_cell_normals.insert(key, output_arrays);
		}
	}
	// Step 17: Extrude the low-dim vertex normals into high-dim vertex normals, such as faces to cells.
	// Here we only read where the input has the key, but the data comes from the output mesh.
	for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &normal_kv : input_all_poly_cell_normals) {
		const Vector2i key = normal_kv.key;
		if (key == PolyMesh4D::PER_FACE_KEY) {
			continue; // Used above to orient the output cells, but cannot be represented for all output faces.
		}
		CRASH_COND(!output_all_poly_cell_normals.has(key)); // We should have copied all other input normals to the output in step 14, so this key should exist in the output.
		const Vector2i dest_key = Vector2i(key.x + 1, key.y);
		if (output_all_poly_cell_normals.has(dest_key)) {
			continue; // Already handled above in step 14.
		}
		if (key.y != 0) {
			continue; // For now hard-code vertex normals, could be generalized later if needed.
		}
		ERR_FAIL_COND_V_MSG(key.x > 3, ret, "Input mesh has normals for poly dimension higher than cells, which is not supported for spin extrusion.");
		// Step 17.1: Prepare the data structures needed to extrude the normals.
		const Vector<PackedVector4Array> &face_vert_normals = output_all_poly_cell_normals[key];
		const Vector<PackedInt32Array> output_face_vert = ret->get_all_poly_cell_vertex_indices(key.x, false);
		const Vector<PackedInt32Array> output_cell_vert = ret->get_all_poly_cell_vertex_indices(dest_key.x, false);
		Vector<PackedVector4Array> cell_vert_normals;
		cell_vert_normals.resize(output_cell_vert.size());
		const int32_t output_face_vert_count = output_face_vert.size();
		// Step 17.2: For each spin-generated output cell, find the first and second copy faces it came from,
		// and copy over their vertex normals. Existing input cells do not follow this layout.
		for (int step = 0; step < p_steps; step++) {
			const PackedInt32Array cell_indices_for_faces = input_faces_to_output_cells[step];
			for (int32_t face_index = 0; face_index < input_face_count; face_index++) {
				const int32_t cell_index = cell_indices_for_faces[face_index];
				const PackedInt32Array &cell_faces = output_cell_indices[cell_index];
				// Step 11 places this as the last face in the cell.
				const int32_t second_copy_face = cell_faces[cell_faces.size() - 1];
				// This may have been swapped in step 15 to ensure correct winding order. The first copy face will have a lower face index than the extruded faces.
				const int32_t first_copy_face = MIN<int32_t, int32_t>(cell_faces[0], cell_faces[1]);
				// Note: first_copy_face may be >= second_copy_face for the last step, when next_step wraps to 0 and second_copy_face becomes the step-0 face (lower index).
				CRASH_COND(first_copy_face >= output_face_vert_count || second_copy_face >= output_face_vert_count);
				const PackedInt32Array &cell_verts = output_cell_vert[cell_index];
				const PackedInt32Array &first_copy_face_verts = output_face_vert[first_copy_face];
				const PackedInt32Array &second_copy_face_verts = output_face_vert[second_copy_face];
				PackedVector4Array this_cell_vert_normals;
				this_cell_vert_normals.resize_uninitialized(cell_verts.size());
				for (int vert_in_cell = 0; vert_in_cell < cell_verts.size(); vert_in_cell++) {
					const int32_t vert_index = cell_verts[vert_in_cell];
					const int64_t vert_in_first = first_copy_face_verts.find(vert_index);
					const int64_t vert_in_second = second_copy_face_verts.find(vert_index);
					Vector4 normal_from_first = vert_in_first != -1 ? face_vert_normals[first_copy_face][vert_in_first] : Vector4();
					Vector4 normal_from_second = vert_in_second != -1 ? face_vert_normals[second_copy_face][vert_in_second] : Vector4();
					// Set the normal for this vertex in the cell based on which face it came from.
					if (vert_in_first == -1) {
						CRASH_COND(vert_in_second == -1); // This vertex should be in at least one of the two faces.
						this_cell_vert_normals.set(vert_in_cell, normal_from_second);
					} else if (vert_in_second == -1) {
						this_cell_vert_normals.set(vert_in_cell, normal_from_first);
					} else {
						// This vertex is shared by both the first and second copy faces (it was deduplicated), so average the normals from both faces for this vertex.
						this_cell_vert_normals.set(vert_in_cell, (normal_from_first + normal_from_second) * 0.5);
					}
				}
				cell_vert_normals.set(cell_index, this_cell_vert_normals);
			}
		}
		output_all_poly_cell_normals.insert(dest_key, cell_vert_normals);
	}
	ret->set_all_poly_cell_normals(output_all_poly_cell_normals);
	// Step 18: Copy and convert the face texture maps if they exist, adding to the Z for each step.
	const HashMap<Vector2i, Vector<PackedVector3Array>> &input_all_poly_cell_texture_maps = p_input_mesh->get_all_poly_cell_texture_maps();
	HashMap<Vector2i, Vector<PackedVector3Array>> output_all_poly_cell_texture_maps = ret->get_all_poly_cell_texture_maps();
	for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &tex_map_kv : input_all_poly_cell_texture_maps) {
		const Vector2i key = tex_map_kv.key;
		// Step 18.1: Get and validate the texture map data for this key.
		const Vector<PackedVector3Array> &tex_maps = tex_map_kv.value;
		if (tex_maps.is_empty()) {
			continue; // Ignore empty texture map data.
		}
		ERR_FAIL_COND_V_MSG(key.x > 3, ret, "Input mesh has texture maps for poly dimension higher than cells, which is not supported for spin extrusion.");
		int32_t input_element_count = 0;
		if (key.x == 0) {
			input_element_count = input_vertex_count;
		} else if (key.x == 1) {
			input_element_count = input_edge_count;
		} else if (key.x == 2) {
			input_element_count = input_face_count;
		}
		if (key.y == key.x) {
			// Step 18.2: Copy and convert the flat array structure of per-element texture maps. If so, skip step 16.3.
			ERR_FAIL_COND_V_MSG(tex_maps.size() != 1, ret, "Expected flat array for per-element texture map data.");
			PackedVector3Array flat_array = tex_maps[0];
			PackedVector3Array output_flat_array;
			output_flat_array.resize(flat_array.size() * p_steps);
			for (int step = 0; step < p_steps; step++) {
				const double z_offset = step / (double)p_steps;
				for (int32_t element_index = 0; element_index < input_element_count; element_index++) {
					Vector3 tex_coord = flat_array[element_index];
					tex_coord.z += z_offset;
					output_flat_array.set(step * input_element_count + element_index, tex_coord);
				}
			}
			output_all_poly_cell_texture_maps.insert(key, Vector<PackedVector3Array>{ output_flat_array });
		} else {
			// Step 18.3: Copy and convert the nested array structure of element-to-subelement texture maps.
			ERR_FAIL_COND_V_MSG(tex_maps.size() != input_element_count, ret, "Expected array with one entry per element for element-to-subelement texture map data.");
			Vector<PackedVector3Array> output_arrays;
			output_arrays.resize(input_element_count * p_steps);
			for (int step = 0; step < p_steps; step++) {
				const double z_offset = step / (double)p_steps;
				for (int32_t element_index = 0; element_index < input_element_count; element_index++) {
					const PackedVector3Array &face_vertex_tex_maps = tex_maps[element_index];
					PackedVector3Array output_face_vertex_tex_maps;
					for (int i = 0; i < face_vertex_tex_maps.size(); i++) {
						Vector3 uv = face_vertex_tex_maps[i];
						uv.z += z_offset;
						output_face_vertex_tex_maps.append(uv);
					}
					output_arrays.set(step * input_element_count + element_index, output_face_vertex_tex_maps);
				}
			}
			output_all_poly_cell_texture_maps.insert(key, output_arrays);
		}
	}
	// Step 19: Extrude the low-dim face texture maps into high-dim cell texture maps, such as faces to cells.
	// Here we only read where the input has the key, but the data comes from the output mesh.
	for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &normal_kv : input_all_poly_cell_texture_maps) {
		const Vector2i key = normal_kv.key;
		CRASH_COND(!output_all_poly_cell_texture_maps.has(key)); // We should have copied all of the input texture maps to the output in step 16, so this key should exist in the output.
		const Vector2i dest_key = Vector2i(key.x + 1, key.y);
		if (output_all_poly_cell_texture_maps.has(dest_key)) {
			continue; // Already handled above in step 16.
		}
		if (key.y != 0) {
			continue; // For now hard-code vertex texture maps, could be generalized later if needed.
		}
		ERR_FAIL_COND_V_MSG(key.x > 3, ret, "Input mesh has texture maps for poly dimension higher than cells, which is not supported for spin extrusion.");
		// Step 19.1: Prepare the data structures needed to extrude the texture maps.
		const Vector<PackedVector3Array> &face_vert_normals = output_all_poly_cell_texture_maps[key];
		const Vector<PackedInt32Array> output_face_vert = ret->get_all_poly_cell_vertex_indices(key.x, false);
		const Vector<PackedInt32Array> output_cell_vert = ret->get_all_poly_cell_vertex_indices(dest_key.x, false);
		Vector<PackedVector3Array> cell_vert_tex_maps;
		cell_vert_tex_maps.resize(output_cell_vert.size());
		const int32_t output_face_vert_count = output_face_vert.size();
		// Step 19.2: For each spin-generated output cell, find the first and second copy faces it came from,
		// and copy over their texture maps. Existing input cells do not follow this layout.
		for (int step = 0; step < p_steps; step++) {
			const PackedInt32Array cell_indices_for_faces = input_faces_to_output_cells[step];
			for (int32_t face_index = 0; face_index < input_face_count; face_index++) {
				const int32_t cell_index = cell_indices_for_faces[face_index];
				const PackedInt32Array &cell_faces = output_cell_indices[cell_index];
				// Step 11 places this as the last face in the cell.
				const int32_t second_copy_face = cell_faces[cell_faces.size() - 1];
				// This may have been swapped in step 13 to ensure correct winding order. The first copy face will have a lower face index than the extruded faces.
				const int32_t first_copy_face = MIN<int32_t, int32_t>(cell_faces[0], cell_faces[1]);
				// Note: first_copy_face may be >= second_copy_face for the last step, when next_step wraps to 0 and second_copy_face becomes the step-0 face (lower index).
				CRASH_COND(first_copy_face >= output_face_vert_count || second_copy_face >= output_face_vert_count);
				const PackedInt32Array &cell_verts = output_cell_vert[cell_index];
				const PackedInt32Array &first_copy_face_verts = output_face_vert[first_copy_face];
				const PackedInt32Array &second_copy_face_verts = output_face_vert[second_copy_face];
				PackedVector3Array this_cell_vert_tex_maps;
				this_cell_vert_tex_maps.resize_uninitialized(cell_verts.size());
				for (int vert_in_cell = 0; vert_in_cell < cell_verts.size(); vert_in_cell++) {
					const int32_t vert_index = cell_verts[vert_in_cell];
					const int64_t vert_in_first = first_copy_face_verts.find(vert_index);
					const int64_t vert_in_second = second_copy_face_verts.find(vert_index);
					Vector3 tex_coord_from_first = vert_in_first != -1 ? face_vert_normals[first_copy_face][vert_in_first] : Vector3();
					Vector3 tex_coord_from_second = vert_in_second != -1 ? face_vert_normals[second_copy_face][vert_in_second] : Vector3();
					// Special case: For the last step, the second copy face is from the first step (due to wrapping),
					// but we don't want the texture map to roll over, so let's artificially add 1.0 to the texture map's Z.
					if (step == p_steps - 1) {
						tex_coord_from_second.z += 1.0f;
					}
					// Set the texture map for this vertex in the cell based on which face it came from.
					if (vert_in_first == -1) {
						CRASH_COND(vert_in_second == -1); // This vertex should be in at least one of the two faces.
						this_cell_vert_tex_maps.set(vert_in_cell, tex_coord_from_second);
					} else if (vert_in_second == -1) {
						this_cell_vert_tex_maps.set(vert_in_cell, tex_coord_from_first);
					} else {
						// This vertex is shared by both the first and second copy faces (it was deduplicated), so average the texture maps from both faces for this vertex.
						this_cell_vert_tex_maps.set(vert_in_cell, (tex_coord_from_first + tex_coord_from_second) * 0.5);
					}
				}
				cell_vert_tex_maps.set(cell_index, this_cell_vert_tex_maps);
			}
		}
		output_all_poly_cell_texture_maps.insert(dest_key, cell_vert_tex_maps);
	}
	ret->set_all_poly_cell_texture_maps(output_all_poly_cell_texture_maps);
	// Step 20: Recalculate boundary normals and validate.
	ret->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
	CRASH_COND(!ret->is_poly_mesh_data_valid());
	return ret;
}

// Everything below this line is `reconstruct_from_tetra_mesh` and
// its dependent helper functions (and also bindings at the bottom).

int64_t PolyMeshBuilder4D::_append_edge_indices_to_array(int32_t p_index_a, int32_t p_index_b, const bool p_deduplicate, PackedInt32Array &r_edge_vertex_indices) {
	const int64_t edge_count = r_edge_vertex_indices.size() / 2;
	if (p_index_a > p_index_b) {
		SWAP(p_index_a, p_index_b);
	}
	if (p_deduplicate) {
		for (int64_t i = 0; i < edge_count; i++) {
			if (r_edge_vertex_indices[i * 2] == p_index_a && r_edge_vertex_indices[i * 2 + 1] == p_index_b) {
				return i;
			}
		}
	}
	r_edge_vertex_indices.append(p_index_a);
	r_edge_vertex_indices.append(p_index_b);
	return edge_count;
}

int64_t PolyMeshBuilder4D::_append_face_internal_to_array(const PackedInt32Array &p_face, Vector<PackedInt32Array> &r_all_face_edge_indices) {
	const int64_t existing_face_count = r_all_face_edge_indices.size();
	const int64_t face_edge_count = p_face.size();
	for (int64_t existing_face_index = 0; existing_face_index < existing_face_count; existing_face_index++) {
		const PackedInt32Array &existing_face = r_all_face_edge_indices[existing_face_index];
		if (face_edge_count != existing_face.size()) {
			continue;
		}
		const int64_t start_index = existing_face.find(p_face[0]);
		if (start_index == -1) {
			continue;
		}
		// Check if the order matches in the same direction.
		bool found = true;
		for (int64_t i = 1; i < face_edge_count; i++) {
			if (p_face[i] != existing_face[(start_index + i) % face_edge_count]) {
				found = false;
				break;
			}
		}
		if (found) {
			return existing_face_index;
		}
		// Check if the order matches in the opposite direction.
		found = true;
		for (int64_t i = 1; i < face_edge_count; i++) {
			int64_t existing_index = start_index - i;
			if (existing_index < 0) {
				existing_index += face_edge_count;
			}
			if (p_face[i] != existing_face[existing_index]) {
				found = false;
				break;
			}
		}
		if (found) {
			return existing_face_index;
		}
	}
	r_all_face_edge_indices.append(p_face);
	return existing_face_count;
}

Vector<PackedInt32Array> PolyMeshBuilder4D::_compose_triangles_into_faces(const PackedVector4Array &p_vertices, const Vector<PackedInt32Array> &p_triangle_vertex_indices, PackedInt32Array &r_edge_vertex_indices) {
	Vector<Vector<PackedInt32Array>> coplanar_triangle_vertex_indices;
	const int64_t triangle_count = p_triangle_vertex_indices.size();
	// Determine which triangles are coplanar with each other so we know what to build the faces with.
	for (int64_t triangle_index = 0; triangle_index < triangle_count; triangle_index++) {
		const PackedInt32Array &triangle = p_triangle_vertex_indices[triangle_index];
		bool is_new_face = true;
		for (int64_t coplanar_index = 0; coplanar_index < coplanar_triangle_vertex_indices.size(); coplanar_index++) {
			const PackedInt32Array &coplanar_representative = coplanar_triangle_vertex_indices[coplanar_index][0];
			bool is_coplanar = true;
			for (int64_t vertex_in_triangle = 0; vertex_in_triangle < 3; vertex_in_triangle++) {
				const Vector4 perp = Vector4D::perpendicular(
						p_vertices[triangle[vertex_in_triangle]].direction_to(p_vertices[coplanar_representative[0]]),
						p_vertices[triangle[vertex_in_triangle]].direction_to(p_vertices[coplanar_representative[1]]),
						p_vertices[triangle[vertex_in_triangle]].direction_to(p_vertices[coplanar_representative[2]]));
				if (!perp.is_zero_approx()) {
					is_coplanar = false; // If it's non-zero, it's not coplanar. Contrapositive of: if it's coplanar, all perp calculations are zero.
					break;
				}
			}
			if (is_coplanar) {
				Vector<PackedInt32Array> coplanar_triangles = coplanar_triangle_vertex_indices[coplanar_index];
				coplanar_triangles.append(triangle);
				coplanar_triangle_vertex_indices.set(coplanar_index, coplanar_triangles);
				is_new_face = false; // If it's coplanar, it's not a new face. Contrapositive of: if it's a new face, all vertices are coplanar.
				break;
			}
		}
		if (is_new_face) {
			// If this isn't coplanar with anything so far, it's a new face.
			coplanar_triangle_vertex_indices.append(Vector<PackedInt32Array>{ triangle });
		}
	}
	// Find the edges which border each face, and insert them in a consistent loop order.
	Vector<PackedInt32Array> all_face_edges;
	for (int64_t coplanar_tri_set_index = 0; coplanar_tri_set_index < coplanar_triangle_vertex_indices.size(); coplanar_tri_set_index++) {
		// The above part of this function groups by coplanarity, but the triangles within each coplanar group
		// might still not be touching each other in rare edge cases. Therefore, we need to further split up
		// each coplanar group into connected components of triangles, and build faces from those.
		// First, build a map of edges to the triangles that use them for this coplanar set, so we can quickly find neighboring triangles.
		const Vector<PackedInt32Array> &coplanar_tri_set = coplanar_triangle_vertex_indices[coplanar_tri_set_index];
		HashMap<Vector2i, PackedInt32Array> edge_to_triangle_indices;
		for (int64_t triangle_index = 0; triangle_index < coplanar_tri_set.size(); triangle_index++) {
			const PackedInt32Array &tri_verts = coplanar_tri_set[triangle_index];
			for (int64_t vertex_in_triangle = 0; vertex_in_triangle < 3; vertex_in_triangle++) {
				Vector2i edge = Vector2i(tri_verts[vertex_in_triangle], tri_verts[(vertex_in_triangle + 1) % 3]);
				if (edge.x > edge.y) {
					SWAP(edge.x, edge.y);
				}
				PackedInt32Array triangles_for_edge = edge_to_triangle_indices.has(edge) ? edge_to_triangle_indices[edge] : PackedInt32Array();
				triangles_for_edge.append(triangle_index);
				edge_to_triangle_indices.insert(edge, triangles_for_edge);
			}
		}
		// Iterate through the triangles, marking any we've dealt with already as visited.
		Vector<bool> visited_triangles;
		visited_triangles.resize_initialized(coplanar_tri_set.size());
		for (int64_t triangle_seed = 0; triangle_seed < coplanar_tri_set.size(); triangle_seed++) {
			if (visited_triangles[triangle_seed]) {
				continue;
			}
			// Gather all connected triangles to this seed triangle using the edge-to-triangle map,
			// mark them as visited as we go, and build a set of the unique edges we encounter.
			HashSet<Vector2i> face_unique_edges;
			PackedInt32Array triangle_stack;
			triangle_stack.append(triangle_seed);
			visited_triangles.set(triangle_seed, true);
			while (!triangle_stack.is_empty()) {
				const int64_t triangle_index = triangle_stack[triangle_stack.size() - 1];
				triangle_stack.resize(triangle_stack.size() - 1);
				const PackedInt32Array &tri_verts = coplanar_tri_set[triangle_index];
				for (int64_t vertex_in_triangle = 0; vertex_in_triangle < 3; vertex_in_triangle++) {
					Vector2i edge = Vector2i(tri_verts[vertex_in_triangle], tri_verts[(vertex_in_triangle + 1) % 3]);
					if (edge.x > edge.y) {
						SWAP(edge.x, edge.y);
					}
					// This assumes that each edge is only used by up to 2 coplanar triangles.
					// This is true of all sane manifold meshes, other cases are not supported.
					if (face_unique_edges.has(edge)) {
						face_unique_edges.erase(edge);
					} else {
						face_unique_edges.insert(edge);
					}
					const PackedInt32Array &neighbor_triangles = edge_to_triangle_indices[edge];
					for (int64_t neighbor_index = 0; neighbor_index < neighbor_triangles.size(); neighbor_index++) {
						const int64_t neighbor_triangle = neighbor_triangles[neighbor_index];
						if (visited_triangles[neighbor_triangle]) {
							continue;
						}
						visited_triangles.set(neighbor_triangle, true);
						triangle_stack.append(neighbor_triangle);
					}
				}
			}
			// Insert the unique edges into the array, in some consistent winding order. Any winding will work fine.
			PackedInt32Array face_edges;
			face_edges.resize(face_unique_edges.size());
			Vector2i last_edge = *(face_unique_edges.begin());
			face_edges.set(0, _append_edge_indices_to_array(last_edge.x, last_edge.y, true, r_edge_vertex_indices));
			face_unique_edges.erase(last_edge);
			int64_t face_edges_index = 1;
			while (!face_unique_edges.is_empty()) {
				bool inserted_something = false;
				for (const Vector2i unique_edge : face_unique_edges) {
					if ((unique_edge.x == last_edge.x) || (unique_edge.x == last_edge.y) || (unique_edge.y == last_edge.x) || (unique_edge.y == last_edge.y)) {
						// This edge shares a vertex with the last edge, so let's insert it.
						const int64_t edge_index = _append_edge_indices_to_array(unique_edge.x, unique_edge.y, true, r_edge_vertex_indices);
						face_edges.set(face_edges_index, edge_index);
						face_edges_index++;
						face_unique_edges.erase(unique_edge);
						last_edge = unique_edge;
						inserted_something = true;
						break;
					}
				}
				// Avoid infinite loops, fail if we didn't insert anything.
				ERR_FAIL_COND_V(!inserted_something, Vector<PackedInt32Array>());
			}
			all_face_edges.append(face_edges);
		}
	}
	return all_face_edges;
}

Vector4 PolyMeshBuilder4D::_compute_cell_normal(const PackedInt32Array &p_cell_first_face, const PackedInt32Array &p_cell_second_face, const PackedInt32Array &p_edge_vertex_indices, const PackedVector4Array &p_vertices) {
	int64_t common_in_first;
	int64_t common_in_second;
	const int32_t common_edge = Math4D::find_common_int32(p_cell_first_face, p_cell_second_face, common_in_first, common_in_second);
	if (common_edge == INT32_MIN) {
		return Vector4();
	}
	const int64_t first_next_index = (common_in_first + 1) % p_cell_first_face.size();
	const int64_t second_next_index = (common_in_second + 1) % p_cell_second_face.size();
	// Use these 3 edges to get 4 vertex indices in a consistent "winding" order.
	const int32_t common_vertex_start = p_edge_vertex_indices[common_edge * 2];
	const int32_t common_vertex_end = p_edge_vertex_indices[common_edge * 2 + 1];
	int32_t first_next_vertex = p_edge_vertex_indices[p_cell_first_face[first_next_index] * 2];
	if (first_next_vertex == common_vertex_start || first_next_vertex == common_vertex_end) {
		first_next_vertex = p_edge_vertex_indices[p_cell_first_face[first_next_index] * 2 + 1];
	}
	int32_t second_next_vertex = p_edge_vertex_indices[p_cell_second_face[second_next_index] * 2];
	if (second_next_vertex == common_vertex_start || second_next_vertex == common_vertex_end) {
		second_next_vertex = p_edge_vertex_indices[p_cell_second_face[second_next_index] * 2 + 1];
	}
	return Vector4D::perpendicular(
			p_vertices[first_next_vertex].direction_to(p_vertices[common_vertex_start]),
			p_vertices[first_next_vertex].direction_to(p_vertices[common_vertex_end]),
			p_vertices[first_next_vertex].direction_to(p_vertices[second_next_vertex]));
}

PackedInt32Array PolyMeshBuilder4D::_save_triangle_vertex_indices_as_faces_and_cell(const Vector<PackedInt32Array> &p_last_triangle_vertex_indices, const Vector4 &p_last_simplex_normal, const PackedVector4Array &p_vertices, Vector<PackedInt32Array> &r_all_face_edge_indices, PackedInt32Array &r_edge_vertex_indices) {
	const Vector<PackedInt32Array> composed_faces = _compose_triangles_into_faces(p_vertices, p_last_triangle_vertex_indices, r_edge_vertex_indices);
	PackedInt32Array cell_face_indices;
	const int64_t composed_face_count = composed_faces.size();
	cell_face_indices.resize(composed_face_count);
	for (int64_t i = 0; i < composed_face_count; i++) {
		cell_face_indices.set(i, _append_face_internal_to_array(composed_faces[i], r_all_face_edge_indices));
	}
	// Now that we have the cell's faces, we still need to decide in which order they appear in.
	// We want an order that results in the correct cell normal being computed.
	for (int64_t index_of_second_face = 1; index_of_second_face < composed_face_count; index_of_second_face++) {
		const Vector4 candidate_normal = _compute_cell_normal(
				r_all_face_edge_indices[cell_face_indices[0]],
				r_all_face_edge_indices[cell_face_indices[index_of_second_face]],
				r_edge_vertex_indices, p_vertices);
		if (candidate_normal.is_zero_approx()) {
			continue;
		}
		const real_t dot = candidate_normal.dot(p_last_simplex_normal);
		if (dot > CMP_EPSILON) {
			// If these are aligned, then this is a correct configuration.
			if (index_of_second_face != 1) {
				const int32_t temp = cell_face_indices[index_of_second_face];
				cell_face_indices.set(index_of_second_face, cell_face_indices[1]);
				cell_face_indices.set(1, temp);
			}
			break;
		}
		if (dot < -CMP_EPSILON) {
			// If these are aligned, then these are a set of correct faces, but in the wrong order.
			const int32_t temp = cell_face_indices[index_of_second_face];
			cell_face_indices.set(index_of_second_face, cell_face_indices[1]);
			cell_face_indices.set(1, cell_face_indices[0]);
			cell_face_indices.set(0, temp);
			break;
		}
	}
	return cell_face_indices;
}

Ref<ArrayPolyMesh4D> PolyMeshBuilder4D::reconstruct_from_tetra_mesh(const Ref<TetraMesh4D> &p_tetra_mesh) {
	ERR_FAIL_COND_V(p_tetra_mesh.is_null(), Ref<ArrayPolyMesh4D>());
	Ref<ArrayPolyMesh4D> ret;
	ret.instantiate();
	const PackedVector4Array vertices = p_tetra_mesh->get_vertices();
	ret->set_poly_cell_vertices(vertices);
	const PackedInt32Array simplex_indices = p_tetra_mesh->get_simplex_cell_indices();
	const int64_t simplex_count = simplex_indices.size() / 4;
	PackedInt32Array boundary_pivot_overrides;
	PackedInt32Array edge_vertex_indices;
	Vector<PackedInt32Array> all_face_edge_indices;
	Vector<PackedInt32Array> all_cell_face_indices;
	{
		Vector<PackedInt32Array> last_triangle_vertex_indices;
		Vector4 last_simplex_normal = Vector4();
		int64_t last_pivot = -1;
		for (int64_t simplex_index = 0; simplex_index < simplex_count; simplex_index++) {
			const int64_t offset = simplex_index * 4;
			const int32_t pivot = simplex_indices[offset];
			if (pivot != last_pivot) {
				// This is a new cell, so save what we found so far, and start a new face list.
				if (simplex_index > 0) {
					all_cell_face_indices.append(_save_triangle_vertex_indices_as_faces_and_cell(
							last_triangle_vertex_indices, last_simplex_normal, vertices,
							all_face_edge_indices, edge_vertex_indices));
					boundary_pivot_overrides.append(last_pivot);
				}
				// Start a new face list.
				last_triangle_vertex_indices = Vector<PackedInt32Array>();
				last_simplex_normal = Vector4D::perpendicular(
						vertices[simplex_indices[offset]].direction_to(vertices[simplex_indices[offset + 1]]),
						vertices[simplex_indices[offset]].direction_to(vertices[simplex_indices[offset + 2]]),
						vertices[simplex_indices[offset]].direction_to(vertices[simplex_indices[offset + 3]]));
				last_pivot = pivot;
			}
			for (int64_t vertex_in_simplex = 0; vertex_in_simplex < 4; vertex_in_simplex++) {
				PackedInt32Array triangle_vertex_indices = {
					simplex_indices[offset + vertex_in_simplex],
					simplex_indices[offset + (vertex_in_simplex + 1) % 4],
					simplex_indices[offset + (vertex_in_simplex + 2) % 4],
				};
				triangle_vertex_indices.sort();
				const int64_t found_index = last_triangle_vertex_indices.find(triangle_vertex_indices);
				if (found_index == -1) {
					last_triangle_vertex_indices.append(triangle_vertex_indices);
				} else {
					last_triangle_vertex_indices.remove_at(found_index); // Internal face, not up for consideration.
				}
			}
		}
		// Save data for the last cell.
		all_cell_face_indices.append(_save_triangle_vertex_indices_as_faces_and_cell(
				last_triangle_vertex_indices, last_simplex_normal, vertices,
				all_face_edge_indices, edge_vertex_indices));
		boundary_pivot_overrides.append(last_pivot);
	}
	ret->set_edge_vertex_indices(edge_vertex_indices);
	ret->set_poly_cell_indices(Vector<Vector<PackedInt32Array>>{ all_face_edge_indices, all_cell_face_indices });
	ret->set_poly_cell_boundary_pivot_overrides(boundary_pivot_overrides);
	return ret;
}

// In-place adjustments to the given mesh.

void PolyMeshBuilder4D::make_boundary_normals_topologically_consistent(const Ref<ArrayPolyMesh4D> &p_mesh_4d, const PackedInt32Array &p_authoritative) {
	// TODO: This function relies on averages and pivot overrides, which breaks in non-convex edge cases.
	// Properly solving this in 4D is non-trivial, this can be improved in the future if needed.
	Vector<Vector<PackedInt32Array>> poly_cell_indices = p_mesh_4d->get_poly_cell_indices();
	ERR_FAIL_COND_MSG(poly_cell_indices.size() < 2, "Mesh must have boundary cells in order to have boundary normals.");
	// Clear out and calculate new normals. The correct ones will either be these, or the negatives of these.
	const PackedVector4Array original_boundary_normals = p_mesh_4d->get_poly_cell_boundary_normals();
	p_mesh_4d->set_poly_cell_boundary_normals(PackedVector4Array());
	p_mesh_4d->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
	CRASH_COND(!p_mesh_4d->is_mesh_data_valid());
	PackedVector4Array boundary_normals = p_mesh_4d->get_poly_cell_boundary_normals();
	for (int64_t auth_index = 0; auth_index < p_authoritative.size(); auth_index++) {
		const int64_t boundary_cell_index = p_authoritative[auth_index];
		ERR_FAIL_INDEX_MSG(boundary_cell_index, boundary_normals.size(), "Authoritative boundary cell index is out of range.");
		boundary_normals.set(boundary_cell_index, original_boundary_normals[boundary_cell_index]);
	}
	// Build a map of faces to the cells that reference them, which will be used to traverse adjacent boundary cells.
	const Vector<PackedInt32Array> faces = poly_cell_indices[0];
	Vector<PackedInt32Array> boundary_cells = poly_cell_indices[1];
	Vector<PackedInt32Array> faces_to_cells;
	faces_to_cells.resize(faces.size());
	for (int64_t cell_index = 0; cell_index < boundary_cells.size(); cell_index++) {
		const PackedInt32Array cell_face_indices = boundary_cells[cell_index];
		for (int64_t face_num = 0; face_num < cell_face_indices.size(); face_num++) {
			const int64_t face_index = cell_face_indices[face_num];
			PackedInt32Array cells_for_face = faces_to_cells[face_index];
			cells_for_face.append(cell_index);
			faces_to_cells.set(face_index, cells_for_face);
		}
	}
	// Compute the average position of each boundary cell.
	const PackedVector4Array &poly_vertices = p_mesh_4d->get_poly_cell_vertices();
	const Vector<PackedInt32Array> boundary_vert = p_mesh_4d->get_all_poly_cell_vertex_indices(3, false);
	const PackedInt32Array &boundary_pivot_overrides = p_mesh_4d->get_poly_cell_boundary_pivot_overrides();
	CRASH_COND(boundary_vert.size() != boundary_cells.size());
	PackedVector4Array boundary_pivot_pos;
	boundary_pivot_pos.resize(boundary_cells.size());
	for (int64_t boundary_cell_index = 0; boundary_cell_index < boundary_cells.size(); boundary_cell_index++) {
		const PackedInt32Array &boundary_cell_vert_indices = boundary_vert[boundary_cell_index];
		Vector4 average_pos = Vector4(0.0, 0.0, 0.0, 0.0);
		for (int64_t i = 0; i < boundary_cell_vert_indices.size(); i++) {
			average_pos += poly_vertices[boundary_cell_vert_indices[i]];
		}
		average_pos /= boundary_cell_vert_indices.size();
		// If the pivot is overridden, use it, but also still take the average and use it for a slight offset.
		// This ensures that pivot overrides on the sides will still give something inside the cell.
		if (boundary_pivot_overrides.size() > boundary_cell_index && boundary_pivot_overrides[boundary_cell_index] != -1) {
			constexpr real_t ONE_PLUS_EPSILON = 1.0 + CMP_EPSILON;
			const int32_t pivot_vert_index = boundary_pivot_overrides[boundary_cell_index];
			average_pos = (average_pos * (ONE_PLUS_EPSILON - 1.0f) + poly_vertices[pivot_vert_index]) / ONE_PLUS_EPSILON;
		}
		boundary_pivot_pos.set(boundary_cell_index, average_pos);
	}
	// Visit each boundary cell, check its neighbors, and flip normals as needed to ensure they are all consistent.
	PackedInt32Array settled = p_authoritative;
	PackedInt32Array to_visit_its_neighbors = p_authoritative;
	while (!to_visit_its_neighbors.is_empty()) {
		const int64_t this_cell_index = to_visit_its_neighbors[to_visit_its_neighbors.size() - 1];
		to_visit_its_neighbors.resize(to_visit_its_neighbors.size() - 1);
		const PackedInt32Array cell_face_indices = boundary_cells[this_cell_index];
		const Vector4 this_cell_pivot = boundary_pivot_pos[this_cell_index];
		const Vector4 this_cell_normal = boundary_normals[this_cell_index];
		for (int64_t face_num = 0; face_num < cell_face_indices.size(); face_num++) {
			const int64_t face_index = cell_face_indices[face_num];
			const PackedInt32Array adjacent_cells = faces_to_cells[face_index];
			for (int64_t adjacent_cell_num = 0; adjacent_cell_num < adjacent_cells.size(); adjacent_cell_num++) {
				const int64_t adjacent_cell_index = adjacent_cells[adjacent_cell_num];
				if (adjacent_cell_index == this_cell_index) {
					continue;
				}
				if (settled.find(adjacent_cell_index) != -1) {
					continue;
				}
				to_visit_its_neighbors.append(adjacent_cell_index);
				settled.append(adjacent_cell_index);
				const Vector4 adjacent_boundary_cell_average = boundary_pivot_pos[adjacent_cell_index];
				const Vector4 adjacent_boundary_cell_normal = boundary_normals[adjacent_cell_index];
				const real_t source_to_adjacent = this_cell_normal.dot(adjacent_boundary_cell_average - this_cell_pivot);
				const real_t adjacent_to_source = adjacent_boundary_cell_normal.dot(this_cell_pivot - adjacent_boundary_cell_average);
				bool should_flip_adjacent = false;
				if (Math::is_zero_approx(source_to_adjacent) || Math::is_zero_approx(adjacent_to_source)) {
					// Coplanar boundary cells should just be oriented the same.
					if (this_cell_normal.dot(adjacent_boundary_cell_normal) < 0.0) {
						should_flip_adjacent = true;
					}
				} else if (SIGN(source_to_adjacent) != SIGN(adjacent_to_source)) {
					should_flip_adjacent = true;
				}
				if (should_flip_adjacent) {
					// This adjacent boundary cell is facing the wrong way, so swap the first two faces to flip.
					PackedInt32Array adjacent_boundary_cell = boundary_cells[adjacent_cell_index];
					int32_t temp = adjacent_boundary_cell[0];
					adjacent_boundary_cell.set(0, adjacent_boundary_cell[1]);
					adjacent_boundary_cell.set(1, temp);
					boundary_cells.set(adjacent_cell_index, adjacent_boundary_cell);
					boundary_normals.set(adjacent_cell_index, -adjacent_boundary_cell_normal);
				}
			}
		}
	}
	poly_cell_indices.set(1, boundary_cells);
	p_mesh_4d->set_poly_cell_indices(poly_cell_indices);
	p_mesh_4d->set_poly_cell_boundary_normals(boundary_normals);
}

PolyMeshBuilder4D *PolyMeshBuilder4D::singleton = nullptr;

void PolyMeshBuilder4D::_bind_methods() {
	// These functions create new meshes from the given data.
	ClassDB::bind_static_method("PolyMeshBuilder4D", D_METHOD("convert_mesh_3d_to_4d_faces_only", "mesh_3d", "which_surface", "deduplicate"), &PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only, DEFVAL(-1), DEFVAL(true));
	ClassDB::bind_static_method("PolyMeshBuilder4D", D_METHOD("extrude_linear", "input_mesh", "extrusion_vector"), &PolyMeshBuilder4D::extrude_linear, DEFVAL(Vector4(0, 0, 0, 1)));
	ClassDB::bind_static_method("PolyMeshBuilder4D", D_METHOD("extrude_spin_from_faces_xw", "input_mesh", "steps"), &PolyMeshBuilder4D::extrude_spin_from_faces_xw, DEFVAL(16));
	ClassDB::bind_static_method("PolyMeshBuilder4D", D_METHOD("reconstruct_from_tetra_mesh", "tetra_mesh"), &PolyMeshBuilder4D::reconstruct_from_tetra_mesh);

	// In-place adjustments to the given mesh.
	ClassDB::bind_static_method("PolyMeshBuilder4D", D_METHOD("make_boundary_normals_topologically_consistent", "mesh_4d", "authoritative_boundary_cells"), &PolyMeshBuilder4D::make_boundary_normals_topologically_consistent);
}
