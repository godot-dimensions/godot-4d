#include "hox_document_4d.h"

#include "../../math/math_4d.h"
#include "../../math/vector_4d.h"
#include "../../nodes/node_4d.h"
#include "../mesh/mesh_instance_4d.h"
#include "../mesh/poly/box_poly_mesh_4d.h"
#include "../mesh/poly/poly_material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image_texture3d.hpp>
#include <godot_cpp/classes/json.hpp>
#elif GODOT_MODULE
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "scene/resources/image_texture.h"
#endif

// It's strange that this format stores 4D vectors both as arrays and as dictionaries in different contexts...
Vector4i HoxDocument4D::_import_parse_vector_int(const Variant &p_json_array_or_dict) {
	Vector4i ret = Vector4i(0, 0, 0, 0);
	if (p_json_array_or_dict.get_type() == Variant::ARRAY) {
		ret = Vector4D::from_json_array_int(p_json_array_or_dict);
	} else if (p_json_array_or_dict.get_type() == Variant::DICTIONARY) {
		const Dictionary json_vector_dict = p_json_array_or_dict;
		if (json_vector_dict.has("x")) {
			ret.x = (int32_t)json_vector_dict["x"];
		}
		if (json_vector_dict.has("y")) {
			ret.y = (int32_t)json_vector_dict["y"];
		}
		if (json_vector_dict.has("z")) {
			ret.z = (int32_t)json_vector_dict["z"];
		}
		if (json_vector_dict.has("w")) {
			ret.w = (int32_t)json_vector_dict["w"];
		}
	}
	return ret;
}

Vector4 HoxDocument4D::_import_parse_vector_float(const Variant &p_json_array_or_dict) {
	Vector4 ret = Vector4(0, 0, 0, 0);
	if (p_json_array_or_dict.get_type() == Variant::ARRAY) {
		ret = Vector4D::from_json_array(p_json_array_or_dict);
	} else if (p_json_array_or_dict.get_type() == Variant::DICTIONARY) {
		const Dictionary json_vector_dict = p_json_array_or_dict;
		if (json_vector_dict.has("x")) {
			ret.x = (real_t)(double)json_vector_dict["x"];
		}
		if (json_vector_dict.has("y")) {
			ret.y = (real_t)(double)json_vector_dict["y"];
		}
		if (json_vector_dict.has("z")) {
			ret.z = (real_t)(double)json_vector_dict["z"];
		}
		if (json_vector_dict.has("w")) {
			ret.w = (real_t)(double)json_vector_dict["w"];
		}
	}
	return ret;
}

void HoxDocument4D::_import_parse_hoxel_grids(const Array &p_json_hoxel_grids) {
	const int64_t hoxel_grid_count = p_json_hoxel_grids.size();
	_hoxel_grids.resize(hoxel_grid_count);
	for (int64_t grid_i = 0; grid_i < hoxel_grid_count; grid_i++) {
		const Dictionary json_hoxel_grid = p_json_hoxel_grids[grid_i];
		// Fill in the grid in place, since copying the hoxel data map of a large grid is expensive.
		HoxelGrid4D &parsed_grid = _hoxel_grids.write[grid_i];
		if (json_hoxel_grid.has("data")) {
			const Array grid_data = json_hoxel_grid["data"];
			const int64_t grid_data_count = grid_data.size();
			for (int64_t grid_data_i = 0; grid_data_i < grid_data_count; grid_data_i++) {
				const Dictionary json_grid_data_entry = grid_data[grid_data_i];
				ERR_CONTINUE_MSG(!json_grid_data_entry.has("i"), "Hox import: Hoxel grid data entry is missing required 'i' coordinate array.");
				Vector4i grid_coord = Vector4D::from_json_array_int(json_grid_data_entry["i"]);
				int64_t mat_index = -1;
				if (json_grid_data_entry.has("m")) {
					mat_index = (int64_t)json_grid_data_entry["m"];
				}
				parsed_grid.data[grid_coord] = mat_index;
			}
		}
		if (json_hoxel_grid.has("dimensions")) {
			parsed_grid.dimensions = _import_parse_vector_int(json_hoxel_grid["dimensions"]);
		}
		// The "rot" properties in .hox files use radians, which matches Godot 4D's Euler4D.
		if (json_hoxel_grid.has("rotWY")) {
			parsed_grid.rotation.wy = (real_t)json_hoxel_grid["rotWY"];
		}
		if (json_hoxel_grid.has("rotXW")) {
			parsed_grid.rotation.xw = (real_t)json_hoxel_grid["rotXW"];
		}
		if (json_hoxel_grid.has("rotXY")) {
			parsed_grid.rotation.xy = (real_t)json_hoxel_grid["rotXY"];
		}
		if (json_hoxel_grid.has("rotYZ")) {
			parsed_grid.rotation.yz = (real_t)json_hoxel_grid["rotYZ"];
		}
		if (json_hoxel_grid.has("rotZW")) {
			parsed_grid.rotation.zw = (real_t)json_hoxel_grid["rotZW"];
		}
		if (json_hoxel_grid.has("rotZX")) {
			parsed_grid.rotation.zx = (real_t)json_hoxel_grid["rotZX"];
		}
		if (json_hoxel_grid.has("translation")) {
			// Unlike the grid's dimensions and hoxel coordinates, the translation is not limited to whole hoxels.
			parsed_grid.translation = _import_parse_vector_float(json_hoxel_grid["translation"]);
		}
	}
}

