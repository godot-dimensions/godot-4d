#pragma once

#include "array_poly_mesh_4d.h"

// Static helper class for 4D poly mesh building functions.
class PolyMeshBuilder4D : public Object {
	GDCLASS(PolyMeshBuilder4D, Object);

	// These helper functions are for `reconstruct_from_tetra_mesh`.
	static int64_t _append_edge_indices_to_array(int32_t p_index_a, int32_t p_index_b, const bool p_deduplicate, PackedInt32Array &r_edge_indices);
	static int64_t _append_face_internal_to_array(const PackedInt32Array &p_face, Vector<PackedInt32Array> &r_all_face_edge_indices);
	static Vector<PackedInt32Array> _compose_triangles_into_faces(const PackedVector4Array &p_vertices, const Vector<PackedInt32Array> &p_triangle_vertex_indices, PackedInt32Array &r_edge_indices);
	static Vector4 _compute_cell_normal(const PackedInt32Array &p_cell_first_face, const PackedInt32Array &p_cell_second_face, const PackedInt32Array &p_edge_vertex_indices, const PackedVector4Array &p_vertices);
	static PackedInt32Array _save_triangle_vertex_indices_as_faces_and_cell(const Vector<PackedInt32Array> &p_last_triangle_vertex_indices, const Vector4 &p_last_simplex_normal, const PackedVector4Array &p_vertices, Vector<PackedInt32Array> &r_all_face_edge_indices, PackedInt32Array &r_edge_vertex_indices);

protected:
	static PolyMeshBuilder4D *singleton;
	static void _bind_methods();

public:
	static Ref<ArrayPolyMesh4D> convert_mesh_3d_to_4d_faces_only(const Ref<ArrayMesh> &p_mesh_3d, const int p_which_surface = -1, const bool p_deduplicate = true);
	static Ref<ArrayPolyMesh4D> extrude_linear(const Ref<ArrayPolyMesh4D> &p_input_mesh, const Vector4 &p_extrusion_vector = Vector4(0, 0, 0, 1));
	static Ref<ArrayPolyMesh4D> extrude_spin_from_faces_xw(const Ref<ArrayPolyMesh4D> &p_input_mesh, const int p_steps = 16);
	static Ref<ArrayPolyMesh4D> reconstruct_from_tetra_mesh(const Ref<TetraMesh4D> &p_tetra_mesh);

	static PolyMeshBuilder4D *get_singleton() { return singleton; }
	PolyMeshBuilder4D() { singleton = this; }
	~PolyMeshBuilder4D() { singleton = nullptr; }
};
