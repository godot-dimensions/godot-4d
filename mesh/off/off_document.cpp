#include "off_document.h"

#if GDEXTENSION
#include <godot_cpp/classes/mesh_instance_3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#elif GODOT_MODULE
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/surface_tool.h"
#endif

#include "../../mesh/mesh_instance_4d.h"
#include "../tetra/tetra_material_4d.h"
#include "../wire/wire_material_4d.h"

void OFFDocument::_count_unique_edges_from_faces() {
	_edge_count = 0;
	HashSet<Vector2i> unique_items;
	for (int face_number = 0; face_number < _face_vertex_indices.size(); face_number++) {
		PackedInt32Array face_indices = _face_vertex_indices[face_number];
		for (int face_index = 0; face_index < face_indices.size(); face_index++) {
			const int second_index = (face_index + 1) % face_indices.size();
			Vector2i edge_indices = Vector2i(face_indices[face_index], face_indices[second_index]);
			if (edge_indices.x > edge_indices.y) {
				SWAP(edge_indices.x, edge_indices.y);
			}
			if (unique_items.has(edge_indices)) {
				continue;
			}
			unique_items.insert(edge_indices);
			_edge_count++;
		}
	}
}

int OFFDocument::_find_or_insert_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices) {
	const int vertex_count = _vertices.size();
	if (p_deduplicate_vertices) {
		for (int vertex_number = 0; vertex_number < vertex_count; vertex_number++) {
			if (_vertices[vertex_number] == p_vertex) {
				return vertex_number;
			}
		}
	}
	_vertices.append(p_vertex);
	return vertex_count;
}

Ref<OFFDocument> OFFDocument::convert_mesh_3d(const Ref<Mesh> &p_mesh, const bool p_deduplicate_vertices) {
	Ref<OFFDocument> off_document;
	ERR_FAIL_COND_V(p_mesh.is_null(), off_document);
	off_document.instantiate();
#if GDEXTENSION
	PackedVector3Array triangle_faces = p_mesh->get_faces();
#elif GODOT_MODULE
	PackedVector3Array triangle_faces = Variant(p_mesh->get_faces());
#endif
	const int face_count = triangle_faces.size() / 3;
	for (int face_number = 0; face_number < face_count; face_number++) {
		PackedInt32Array face_vertex_indices;
		for (int vertex_in_face_iter = 0; vertex_in_face_iter < 3; vertex_in_face_iter++) {
			const Vector3 vertex = triangle_faces[face_number * 3 + vertex_in_face_iter];
			const int vertex_index = off_document->_find_or_insert_vertex(Vector4(vertex.x, vertex.y, vertex.z, 0.0f), p_deduplicate_vertices);
			face_vertex_indices.append(vertex_index);
		}
		off_document->_face_vertex_indices.append(face_vertex_indices);
	}
	off_document->_count_unique_edges_from_faces();
	return off_document;
}

bool _do_triangle_faces_match(const PackedInt32Array &p_face_a, const PackedInt32Array &p_face_b) {
	return ((p_face_a[0] == p_face_b[0] && p_face_a[1] == p_face_b[1] && p_face_a[2] == p_face_b[2]) ||
			(p_face_a[0] == p_face_b[0] && p_face_a[1] == p_face_b[2] && p_face_a[2] == p_face_b[1]) ||
			(p_face_a[0] == p_face_b[1] && p_face_a[1] == p_face_b[0] && p_face_a[2] == p_face_b[2]) ||
			(p_face_a[0] == p_face_b[1] && p_face_a[1] == p_face_b[2] && p_face_a[2] == p_face_b[0]) ||
			(p_face_a[0] == p_face_b[2] && p_face_a[1] == p_face_b[0] && p_face_a[2] == p_face_b[1]) ||
			(p_face_a[0] == p_face_b[2] && p_face_a[1] == p_face_b[1] && p_face_a[2] == p_face_b[0]));
}

