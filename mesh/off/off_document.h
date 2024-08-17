#pragma once

#if GDEXTENSION
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/resource.hpp>
#elif GODOT_MODULE
#include "core/io/resource.h"
#include "scene/resources/mesh.h"
#endif

#include "../tetra/array_tetra_mesh_4d.h"
#include "../wire/array_wire_mesh_4d.h"

class OFFDocument : public Resource {
	GDCLASS(OFFDocument, Resource);

	PackedColorArray _cell_colors;
	TypedArray<PackedInt32Array> _cell_face_indices;
	PackedColorArray _face_colors;
	TypedArray<PackedInt32Array> _face_vertex_indices;
	PackedVector4Array _vertices;
	int _edge_count = 0;
	bool _has_any_face_colors = false;
	bool _has_any_cell_colors = false;

	void _count_unique_edges_from_faces();
	int _find_or_insert_face(const int p_a, const int p_b, const int p_c, const bool p_deduplicate_faces = true);
	int _find_or_insert_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices = true);

protected:
	static void _bind_methods();

public:
	static Ref<OFFDocument> convert_mesh_3d(const Ref<Mesh> &p_mesh, const bool p_deduplicate_vertices = true);
	static Ref<OFFDocument> convert_mesh_4d(const Ref<TetraMesh4D> &p_mesh, const bool p_deduplicate_faces = true);
	Ref<ArrayMesh> generate_mesh_3d(const bool p_per_face_vertices = true);
	Ref<ArrayTetraMesh4D> generate_tetra_mesh_4d();
	Ref<ArrayWireMesh4D> generate_wire_mesh_4d(const bool p_deduplicate_edges = true);
	Node *generate_node(const bool p_deduplicate_edges = true, const bool p_per_face_vertices = true);

	PackedColorArray get_cell_colors() const;
	void set_cell_colors(const PackedColorArray &p_cell_colors);

	TypedArray<PackedInt32Array> get_cell_face_indices() const;
	void set_cell_face_indices(const TypedArray<PackedInt32Array> &p_cell_face_indices);

	int get_edge_count() const;
	void set_edge_count(const int p_edge_count);

	PackedColorArray get_face_colors() const;
	void set_face_colors(const PackedColorArray &p_face_colors);

	TypedArray<PackedInt32Array> get_face_vertex_indices() const;
	void set_face_vertex_indices(const TypedArray<PackedInt32Array> &p_face_vertex_indices);

	PackedVector4Array get_vertices() const;
	void set_vertices(const PackedVector4Array &p_vertices);

	static Ref<OFFDocument> load_from_file(const String &p_path);
	void save_to_file_3d(const String &p_path);
	void save_to_file_4d(const String &p_path);
};