void HoxDocument4D::_import_parse_materials(const Array &p_json_materials) {
	const int64_t material_count = p_json_materials.size();
	_materials.resize(material_count);
	for (int64_t mat_i = 0; mat_i < material_count; mat_i++) {
		const Dictionary json_material = p_json_materials[mat_i];
		HoxMaterial4D parsed_mat;
		if (json_material.has("albedo")) {
			const Dictionary json_albedo = json_material["albedo"];
			// Read properties alphabetically.
			if (json_albedo.has("b")) {
				parsed_mat.albedo_color.b = (float)json_albedo["b"];
			}
			if (json_albedo.has("g")) {
				parsed_mat.albedo_color.g = (float)json_albedo["g"];
			}
			if (json_albedo.has("r")) {
				parsed_mat.albedo_color.r = (float)json_albedo["r"];
			}
		}
		_materials.set(mat_i, parsed_mat);
	}
}

Error HoxDocument4D::_import_parse_json_data(const Dictionary &p_hox_json) {
	if (p_hox_json.has("materials")) {
		const Array json_materials = p_hox_json["materials"];
		_import_parse_materials(json_materials);
	}
	if (p_hox_json.has("scene")) {
		const Dictionary json_scene = p_hox_json["scene"];
		if (json_scene.has("hoxelGrids")) {
			const Array json_hoxel_grids = json_scene["hoxelGrids"];
			_import_parse_hoxel_grids(json_hoxel_grids);
		}
	}
	if (p_hox_json.has("version")) {
		_hoxel_draw_version = p_hox_json["version"];
	}
	return OK;
}

