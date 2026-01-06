#pragma once

#include "../poly/array_poly_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#elif GODOT_MODULE
class ArrayMesh;
#endif

class ArrayFaceMesh4D : public ArrayPolyMesh4D {
	GDCLASS(ArrayFaceMesh4D, ArrayPolyMesh4D);

private:
	// Per-face per-vertex attributes
	Vector<PackedInt32Array> _vertex_bones;
	Vector<PackedFloat32Array> _vertex_bone_weights;
	PackedColorArray _vertex_colors;
	PackedVector4Array _vertex_custom1;
	PackedVector4Array _vertex_custom2;
	PackedVector4Array _vertex_custom3;
	PackedVector4Array _vertex_normals;
	PackedVector2Array _vertex_uv;
	PackedVector2Array _vertex_uv2;

	Vector<PackedInt32Array> _face_vertex_indices;

	// Generated mesh for faceframe rendering
	Ref<ArrayMesh> _faceframe_mesh;
	// Every face gets a list of triangles that make it up
	Vector<PackedInt32Array> _triangle_vertex_indices;

	void _save_face_vertex_indices();
	void _generate_triangles();
	void _set_smooth_shading_face_vertex_normals(const ComputeNormalsMode p_mode, const bool p_recalculate_boundary_normals);

protected:
	static void _bind_methods();

	virtual void update_cross_section_mesh() override;

public:
	void face_mesh_clear_cache(bool changed_geometry);
	virtual void poly_mesh_clear_cache(const bool p_normals_only) override;

	PackedInt32Array get_vert_bones(int vert);
	PackedFloat32Array get_vert_weights(int vert);
	Color get_vert_color(int vert);
	Vector4 get_vert_custom1(int vert);
	Vector4 get_vert_custom2(int vert);
	Vector4 get_vert_custom3(int vert);
	Vector4 get_vert_normal(int vert);
	Vector2 get_vert_uv(int vert);
	Vector2 get_vert_uv2(int vert);
	
	Vector<PackedInt32Array> get_all_vertex_bones();
	void set_all_vertex_bones(Vector<PackedInt32Array> value);
	Vector<PackedFloat32Array> get_all_vertex_weights();
	void set_all_vertex_weights(Vector<PackedFloat32Array> value);
	PackedColorArray get_all_vertex_colors();
	void set_all_vertex_colors(PackedColorArray value);
	PackedVector4Array get_all_vertex_custom1();
	void set_all_vertex_custom1(PackedVector4Array value);
	PackedVector4Array get_all_vertex_custom2();
	void set_all_vertex_custom2(PackedVector4Array value);
	PackedVector4Array get_all_vertex_custom3();
	void set_all_vertex_custom3(PackedVector4Array value);
	PackedVector4Array get_all_vertex_normals();
	void set_all_vertex_normals(PackedVector4Array value);
	PackedVector2Array get_all_vertex_uv();
	void set_all_vertex_uv(PackedVector2Array value);
	PackedVector2Array get_all_vertex_uv2();
	void set_all_vertex_uv2(PackedVector2Array value);

	TypedArray<PackedInt32Array> get_all_vertex_bones_bind();
	void set_all_vertex_bones_bind(const TypedArray<PackedInt32Array> &vertex_bones);
	TypedArray<PackedFloat32Array> get_all_vertex_weights_bind();
	void set_all_vertex_weights_bind(const TypedArray<PackedFloat32Array> &vertex_weights);

};