int OFFDocument::_find_or_insert_face(const int p_a, const int p_b, const int p_c, const bool p_deduplicate_faces) {
	PackedInt32Array face = PackedInt32Array{ p_a, p_b, p_c };
	const int face_count = _face_vertex_indices.size();
	if (p_deduplicate_faces) {
		for (int face_number = 0; face_number < face_count; face_number++) {
			if (_do_triangle_faces_match(_face_vertex_indices[face_number], face)) {
				return face_number;
			}
		}
	}
	_face_vertex_indices.append(face);
	return face_count;
}

Ref<OFFDocument> OFFDocument::convert_mesh_4d(const Ref<TetraMesh4D> &p_tetra_mesh, const bool p_deduplicate_faces) {
	Ref<OFFDocument> off_document;
	ERR_FAIL_COND_V(p_tetra_mesh.is_null(), off_document);
	off_document.instantiate();
	off_document->_vertices = p_tetra_mesh->get_vertices();
	const PackedInt32Array cell_indices = p_tetra_mesh->get_cell_indices();
	// TetraMesh4D references cells by their vertex indices, but OFF files reference them by their face indices.
	for (int i = 0; i < cell_indices.size(); i += 4) {
		const int a = off_document->_find_or_insert_face(cell_indices[i], cell_indices[i + 1], cell_indices[i + 2], p_deduplicate_faces);
		const int b = off_document->_find_or_insert_face(cell_indices[i], cell_indices[i + 2], cell_indices[i + 3], p_deduplicate_faces);
		const int c = off_document->_find_or_insert_face(cell_indices[i], cell_indices[i + 3], cell_indices[i + 1], p_deduplicate_faces);
		const int d = off_document->_find_or_insert_face(cell_indices[i + 1], cell_indices[i + 3], cell_indices[i + 2], p_deduplicate_faces);
		off_document->_cell_face_indices.append(PackedInt32Array{ a, b, c, d });
	}
	Ref<Material4D> material = p_tetra_mesh->get_material();
	if (material.is_valid()) {
		if (material->get_albedo_source_flags() & Material4D::COLOR_SOURCE_FLAG_PER_CELL) {
			off_document->_cell_colors = material->get_albedo_color_array();
			off_document->_has_any_cell_colors = true;
		}
	}
	off_document->_count_unique_edges_from_faces();
	return off_document;
}

Ref<ArrayMesh> OFFDocument::generate_mesh_3d(const bool p_per_face_vertices) {
	PackedVector3Array vertices_3d;
	for (int vert_index = 0; vert_index < _vertices.size(); vert_index++) {
		vertices_3d.append(Vector3(_vertices[vert_index].x, _vertices[vert_index].y, _vertices[vert_index].z));
	}
	if (p_per_face_vertices) {
		Ref<SurfaceTool> surface_tool;
		surface_tool.instantiate();
		surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
		surface_tool->set_smooth_group(-1);
		Ref<StandardMaterial3D> material;
		material.instantiate();
		material->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		material->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
		surface_tool->set_material(material);
		for (int face_number = 0; face_number < _face_vertex_indices.size(); face_number++) {
			surface_tool->set_color(_face_colors[face_number]);
			PackedInt32Array face_indices = _face_vertex_indices[face_number];
			// OFF face indices may contain non-triangle faces. We need to triangulate those now.
			for (int poly_index = 2; poly_index < face_indices.size(); poly_index++) {
				surface_tool->add_vertex(vertices_3d[face_indices[poly_index]]);
				surface_tool->add_vertex(vertices_3d[face_indices[poly_index - 1]]);
				surface_tool->add_vertex(vertices_3d[face_indices[0]]);
			}
		}
		surface_tool->generate_normals();
		return surface_tool->commit();
	}
	// When not using per-face vertices, we can just use an ArrayMesh.
	Ref<ArrayMesh> array_mesh;
	array_mesh.instantiate();
	Array mesh_arrays;
	mesh_arrays.resize(Mesh::ARRAY_MAX);
	mesh_arrays[Mesh::ARRAY_VERTEX] = vertices_3d;
	PackedInt32Array indices;
	for (int face_number = 0; face_number < _face_vertex_indices.size(); face_number++) {
		PackedInt32Array face_indices = _face_vertex_indices[face_number];
		// OFF face indices may contain non-triangle faces. We need to triangulate those now.
		for (int poly_index = 2; poly_index < face_indices.size(); poly_index++) {
			indices.append(face_indices[poly_index]);
			indices.append(face_indices[poly_index - 1]);
			indices.append(face_indices[0]);
		}
	}
	mesh_arrays[Mesh::ARRAY_INDEX] = indices;
	// Note: ArrayMesh doesn't support face colors, only vertex colors.
	// We have to exclude colors when not using per-face vertices.
	array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_arrays);
	return array_mesh;
}