HoxDocument4D::HoxelTemplate4D HoxDocument4D::_make_hoxel_template() {
	// Derive everything from a unit BoxPolyMesh4D so that the element ordering, and therefore the
	// orientation of each cell, is exactly what the box uses. Corners are identified by which side
	// of each axis they are on, which is what the hoxel grid coordinates need.
	Ref<BoxPolyMesh4D> box_mesh;
	box_mesh.instantiate();
	const PackedVector4Array box_vertices = box_mesh->get_vertex_positions();
	const PackedInt32Array box_edge_indices = box_mesh->get_edge_indices();
	const Vector<Vector<PackedInt32Array>> box_poly_cell_indices = box_mesh->get_poly_cell_indices();
	HoxelTemplate4D box;
	CRASH_COND(box_vertices.size() != 16 || box_poly_cell_indices.size() < 3);
	// Map each box vertex to its corner index.
	PackedInt32Array vertex_corners;
	vertex_corners.resize(16);
	for (int i = 0; i < 16; i++) {
		const Vector4 &v = box_vertices[i];
		vertex_corners.set(i, (v.x > 0 ? 1 : 0) | (v.y > 0 ? 2 : 0) | (v.z > 0 ? 4 : 0) | (v.w > 0 ? 8 : 0));
	}
	// Edges: corners of both ends, plus the spanned axis and minimum corner.
	const int64_t edge_count = box_edge_indices.size() / 2;
	box.edge_corners.resize(edge_count * 2);
	box.edge_min_corner.resize(edge_count);
	box.edge_span_mask.resize(edge_count);
	for (int64_t e = 0; e < edge_count; e++) {
		const int a = vertex_corners[box_edge_indices[e * 2]];
		const int b = vertex_corners[box_edge_indices[e * 2 + 1]];
		box.edge_corners.set(e * 2, a);
		box.edge_corners.set(e * 2 + 1, b);
		box.edge_min_corner.set(e, a & b);
		box.edge_span_mask.set(e, (a | b) & ~(a & b));
	}
	// Faces: the edges they use, plus the spanned axes and minimum corner from all their corners.
	box.face_edges = box_poly_cell_indices[0];
	const int64_t face_count = box.face_edges.size();
	box.face_min_corner.resize(face_count);
	box.face_span_mask.resize(face_count);
	for (int64_t f = 0; f < face_count; f++) {
		int all_and = 15;
		int all_or = 0;
		for (const int32_t e : box.face_edges[f]) {
			all_and &= box.edge_min_corner[e];
			all_or |= box.edge_min_corner[e] | box.edge_span_mask[e];
		}
		box.face_min_corner.set(f, all_and);
		box.face_span_mask.set(f, all_or & ~all_and);
	}
	// Cells: the faces they use, plus which axis they don't span and which side of the hoxel they are on.
	box.cell_faces = box_poly_cell_indices[1];
	const int64_t cell_count = box.cell_faces.size();
	box.cell_min_corner.resize(cell_count);
	box.cell_fixed_axis.resize(cell_count);
	box.cell_positive_side.resize(cell_count);
	for (int64_t cell = 0; cell < cell_count; cell++) {
		int all_and = 15;
		int all_or = 0;
		for (const int32_t f : box.cell_faces[cell]) {
			all_and &= box.face_min_corner[f];
			all_or |= box.face_min_corner[f] | box.face_span_mask[f];
		}
		const int span_mask = all_or & ~all_and;
		int fixed_axis = -1;
		for (int axis = 0; axis < 4; axis++) {
			if (!(span_mask & (1 << axis))) {
				CRASH_COND_MSG(fixed_axis != -1, "HoxDocument4D: Box cell spans fewer than 3 axes.");
				fixed_axis = axis;
			}
		}
		CRASH_COND_MSG(fixed_axis == -1, "HoxDocument4D: Box cell spans all 4 axes.");
		box.cell_min_corner.set(cell, all_and);
		box.cell_fixed_axis.set(cell, fixed_axis);
		box.cell_positive_side.set(cell, (all_and >> fixed_axis) & 1);
	}
	// Volumes: the box is a single 4D volume made of all of its cells.
	box.volume_cells = box_poly_cell_indices[2];
	CRASH_COND(box.volume_cells.size() != 1);
	return box;
}

Vector4i HoxDocument4D::_hoxel_corner_offset(const int p_corner) {
	return Vector4i(p_corner & 1, (p_corner >> 1) & 1, (p_corner >> 2) & 1, (p_corner >> 3) & 1);
}

// Packs a grid-local element identity into a hashable key: its minimum corner (14 bits per axis, offset
// to be non-negative) and the axes it spans (4 bits). Elements of different dimensions never collide
// because their span masks have different popcounts. The key uses 60 bits, so it never touches the sign bit.
int64_t HoxDocument4D::_hoxel_element_key(const Vector4i &p_min_corner, const int p_span_mask) {
	constexpr int64_t offset = 8192;
	int64_t key = 0;
	for (int axis = 0; axis < 4; axis++) {
		key = (key << 14) | (int64_t)(p_min_corner[axis] + offset);
	}
	return (key << 4) | (int64_t)p_span_mask;
}

