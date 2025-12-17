#pragma once

#include "../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/resource.hpp>
#elif GODOT_MODULE
#include "core/io/resource.h"
#include "scene/resources/mesh.h"
#endif

#include "../mesh/tetra/array_tetra_mesh_4d.h"
#include "../mesh/wire/array_wire_mesh_4d.h"

class OFFDocument4D : public Resource {
	GDCLASS(OFFDocument4D, Resource);

	PackedColorArray _cell_colors;
	TypedArray<PackedInt32Array> _cell_face_indices;
	PackedColorArray _face_colors;
	TypedArray<PackedInt32Array> _face_vertex_indices;
	PackedVector4Array _vertices;
	int _edge_count = 0;
	bool _has_any_face_colors = false;
	bool _has_any_cell_colors = false;

	static String _vector4_to_off_3d(const Vector4 &p_vertex);
	static String _vector4_to_off_4d(const Vector4 &p_vertex);
	static String _color_to_off_string(const Color &p_color);
	static String _cell_or_face_to_off_string(const PackedInt32Array &p_face);

	void _count_unique_edges_from_faces();
	int64_t _find_or_insert_face(const int32_t p_a, const int32_t p_b, const int32_t p_c, const bool p_deduplicate_faces = true);
	int64_t _find_or_insert_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices = true);
	TypedArray<PackedInt32Array> _calculate_cell_vertex_indices();
	TypedArray<PackedInt32Array> _calculate_simplex_vertex_indices(const TypedArray<PackedInt32Array> &p_cell_vertex_indices);

	String _export_save_to_string();
	static Ref<OFFDocument4D> _import_load_from_raw_text(const String &p_raw_text, const String &p_path);

protected:
	static void _bind_methods();

public:
	static Ref<OFFDocument4D> export_convert_mesh_3d(const Ref<Mesh> &p_mesh, const bool p_deduplicate_vertices = true);
	static Ref<OFFDocument4D> export_convert_mesh_4d(const Ref<TetraMesh4D> &p_mesh, const bool p_deduplicate_faces = true);
	PackedByteArray export_save_to_byte_array();
	void export_save_to_file(const String &p_path);

	static Ref<OFFDocument4D> import_load_from_byte_array(const PackedByteArray &p_data);
	static Ref<OFFDocument4D> import_load_from_file(const String &p_path);
	Ref<ArrayMesh> import_generate_mesh_3d(const bool p_per_face_vertices = true);
	Ref<ArrayTetraMesh4D> import_generate_tetra_mesh_4d();
	Ref<ArrayWireMesh4D> import_generate_wire_mesh_4d(const bool p_deduplicate_edges = true);
	Node *import_generate_node(const bool p_deduplicate_edges = true, const bool p_per_face_vertices = true);

	PackedColorArray get_cell_colors() const;
	void set_cell_colors(const PackedColorArray &p_cell_colors);

	TypedArray<PackedInt32Array> get_cell_face_indices() const;
	void set_cell_face_indices(const TypedArray<PackedInt32Array> &p_cell_face_indices);
	bool has_any_4d_cells() const { return !_cell_face_indices.is_empty(); }

	int get_edge_count() const;
	void set_edge_count(const int p_edge_count);

	PackedColorArray get_face_colors() const;
	void set_face_colors(const PackedColorArray &p_face_colors);

	TypedArray<PackedInt32Array> get_face_vertex_indices() const;
	void set_face_vertex_indices(const TypedArray<PackedInt32Array> &p_face_vertex_indices);

	PackedVector4Array get_vertices() const;
	void set_vertices(const PackedVector4Array &p_vertices);
};