Ref<ArrayTetraMesh4D> OFFDocument::generate_tetra_mesh_4d() {
	Ref<ArrayTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_vertices(_vertices);
	Ref<TetraMaterial4D> tetra_material;
	if (_has_any_cell_colors) {
		tetra_material.instantiate();
		tetra_material->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		tetra_mesh->set_material(tetra_material);
	}
	bool can_warn = true;
	for (int cell_number = 0; cell_number < _cell_face_indices.size(); cell_number++) {
		PackedInt32Array cell_indices = _cell_face_indices[cell_number];
		const int cell_size = cell_indices.size();
		if (cell_size != 4) {
			if (can_warn) {
				can_warn = false;
				WARN_PRINT("Warning: OFF file contains a cell with " + itos(cell_size) + " face indices. Converting this to tetrahedra is not yet implemented. You may wish to import this file as an ArrayWireMesh4D instead.");
			}
			continue;
		}
		// OFF files reference cells by their face indices, but TetraMesh4D references them by their vertex indices.
		// We can get a complete list of vertices using only 2 of the 4 faces.
		PackedInt32Array first_face_vertex_indices = _face_vertex_indices[cell_indices[0]];
		PackedInt32Array second_face_vertex_indices = _face_vertex_indices[cell_indices[1]];
		for (int second_face_index = 0; second_face_index < 3; second_face_index++) {
			if (!first_face_vertex_indices.has(second_face_vertex_indices[second_face_index])) {
				tetra_mesh->append_tetra_cell_indices(first_face_vertex_indices[0], first_face_vertex_indices[1], first_face_vertex_indices[2], second_face_vertex_indices[second_face_index]);
				break;
			}
		}
		if (_has_any_cell_colors) {
			tetra_material->append_albedo_color(_cell_colors[cell_number]);
		}
	}
	return tetra_mesh;
}

Ref<ArrayWireMesh4D> OFFDocument::generate_wire_mesh_4d(const bool p_deduplicate_edges) {
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertices(_vertices);
	Ref<WireMaterial4D> wire_material;
	if (_has_any_face_colors) {
		wire_material.instantiate();
		wire_material->set_albedo_source(WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_ONLY);
		wire_mesh->set_material(wire_material);
	}
	for (int face_number = 0; face_number < _face_vertex_indices.size(); face_number++) {
		PackedInt32Array face_indices = _face_vertex_indices[face_number];
		const int face_size = face_indices.size();
		for (int face_index = 0; face_index < face_size; face_index++) {
			const int second_index = (face_index + 1) % face_size;
			if (p_deduplicate_edges) {
				if (wire_mesh->has_edge_indices(face_indices[face_index], face_indices[second_index])) {
					continue;
				}
			}
			wire_mesh->append_edge_indices(face_indices[face_index], face_indices[second_index]);
			if (_has_any_face_colors) {
				wire_material->append_albedo_color(_face_colors[face_number]);
			}
		}
	}
	return wire_mesh;
}