void HoxDocument4D::_import_generate_grid_hoxel_cells(HashMap<int64_t, HoxSurfaceBuilder4D> &r_builders, const HoxelTemplate4D &p_hoxel_template, const int64_t p_hoxel_grid_index, const bool p_apply_grid_transform, const bool p_merge_materials, const bool p_include_interior) const {
	const HoxelGrid4D &hoxel_grid = _hoxel_grids[p_hoxel_grid_index];
	const Transform4D grid_transform = p_apply_grid_transform ? Transform4D(hoxel_grid.get_grid_rotation(), hoxel_grid.translation) : Transform4D();
	// The element keys are grid-local, and grids have different transforms, so nothing is shared between grids.
	for (KeyValue<int64_t, HoxSurfaceBuilder4D> &kv : r_builders) {
		kv.value.vertex_map.clear();
		kv.value.edge_map.clear();
		kv.value.face_map.clear();
		kv.value.cell_map.clear();
	}
	constexpr int32_t max_abs_coord = 8000; // Leaves room for the +1 corner offset within the 14-bit key fields.
	for (const KeyValue<Vector4i, int64_t> &grid_entry : hoxel_grid.data) {
		const Vector4i &g = grid_entry.key;
		ERR_FAIL_COND_MSG(ABS(g.x) > max_abs_coord || ABS(g.y) > max_abs_coord || ABS(g.z) > max_abs_coord || ABS(g.w) > max_abs_coord, "Hox import: Hoxel grid coordinates are too large to import.");
	}
	PackedInt32Array cell_face_indices;
	cell_face_indices.resize(6);
	PackedInt32Array face_edge_indices;
	face_edge_indices.resize(4);
	PackedInt32Array hoxel_cell_indices;
	hoxel_cell_indices.resize(8);
	PackedInt32Array volume_cell_indices;
	volume_cell_indices.resize(8);
	for (const KeyValue<Vector4i, int64_t> &grid_entry : hoxel_grid.data) {
		const Vector4i &grid_coord = grid_entry.key;
		const int64_t mat_index = grid_entry.value;
		// When merging materials, everything goes into one builder. Its key is arbitrary, and the
		// cell material indices are recorded separately so the caller can still color per cell.
		const int64_t builder_key = p_merge_materials ? 0 : mat_index;
		HoxSurfaceBuilder4D *builder = r_builders.getptr(builder_key);
		if (builder == nullptr) {
			r_builders.insert(builder_key, HoxSurfaceBuilder4D());
			builder = r_builders.getptr(builder_key);
		}
		for (int64_t cell = 0; cell < p_hoxel_template.cell_faces.size(); cell++) {
			int64_t cell_key = 0;
			if (p_include_interior) {
				// Keep every cell. Within a surface, neighboring hoxels share their interior cell,
				// so its two volume references prevent it from rendering. Separate material surfaces
				// intentionally keep their interface cells visible, for transparent materials such as glass.
				cell_key = _hoxel_element_key(grid_coord + _hoxel_corner_offset(p_hoxel_template.cell_min_corner[cell]), 15 & ~(1 << p_hoxel_template.cell_fixed_axis[cell]));
				const int32_t *existing_cell = builder->cell_map.getptr(cell_key);
				if (existing_cell != nullptr) {
					hoxel_cell_indices.set(cell, *existing_cell);
					continue;
				}
			} else {
				// Cull cells between two hoxels: they can never be visible, so don't even store them.
				// This applies regardless of material, since a cell between two different materials is
				// just as buried inside the solid as one between two hoxels of the same material.
				Vector4i neighbor = grid_coord;
				neighbor[p_hoxel_template.cell_fixed_axis[cell]] += p_hoxel_template.cell_positive_side[cell] ? 1 : -1;
				if (hoxel_grid.data.has(neighbor)) {
					continue;
				}
			}
			// This cell is new. Look up or create its faces, edges, and vertices.
			const PackedInt32Array &box_faces = p_hoxel_template.cell_faces[cell];
			for (int64_t f = 0; f < box_faces.size(); f++) {
				const int32_t box_face = box_faces[f];
				const int64_t face_key = _hoxel_element_key(grid_coord + _hoxel_corner_offset(p_hoxel_template.face_min_corner[box_face]), p_hoxel_template.face_span_mask[box_face]);
				const int32_t *existing_face = builder->face_map.getptr(face_key);
				if (existing_face != nullptr) {
					cell_face_indices.set(f, *existing_face);
					continue;
				}
				const PackedInt32Array &box_edges = p_hoxel_template.face_edges[box_face];
				for (int64_t e = 0; e < box_edges.size(); e++) {
					const int32_t box_edge = box_edges[e];
					const int64_t edge_key = _hoxel_element_key(grid_coord + _hoxel_corner_offset(p_hoxel_template.edge_min_corner[box_edge]), p_hoxel_template.edge_span_mask[box_edge]);
					const int32_t *existing_edge = builder->edge_map.getptr(edge_key);
					if (existing_edge != nullptr) {
						face_edge_indices.set(e, *existing_edge);
						continue;
					}
					int32_t edge_vertex_indices[2];
					for (int end = 0; end < 2; end++) {
						const Vector4i corner = grid_coord + _hoxel_corner_offset(p_hoxel_template.edge_corners[box_edge * 2 + end]);
						const int64_t vertex_key = _hoxel_element_key(corner, 0);
						const int32_t *existing_vertex = builder->vertex_map.getptr(vertex_key);
						if (existing_vertex != nullptr) {
							edge_vertex_indices[end] = *existing_vertex;
							continue;
						}
						const int32_t vertex_index = (int32_t)builder->vertices.size();
						const Vector4 position = Vector4(corner);
						builder->vertices.append(p_apply_grid_transform ? grid_transform * position : position);
						builder->vertex_map.insert(vertex_key, vertex_index);
						edge_vertex_indices[end] = vertex_index;
					}
					const int32_t edge_index = (int32_t)(builder->edge_vertex_indices.size() / 2);
					builder->edge_vertex_indices.append(MIN(edge_vertex_indices[0], edge_vertex_indices[1]));
					builder->edge_vertex_indices.append(MAX(edge_vertex_indices[0], edge_vertex_indices[1]));
					builder->edge_map.insert(edge_key, edge_index);
					face_edge_indices.set(e, edge_index);
				}
				const int32_t face_index = (int32_t)builder->faces.size();
				builder->faces.append(face_edge_indices);
				builder->face_map.insert(face_key, face_index);
				cell_face_indices.set(f, face_index);
			}
			const int32_t cell_index = (int32_t)builder->cells.size();
			builder->cells.append(cell_face_indices);
			// The cell's outward normal is along its fixed axis, pointing away from the hoxel it belongs to.
			// This can't be derived from the cell's orientation later, because the shared faces and edges
			// were created in whatever order the neighboring hoxels happened to be visited.
			Vector4 boundary_normal = Vector4();
			boundary_normal[p_hoxel_template.cell_fixed_axis[cell]] = p_hoxel_template.cell_positive_side[cell] ? 1.0f : -1.0f;
			if (p_apply_grid_transform) {
				boundary_normal = grid_transform.basis.xform(boundary_normal).normalized();
			}
			builder->cell_boundary_normals.append(boundary_normal);
			if (p_merge_materials) {
				builder->cell_material_indices.append((int32_t)mat_index);
			}
			if (p_include_interior) {
				builder->cell_map.insert(cell_key, cell_index);
				hoxel_cell_indices.set(cell, cell_index);
			}
		}
		if (p_include_interior) {
			// The hoxel itself is a 4D volume made of its 8 cells, in the box's order.
			const PackedInt32Array &box_volume_cells = p_hoxel_template.volume_cells[0];
			for (int64_t i = 0; i < box_volume_cells.size(); i++) {
				volume_cell_indices.set(i, hoxel_cell_indices[box_volume_cells[i]]);
			}
			builder->volumes.append(volume_cell_indices);
		}
	}
}

