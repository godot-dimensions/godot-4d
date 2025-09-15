#include "off_document_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#elif GODOT_MODULE
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/surface_tool.h"
#endif

#include "../mesh_instance_4d.h"
#include "../tetra/tetra_material_4d.h"
#include "../wire/wire_material_4d.h"

void OFFDocument4D::_count_unique_edges_from_faces() {
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

int OFFDocument4D::_find_or_insert_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices) {
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

// OFF stores cells with indices to faces, but this provides indices of vertices.
TypedArray<PackedInt32Array> OFFDocument4D::_calculate_cell_vertex_indices() {
	const int64_t cell_face_indices_size = _cell_face_indices.size();
	TypedArray<PackedInt32Array> ret;
	if (cell_face_indices_size == 0) {
		return ret;
	}
	ret.resize(cell_face_indices_size);
	for (int64_t cell_number = 0; cell_number < cell_face_indices_size; cell_number++) {
		PackedInt32Array face_indices = _cell_face_indices[cell_number];
		const int64_t face_size = face_indices.size();
		PackedInt32Array this_cell_vertex_indices;
		for (int64_t face_number = 0; face_number < face_size; face_number++) {
			const int32_t face_index = face_indices[face_number];
			ERR_CONTINUE(face_index >= _face_vertex_indices.size());
			const PackedInt32Array this_face_vertex_indices = _face_vertex_indices[face_index];
			for (int64_t vertex_number = 0; vertex_number < this_face_vertex_indices.size(); vertex_number++) {
				const int32_t vertex_index = this_face_vertex_indices[vertex_number];
				if (!this_cell_vertex_indices.has(vertex_index)) {
					this_cell_vertex_indices.append(vertex_index);
				}
			}
		}
		ret[cell_number] = this_cell_vertex_indices;
	}
	return ret;
}

TypedArray<PackedInt32Array> OFFDocument4D::_calculate_simplex_vertex_indices(const TypedArray<PackedInt32Array> &p_cell_vertex_indices) {
	const int64_t cell_face_indices_size = _cell_face_indices.size();
	CRASH_COND(p_cell_vertex_indices.size() != cell_face_indices_size);
	TypedArray<PackedInt32Array> ret;
	if (cell_face_indices_size == 0) {
		return ret;
	}
	// OFF faces store vertices, so we need to compose them into edges (1D simplexes) for triangles.
	TypedArray<PackedInt32Array> triangle_faces_vertex_indices;
	{
		for (int64_t i = 0; i < _face_vertex_indices.size(); i++) {
			const PackedInt32Array polygon_vertex_indices = _face_vertex_indices[i];
			const int64_t triangle_count = polygon_vertex_indices.size() - 2;
			ERR_CONTINUE(triangle_count < 1);
			PackedInt32Array triangle_vertex_indices;
			triangle_vertex_indices.resize(3 * triangle_count);
			// This logic assumes that the face is a flat convex 2D polygon with vertices in a consistent winding order.
			// We break it into triangles by connecting the first vertex to every pair of adjacent vertices.
			const int32_t pivot_vertex_index = polygon_vertex_indices[0];
			int64_t simplex_iter = 0;
			for (int64_t triangle_index = 0; triangle_index < triangle_count; triangle_index++) {
				triangle_vertex_indices.set(simplex_iter++, pivot_vertex_index);
				triangle_vertex_indices.set(simplex_iter++, polygon_vertex_indices[triangle_index + 1]);
				triangle_vertex_indices.set(simplex_iter++, polygon_vertex_indices[triangle_index + 2]);
			}
			triangle_faces_vertex_indices.append(triangle_vertex_indices);
		}
	}
	// OFF cells store faces from the previous dimension (which we've calculated triangle simplexes for).
	CRASH_COND(_cell_face_indices.size() != p_cell_vertex_indices.size());
	constexpr int64_t prev_dimension_verts_per_simplex = 3;
	constexpr int64_t current_dimension_verts_per_simplex = 4;
	ret.resize(_cell_face_indices.size());
	int32_t prev_cell_pivot_vertex = -1;
	for (int cell_number = 0; cell_number < _cell_face_indices.size(); cell_number++) {
		const PackedInt32Array face_indices = _cell_face_indices[cell_number];
		const PackedInt32Array vertex_indices = p_cell_vertex_indices[cell_number];
		ERR_FAIL_COND_V(vertex_indices.size() < 2, ret);
		PackedInt32Array simplex_indices;
		int64_t simplex_iter = 0;
		const int64_t face_size = face_indices.size();
		// This logic assumes that the face is a "flat" convex polytope where all vertices are "visible" to the chosen pivot vertex.
		const int32_t pivot_vertex_index = (vertex_indices[0] == prev_cell_pivot_vertex) ? vertex_indices[1] : vertex_indices[0];
		for (int64_t face_number = 0; face_number < face_size; face_number++) {
			PackedInt32Array face_vertex_indices = triangle_faces_vertex_indices[face_indices[face_number]];
			if (face_vertex_indices.has(pivot_vertex_index)) {
				// Skip any faces connected to the pivot vertex.
				// For example, if making tetrahedra out of a box, we only want the 3 opposite faces to connect 6 tetrahedra.
				continue;
			}
			int64_t face_vert_iter = 0;
			const int64_t face_simplex_count = face_vertex_indices.size() / prev_dimension_verts_per_simplex;
			simplex_indices.resize(simplex_iter + current_dimension_verts_per_simplex * face_simplex_count);
			for (int64_t new_simplex_number = 0; new_simplex_number < face_simplex_count; new_simplex_number++) {
				simplex_indices.set(simplex_iter++, pivot_vertex_index);
				for (int64_t face_simplex_number = 0; face_simplex_number < prev_dimension_verts_per_simplex; face_simplex_number++) {
					simplex_indices.set(simplex_iter++, face_vertex_indices[face_vert_iter++]);
				}
			}
		}
		ret[cell_number] = simplex_indices;
		prev_cell_pivot_vertex = pivot_vertex_index;
	}
	return ret;
}

Ref<OFFDocument4D> OFFDocument4D::export_convert_mesh_3d(const Ref<Mesh> &p_mesh, const bool p_deduplicate_vertices) {
	Ref<OFFDocument4D> off_document;
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

int OFFDocument4D::_find_or_insert_face(const int p_a, const int p_b, const int p_c, const bool p_deduplicate_faces) {
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

Ref<OFFDocument4D> OFFDocument4D::export_convert_mesh_4d(const Ref<TetraMesh4D> &p_tetra_mesh, const bool p_deduplicate_faces) {
	Ref<OFFDocument4D> off_document;
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

Ref<ArrayMesh> OFFDocument4D::import_generate_mesh_3d(const bool p_per_face_vertices) {
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

Ref<ArrayTetraMesh4D> OFFDocument4D::import_generate_tetra_mesh_4d() {
	Ref<ArrayTetraMesh4D> cell_mesh;
	cell_mesh.instantiate();
	cell_mesh->set_vertices(_vertices);
	if (_cell_face_indices.is_empty()) {
		ERR_FAIL_COND_V_MSG(_face_vertex_indices.is_empty(), cell_mesh, "OFFDocument4D: This OFF document does not contain any cells or faces, so it cannot be converted to a tetrahedral cell mesh. Perhaps this is a vertex-only OFF file, or a 0D or 1D OFF file?");
		ERR_FAIL_V_MSG(cell_mesh, "OFFDocument4D: This OFF document does not contain any cells, so it cannot be converted to a tetrahedral cell mesh. Perhaps this is a 2D or 3D OFF file?");
	}
	const TypedArray<PackedInt32Array> cell_vertex_indices = _calculate_cell_vertex_indices();
	const TypedArray<PackedInt32Array> simplex_vertex_indices = _calculate_simplex_vertex_indices(cell_vertex_indices);
	const int64_t simplex_vertex_indices_size = simplex_vertex_indices.size();
	CRASH_COND(simplex_vertex_indices_size != _cell_colors.size());
	ERR_FAIL_COND_V_MSG(simplex_vertex_indices_size == 0, cell_mesh, "OFFDocument4D: Failed to generate tetrahedral simplexes from OFF cells.");
	// After those calculations, we can discard most of the data.
	// We just need the last array of the simplex vertex indices.
	constexpr int64_t vertices_per_simplex = 4;
	int64_t cell_colors_iter = 0;
	PackedColorArray packed_cell_colors;
	PackedInt32Array packed_cell_indices;
	for (int i = 0; i < simplex_vertex_indices.size(); i++) {
		PackedInt32Array simplex_vertex_indices_for_cell = simplex_vertex_indices[i];
		packed_cell_indices.append_array(simplex_vertex_indices_for_cell);
		if (_has_any_cell_colors) {
			const int64_t simplexes_in_face = simplex_vertex_indices_for_cell.size() / vertices_per_simplex;
			packed_cell_colors.resize(cell_colors_iter + simplexes_in_face);
			for (int simplex_number = 0; simplex_number < simplexes_in_face; simplex_number++) {
				packed_cell_colors.set(cell_colors_iter++, _cell_colors[i]);
			}
		}
	}
	cell_mesh->set_cell_indices(packed_cell_indices);
	if (_has_any_cell_colors) {
		Ref<TetraMaterial4D> cell_material;
		cell_material.instantiate();
		cell_material->set_albedo_source_flags(Material4D::COLOR_SOURCE_FLAG_PER_CELL);
		cell_material->set_albedo_color_array(packed_cell_colors);
		cell_mesh->set_material(cell_material);
	}
	ERR_FAIL_COND_V_MSG(!cell_mesh->is_mesh_data_valid(), cell_mesh, "OFFDocument4D: Failed to import OFF as cell mesh, mesh data is not valid.");
	return cell_mesh;
}

Ref<ArrayWireMesh4D> OFFDocument4D::import_generate_wire_mesh_4d(const bool p_deduplicate_edges) {
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertices(_vertices);
	Ref<WireMaterial4D> wire_material;
	if (_has_any_face_colors) {
		wire_material.instantiate();
		wire_material->set_albedo_source(WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_ONLY);
		wire_mesh->set_material(wire_material);
	}
	const int face_count = _face_vertex_indices.size();
	for (int face_number = 0; face_number < face_count; face_number++) {
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
	if (face_count == 0 && _vertices.size() == 2) {
		// Special case: OFF file with just two vertices and one implicit edge (such as a 1D OFF file).
		wire_mesh->append_edge_indices(0, 1);
	}
	return wire_mesh;
}

Node *OFFDocument4D::import_generate_node(const bool p_deduplicate_edges, const bool p_per_face_vertices) {
	// If there are no cells, this isn't a 4D mesh, but instead a 3D mesh.
	if (_cell_face_indices.is_empty()) {
		MeshInstance3D *mesh_instance_3d = memnew(MeshInstance3D);
		Ref<ArrayMesh> mesh_3d = import_generate_mesh_3d(p_per_face_vertices);
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
			Ref<ArrayWireMesh4D> wire_mesh = import_generate_wire_mesh_4d(p_deduplicate_edges);
			mesh_instance_4d->set_mesh(wire_mesh);
			return mesh_instance_4d;
		}
	}
	// Tetrahedral mesh.
	Ref<ArrayTetraMesh4D> tetra_mesh = import_generate_tetra_mesh_4d();
	mesh_instance_4d->set_mesh(tetra_mesh);
	return mesh_instance_4d;
}

PackedColorArray OFFDocument4D::get_cell_colors() const {
	return _cell_colors;
}

void OFFDocument4D::set_cell_colors(const PackedColorArray &p_cell_colors) {
	_cell_colors = p_cell_colors;
	_has_any_cell_colors = true;
}

TypedArray<PackedInt32Array> OFFDocument4D::get_cell_face_indices() const {
	return _cell_face_indices;
}

void OFFDocument4D::set_cell_face_indices(const TypedArray<PackedInt32Array> &p_cell_face_indices) {
	_cell_face_indices = p_cell_face_indices;
}

int OFFDocument4D::get_edge_count() const {
	return _edge_count;
}

void OFFDocument4D::set_edge_count(const int p_edge_count) {
	_edge_count = p_edge_count;
}

PackedColorArray OFFDocument4D::get_face_colors() const {
	return _face_colors;
}

void OFFDocument4D::set_face_colors(const PackedColorArray &p_face_colors) {
	_face_colors = p_face_colors;
	_has_any_face_colors = !p_face_colors.is_empty();
}

TypedArray<PackedInt32Array> OFFDocument4D::get_face_vertex_indices() const {
	return _face_vertex_indices;
}

void OFFDocument4D::set_face_vertex_indices(const TypedArray<PackedInt32Array> &p_face_vertex_indices) {
	_face_vertex_indices = p_face_vertex_indices;
}

PackedVector4Array OFFDocument4D::get_vertices() const {
	return _vertices;
}

void OFFDocument4D::set_vertices(const PackedVector4Array &p_vertices) {
	_vertices = p_vertices;
}

enum class OFFDocumentReadState {
	READ_SIZE,
	READ_VERTICES,
	READ_FACES,
	READ_CELLS,
};

Ref<OFFDocument4D> OFFDocument4D::import_load_from_byte_array(const PackedByteArray &p_data) {
	ERR_FAIL_COND_V_MSG(p_data.is_empty(), Ref<OFFDocument4D>(), "OFF import: Error: Given byte array is empty.");
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
	return OFFDocument4D::_import_load_from_raw_text(as_string, "(in-memory data)");
}

Ref<OFFDocument4D> OFFDocument4D::import_load_from_file(const String &p_path) {
#if GDEXTENSION
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), Ref<OFFDocument4D>(), "OFF import: Error: Could not open file " + p_path + ".");
#elif GODOT_MODULE
	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ, &err);
	ERR_FAIL_COND_V_MSG(err != OK, Ref<OFFDocument4D>(), "OFF import: Error: Could not open file " + p_path + ".");
#endif
	const String file_text = file->get_as_text();
	return _import_load_from_raw_text(file_text, p_path);
}

Ref<OFFDocument4D> OFFDocument4D::_import_load_from_raw_text(const String &p_raw_text, const String &p_path) {
	Ref<OFFDocument4D> off_document;
	off_document.instantiate();
	OFFDocumentReadState read_state = OFFDocumentReadState::READ_SIZE;
	int cell_count = 0;
	int face_count = 0;
	int vertex_count = 0;
	int current_cell_index = 0;
	int current_face_index = 0;
	int current_vertex_index = 0;
	int min_items_per_line = 3;
	bool can_warn = true;
	const PackedStringArray lines = p_raw_text.split("\n", false);
	for (const String &line : lines) {
		if (line.is_empty() || line.begins_with("#")) {
			continue;
		}
		if (line.contains("OFF")) {
			// "OFF" by itself is 3D OFF.
			if (line != "OFF") {
				const int declared_dimension = line.to_int();
				if (declared_dimension < 3) {
					min_items_per_line = declared_dimension;
				}
			}
			continue;
		}
		PackedStringArray items = line.split(" ", false);
		const int item_count = items.size();
		if (item_count < min_items_per_line) {
			if (can_warn) {
				can_warn = false;
				WARN_PRINT("OFF import: Warning: OFF file " + p_path + " contains invalid line: '" + line + "'. Skipping this line and attempting to read the rest of the file anyway.");
			}
			continue;
		}
		switch (read_state) {
			case OFFDocumentReadState::READ_SIZE: {
				vertex_count = items[0].to_int();
				off_document->_vertices.resize(vertex_count);
				if (item_count > 1) {
					face_count = items[1].to_int();
					off_document->_face_vertex_indices.resize(face_count);
					off_document->_face_colors.resize(face_count);
					if (item_count > 2) {
						off_document->_edge_count = items[2].to_int();
						if (item_count > 3) {
							cell_count = items[3].to_int();
							off_document->_cell_face_indices.resize(cell_count);
							off_document->_cell_colors.resize(cell_count);
						}
					}
				}
				read_state = OFFDocumentReadState::READ_VERTICES;
			} break;
			case OFFDocumentReadState::READ_VERTICES: {
				if (can_warn && item_count >= 3) {
					if (!items[0].contains(".") || !items[1].contains(".") || !items[2].contains(".")) {
						can_warn = false;
						//WARN_PRINT("Warning: OFF file " + p_path + " contains invalid vertex line: '" + line + "'. Every number in a vertex should be a floating-point number, whole numbers should end with '.0'. Reading this as a vertex anyway.");
					}
				}
				if (item_count == 1) {
					off_document->_vertices.set(current_vertex_index, Vector4(items[0].to_float(), 0.0, 0.0, 0.0));
				} else if (item_count == 2) {
					off_document->_vertices.set(current_vertex_index, Vector4(items[0].to_float(), items[1].to_float(), 0.0, 0.0));
				} else if (item_count == 3) {
					off_document->_vertices.set(current_vertex_index, Vector4(items[0].to_float(), items[1].to_float(), items[2].to_float(), 0.0));
				} else {
					if (can_warn) {
						if (!items[3].contains(".")) {
							can_warn = false;
							//WARN_PRINT("Warning: OFF file " + p_path + " contains invalid vertex line: '" + line + "'. Every number in a vertex should be a floating-point number, whole numbers should end with '.0'. Reading this as a vertex anyway.");
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
				off_document->_face_vertex_indices[current_face_index] = this_face_indices;
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
				off_document->_cell_face_indices[current_cell_index] = this_cell_indices;
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

String OFFDocument4D::_vector4_to_off_3d(const Vector4 &p_vertex) {
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

String OFFDocument4D::_vector4_to_off_4d(const Vector4 &p_vertex) {
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

String OFFDocument4D::_color_to_off_string(const Color &p_color) {
	return " " + String::num_int64(p_color.r * 255.0f) + " " + String::num_int64(p_color.g * 255.0f) + " " + String::num_int64(p_color.b * 255.0f);
}

String OFFDocument4D::_cell_or_face_to_off_string(const PackedInt32Array &p_face) {
	String ret = String::num(p_face.size());
	for (int i = 0; i < p_face.size(); i++) {
		ret += String(" ") + String::num_int64(p_face[i]);
	}
	return ret;
}

PackedByteArray OFFDocument4D::export_save_to_byte_array() {
	const String file_contents = _export_save_to_string();
	return file_contents.to_utf8_buffer();
}

void OFFDocument4D::export_save_to_file(const String &p_path) {
#if GDEXTENSION
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_MSG(file.is_null(), "Error: Could not open file " + p_path + " for writing.");
#elif GODOT_MODULE
	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);
	ERR_FAIL_COND_MSG(err != OK, "Error: Could not open file " + p_path + " for writing.");
#endif
	const String file_contents = _export_save_to_string();
	file->store_string(file_contents);
	file->close();
}

String OFFDocument4D::_export_save_to_string() {
	if (_edge_count == 0) {
		_count_unique_edges_from_faces();
	}
	const bool has_4d_cells = has_any_4d_cells();
	PackedStringArray lines;
	if (has_4d_cells) {
		lines.append("4OFF");
		lines.append("# Vertices, Faces, Edges, Cells");
		lines.append(String::num(_vertices.size()) + " " + String::num(_face_vertex_indices.size()) + " " + String::num(_edge_count) + " " + String::num(_cell_face_indices.size()));
	} else {
		lines.append("OFF");
		lines.append("# Vertices, Faces, Edges");
		lines.append(String::num(_vertices.size()) + " " + String::num(_face_vertex_indices.size()) + " " + String::num(_edge_count));
	}
	lines.append("\n# Vertices");
	for (int i = 0; i < _vertices.size(); i++) {
		lines.append(has_4d_cells ? _vector4_to_off_4d(_vertices[i]) : _vector4_to_off_3d(_vertices[i]));
	}
	lines.append("\n# Faces");
	for (int i = 0; i < _face_vertex_indices.size(); i++) {
		String face_str = _cell_or_face_to_off_string(_face_vertex_indices[i]);
		if (i < _face_colors.size() && !_face_colors[i].is_equal_approx(Color(1.0f, 1.0f, 1.0f))) {
			face_str += _color_to_off_string(_face_colors[i]);
		}
		lines.append(face_str);
	}
	if (has_4d_cells) {
		lines.append("\n# Cells");
		for (int i = 0; i < _cell_face_indices.size(); i++) {
			String cell_str = _cell_or_face_to_off_string(_cell_face_indices[i]);
			if (i < _cell_colors.size() && !_cell_colors[i].is_equal_approx(Color(1.0f, 1.0f, 1.0f))) {
				cell_str += _color_to_off_string(_cell_colors[i]);
			}
			lines.append(cell_str);
		}
	}
	return String("\n").join(lines) + String("\n");
}

void OFFDocument4D::_bind_methods() {
	ClassDB::bind_static_method("OFFDocument4D", D_METHOD("export_convert_mesh_3d", "mesh", "deduplicate_vertices"), &OFFDocument4D::export_convert_mesh_3d, DEFVAL(true));
	ClassDB::bind_static_method("OFFDocument4D", D_METHOD("export_convert_mesh_4d", "mesh", "deduplicate_faces"), &OFFDocument4D::export_convert_mesh_4d, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("export_save_to_byte_array"), &OFFDocument4D::export_save_to_byte_array);
	ClassDB::bind_method(D_METHOD("export_save_to_file", "path"), &OFFDocument4D::export_save_to_file);

	ClassDB::bind_static_method("OFFDocument4D", D_METHOD("import_load_from_byte_array", "data"), &OFFDocument4D::import_load_from_byte_array);
	ClassDB::bind_static_method("OFFDocument4D", D_METHOD("import_load_from_file", "path"), &OFFDocument4D::import_load_from_file);
	ClassDB::bind_method(D_METHOD("import_generate_mesh_3d", "per_face_vertices"), &OFFDocument4D::import_generate_mesh_3d, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("import_generate_tetra_mesh_4d"), &OFFDocument4D::import_generate_tetra_mesh_4d);
	ClassDB::bind_method(D_METHOD("import_generate_wire_mesh_4d", "deduplicate_edges"), &OFFDocument4D::import_generate_wire_mesh_4d, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("import_generate_node", "deduplicate_edges", "per_face_vertices"), &OFFDocument4D::import_generate_node, DEFVAL(true), DEFVAL(true));

	ClassDB::bind_method(D_METHOD("get_cell_colors"), &OFFDocument4D::get_cell_colors);
	ClassDB::bind_method(D_METHOD("set_cell_colors", "cell_colors"), &OFFDocument4D::set_cell_colors);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "cell_colors"), "set_cell_colors", "get_cell_colors");

	ClassDB::bind_method(D_METHOD("get_cell_face_indices"), &OFFDocument4D::get_cell_face_indices);
	ClassDB::bind_method(D_METHOD("set_cell_face_indices", "cell_face_indices"), &OFFDocument4D::set_cell_face_indices);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cell_face_indices"), "set_cell_face_indices", "get_cell_face_indices");

	ClassDB::bind_method(D_METHOD("get_edge_count"), &OFFDocument4D::get_edge_count);
	ClassDB::bind_method(D_METHOD("set_edge_count", "edge_count"), &OFFDocument4D::set_edge_count);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "edge_count"), "set_edge_count", "get_edge_count");

	ClassDB::bind_method(D_METHOD("get_face_colors"), &OFFDocument4D::get_face_colors);
	ClassDB::bind_method(D_METHOD("set_face_colors", "face_colors"), &OFFDocument4D::set_face_colors);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "face_colors"), "set_face_colors", "get_face_colors");

	ClassDB::bind_method(D_METHOD("get_face_vertex_indices"), &OFFDocument4D::get_face_vertex_indices);
	ClassDB::bind_method(D_METHOD("set_face_vertex_indices", "face_vertex_indices"), &OFFDocument4D::set_face_vertex_indices);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "face_vertex_indices"), "set_face_vertex_indices", "get_face_vertex_indices");

	ClassDB::bind_method(D_METHOD("get_vertices"), &OFFDocument4D::get_vertices);
	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &OFFDocument4D::set_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");
}
