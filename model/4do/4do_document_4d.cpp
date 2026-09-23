#include "4do_document_4d.h"

#include "../../math/math_4d.h"
#include "../../math/vector_4d.h"
#include "../mesh/mesh_instance_4d.h"
#include "../mesh/multi_surface_mesh_4d.h"
#include "../mesh/poly/poly_material_4d.h"
#include "../mesh/wire/wire_material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/file_access.hpp>
#elif GODOT_MODULE
#include "core/io/file_access.h"
#endif

FourDODocument4D::FourDOVertexInstance4D FourDODocument4D::_import_parse_vertex_instance(const String &p_vertex_instance_string, FourDOSurface4D &r_for_surface) const {
	FourDOVertexInstance4D vertex_instance = FourDOVertexInstance4D();
	// Keep empty pieces so that positional elements stay aligned with the vertex format, like OBJ's `1//3`.
	const PackedStringArray vertex_instance_pieces = p_vertex_instance_string.split("/", true);
	const int64_t vertex_instance_piece_count = vertex_instance_pieces.size();
	ERR_FAIL_COND_V_MSG(vertex_instance_piece_count < 1, vertex_instance, "4DO import: Error: Vertex instance token has too few pieces.");
	for (int64_t i = 0; i < vertex_instance_piece_count; i++) {
		const String vertex_instance_token = vertex_instance_pieces[i];
		if (vertex_instance_token.is_empty()) {
			continue; // Missing data for this element, leave it as -1.
		} else if (vertex_instance_token.begins_with("vn")) {
			vertex_instance.vn = vertex_instance_token.substr(2, vertex_instance_token.length() - 2).to_int();
			r_for_surface.has_any_normal_indices = true;
		} else if (vertex_instance_token.begins_with("vt")) {
			vertex_instance.vt = vertex_instance_token.substr(2, vertex_instance_token.length() - 2).to_int();
			r_for_surface.has_any_texture_map_indices = true;
		} else if (vertex_instance_token.begins_with("vc")) {
			vertex_instance.vc = vertex_instance_token.substr(2, vertex_instance_token.length() - 2).to_int();
			r_for_surface.has_any_color_indices = true;
		} else if (vertex_instance_token.begins_with("v")) {
			vertex_instance.v = vertex_instance_token.substr(1, vertex_instance_token.length() - 1).to_int();
		} else {
			// Parse positional elements in vertex instances by matching the index to the vertex format.
			ERR_FAIL_COND_V_MSG(i >= _vertex_format.size(), vertex_instance, "4DO import: Error: Vertex instance token has too many positional elements for the vertex format.");
			const VertexIndexType vertex_index_type = _vertex_format[i];
			switch (vertex_index_type) {
				case VERTEX_INDEX_TYPE_POSITION:
					vertex_instance.v = vertex_instance_token.to_int();
					break;
				case VERTEX_INDEX_TYPE_NORMAL:
					vertex_instance.vn = vertex_instance_token.to_int();
					r_for_surface.has_any_normal_indices = true;
					break;
				case VERTEX_INDEX_TYPE_TEXTURE_COORDINATE:
					vertex_instance.vt = vertex_instance_token.to_int();
					r_for_surface.has_any_texture_map_indices = true;
					break;
				case VERTEX_INDEX_TYPE_COLOR:
					vertex_instance.vc = vertex_instance_token.to_int();
					r_for_surface.has_any_color_indices = true;
					break;
				case VERTEX_INDEX_TYPE_UNKNOWN:
					// Do nothing, unknown vertex index type.
					break;
			}
		}
	}
	return vertex_instance;
}