Ref<ArrayPolyMesh4D> HoxDocument4D::_import_generate_poly_mesh_4d_from_builder(const HoxSurfaceBuilder4D &p_builder) {
	// Nothing needs deduplicating: the builder already shared elements between hoxels.
	// Volumes are only present when interior geometry was included, otherwise the mesh is hollow.
	Ref<ArrayPolyMesh4D> poly_mesh;
	poly_mesh.instantiate();
	poly_mesh->set_poly_cell_vertex_positions(p_builder.vertices);
	poly_mesh->set_edge_vertex_indices(p_builder.edge_vertex_indices);
	Vector<Vector<PackedInt32Array>> poly_cell_indices = { p_builder.faces, p_builder.cells };
	if (!p_builder.volumes.is_empty()) {
		poly_cell_indices.append(p_builder.volumes);
	}
	poly_mesh->set_poly_cell_indices(poly_cell_indices);
	// Flip any cells whose orientation faces into their hoxel, so that the cell structure and the boundary
	// normals both agree on the outward side, which rendering and tetrahedral decomposition rely on.
	poly_mesh->orient_cells_to_boundary_normals(p_builder.cell_boundary_normals);
	return poly_mesh;
}

Ref<MultiSurfaceMesh4D> HoxDocument4D::_import_generate_multi_surface_mesh_4d_from_builders(const HashMap<int64_t, HoxSurfaceBuilder4D> &p_builders, const HoxMeshFormat p_mesh_format) const {
	Vector<Ref<SingleSurfaceMesh4D>> surface_meshes;
	for (const KeyValue<int64_t, HoxSurfaceBuilder4D> &mat_builder : p_builders) {
		const int64_t mat_index = mat_builder.key;
		const Ref<ArrayPolyMesh4D> poly_mesh = _import_generate_poly_mesh_4d_from_builder(mat_builder.value);
		Ref<SingleSurfaceMesh4D> surface_mesh = poly_mesh;
		if (p_mesh_format == HOX_MESH_FORMAT_TETRAHEDRAL) {
			// Decompose the boundary cells into tetrahedra now instead of at load time. This keeps
			// only the renderable simplexes, so it is smaller and faster to load, but loses the
			// polyhedral structure (including any interior cells and 4D volumes).
			surface_mesh = poly_mesh->to_array_tetra_mesh();
		}
		surface_mesh->set_name(mat_index < 0 ? String("SurfaceForEmptyMaterial") : String("SurfaceForMaterial") + String::num_int64(mat_index));
		if (0 <= mat_index && mat_index < _materials.size()) {
			const HoxMaterial4D &hox_material = _materials[mat_index];
			Ref<PolyMaterial4D> poly_material;
			poly_material.instantiate();
			poly_material->set_albedo_color(hox_material.albedo_color);
			poly_material->set_name(String("Material") + String::num_int64(mat_index));
			surface_mesh->set_material(poly_material);
		}
		surface_meshes.append(surface_mesh);
	}
	Ref<MultiSurfaceMesh4D> multi_surface_mesh;
	multi_surface_mesh.instantiate();
	multi_surface_mesh->set_surface_meshes(surface_meshes);
	multi_surface_mesh->set_name(_hox_filename);
	return multi_surface_mesh;
}

