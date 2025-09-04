#pragma once

#include "editor_viewport_4d_defines.h"

#include "../../math/rect4.h"
#include "../../model/mesh_instance_4d.h"

class Camera4D;
class RenderingEngine4D;
class WireMaterial4D;

class EditorTransformGizmo4D : public Node4D {
	GDCLASS(EditorTransformGizmo4D, Node4D);

public:
	// Keep this in sync with the toolbar buttons in EditorMainScreen4D.
	enum class GizmoMode {
		SELECT,
		MOVE,
		ROTATE,
		SCALE,
		STRETCH,
	};

	// Keep this in sync with the transform settings items in EditorMainScreen4D.
	enum class KeepMode {
		FREEFORM,
		ORTHOGONAL,
		CONFORMAL,
		ORTHONORMAL,
	};

private:
	enum TransformPart {
		TRANSFORM_NONE,
		TRANSFORM_MOVE_X,
		TRANSFORM_MOVE_Y,
		TRANSFORM_MOVE_Z,
		TRANSFORM_MOVE_W,
		TRANSFORM_MOVE_XY,
		TRANSFORM_MOVE_XZ,
		TRANSFORM_MOVE_XW,
		TRANSFORM_MOVE_YZ,
		TRANSFORM_MOVE_YW,
		TRANSFORM_MOVE_ZW,
		TRANSFORM_ROTATE_XY,
		TRANSFORM_ROTATE_XZ,
		TRANSFORM_ROTATE_XW,
		TRANSFORM_ROTATE_YZ,
		TRANSFORM_ROTATE_YW,
		TRANSFORM_ROTATE_ZW,
		TRANSFORM_SCALE_X,
		TRANSFORM_SCALE_Y,
		TRANSFORM_SCALE_Z,
		TRANSFORM_SCALE_W,
		TRANSFORM_SCALE_XY,
		TRANSFORM_SCALE_XZ,
		TRANSFORM_SCALE_XW,
		TRANSFORM_SCALE_YZ,
		TRANSFORM_SCALE_YW,
		TRANSFORM_SCALE_ZW,
		TRANSFORM_STRETCH_POS_X,
		TRANSFORM_STRETCH_NEG_X,
		TRANSFORM_STRETCH_POS_Y,
		TRANSFORM_STRETCH_NEG_Y,
		TRANSFORM_STRETCH_POS_Z,
		TRANSFORM_STRETCH_NEG_Z,
		TRANSFORM_STRETCH_POS_W,
		TRANSFORM_STRETCH_NEG_W,
		TRANSFORM_MAX,
	};

	Node4D *_mesh_holder = nullptr;
	MeshInstance4D *_meshes[TRANSFORM_MAX] = { nullptr };
	EditorUndoRedoManager *_undo_redo = nullptr;

	KeepMode _keep_mode = KeepMode::FREEFORM;
	TransformPart _current_transformation = TRANSFORM_NONE;
	TransformPart _highlighted_transformation = TRANSFORM_NONE;

	Transform4D _old_transform = Transform4D();
	Variant _transform_reference_value = Variant();
	TypedArray<Node> _selected_top_nodes;
	Vector<Transform4D> _selected_top_node_old_transforms;

	bool _is_move_linear_enabled = true;
	bool _is_move_planar_enabled = false;
	bool _is_rotation_enabled = false;
	bool _is_scale_linear_enabled = false;
	bool _is_scale_planar_enabled = false;
	bool _is_stretch_enabled = false;
	bool _is_use_local_rotation = false;
	bool _are_generated_meshes_wireframes = false;

	// Setup functions.
	MeshInstance4D *_make_mesh_instance_4d(const StringName &p_name, const Ref<ArrayWireMesh4D> &p_mesh, const Ref<WireMaterial4D> &p_material);
	void _generate_gizmo_meshes(const PackedColorArray &p_axis_colors);

	// Misc internal functions.
	void _on_rendering_server_pre_render(Camera4D *p_camera, Viewport *p_viewport, RenderingEngine4D *p_rendering_engine);
	void _on_editor_inspector_property_edited(const String &p_prop);
	void _on_undo_redo_version_changed();
	void _update_gizmo_transform();
	void _update_gizmo_mesh_transform(const Camera4D *p_camera);
	Rect4 _get_rect_bounds_of_selection(const Transform4D &p_inv_relative_to) const;

	// Highlighting functions, used when not transforming.
	TransformPart _check_for_best_hit(const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction) const;
	void _unhighlight_mesh(TransformPart p_transformation);
	void _highlight_mesh(TransformPart p_transformation);

	// Transformation functions.
	Variant _get_transform_raycast_value(const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction);
	void _begin_transformation(const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction);
	void _end_transformation();
	void _process_transform(const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction);

protected:
	static void _bind_methods() {}

public:
	void selected_nodes_changed(const TypedArray<Node> &p_top_nodes);
	void theme_changed(const PackedColorArray &p_axis_colors);
	void set_keep_mode(const KeepMode p_mode);
	void set_gizmo_mode(const GizmoMode p_mode);
	bool gizmo_mouse_input(const Ref<InputEventMouse> &p_mouse_event, const Camera4D *p_camera);
	bool gizmo_mouse_raycast(const Ref<InputEventMouse> &p_mouse_event, const Vector4 &p_ray_origin, const Vector4 &p_ray_direction, const Vector4 &p_perp_direction);

	bool get_use_local_rotation() const;
	void set_use_local_rotation(const bool p_use_local_transform);

	void setup(EditorUndoRedoManager *p_undo_redo_manager);
};