Node *OFFDocument::generate_node(const bool p_deduplicate_edges, const bool p_per_face_vertices) {
	if (_cell_face_indices.is_empty()) {
		MeshInstance3D *mesh_instance_3d = memnew(MeshInstance3D);
		Ref<ArrayMesh> mesh_3d = generate_mesh_3d(p_per_face_vertices);
		ERR_FAIL_COND_V(mesh_3d.is_null(), nullptr);
		mesh_instance_3d->set_mesh(mesh_3d);
		return mesh_instance_3d;
	}
	// Some kind of 4D mesh.
	MeshInstance4D *mesh_instance_4d = memnew(MeshInstance4D);
	for (int i = 0; i < _cell_face_indices.size(); i++) {
		PackedInt32Array cell_face_index = _cell_face_indices[i];
		if (cell_face_index.size() > 4) {
			// Not a tetrahedral mesh. Let's generate a wireframe instead.
			Ref<ArrayWireMesh4D> wire_mesh = generate_wire_mesh_4d(p_deduplicate_edges);
			mesh_instance_4d->set_mesh(wire_mesh);
			return mesh_instance_4d;
		}
	}
	// Tetrahedral mesh.
	Ref<ArrayTetraMesh4D> tetra_mesh = generate_tetra_mesh_4d();
	mesh_instance_4d->set_mesh(tetra_mesh);
	return mesh_instance_4d;
}

PackedColorArray OFFDocument::get_cell_colors() const {
	return _cell_colors;
}

void OFFDocument::set_cell_colors(const PackedColorArray &p_cell_colors) {
	_cell_colors = p_cell_colors;
	_has_any_cell_colors = true;
}

TypedArray<PackedInt32Array> OFFDocument::get_cell_face_indices() const {
	return _cell_face_indices;
}

void OFFDocument::set_cell_face_indices(const TypedArray<PackedInt32Array> &p_cell_face_indices) {
	_cell_face_indices = p_cell_face_indices;
}

int OFFDocument::get_edge_count() const {
	return _edge_count;
}

void OFFDocument::set_edge_count(const int p_edge_count) {
	_edge_count = p_edge_count;
}

PackedColorArray OFFDocument::get_face_colors() const {
	return _face_colors;
}

void OFFDocument::set_face_colors(const PackedColorArray &p_face_colors) {
	_face_colors = p_face_colors;
	_has_any_face_colors = true;
}

TypedArray<PackedInt32Array> OFFDocument::get_face_vertex_indices() const {
	return _face_vertex_indices;
}

void OFFDocument::set_face_vertex_indices(const TypedArray<PackedInt32Array> &p_face_vertex_indices) {
	_face_vertex_indices = p_face_vertex_indices;
}

PackedVector4Array OFFDocument::get_vertices() const {
	return _vertices;
}

void OFFDocument::set_vertices(const PackedVector4Array &p_vertices) {
	_vertices = p_vertices;
}

enum class OFFDocumentReadState {
	READ_SIZE,
	READ_VERTICES,
	READ_FACES,
	READ_CELLS,
};