Ref<HoxDocument4D> HoxDocument4D::import_read_from_byte_array(const PackedByteArray &p_data, const String &p_filename) {
	Ref<HoxDocument4D> ret;
	// Expect the hox file to at least contain enough JSON to have a version string.
	ERR_FAIL_COND_V_MSG(p_data.size() < 12, ret, "Hox import: Byte array is too small to be a valid hox file.");
	const String json_string = String::utf8(reinterpret_cast<const char *>(p_data.ptr()), p_data.size());
	ERR_FAIL_COND_V_MSG(json_string.length() < 12, ret, "Hox import: String is too small to be a valid hox file.");
	const Dictionary hox_json = JSON::parse_string(json_string);
	ERR_FAIL_COND_V_MSG(hox_json.is_empty(), ret, "Hox import: Failed to parse hox JSON data from byte array.");
	ret.instantiate();
	Error err = ret->_import_parse_json_data(hox_json);
	ERR_FAIL_COND_V_MSG(err != OK, Ref<HoxDocument4D>(), "Hox import: Failed to parse hox JSON data from byte array.");
	ret->_hox_filename = p_filename;
	return ret;
}

Ref<HoxDocument4D> HoxDocument4D::import_read_from_file(const String &p_path) {
	Ref<HoxDocument4D> ret;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), ret, "Hox import: Failed to open file for reading.");
	// Expect the hox file to at least contain enough JSON to have a version string.
	ERR_FAIL_COND_V_MSG(file->get_length() < 12, ret, "Hox import: File is too small to be a valid hox file.");
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
	Dictionary hox_json = JSON::parse_string(file->get_as_text(true));
#else
	Dictionary hox_json = JSON::parse_string(file->get_as_text());
#endif
	ERR_FAIL_COND_V_MSG(hox_json.is_empty(), ret, "Hox import: Failed to parse hox JSON file at: " + p_path);
	ret.instantiate();
	Error err = ret->_import_parse_json_data(hox_json);
	ERR_FAIL_COND_V_MSG(err != OK, Ref<HoxDocument4D>(), "Hox import: Failed to parse hox JSON data at: " + p_path);
	ret->_hox_filename = p_path.get_file();
	return ret;
}

Ref<MultiSurfaceMesh4D> HoxDocument4D::import_generate_multi_surface_mesh_4d(const bool p_include_interior, const HoxMeshFormat p_mesh_format) const {
	HashMap<int64_t, HoxSurfaceBuilder4D> builders;
	const HoxelTemplate4D hoxel_template = _make_hoxel_template();
	for (int64_t hoxel_grid_index = 0; hoxel_grid_index < _hoxel_grids.size(); hoxel_grid_index++) {
		_import_generate_grid_hoxel_cells(builders, hoxel_template, hoxel_grid_index, true, false, p_include_interior);
	}
	Ref<MultiSurfaceMesh4D> multi_surface_mesh_4d = _import_generate_multi_surface_mesh_4d_from_builders(builders, p_mesh_format);
	const bool valid = multi_surface_mesh_4d->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!valid, Ref<MultiSurfaceMesh4D>(), "Hox import: Generated multi-surface mesh is invalid.");
	return multi_surface_mesh_4d;
}