Error FourDODocument4D::_import_parse_4do_raw_text(const String &p_raw_text) {
	// Fallback vertex format in case none is specified.
	_vertex_format = { VERTEX_INDEX_TYPE_POSITION };
	// Read the lines of the file, ignoring comments and empty lines.
	// Keep empty lines so that the reported line numbers match the file.
	const PackedStringArray lines = p_raw_text.split("\n", true);
	FourDOSurface4D current_surface = FourDOSurface4D();
	FourDOPolylineSurface4D current_polyline_surface = FourDOPolylineSurface4D();
	String current_surface_name = "";
	for (int64_t line_index = 0; line_index < lines.size(); line_index++) {
		String line = lines[line_index];
		const int line_comment_index = line.find("#");
		if (line_comment_index >= 0) {
			line = line.substr(0, line_comment_index);
		}
		line = line.strip_edges();
		if (line.is_empty()) {
			continue;
		}
		const PackedStringArray line_tokens = line.split(" ", false);
		const int64_t line_token_count = line_tokens.size();
		if (line_token_count < 2) {
			WARN_PRINT("4DO import: Line " + itos(line_index + 1) + " has too few tokens and will be ignored: " + line);
			continue;
		}
		const String command_token = line_tokens[0];
		if (command_token == "4DO") {
			_4do_version = line_tokens[1].to_int();
		} else if (_4do_version < 0) {
			ERR_FAIL_V_MSG(ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " is not a valid 4DO file. The first non-comment line must be the 4DO version.");
		} else if (command_token == "mtllib") {
			// The filename may contain spaces, in which case it is quoted, so rejoin the tokens and strip the quotes.
			String material_filename = line_tokens[1];
			for (int64_t i = 2; i < line_token_count; i++) {
				material_filename += " " + line_tokens[i];
			}
			if (material_filename.length() >= 2 && material_filename.begins_with("\"") && material_filename.ends_with("\"")) {
				material_filename = material_filename.substr(1, material_filename.length() - 2);
			}
			ERR_FAIL_COND_V_MSG(material_filename.is_empty(), ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has an empty material library filename.");
			_material_filenames.append(material_filename);
			// Skip reading material library files when base path is empty (such as reading from byte arrays),
			// where the material library may be provided by other mechanisms, or simply not be available.
			if (!_4do_base_path.is_empty()) {
				const String material_file_path = _4do_base_path.path_join(material_filename);
				const Error err = import_read_materials_from_file(material_file_path);
				// In most cases, error on invalid files, but for missing materials, continue without them.
				if (err != OK) {
					WARN_PRINT("4DO import: Error: Line " + itos(line_index + 1) + " failed to read material library file: " + material_file_path + ". Continuing without these materials.");
				}
			}
		} else if (command_token == "usemtl") {
			const String material_name = line_tokens[1];
			if (!current_surface.is_empty()) {
				_surfaces[current_surface_name] = current_surface;
			}
			if (!current_polyline_surface.polylines.is_empty()) {
				_polyline_surfaces[current_surface_name] = current_polyline_surface;
			}
			current_surface_name = material_name;
			// A material may be used again later in the file, so resume its surfaces. The solid and polyline
			// surfaces are stored separately, so check each one on its own.
			current_surface = _surfaces.has(current_surface_name) ? _surfaces[current_surface_name] : FourDOSurface4D();
			current_polyline_surface = _polyline_surfaces.has(current_surface_name) ? _polyline_surfaces[current_surface_name] : FourDOPolylineSurface4D();
		} else if (command_token == "cellformat" || command_token == "tformat" || command_token == "plformat") {
			// Version 1 cell-level formats, replaced by `vtxformat` in version 2, but HoxelDraw still writes them.
			// The only cell-level datum we care about is a leading color index, denoted by "co" (or "vc").
			for (int64_t token_index = 1; token_index < line_token_count; token_index++) {
				for (const String &piece : line_tokens[token_index].split("/", false)) {
					if (piece == "co" || piece == "vc") {
						_cell_format_has_color_index = true;
					}
				}
			}
		} else if (command_token == "vtxformat") {
			const PackedStringArray vertex_format_pieces = line_tokens[1].split("/", false);
			ERR_FAIL_COND_V_MSG(vertex_format_pieces.size() < 1, ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has too few tokens for vertex format.");
			_vertex_format.clear();
			for (int64_t piece_index = 0; piece_index < vertex_format_pieces.size(); piece_index++) {
				const String vertex_format_piece = vertex_format_pieces[piece_index];
				if (vertex_format_piece == "v") {
					_vertex_format.append(VERTEX_INDEX_TYPE_POSITION);
				} else if (vertex_format_piece == "vn") {
					_vertex_format.append(VERTEX_INDEX_TYPE_NORMAL);
				} else if (vertex_format_piece == "vt") {
					_vertex_format.append(VERTEX_INDEX_TYPE_TEXTURE_COORDINATE);
				} else if (vertex_format_piece == "vc") {
					_vertex_format.append(VERTEX_INDEX_TYPE_COLOR);
				} else {
					ERR_FAIL_V_MSG(ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has an unrecognized vertex format piece: " + vertex_format_piece);
				}
			}
		} else if (command_token == "v") {
			// Vertex position.
			ERR_FAIL_COND_V_MSG(line_token_count < 5, ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has too few tokens for vertex.");
			_vertex_positions.append(Vector4(line_tokens[1].to_float(), line_tokens[2].to_float(), line_tokens[3].to_float(), line_tokens[4].to_float()));
		} else if (command_token == "vn") {
			// Vertex normal.
			ERR_FAIL_COND_V_MSG(line_token_count < 5, ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has too few tokens for vertex normal.");
			_vertex_normals.append(Vector4(line_tokens[1].to_float(), line_tokens[2].to_float(), line_tokens[3].to_float(), line_tokens[4].to_float()));
		} else if (command_token == "vt") {
			// Vertex texture coordinate.
			ERR_FAIL_COND_V_MSG(line_token_count < 4, ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has too few tokens for vertex texture coordinate.");
			_vertex_texture_coordinates.append(Vector3(line_tokens[1].to_float(), line_tokens[2].to_float(), line_tokens[3].to_float()));
		} else if (command_token == "vc" || command_token == "co") {
			// Vertex color.
			ERR_FAIL_COND_V_MSG(line_token_count < 2, ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has too few tokens for vertex color.");
			Color vertex_color;
			if (line_token_count < 4) {
				vertex_color = Color::html(line_tokens[1].trim_prefix("0x"));
			} else {
				const int64_t alpha = line_token_count < 5 ? 255 : line_tokens[4].to_int();
#if GDEXTENSION || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 4)
				vertex_color = Color(line_tokens[1].to_int() / 255.0f, line_tokens[2].to_int() / 255.0f, line_tokens[3].to_int() / 255.0f, alpha / 255.0f);
#elif GODOT_MODULE
				vertex_color = Color::from_rgba8(line_tokens[1].to_int(), line_tokens[2].to_int(), line_tokens[3].to_int(), alpha);
#endif
			}
			_vertex_colors.append(vertex_color);
		} else if (command_token == "c") {
			if (_4do_version < 2) {
				// In version 1, `c` was a cell group listing tetrahedron indices, which adds no geometry. Skip it.
				WARN_PRINT_ONCE("4DO import: This version 1 file contains cell groups, which are not supported and will be ignored.");
				continue;
			}
			// Cubic cell, made of vertices, optionally preceded by a color index.
			const bool has_color_index = _cell_format_has_color_index || line_token_count == 10;
			const int64_t vert_start_index = has_color_index ? 2 : 1;
			ERR_FAIL_COND_V_MSG(line_token_count < vert_start_index + 8, ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has too few tokens for cubic cell.");
			FourDOCubicCell4D cell;
			if (has_color_index) {
				cell.color_index = line_tokens[1].to_int();
			}
			for (int64_t i = 0; i < 8; i++) {
				cell.verts[i] = _import_parse_vertex_instance(line_tokens[i + vert_start_index], current_surface);
			}
			ERR_FAIL_COND_V_MSG(_has_repeated_vertex(cell.verts, 8), ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has a degenerate cubic cell that repeats a vertex.");
			current_surface.cubic_cells.append(cell);
		} else if (command_token == "t") {
			// Tetrahedral cell, made of vertices, optionally preceded by a color index.
			const bool has_color_index = _cell_format_has_color_index || line_token_count == 6;
			const int64_t vert_start_index = has_color_index ? 2 : 1;
			ERR_FAIL_COND_V_MSG(line_token_count < vert_start_index + 4, ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has too few tokens for tetrahedral cell.");
			FourDOTetrahedralCell4D cell;
			if (has_color_index) {
				cell.color_index = line_tokens[1].to_int();
			}
			for (int64_t i = 0; i < 4; i++) {
				cell.verts[i] = _import_parse_vertex_instance(line_tokens[i + vert_start_index], current_surface);
			}
			ERR_FAIL_COND_V_MSG(_has_repeated_vertex(cell.verts, 4), ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(line_index + 1) + " has a degenerate tetrahedral cell that repeats a vertex.");
			current_surface.tetrahedral_cells.append(cell);
		} else if (command_token == "f") {
			// Face, made of vertices.
			Vector<FourDOVertexInstance4D> this_face_vertex_indices;
			for (int64_t i = 1; i < line_token_count; i++) {
				const String vertex_instance_string = line_tokens[i];
				this_face_vertex_indices.append(_import_parse_vertex_instance(vertex_instance_string, current_surface));
			}
			_face_vertex_indices.append(this_face_vertex_indices);
		} else if (command_token == "pl" || (_4do_version == 1 && command_token == "p")) {
			// Polyline, made of vertices, optionally preceded by a color index (which polylines have no use for).
			const int64_t vert_start_index = _cell_format_has_color_index ? 2 : 1;
			Vector<FourDOVertexInstance4D> this_polyline;
			for (int64_t i = vert_start_index; i < line_token_count; i++) {
				const String vertex_instance_string = line_tokens[i];
				this_polyline.append(_import_parse_vertex_instance(vertex_instance_string, current_surface));
			}
			current_polyline_surface.polylines.append(this_polyline);
		} else if (command_token == "p") {
			// Polyhedron, made of faces. A negated face index means the face is reversed for this polyhedron,
			// which the poly mesh handles itself based on the cell structure, so only the magnitude is needed.
			PackedInt32Array this_polyhedron_face_indices;
			for (int64_t i = 1; i < line_token_count; i++) {
				const int32_t face_index = Math::abs((int32_t)line_tokens[i].to_int());
				this_polyhedron_face_indices.append(face_index);
			}
			current_surface.polyhedron_face_indices.append(this_polyhedron_face_indices);
		} else if (command_token == "orient") {
			// HoxelDraw does not write this, and its files are Z-up despite the specification's Y-up default,
			// so the coordinate system is left as authored and the editor import plugins convert Z-up to Y-up.
			WARN_PRINT_ONCE("4DO import: The 'orient' command is not supported. The file's coordinate system is imported as-is, use the import option to convert Z-up to Y-up if needed.");
		} else {
			// The specification allows parsers to disregard data they do not understand, as long as the user is informed.
			WARN_PRINT("4DO import: Line " + itos(line_index + 1) + " has an unrecognized command token and will be ignored: " + command_token);
		}
	}
	// Commit the last surface if it has any content.
	if (!current_surface.is_empty()) {
		_surfaces[current_surface_name] = current_surface;
	}
	if (!current_polyline_surface.polylines.is_empty()) {
		_polyline_surfaces[current_surface_name] = current_polyline_surface;
	}
	return OK;
}

Error FourDODocument4D::_import_parse_material_raw_text(const String &p_material_raw_text) {
	// Keep empty lines so that the reported line numbers match the file.
	const PackedStringArray lines = p_material_raw_text.split("\n", true);
	FourDOMaterial4D current_material = FourDOMaterial4D();
	String current_material_name = "";
	for (int64_t i = 0; i < lines.size(); i++) {
		String line = lines[i];
		const int line_comment_index = line.find("#");
		if (line_comment_index >= 0) {
			line = line.substr(0, line_comment_index);
		}
		line = line.strip_edges();
		if (line.is_empty()) {
			continue;
		}
		const PackedStringArray line_tokens = line.split(" ", false);
		const int64_t line_token_count = line_tokens.size();
		if (line_token_count < 2) {
			WARN_PRINT("4DO import: Material line " + itos(i + 1) + " has too few tokens and will be ignored: " + line);
			continue;
		}
		const String command_token = line_tokens[0];
		if (command_token == "newmtl") {
			if (!current_material_name.is_empty()) {
				_materials[current_material_name] = current_material;
			}
			current_material = FourDOMaterial4D();
			current_material_name = line_tokens[1];
		} else if (command_token == "baseColorFactor") {
			ERR_FAIL_COND_V_MSG(line_token_count < 4, ERR_PARSE_ERROR, "4DO import: Error: Line " + itos(i + 1) + " has too few tokens for baseColorFactor.");
			current_material.base_color_factor.r = line_tokens[1].to_float();
			current_material.base_color_factor.g = line_tokens[2].to_float();
			current_material.base_color_factor.b = line_tokens[3].to_float();
			if (line_token_count > 4) {
				current_material.base_color_factor.a = line_tokens[4].to_float();
			}
		} else if (command_token == "metallicFactor") {
			current_material.metallic_factor = line_tokens[1].to_float();
		} else if (command_token == "roughnessFactor") {
			current_material.roughness_factor = line_tokens[1].to_float();
		} else {
			// Texture maps and other properties are not supported yet, so skip them rather than rejecting the whole library.
			WARN_PRINT_ONCE("4DO import: Material line " + itos(i + 1) + " has an unsupported or unrecognized command token and will be ignored: " + command_token);
		}
	}
	// Commit the last material, always, even if the name is empty, to support uniform material handling for the whole document.
	_materials[current_material_name] = current_material;
	return OK;
}

bool FourDODocument4D::_has_repeated_vertex(const FourDOVertexInstance4D *p_first_vert_inst, const int64_t p_vert_count) {
	for (int64_t i = 1; i < p_vert_count; i++) {
		for (int64_t j = 0; j < i; j++) {
			if (p_first_vert_inst[i].v == p_first_vert_inst[j].v) {
				return true;
			}
		}
	}
	return false;
}

void FourDODocument4D::_remap_vert_indices_for_poly(const FourDODocument4D::FourDOVertexInstance4D *p_first_vert_inst, const int64_t p_vert_count, const Vector<PackedInt32Array> &p_boundary_cell_vertex_indices, const int64_t p_poly_cell_index, Vector<PackedInt32Array> &r_cell_normal_indices, Vector<PackedInt32Array> &r_cell_texture_map_indices, const bool p_has_any_normal_indices, bool p_has_any_texture_map_indices) {
	const PackedInt32Array &tet_vertex_indices = p_boundary_cell_vertex_indices[p_poly_cell_index];
	CRASH_COND(tet_vertex_indices.size() != p_vert_count);
	PackedInt32Array remapped_tet_normal_indices;
	PackedInt32Array remapped_tet_texture_map_indices;
	if (p_has_any_normal_indices) {
		// Start with the existing normal indices to preserve flat shading.
		remapped_tet_normal_indices = r_cell_normal_indices[p_poly_cell_index];
	}
	if (p_has_any_texture_map_indices) {
		remapped_tet_texture_map_indices.resize(p_vert_count);
	}
	for (int64_t i = 0; i < p_vert_count; i++) {
		const int64_t dest = tet_vertex_indices.find(p_first_vert_inst[i].v);
		if (dest >= 0) {
			if (p_has_any_normal_indices) {
				const int32_t vn = p_first_vert_inst[i].vn;
				if (vn >= 0) {
					remapped_tet_normal_indices.set(dest, vn);
				}
			}
			if (p_has_any_texture_map_indices) {
				const int32_t vt = p_first_vert_inst[i].vt;
				if (vt >= 0) {
					remapped_tet_texture_map_indices.set(dest, vt);
				} else {
					// For texture maps only: Use an empty array to indicate an unmapped cell.
					// If even one vertex has no valid texture map index, we consider the whole cell unmapped.
					remapped_tet_texture_map_indices.clear();
					p_has_any_texture_map_indices = false;
				}
			}
		}
	}
	if (p_has_any_normal_indices) {
		r_cell_normal_indices.set(p_poly_cell_index, remapped_tet_normal_indices);
	}
	if (p_has_any_texture_map_indices) {
		r_cell_texture_map_indices.set(p_poly_cell_index, remapped_tet_texture_map_indices);
	}
}

void FourDODocument4D::_import_gather_cell_color(const int32_t p_color_index, const int64_t p_cell_index, const int64_t p_cell_count, PackedColorArray &r_per_cell_colors) const {
	if (p_color_index < 0) {
		return; // This cell has no color, which is normal for version 2 files.
	}
	if (r_per_cell_colors.is_empty()) {
		// Cells without a color are white, which lets the material's base color show through.
		r_per_cell_colors.resize(p_cell_count);
		r_per_cell_colors.fill(Color(1, 1, 1, 1));
	}
	if (p_color_index >= _vertex_colors.size()) {
		WARN_PRINT_ONCE("4DO import: A cell references a color index that is out of range. It will be white instead.");
		return;
	}
	r_per_cell_colors.set(p_cell_index, _vertex_colors[p_color_index]);
}

void FourDODocument4D::_import_gather_vertex_colors(const FourDOVertexInstance4D *p_first_vert_inst, const int64_t p_vert_count, PackedColorArray &r_per_vertex_colors) const {
	for (int64_t i = 0; i < p_vert_count; i++) {
		const FourDOVertexInstance4D &vert_inst = p_first_vert_inst[i];
		if (vert_inst.vc < 0 || vert_inst.v < 0 || vert_inst.v >= _vertex_positions.size()) {
			continue;
		}
		if (r_per_vertex_colors.is_empty()) {
			// Vertices without a color are white, which lets the material's base color show through.
			r_per_vertex_colors.resize(_vertex_positions.size());
			r_per_vertex_colors.fill(Color(1, 1, 1, 1));
		}
		if (vert_inst.vc >= _vertex_colors.size()) {
			WARN_PRINT_ONCE("4DO import: A vertex references a color index that is out of range. It will be white instead.");
			continue;
		}
		// Godot 4D colors are per vertex position, so if a vertex is referenced with different colors, the last one wins.
		r_per_vertex_colors.set(vert_inst.v, _vertex_colors[vert_inst.vc]);
	}
}

Ref<Material4D> FourDODocument4D::_import_generate_material(const String &p_material_name, const PackedColorArray &p_per_cell_colors, const PackedColorArray &p_per_vertex_colors, const bool p_for_poly_mesh) const {
	const bool has_4do_material = _materials.has(p_material_name);
	if (!has_4do_material && p_per_cell_colors.is_empty() && p_per_vertex_colors.is_empty()) {
		return Ref<Material4D>(); // Nothing to describe, let the mesh use the default material.
	}
	const Color base_color = has_4do_material ? _materials[p_material_name].base_color_factor : Color(1, 1, 1, 1);
	const bool has_single_color = !Color(1, 1, 1, 1).is_equal_approx(base_color);
	Ref<TetraMaterial4D> material;
	if (!p_per_cell_colors.is_empty()) {
		if (!p_per_vertex_colors.is_empty()) {
			WARN_PRINT_ONCE("4DO import: A surface has both per-cell and per-vertex colors. Only the per-cell colors will be used.");
		}
		if (p_for_poly_mesh) {
			// PolyMaterial4D colors the polyhedral cells, not the tetrahedra they decompose into.
			Ref<PolyMaterial4D> poly_material;
			poly_material.instantiate();
			poly_material->set_poly_albedo_color_array(p_per_cell_colors);
			material = poly_material;
		} else {
			material.instantiate();
			material->set_albedo_color_array(p_per_cell_colors);
		}
		material->set_albedo_source(has_single_color ? TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE : TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
	} else if (!p_per_vertex_colors.is_empty()) {
		// Per-vertex colors index the vertex positions, which works the same way for poly meshes,
		// and is not something PolyMaterial4D's per-cell array can express, so use TetraMaterial4D for both.
		material.instantiate();
		material->set_albedo_color_array(p_per_vertex_colors);
		material->set_albedo_source(has_single_color ? TetraMaterial4D::TETRA_COLOR_SOURCE_PER_VERT_AND_SINGLE : TetraMaterial4D::TETRA_COLOR_SOURCE_PER_VERT_ONLY);
	} else if (p_for_poly_mesh) {
		Ref<PolyMaterial4D> poly_material;
		poly_material.instantiate();
		material = poly_material;
	} else {
		material.instantiate();
	}
	material->set_albedo_color(base_color);
	return material;
}

// Static functions for starting the import process and creating a FourDODocument4D from the data.

Ref<FourDODocument4D> FourDODocument4D::import_read_from_byte_array(const PackedByteArray &p_data, const String &p_base_path, const String &p_filename) {
	Error err;
	ERR_FAIL_COND_V_MSG(p_data.is_empty(), Ref<FourDODocument4D>(), "4DO import: Error: Given byte array is empty.");
#if GDEXTENSION
	const String as_string = p_data.get_string_from_utf8();
#elif GODOT_MODULE
	String as_string;
	if (p_data.size() > 0) {
		const uint8_t *r = p_data.ptr();
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 5
		as_string.parse_utf8((const char *)r, p_data.size(), true);
#else
		as_string = String::utf8((const char *)r, p_data.size());
#endif
	}
#endif
	Ref<FourDODocument4D> the_4do_document;
	the_4do_document.instantiate();
	the_4do_document->_4do_base_path = p_base_path;
	the_4do_document->_4do_filename = p_filename;
	err = the_4do_document->_import_parse_4do_raw_text(as_string);
	ERR_FAIL_COND_V_MSG(err != OK, Ref<FourDODocument4D>(), "4DO import: Error: Failed to read 4DO document from byte array.");
	return the_4do_document;
}

Ref<FourDODocument4D> FourDODocument4D::import_read_from_file(const String &p_path) {
	Error err;
#if GDEXTENSION
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), Ref<FourDODocument4D>(), "4DO import: Error: Could not open file " + p_path + ".");
#elif GODOT_MODULE
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ, &err);
	ERR_FAIL_COND_V_MSG(err != OK, Ref<FourDODocument4D>(), "4DO import: Error: Could not open file " + p_path + ".");
#endif
	const String file_text = file->get_as_text();
	Ref<FourDODocument4D> the_4do_document;
	the_4do_document.instantiate();
	the_4do_document->_4do_base_path = p_path.get_base_dir();
	the_4do_document->_4do_filename = p_path.get_file();
	err = the_4do_document->_import_parse_4do_raw_text(file_text);
	ERR_FAIL_COND_V_MSG(err != OK, Ref<FourDODocument4D>(), "4DO import: Error: Failed to read 4DO document from file.");
	return the_4do_document;
}

// Supplemental import functions.

Error FourDODocument4D::import_read_materials_from_byte_array(const PackedByteArray &p_data) {
	ERR_FAIL_COND_V_MSG(p_data.is_empty(), ERR_INVALID_PARAMETER, "4DO import: Error: Given byte array is empty.");
#if GDEXTENSION
	const String as_string = p_data.get_string_from_utf8();
#elif GODOT_MODULE
	String as_string;
	if (p_data.size() > 0) {
		const uint8_t *r = p_data.ptr();
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 5
		as_string.parse_utf8((const char *)r, p_data.size(), true);
#else
		as_string = String::utf8((const char *)r, p_data.size());
#endif
	}
#endif
	return _import_parse_material_raw_text(as_string);
}

Error FourDODocument4D::import_read_materials_from_file(const String &p_path) {
	Error err;
#if GDEXTENSION
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_FILE_CANT_OPEN, "4DO import: Error: Could not open material file " + p_path + ".");
#elif GODOT_MODULE
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ, &err);
	ERR_FAIL_COND_V_MSG(err != OK, ERR_FILE_CANT_OPEN, "4DO import: Error: Could not open material file " + p_path + ".");
#endif
	const String file_text = file->get_as_text();
	return _import_parse_material_raw_text(file_text);
}

Ref<MultiSurfaceMesh4D> FourDODocument4D::import_generate_multi_surface_mesh_4d() const {
	// It would be nice if these were constexpr but we can't do that.
	// For all cells, the first two elements MUST be connected. For faces, form a closed loop of edges.
	/* clang-format off */
	const Vector<Vector<PackedInt32Array>> tetrahedral_cell_poly_indices = Vector<Vector<PackedInt32Array>> {
		Vector<PackedInt32Array> {
			PackedInt32Array {0, 1, 2}, // Triangle face using verts 0, 1, 2 (edges 0, 1, 2).
			PackedInt32Array {0, 3, 4}, // Triangle face using verts 0, 1, 3 (edges 0, 3, 4).
			PackedInt32Array {1, 3, 5}, // Triangle face using verts 0, 2, 3 (edges 1, 3, 5).
			PackedInt32Array {2, 4, 5}, // Triangle face using verts 1, 2, 3 (edges 2, 4, 5).
		},
		Vector<PackedInt32Array> {
			PackedInt32Array {0, 1, 2, 3}, // Tetrahedral cell using all faces.
		}
	};
	const Vector<Vector<PackedInt32Array>> cubic_cell_poly_indices = Vector<Vector<PackedInt32Array>> {
		Vector<PackedInt32Array> {
			PackedInt32Array {0, 1, 2, 3}, // Quad face using verts 0, 1, 2, 3 (edges 0, 1, 2, 3).
			PackedInt32Array {0, 5, 8, 7}, // Quad face using verts 1, 3, 5, 7 (edges 0, 5, 8, 7).
			PackedInt32Array {1, 4, 9, 5}, // Quad face using verts 0, 1, 4, 5 (edges 1, 4, 9, 5).
			PackedInt32Array {2, 4, 10, 6}, // Quad face using verts 0, 2, 4, 6 (edges 2, 4, 10, 6).
			PackedInt32Array {3, 6, 11, 7}, // Quad face using verts 2, 3, 6, 7 (edges 3, 6, 11, 7).
			PackedInt32Array {8, 9, 10, 11}, // Quad face using verts 4, 5, 6, 7 (edges 8, 9, 10, 11).
		},
		Vector<PackedInt32Array> {
			PackedInt32Array {0, 1, 2, 3, 4, 5}, // Cubic cell using all 6 faces.
		}
	};
	/* clang-format on */
	// Convert the face vertex indices to a format suitable for ArrayPolyMesh4D.
	const int64_t face_count = _face_vertex_indices.size();
	PackedInt32Array poly_edge_vertex_indices;
	Vector<PackedInt32Array> face_edge_indices;
	face_edge_indices.resize(face_count);
	for (int64_t face_index = 0; face_index < face_count; face_index++) {
		const Vector<FourDOVertexInstance4D> &face_vertex_indices = _face_vertex_indices[face_index];
		const int64_t vertex_count = face_vertex_indices.size();
		PackedInt32Array this_face_edges;
		this_face_edges.resize(vertex_count);
		for (int64_t vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
			this_face_edges.set(vertex_index, poly_edge_vertex_indices.size() / 2);
			if (vertex_index == vertex_count - 1) {
				// Connect back to the first vertex of the face, which must come first.
				poly_edge_vertex_indices.append(face_vertex_indices[0].v);
				poly_edge_vertex_indices.append(face_vertex_indices[vertex_index].v);
			} else {
				poly_edge_vertex_indices.append(face_vertex_indices[vertex_index].v);
				poly_edge_vertex_indices.append(face_vertex_indices[vertex_index + 1].v);
			}
		}
		face_edge_indices.set(face_index, this_face_edges);
	}
	const Vector<Vector<PackedInt32Array>> poly_cell_indices = Vector<Vector<PackedInt32Array>>{ face_edge_indices };
	// Populate the surface_meshes array with data from the FourDODocument4D as needed.
	Vector<Ref<SingleSurfaceMesh4D>> surface_meshes;
	for (const KeyValue<String, FourDODocument4D::FourDOSurface4D> &surface_pair : _surfaces) {
		const FourDODocument4D::FourDOSurface4D &surface = surface_pair.value;
		if (surface.is_empty()) {
			continue; // Nothing at all, skip.
		}
		const String &surface_name = surface_pair.key;
		Ref<SingleSurfaceMesh4D> single_surface_mesh;
		if (surface.cubic_cells.is_empty() && surface.polyhedron_face_indices.is_empty()) {
			// Tetrahedra-only 4DO surface, so generate this as ArrayTetraMesh4D.
			Ref<ArrayTetraMesh4D> tetra_mesh_4d;
			tetra_mesh_4d.instantiate();
			tetra_mesh_4d->set_vertex_positions(_vertex_positions);
			const int64_t tet_count = surface.tetrahedral_cells.size();
			const int64_t simplex_indices_count = tet_count * 4;
			// 4DO lists tetrahedron vertices such that Godot 4D's orientation convention, the perpendicular of the edges
			// from vertex 0 to vertices 1, 2, and 3, points into the mesh. Swapping two vertices flips it to point outward.
			const int flipped_corner_order[4] = { 0, 2, 1, 3 };
			// All mesh surfaces have vertex indices that reference vertex positions.
			PackedInt32Array simplex_cell_vertex_indices;
			simplex_cell_vertex_indices.resize(simplex_indices_count);
			for (int64_t i = 0; i < tet_count; i++) {
				const FourDODocument4D::FourDOTetrahedralCell4D &tet = surface.tetrahedral_cells[i];
				for (int64_t j = 0; j < 4; j++) {
					simplex_cell_vertex_indices.set(i * 4 + j, tet.verts[flipped_corner_order[j]].v);
				}
			}
			tetra_mesh_4d->set_simplex_cell_vertex_indices(simplex_cell_vertex_indices);
			ERR_FAIL_COND_V_MSG(!tetra_mesh_4d->is_mesh_data_valid(), Ref<MultiSurfaceMesh4D>(), "4DO import: The tetrahedra of material '" + surface_name + "' do not form a valid mesh.");
			// Normal indices and texture map indices may or may not exist.
			PackedVector4Array normal_values = _vertex_normals;
			if (surface.has_any_normal_indices) {
				// Vertices without a normal index, or with an out-of-range one, use the flat shading normal of their cell.
				int64_t flat_normal_start = -1;
				PackedInt32Array simplex_cell_normal_indices;
				simplex_cell_normal_indices.resize(simplex_indices_count);
				for (int64_t i = 0; i < tet_count; i++) {
					const FourDODocument4D::FourDOTetrahedralCell4D &tet = surface.tetrahedral_cells[i];
					for (int64_t j = 0; j < 4; j++) {
						const int32_t vn = tet.verts[flipped_corner_order[j]].vn;
						if (vn >= 0 && vn < _vertex_normals.size()) {
							simplex_cell_normal_indices.set(i * 4 + j, vn);
						} else {
							if (flat_normal_start < 0) {
								tetra_mesh_4d->calculate_boundary_normals();
								flat_normal_start = normal_values.size();
								normal_values.append_array(tetra_mesh_4d->get_simplex_cell_boundary_normals());
							}
							simplex_cell_normal_indices.set(i * 4 + j, (int32_t)(flat_normal_start + i));
						}
					}
				}
				tetra_mesh_4d->set_simplex_cell_normal_indices(simplex_cell_normal_indices);
			}
			tetra_mesh_4d->set_normal_values(normal_values);
			PackedVector3Array texture_map_values = _vertex_texture_coordinates;
			if (surface.has_any_texture_map_indices) {
				// Tetrahedral meshes cannot leave individual cells unmapped, so vertices without a
				// texture coordinate index, or with an out-of-range one, point at a zero texture coordinate.
				int64_t zero_texture_map_index = -1;
				PackedInt32Array simplex_cell_texture_map_indices;
				simplex_cell_texture_map_indices.resize(simplex_indices_count);
				for (int64_t i = 0; i < tet_count; i++) {
					const FourDODocument4D::FourDOTetrahedralCell4D &tet = surface.tetrahedral_cells[i];
					for (int64_t j = 0; j < 4; j++) {
						const int32_t vt = tet.verts[flipped_corner_order[j]].vt;
						if (vt >= 0 && vt < _vertex_texture_coordinates.size()) {
							simplex_cell_texture_map_indices.set(i * 4 + j, vt);
						} else {
							if (zero_texture_map_index < 0) {
								WARN_PRINT_ONCE("4DO import: Some tetrahedron vertices are missing texture coordinates. They will use a texture coordinate of zero.");
								zero_texture_map_index = texture_map_values.size();
								texture_map_values.append(Vector3());
							}
							simplex_cell_texture_map_indices.set(i * 4 + j, (int32_t)zero_texture_map_index);
						}
					}
				}
				tetra_mesh_4d->set_simplex_cell_texture_map_indices(simplex_cell_texture_map_indices);
			}
			tetra_mesh_4d->set_texture_map_values(texture_map_values);
			// Validation catches vertex indices that are out of range for this file, which the code above copies as-is.
			ERR_FAIL_COND_V_MSG(!tetra_mesh_4d->is_mesh_data_valid(), Ref<MultiSurfaceMesh4D>(), "4DO import: The normal or texture map bindings of material '" + surface_name + "' are invalid.");
			// Gather the per-cell colors (version 1 cell format) and per-vertex colors (version 2 vertex format), if any.
			PackedColorArray per_cell_colors;
			PackedColorArray per_vertex_colors;
			for (int64_t i = 0; i < tet_count; i++) {
				const FourDODocument4D::FourDOTetrahedralCell4D &tet = surface.tetrahedral_cells[i];
				_import_gather_cell_color(tet.color_index, i, tet_count, per_cell_colors);
				_import_gather_vertex_colors(tet.verts, 4, per_vertex_colors);
			}
			const Ref<Material4D> material = _import_generate_material(surface_name, per_cell_colors, per_vertex_colors, false);
			if (material.is_valid()) {
				tetra_mesh_4d->set_material(material);
			}
			single_surface_mesh = tetra_mesh_4d;
		} else {
			// At least some cubic cells or polyhedra exist, so generate this as ArrayPolyMesh4D.
			Ref<ArrayPolyMesh4D> poly_mesh_4d;
			poly_mesh_4d.instantiate();
			poly_mesh_4d->set_poly_cell_vertex_positions(_vertex_positions);
			poly_mesh_4d->set_poly_cell_normal_values(_vertex_normals);
			poly_mesh_4d->set_poly_cell_texture_map_values(_vertex_texture_coordinates);
			// Append polyhedral cells to the poly mesh.
			poly_mesh_4d->set_edge_vertex_indices(poly_edge_vertex_indices);
			poly_mesh_4d->set_poly_cell_indices(poly_cell_indices);
			const int64_t polyhedron_count = surface.polyhedron_face_indices.size();
			for (int64_t polyhedron_index = 0; polyhedron_index < polyhedron_count; polyhedron_index++) {
				const PackedInt32Array &polyhedron_face_indices = surface.polyhedron_face_indices[polyhedron_index];
				const int64_t poly_cell_index = poly_mesh_4d->append_poly_cell(3, polyhedron_face_indices);
				ERR_FAIL_COND_V_MSG(poly_cell_index < 0, Ref<MultiSurfaceMesh4D>(), "4DO import: Polyhedron " + itos(polyhedron_index) + " of material '" + surface_name + "' references a face that does not exist.");
			}
			// Append tetrahedral cells to the poly mesh.
			const int64_t tet_count = surface.tetrahedral_cells.size();
			PackedInt32Array tet_cell_to_poly_cell;
			tet_cell_to_poly_cell.resize(tet_count);
			for (int64_t tet_index = 0; tet_index < tet_count; tet_index++) {
				const FourDODocument4D::FourDOTetrahedralCell4D &tet = surface.tetrahedral_cells[tet_index];
				const PackedInt32Array tet_edge_indices = {
					tet.verts[0].v, tet.verts[1].v, tet.verts[0].v, tet.verts[2].v, tet.verts[1].v, tet.verts[2].v,
					tet.verts[0].v, tet.verts[3].v, tet.verts[1].v, tet.verts[3].v, tet.verts[2].v, tet.verts[3].v
				};
				const int64_t poly_cell_index = poly_mesh_4d->append_poly_hierarchy(tetrahedral_cell_poly_indices, tet_edge_indices);
				// The hierarchy itself is always valid, so this can only fail when the file's vertex indices are out of range.
				ERR_FAIL_COND_V_MSG(poly_cell_index < 0, Ref<MultiSurfaceMesh4D>(), "4DO import: Tetrahedron " + itos(tet_index) + " of material '" + surface_name + "' references a vertex that does not exist.");
				tet_cell_to_poly_cell.set(tet_index, (int32_t)poly_cell_index);
			}
			// Append cubic cells to the poly mesh.
			const int64_t cubic_count = surface.cubic_cells.size();
			PackedInt32Array cubic_cell_to_poly_cell;
			cubic_cell_to_poly_cell.resize(cubic_count);
			for (int64_t cubic_index = 0; cubic_index < cubic_count; cubic_index++) {
				const FourDODocument4D::FourDOCubicCell4D &cubic = surface.cubic_cells[cubic_index];
				/* clang-format off */
				const PackedInt32Array cubic_edge_indices = {
					cubic.verts[1].v, cubic.verts[3].v, cubic.verts[0].v, cubic.verts[1].v, // Edges 0 and 1.
					cubic.verts[0].v, cubic.verts[2].v, cubic.verts[2].v, cubic.verts[3].v, // Edges 2 and 3.
					cubic.verts[0].v, cubic.verts[4].v, cubic.verts[1].v, cubic.verts[5].v, // Edges 4 and 5.
					cubic.verts[2].v, cubic.verts[6].v, cubic.verts[3].v, cubic.verts[7].v, // Edges 6 and 7.
					cubic.verts[5].v, cubic.verts[7].v, cubic.verts[4].v, cubic.verts[5].v, // Edges 8 and 9.
					cubic.verts[4].v, cubic.verts[6].v, cubic.verts[6].v, cubic.verts[7].v, // Edges 10 and 11.
				};
				/* clang-format on */
				const int64_t poly_cell_index = poly_mesh_4d->append_poly_hierarchy(cubic_cell_poly_indices, cubic_edge_indices);
				// The hierarchy itself is always valid, so this can only fail when the file's vertex indices are out of range.
				ERR_FAIL_COND_V_MSG(poly_cell_index < 0, Ref<MultiSurfaceMesh4D>(), "4DO import: Cuboid " + itos(cubic_index) + " of material '" + surface_name + "' references a vertex that does not exist.");
				cubic_cell_to_poly_cell.set(cubic_index, (int32_t)poly_cell_index);
			}
			// Every appended cell is a boundary cell here, so the poly cell indices returned above are also the indices into per-cell data.
			const Vector<Vector<PackedInt32Array>> &appended_poly_cell_indices = poly_mesh_4d->get_poly_cell_indices();
			const int64_t poly_cell_count = appended_poly_cell_indices.size() > 1 ? appended_poly_cell_indices[1].size() : 0;
			// 4DO lists cell vertices such that Godot 4D's orientation convention, the perpendicular of the edges from
			// vertex 0, points into the mesh, so negate it to get the outward boundary normal. For cuboids in Z-order, the
			// edges from vertex 0 go to vertices 1, 2, and 4. The orientation the poly mesh derives from the cell structure
			// does not reliably match the vertex order, so flip any cells that disagree. Polyhedra have no such convention,
			// so their desired normals are left as zero, which keeps their cell orientation as-is.
			{
				PackedVector4Array cell_boundary_normals;
				cell_boundary_normals.resize(poly_cell_count);
				cell_boundary_normals.fill(Vector4());
				const int64_t vertex_count = _vertex_positions.size();
				for (int64_t tet_index = 0; tet_index < tet_count; tet_index++) {
					const FourDODocument4D::FourDOTetrahedralCell4D &tet = surface.tetrahedral_cells[tet_index];
					const int32_t v0 = tet.verts[0].v, v1 = tet.verts[1].v, v2 = tet.verts[2].v, v3 = tet.verts[3].v;
					if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0 || v0 >= vertex_count || v1 >= vertex_count || v2 >= vertex_count || v3 >= vertex_count) {
						continue; // Invalid indices are reported by mesh validation below.
					}
					const Vector4 &origin = _vertex_positions[v0];
					const Vector4 inward = Vector4D::perpendicular(_vertex_positions[v1] - origin, _vertex_positions[v2] - origin, _vertex_positions[v3] - origin);
					cell_boundary_normals.set(tet_cell_to_poly_cell[tet_index], -inward.normalized());
				}
				for (int64_t cubic_index = 0; cubic_index < cubic_count; cubic_index++) {
					const FourDODocument4D::FourDOCubicCell4D &cubic = surface.cubic_cells[cubic_index];
					const int32_t v0 = cubic.verts[0].v, v1 = cubic.verts[1].v, v2 = cubic.verts[2].v, v4 = cubic.verts[4].v;
					if (v0 < 0 || v1 < 0 || v2 < 0 || v4 < 0 || v0 >= vertex_count || v1 >= vertex_count || v2 >= vertex_count || v4 >= vertex_count) {
						continue; // Invalid indices are reported by mesh validation below.
					}
					const Vector4 &origin = _vertex_positions[v0];
					const Vector4 inward = Vector4D::perpendicular(_vertex_positions[v1] - origin, _vertex_positions[v2] - origin, _vertex_positions[v4] - origin);
					cell_boundary_normals.set(cubic_cell_to_poly_cell[cubic_index], -inward.normalized());
				}
				poly_mesh_4d->orient_cells_to_boundary_normals(cell_boundary_normals);
			}
			ERR_FAIL_COND_V_MSG(!poly_mesh_4d->is_mesh_data_valid(), Ref<MultiSurfaceMesh4D>(), "4DO import: The polytope cells of material '" + surface_name + "' do not form a valid mesh.");
			// Insert normal and texture map indices based on the computed boundary cell vertex indices.
			// TODO: Import normal and texture map bindings from the faces of general polyhedra too.
			// Currently only tetrahedra and cuboids retain these attributes; polyhedra use generated normals and no texture map.
			if (surface.has_any_normal_indices || surface.has_any_texture_map_indices) {
				const Vector<PackedInt32Array> boundary_cell_vertex_indices = poly_mesh_4d->get_all_boundary_cell_vertex_indices(false);
				Vector<PackedInt32Array> cell_normal_indices;
				Vector<PackedInt32Array> cell_texture_map_indices;
				if (surface.has_any_normal_indices) {
					// Start off with flat shading normals from the boundary normals set above, but override them based on what's in the file.
					poly_mesh_4d->set_flat_shading_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, false);
					const HashMap<Vector2i, Vector<PackedInt32Array>> poly_cell_normal_indices = poly_mesh_4d->get_all_poly_cell_normal_indices();
					cell_normal_indices = poly_cell_normal_indices[PolyMesh4D::CELL_TO_VERT_KEY];
					CRASH_COND(cell_normal_indices.size() != boundary_cell_vertex_indices.size());
				}
				if (surface.has_any_texture_map_indices) {
					// Start off with empty texture map indices.
					cell_texture_map_indices.resize(boundary_cell_vertex_indices.size());
				}
				for (int64_t tet_index = 0; tet_index < tet_count; tet_index++) {
					const FourDODocument4D::FourDOTetrahedralCell4D &tet = surface.tetrahedral_cells[tet_index];
					_remap_vert_indices_for_poly(tet.verts, 4, boundary_cell_vertex_indices, tet_cell_to_poly_cell[tet_index], cell_normal_indices, cell_texture_map_indices, surface.has_any_normal_indices, surface.has_any_texture_map_indices);
				}
				for (int64_t cubic_index = 0; cubic_index < cubic_count; cubic_index++) {
					const FourDODocument4D::FourDOCubicCell4D &cubic = surface.cubic_cells[cubic_index];
					_remap_vert_indices_for_poly(cubic.verts, 8, boundary_cell_vertex_indices, cubic_cell_to_poly_cell[cubic_index], cell_normal_indices, cell_texture_map_indices, surface.has_any_normal_indices, surface.has_any_texture_map_indices);
				}
				// Set the remapped data into the poly mesh.
				if (surface.has_any_normal_indices) {
					// Keep the per-cell boundary normals that are already in the map, only replacing the per-vertex ones.
					HashMap<Vector2i, Vector<PackedInt32Array>> all_poly_cell_normal_indices = poly_mesh_4d->get_all_poly_cell_normal_indices();
					all_poly_cell_normal_indices.insert(PolyMesh4D::CELL_TO_VERT_KEY, cell_normal_indices);
					poly_mesh_4d->set_all_poly_cell_normal_indices(all_poly_cell_normal_indices);
				}
				if (surface.has_any_texture_map_indices) {
					HashMap<Vector2i, Vector<PackedInt32Array>> all_poly_cell_texture_map_indices;
					all_poly_cell_texture_map_indices.insert(PolyMesh4D::CELL_TO_VERT_KEY, cell_texture_map_indices);
					poly_mesh_4d->set_all_poly_cell_texture_map_indices(all_poly_cell_texture_map_indices);
				}
			}
			// Gather the per-cell colors (version 1 cell format) and per-vertex colors (version 2 vertex format), if any.
			PackedColorArray per_cell_colors;
			PackedColorArray per_vertex_colors;
			for (int64_t tet_index = 0; tet_index < tet_count; tet_index++) {
				const FourDODocument4D::FourDOTetrahedralCell4D &tet = surface.tetrahedral_cells[tet_index];
				_import_gather_cell_color(tet.color_index, tet_cell_to_poly_cell[tet_index], poly_cell_count, per_cell_colors);
				_import_gather_vertex_colors(tet.verts, 4, per_vertex_colors);
			}
			for (int64_t cubic_index = 0; cubic_index < cubic_count; cubic_index++) {
				const FourDODocument4D::FourDOCubicCell4D &cubic = surface.cubic_cells[cubic_index];
				_import_gather_cell_color(cubic.color_index, cubic_cell_to_poly_cell[cubic_index], poly_cell_count, per_cell_colors);
				_import_gather_vertex_colors(cubic.verts, 8, per_vertex_colors);
			}
			for (int64_t polyhedron_index = 0; polyhedron_index < polyhedron_count; polyhedron_index++) {
				for (const int32_t face_index : surface.polyhedron_face_indices[polyhedron_index]) {
					if (face_index >= 0 && face_index < face_count) {
						const Vector<FourDOVertexInstance4D> &face_vertex_indices = _face_vertex_indices[face_index];
						_import_gather_vertex_colors(face_vertex_indices.ptr(), face_vertex_indices.size(), per_vertex_colors);
					}
				}
			}
			// Deduplicate up to dimension 2 (faces) but not dimension 3 (cells).
			// This function handles deduplication of bound elements such as normals and texture map indices as well.
			poly_mesh_4d->deduplicate_all_elements(2);
			ERR_FAIL_COND_V(!poly_mesh_4d->is_mesh_data_valid(), Ref<MultiSurfaceMesh4D>());
			if (!per_vertex_colors.is_empty()) {
				// Deduplication may have merged vertices, so move the colors over to the surviving vertices by position.
				const PackedVector4Array &deduplicated_vertices = poly_mesh_4d->get_poly_cell_vertex_positions();
				if (deduplicated_vertices.size() != _vertex_positions.size()) {
					PackedColorArray remapped_vertex_colors;
					remapped_vertex_colors.resize(deduplicated_vertices.size());
					remapped_vertex_colors.fill(Color(1, 1, 1, 1));
					for (int64_t new_index = 0; new_index < deduplicated_vertices.size(); new_index++) {
						for (int64_t old_index = 0; old_index < _vertex_positions.size(); old_index++) {
							if (deduplicated_vertices[new_index].is_equal_approx(_vertex_positions[old_index])) {
								remapped_vertex_colors.set(new_index, per_vertex_colors[old_index]);
								break;
							}
						}
					}
					per_vertex_colors = remapped_vertex_colors;
				}
			}
			const Ref<Material4D> material = _import_generate_material(surface_name, per_cell_colors, per_vertex_colors, true);
			if (material.is_valid()) {
				poly_mesh_4d->set_material(material);
			}
			single_surface_mesh = poly_mesh_4d;
		}
		single_surface_mesh->set_name(surface_name);
		surface_meshes.append(single_surface_mesh);
	}
	for (const KeyValue<String, FourDODocument4D::FourDOPolylineSurface4D> &surface_pair : _polyline_surfaces) {
		const FourDODocument4D::FourDOPolylineSurface4D &surface = surface_pair.value;
		if (surface.polylines.is_empty()) {
			continue; // Nothing at all, skip.
		}
		String surface_name = surface_pair.key;
		if (_surfaces.has(surface_name)) {
			surface_name += String("_polyline");
		}
		PackedInt32Array edge_indices;
		for (const Vector<FourDODocument4D::FourDOVertexInstance4D> &polyline : surface.polylines) {
			for (int64_t i = 0; i < polyline.size() - 1; i++) {
				edge_indices.append(polyline[i].v);
				edge_indices.append(polyline[i + 1].v);
			}
		}
		Ref<ArrayWireMesh4D> wire_mesh_4d;
		wire_mesh_4d.instantiate();
		wire_mesh_4d->set_vertex_positions(_vertex_positions);
		wire_mesh_4d->set_edge_indices(edge_indices);
		wire_mesh_4d->set_name(surface_name);
		if (_materials.has(surface_pair.key)) {
			const FourDODocument4D::FourDOMaterial4D &the_4do_mat = _materials[surface_pair.key];
			Ref<WireMaterial4D> wire_material;
			wire_material.instantiate();
			wire_material->set_albedo_color(the_4do_mat.base_color_factor);
			wire_mesh_4d->set_material(wire_material);
		}
		surface_meshes.append(wire_mesh_4d);
	}
	// Create the multi-surface mesh and populate it with the collected surface meshes.
	Ref<MultiSurfaceMesh4D> multi_surface_mesh_4d;
	multi_surface_mesh_4d.instantiate();
	multi_surface_mesh_4d->set_surface_meshes(surface_meshes);
	multi_surface_mesh_4d->set_name(_4do_filename);
	return multi_surface_mesh_4d;
}

void FourDODocument4D::_bind_methods() {
	ClassDB::bind_static_method("FourDODocument4D", D_METHOD("import_read_from_byte_array", "data", "base_path", "filename"), &FourDODocument4D::import_read_from_byte_array);
	ClassDB::bind_static_method("FourDODocument4D", D_METHOD("import_read_from_file", "path"), &FourDODocument4D::import_read_from_file);
	ClassDB::bind_method(D_METHOD("import_read_materials_from_byte_array", "data"), &FourDODocument4D::import_read_materials_from_byte_array);
	ClassDB::bind_method(D_METHOD("import_read_materials_from_file", "path"), &FourDODocument4D::import_read_materials_from_file);
	ClassDB::bind_method(D_METHOD("import_generate_multi_surface_mesh_4d"), &FourDODocument4D::import_generate_multi_surface_mesh_4d);
}