Ref<OFFDocument> OFFDocument::load_from_file(const String &p_path) {
	Ref<OFFDocument> off_document;
	bool can_warn = true;
	OFFDocumentReadState read_state = OFFDocumentReadState::READ_SIZE;
	int cell_count = 0;
	int face_count = 0;
	int vertex_count = 0;
	int current_cell_index = 0;
	int current_face_index = 0;
	int current_vertex_index = 0;
	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ, &err);
	ERR_FAIL_COND_V_MSG(err != OK, off_document, "Error: Could not open file " + p_path + ".");
	off_document.instantiate();
	while (!file->eof_reached()) {
		const String line = file->get_line();
		if (line.is_empty() || line.begins_with("#") || line.contains("OFF")) {
			continue;
		}
		PackedStringArray items = line.split(" ", false);
		const int item_count = items.size();
		if (item_count < 3) {
			if (can_warn) {
				can_warn = false;
				WARN_PRINT("Warning: OFF file " + p_path + " contains invalid line: '" + line + "'. Skipping this line and attempting to read the rest of the file anyway.");
			}
			continue;
		}
		switch (read_state) {
			case OFFDocumentReadState::READ_SIZE: {
				vertex_count = items[0].to_int();
				off_document->_vertices.resize(vertex_count);
				face_count = items[1].to_int();
				off_document->_face_vertex_indices.resize(face_count);
				off_document->_face_colors.resize(face_count);
				off_document->_edge_count = items[2].to_int();
				if (item_count > 3) {
					cell_count = items[3].to_int();
					off_document->_cell_face_indices.resize(cell_count);
					off_document->_cell_colors.resize(cell_count);
				}
				read_state = OFFDocumentReadState::READ_VERTICES;
			} break;
			case OFFDocumentReadState::READ_VERTICES: {
				if (can_warn) {
					if (!items[0].contains(".") || !items[1].contains(".") || !items[2].contains(".")) {
						can_warn = false;
						WARN_PRINT("Warning: OFF file " + p_path + " contains invalid vertex line: '" + line + "'. Every number in a vertex should be a floating-point number, whole numbers should end with '.0'. Reading this as a vertex anyway.");
					}
				}
				if (item_count == 3) {
					off_document->_vertices.set(current_vertex_index, Vector4(items[0].to_float(), items[1].to_float(), items[2].to_float(), 0.0));
				} else {
					if (can_warn) {
						if (!items[3].contains(".")) {
							can_warn = false;
							WARN_PRINT("Warning: OFF file " + p_path + " contains invalid vertex line: '" + line + "'. Every number in a vertex should be a floating-point number, whole numbers should end with '.0'. Reading this as a vertex anyway.");
						}
					}
					off_document->_vertices.set(current_vertex_index, Vector4(items[0].to_float(), items[1].to_float(), items[2].to_float(), items[3].to_float()));
				}
				current_vertex_index++;
				if (current_vertex_index >= vertex_count) {
					read_state = OFFDocumentReadState::READ_FACES;
				}
			} break;
			case OFFDocumentReadState::READ_FACES: {
				const int this_face_count = items[0].to_int();
				PackedInt32Array this_face_indices;
				if (item_count > this_face_count) {
					for (int i = 1; i <= this_face_count; i++) {
						this_face_indices.append(items[i].to_int());
					}
				}
				off_document->_face_vertex_indices.set(current_face_index, this_face_indices);
				if (item_count > this_face_count + 3) {
					// The last 3 numbers are the face color on the range 0-255.
					off_document->_face_colors.set(current_face_index, Color(items[this_face_count + 1].to_int() / 255.0f, items[this_face_count + 2].to_int() / 255.0f, items[this_face_count + 3].to_int() / 255.0f));
					off_document->_has_any_face_colors = true;
				} else {
					off_document->_face_colors.set(current_face_index, Color(1.0f, 1.0f, 1.0f));
				}
				current_face_index++;
				if (current_face_index >= face_count) {
					read_state = OFFDocumentReadState::READ_CELLS;
				}
			} break;
			case OFFDocumentReadState::READ_CELLS: {
				const int this_cell_count = items[0].to_int();
				PackedInt32Array this_cell_indices;
				if (item_count > this_cell_count) {
					for (int i = 1; i <= this_cell_count; i++) {
						this_cell_indices.append(items[i].to_int());
					}
				}
				off_document->_cell_face_indices.set(current_cell_index, this_cell_indices);
				if (item_count > this_cell_count + 3) {
					// The last 3 numbers are the cell color on the range 0-255.
					off_document->_cell_colors.set(current_cell_index, Color(items[this_cell_count + 1].to_int() / 255.0f, items[this_cell_count + 2].to_int() / 255.0f, items[this_cell_count + 3].to_int() / 255.0f));
					off_document->_has_any_cell_colors = true;
				} else {
					off_document->_cell_colors.set(current_cell_index, Color(1.0f, 1.0f, 1.0f));
				}
				current_cell_index++;
				if (current_cell_index >= cell_count) {
					return off_document;
				}
			} break;
		}
	}
	return off_document;
}