Ref<ArrayPolyMesh4D> HoxDocument4D::import_generate_poly_mesh_4d(const bool p_include_interior) const {
	// Generate one surface for all materials, and remember each cell's material.
	HashMap<int64_t, HoxSurfaceBuilder4D> builders;
	const HoxelTemplate4D hoxel_template = _make_hoxel_template();
	for (int64_t hoxel_grid_index = 0; hoxel_grid_index < _hoxel_grids.size(); hoxel_grid_index++) {
		_import_generate_grid_hoxel_cells(builders, hoxel_template, hoxel_grid_index, true, true, p_include_interior);
	}
	const HoxSurfaceBuilder4D *builder = builders.getptr(0);
	const HoxSurfaceBuilder4D empty_builder;
	if (builder == nullptr) {
		builder = &empty_builder;
	}
	Ref<ArrayPolyMesh4D> ret = _import_generate_poly_mesh_4d_from_builder(*builder);
	// This function needs to return an ArrayPolyMesh4D, which means one surface, one material.
	// Create an image to represent the colors for each material in a tiled layout, and point each
	// cell at the center of its material's tile. To avoid texture bleeding, each tile is 4x4 pixels.
	HashMap<int64_t, int32_t> material_to_tile;
	for (const int32_t mat_index : builder->cell_material_indices) {
		if (!material_to_tile.has(mat_index)) {
			material_to_tile.insert(mat_index, (int32_t)material_to_tile.size());
		}
	}
	const int64_t tile_count = MAX((int64_t)1, (int64_t)material_to_tile.size());
	const int64_t image_tile_width = Math::ceil(Math::sqrt((double)tile_count));
	const int64_t image_tile_height = Math::ceil((double)tile_count / (double)image_tile_width);
	const int64_t image_pixel_width = image_tile_width * 4;
	const int64_t image_pixel_height = image_tile_height * 4;
	Ref<Image> image = Image::create_empty(image_pixel_width, image_pixel_height, false, Image::FORMAT_RGBA8);
	PackedVector3Array tile_texture_coordinates;
	tile_texture_coordinates.resize(material_to_tile.size());
	for (const KeyValue<int64_t, int32_t> &mat_tile : material_to_tile) {
		const int64_t mat_index = mat_tile.key;
		const int64_t tile_x = mat_tile.value % image_tile_width;
		const int64_t tile_y = mat_tile.value / image_tile_width;
		const Color albedo_color = (0 <= mat_index && mat_index < _materials.size()) ? _materials[mat_index].albedo_color : Color(1, 1, 1, 1);
		image->fill_rect(Rect2i(tile_x * 4, tile_y * 4, 4, 4), albedo_color);
		tile_texture_coordinates.set(mat_tile.value, Vector3((tile_x * 4 + 2) / (double)image_pixel_width, (tile_y * 4 + 2) / (double)image_pixel_height, 0.5));
	}
	PackedInt32Array per_cell_texture_map_indices;
	per_cell_texture_map_indices.resize(builder->cell_material_indices.size());
	for (int64_t cell = 0; cell < builder->cell_material_indices.size(); cell++) {
		per_cell_texture_map_indices.set(cell, material_to_tile[builder->cell_material_indices[cell]]);
	}
	ret->set_poly_cell_texture_map_values(tile_texture_coordinates);
	HashMap<Vector2i, Vector<PackedInt32Array>> all_poly_cell_texture_map_indices;
	all_poly_cell_texture_map_indices.insert(PolyMesh4D::PER_CELL_KEY, Vector<PackedInt32Array>{ per_cell_texture_map_indices });
	ret->set_all_poly_cell_texture_map_indices(all_poly_cell_texture_map_indices);
	// Create a Texture3D for the albedo / base color.
#if GDEXTENSION
	TypedArray<Image> albedo_images;
#elif GODOT_MODULE
	Vector<Ref<Image>> albedo_images;
#endif
	albedo_images.append(image);
	Ref<ImageTexture3D> albedo_texture_3d;
	albedo_texture_3d.instantiate();
	albedo_texture_3d->create(Image::FORMAT_RGBA8, image_pixel_width, image_pixel_height, 1, false, albedo_images);
	albedo_texture_3d->set_name(_hox_filename + String("_albedo"));
	// Create a 4D material using the Texture3D.
	Ref<PolyMaterial4D> poly_material;
	poly_material.instantiate();
	poly_material->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_TEXTURE3D_CELL_UVW_ONLY);
	poly_material->set_albedo_texture_3d(albedo_texture_3d);
	poly_material->set_name(_hox_filename + String("_material"));
	ret->set_material(poly_material);
	ret->set_name(_hox_filename);
	const bool valid = ret->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!valid, Ref<ArrayPolyMesh4D>(), "Hox import: Generated single-surface mesh is invalid.");
	return ret;
}