String _vec4_to_off_3d(const Vector4 &p_vertex) {
	String ret = String::num(p_vertex.x);
	if (p_vertex.x == (real_t)(int64_t)p_vertex.x) {
		ret += String(".0");
	}
	ret += " " + String::num(p_vertex.y);
	if (p_vertex.y == (real_t)(int64_t)p_vertex.y) {
		ret += String(".0");
	}
	ret += " " + String::num(p_vertex.z);
	if (p_vertex.z == (real_t)(int64_t)p_vertex.z) {
		ret += String(".0");
	}
	return ret;
}

String _vec4_to_off_4d(const Vector4 &p_vertex) {
	String ret = String::num(p_vertex.x);
	if (p_vertex.x == (real_t)(int64_t)p_vertex.x) {
		ret += String(".0");
	}
	ret += " " + String::num(p_vertex.y);
	if (p_vertex.y == (real_t)(int64_t)p_vertex.y) {
		ret += String(".0");
	}
	ret += " " + String::num(p_vertex.z);
	if (p_vertex.z == (real_t)(int64_t)p_vertex.z) {
		ret += String(".0");
	}
	ret += " " + String::num(p_vertex.w);
	if (p_vertex.w == (real_t)(int64_t)p_vertex.w) {
		ret += String(".0");
	}
	return ret;
}

String _color_to_off_string(const Color &p_color) {
	return " " + String::num_int64(p_color.r * 255.0f) + " " + String::num_int64(p_color.g * 255.0f) + " " + String::num_int64(p_color.b * 255.0f);
}

String _cell_or_face_to_off_string(const PackedInt32Array &p_face) {
	String ret = String::num(p_face.size());
	for (int i = 0; i < p_face.size(); i++) {
		ret += String(" ") + String::num_int64(p_face[i]);
	}
	return ret;
}

void OFFDocument::save_to_file_3d(const String &p_path) {
	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);
	ERR_FAIL_COND_MSG(err != OK, "Error: Could not open file " + p_path + " for writing.");
	file->store_line("OFF");
	file->store_line(String::num(_vertices.size()) + " " + String::num(_face_vertex_indices.size()) + " " + String::num(_edge_count));
	file->store_line("\n# Vertices");
	for (int i = 0; i < _vertices.size(); i++) {
		file->store_line(_vec4_to_off_3d(_vertices[i]));
	}
	file->store_line("\n# Faces");
	for (int i = 0; i < _face_vertex_indices.size(); i++) {
		String face_str = _cell_or_face_to_off_string(_face_vertex_indices[i]);
		if (i < _face_colors.size() && _face_colors[i] != Color(1.0f, 1.0f, 1.0f)) {
			face_str += _color_to_off_string(_face_colors[i]);
		}
		file->store_line(face_str);
	}
}

void OFFDocument::save_to_file_4d(const String &p_path) {
	if (_edge_count == 0) {
		_count_unique_edges_from_faces();
	}
	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);
	ERR_FAIL_COND_MSG(err != OK, "Error: Could not open file " + p_path + " for writing.");
	file->store_line("4OFF");
	file->store_line(String::num(_vertices.size()) + " " + String::num(_face_vertex_indices.size()) + " " + String::num(_edge_count) + " " + String::num(_cell_face_indices.size()));
	file->store_line("\n# Vertices");
	for (int i = 0; i < _vertices.size(); i++) {
		file->store_line(_vec4_to_off_4d(_vertices[i]));
	}
	file->store_line("\n# Faces");
	for (int i = 0; i < _face_vertex_indices.size(); i++) {
		String face_str = _cell_or_face_to_off_string(_face_vertex_indices[i]);
		if (i < _face_colors.size() && !_face_colors[i].is_equal_approx(Color(1.0f, 1.0f, 1.0f))) {
			face_str += _color_to_off_string(_face_colors[i]);
		}
		file->store_line(face_str);
	}
	file->store_line("\n# Cells");
	for (int i = 0; i < _cell_face_indices.size(); i++) {
		String cell_str = _cell_or_face_to_off_string(_cell_face_indices[i]);
		if (i < _cell_colors.size() && !_cell_colors[i].is_equal_approx(Color(1.0f, 1.0f, 1.0f))) {
			cell_str += _color_to_off_string(_cell_colors[i]);
		}
		file->store_line(cell_str);
	}
}

void OFFDocument::_bind_methods() {
	ClassDB::bind_static_method("OFFDocument", D_METHOD("convert_mesh_3d", "mesh", "deduplicate_vertices"), &OFFDocument::convert_mesh_3d, DEFVAL(true));
	ClassDB::bind_static_method("OFFDocument", D_METHOD("convert_mesh_4d", "mesh", "deduplicate_faces"), &OFFDocument::convert_mesh_4d, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("generate_mesh_3d", "per_face_vertices"), &OFFDocument::generate_mesh_3d, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("generate_tetra_mesh_4d"), &OFFDocument::generate_tetra_mesh_4d);
	ClassDB::bind_method(D_METHOD("generate_wire_mesh_4d", "deduplicate_edges"), &OFFDocument::generate_wire_mesh_4d, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("generate_node", "deduplicate_edges", "per_face_vertices"), &OFFDocument::generate_node, DEFVAL(true), DEFVAL(true));

	ClassDB::bind_method(D_METHOD("get_cell_colors"), &OFFDocument::get_cell_colors);
	ClassDB::bind_method(D_METHOD("set_cell_colors", "cell_colors"), &OFFDocument::set_cell_colors);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "cell_colors"), "set_cell_colors", "get_cell_colors");

	ClassDB::bind_method(D_METHOD("get_cell_face_indices"), &OFFDocument::get_cell_face_indices);
	ClassDB::bind_method(D_METHOD("set_cell_face_indices", "cell_face_indices"), &OFFDocument::set_cell_face_indices);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cell_face_indices"), "set_cell_face_indices", "get_cell_face_indices");

	ClassDB::bind_method(D_METHOD("get_edge_count"), &OFFDocument::get_edge_count);
	ClassDB::bind_method(D_METHOD("set_edge_count", "edge_count"), &OFFDocument::set_edge_count);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "edge_count"), "set_edge_count", "get_edge_count");

	ClassDB::bind_method(D_METHOD("get_face_colors"), &OFFDocument::get_face_colors);
	ClassDB::bind_method(D_METHOD("set_face_colors", "face_colors"), &OFFDocument::set_face_colors);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "face_colors"), "set_face_colors", "get_face_colors");

	ClassDB::bind_method(D_METHOD("get_face_vertex_indices"), &OFFDocument::get_face_vertex_indices);
	ClassDB::bind_method(D_METHOD("set_face_vertex_indices", "face_vertex_indices"), &OFFDocument::set_face_vertex_indices);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "face_vertex_indices"), "set_face_vertex_indices", "get_face_vertex_indices");

	ClassDB::bind_method(D_METHOD("get_vertices"), &OFFDocument::get_vertices);
	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &OFFDocument::set_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");

	ClassDB::bind_static_method("OFFDocument", D_METHOD("load_from_file", "path"), &OFFDocument::load_from_file);
	ClassDB::bind_method(D_METHOD("save_to_file_3d", "path"), &OFFDocument::save_to_file_3d);
	ClassDB::bind_method(D_METHOD("save_to_file_4d", "path"), &OFFDocument::save_to_file_4d);
}