Node4D *HoxDocument4D::import_generate_scene(const bool p_include_interior, const HoxMeshFormat p_mesh_format) const {
	const int64_t hoxel_grid_count = _hoxel_grids.size();
	const HoxelTemplate4D hoxel_template = _make_hoxel_template();
	Node4D *scene_root = nullptr;
	if (hoxel_grid_count == 1) {
		HashMap<int64_t, HoxSurfaceBuilder4D> builders;
		_import_generate_grid_hoxel_cells(builders, hoxel_template, 0, true, false, p_include_interior);
		Ref<MultiSurfaceMesh4D> multi_surface_mesh = _import_generate_multi_surface_mesh_4d_from_builders(builders, p_mesh_format);
		const bool valid = multi_surface_mesh->is_mesh_data_valid();
		ERR_FAIL_COND_V_MSG(!valid, nullptr, "Hox import: Generated multi-surface mesh is invalid.");
		// When the file has only one grid, just use a single MeshInstance4D as the root node.
		// This means that the transform of the grid is applied during generation time (the first `true` above).
		MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
		mesh_instance->set_mesh(multi_surface_mesh);
		scene_root = mesh_instance;
	} else {
		scene_root = memnew(Node4D);
		for (int64_t hoxel_grid_index = 0; hoxel_grid_index < hoxel_grid_count; hoxel_grid_index++) {
			HashMap<int64_t, HoxSurfaceBuilder4D> builders;
			_import_generate_grid_hoxel_cells(builders, hoxel_template, hoxel_grid_index, false, false, p_include_interior);
			Ref<MultiSurfaceMesh4D> multi_surface_mesh = _import_generate_multi_surface_mesh_4d_from_builders(builders, p_mesh_format);
			if (!multi_surface_mesh->is_mesh_data_valid()) {
				memdelete(scene_root); // Also frees the children generated so far.
				ERR_FAIL_V_MSG(nullptr, "Hox import: Generated multi-surface mesh for hoxel grid " + itos(hoxel_grid_index) + " is invalid.");
			}
			MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
			mesh_instance->set_name("HoxelGrid" + itos(hoxel_grid_index));
			mesh_instance->set_mesh(multi_surface_mesh);
			const Transform4D grid_transform = Transform4D(_hoxel_grids[hoxel_grid_index].get_grid_rotation(), _hoxel_grids[hoxel_grid_index].translation);
			mesh_instance->set_transform(grid_transform);
			scene_root->add_child(mesh_instance);
			// Packing a scene only includes nodes owned by the root.
			mesh_instance->set_owner(scene_root);
		}
	}
	// Nodes should be PascalCase: `my_model.hox` -> `MyModel`.
	scene_root->set_name(_hox_filename.get_basename().to_pascal_case());
	return scene_root;
}

void HoxDocument4D::_bind_methods() {
	ClassDB::bind_static_method("HoxDocument4D", D_METHOD("import_read_from_byte_array", "data", "filename"), &HoxDocument4D::import_read_from_byte_array);
	ClassDB::bind_static_method("HoxDocument4D", D_METHOD("import_read_from_file", "path"), &HoxDocument4D::import_read_from_file);
	ClassDB::bind_method(D_METHOD("import_generate_multi_surface_mesh_4d", "include_interior", "mesh_format"), &HoxDocument4D::import_generate_multi_surface_mesh_4d, DEFVAL(false), DEFVAL(HOX_MESH_FORMAT_POLYTOPE));
	ClassDB::bind_method(D_METHOD("import_generate_poly_mesh_4d", "include_interior"), &HoxDocument4D::import_generate_poly_mesh_4d, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("import_generate_scene", "include_interior", "mesh_format"), &HoxDocument4D::import_generate_scene, DEFVAL(false), DEFVAL(HOX_MESH_FORMAT_POLYTOPE));

	BIND_ENUM_CONSTANT(HOX_MESH_FORMAT_POLYTOPE);
	BIND_ENUM_CONSTANT(HOX_MESH_FORMAT_TETRAHEDRAL);
}
