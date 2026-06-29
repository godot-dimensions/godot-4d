#include "editor_main_viewport_4d.h"

#include "../../model/mesh/tetra/tetra_mesh_4d.h"
#include "../../nodes/camera_4d.h"
#include "editor_camera_4d.h"
#include "editor_input_surface_4d.h"
#include "editor_transform_gizmo_4d.h"
#include "editor_viewport_rotation_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/input_event_screen_drag.hpp>
#include <godot_cpp/classes/input_event_screen_touch.hpp>
#elif GODOT_MODULE
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#endif

Vector2 EditorMainViewport4D::_get_warped_mouse_motion(const Ref<InputEvent> &p_input_event) const {
	Ref<InputEventMouseMotion> ev_mouse_motion = p_input_event;
	if (ev_mouse_motion.is_valid()) {
#if GODOT_MODULE
		if (bool(EDITOR_GET("editors/3d/navigation/warped_mouse_panning"))) {
			return Input::get_singleton()->warp_mouse_motion(ev_mouse_motion, _input_surface_4d->get_global_rect());
		}
#endif // GODOT_MODULE
		return ev_mouse_motion->get_relative();
	}
	Ref<InputEventScreenDrag> ev_screen_drag = p_input_event;
	if (ev_screen_drag.is_valid()) {
		return ev_screen_drag->get_relative();
	}
	ERR_FAIL_V_MSG(Vector2(), "Expected InputEventMouseMotion or InputEventScreenDrag.");
}

void EditorMainViewport4D::_update_theme() {
	// Set the information label offsets.
	_information_label->set_offset(SIDE_LEFT, 6.0f * EDSCALE);
	_information_label->set_offset(SIDE_RIGHT, -6.0f * EDSCALE);
	_information_label->set_offset(SIDE_TOP, -40.0f * EDSCALE);
	_information_label->set_offset(SIDE_BOTTOM, -6.0f * EDSCALE);
	// Set the axis colors.
	_axis_colors.clear();
	_axis_colors.push_back(get_theme_color(StringName("axis_x_color"), StringName("Editor")));
	_axis_colors.push_back(get_theme_color(StringName("axis_y_color"), StringName("Editor")));
	_axis_colors.push_back(get_theme_color(StringName("axis_z_color"), StringName("Editor")));
	_axis_colors.push_back(Color(0.9f, 0.75f, 0.1f)); // W axis color.
	_transform_gizmo_4d->theme_changed(_axis_colors);
}

void EditorMainViewport4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			_information_label_auto_hide_time -= get_process_delta_time();
			if (_information_label_auto_hide_time <= 0.0) {
				_information_label->hide();
				set_process(false);
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			_update_theme();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme();
		} break;
	}
}

void EditorMainViewport4D::focus_selected_nodes() {
	TypedArray<Node> selected_nodes = EditorInterface::get_singleton()->get_selection()->get_selected_nodes();
	Vector4 position_sum = Vector4();
	int position_count = 0;
	for (int i = 0; i < selected_nodes.size(); i++) {
		Node4D *node_4d = Object::cast_to<Node4D>(selected_nodes[i]);
		if (node_4d != nullptr) {
			position_sum += node_4d->get_global_position();
			position_count++;
		}
	}
	if (position_count > 0) {
		_editor_camera_4d->set_target_position(position_sum / position_count);
	}
}

PackedColorArray EditorMainViewport4D::get_axis_colors() const {
	return _axis_colors;
}

Basis4D EditorMainViewport4D::get_view_camera_basis() const {
	return _editor_camera_4d->get_basis();
}

bool EditorMainViewport4D::_should_mouse_motion_affect_4d(const Ref<InputEventMouseMotion> &p_ev_mouse_motion) const {
	if (p_ev_mouse_motion.is_null()) {
		return false;
	}
	return p_ev_mouse_motion->is_ctrl_pressed() || p_ev_mouse_motion->is_command_or_control_pressed() || Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT);
}

void EditorMainViewport4D::navigation_freelook(const Ref<InputEvent> &p_input_event) {
	Vector2 relative = _get_warped_mouse_motion(p_input_event);
	const real_t degrees_per_pixel = EDITOR_GET("editors/3d/freelook/freelook_sensitivity");
	const real_t radians_per_pixel = Math::deg_to_rad(degrees_per_pixel);
	const bool invert_y_axis = EDITOR_GET("editors/3d/navigation/invert_y_axis");
	Vector2 rotation_radians = relative * radians_per_pixel;
	if (invert_y_axis) {
		rotation_radians.y = -rotation_radians.y;
	}
	if (_should_mouse_motion_affect_4d(p_input_event)) {
		_editor_camera_4d->freelook_rotate_ground_basis(Basis4D::from_xw(-rotation_radians.x) * Basis4D::from_zw(rotation_radians.y));
	} else {
		_editor_camera_4d->freelook_rotate_ground_basis_and_pitch(Basis4D::from_zx(-rotation_radians.x), -rotation_radians.y);
	}
	_viewport_rotation_4d->queue_redraw();
}

void EditorMainViewport4D::navigation_orbit(const Ref<InputEvent> &p_input_event) {
	Vector2 relative = _get_warped_mouse_motion(p_input_event);
	const real_t degrees_per_pixel = EDITOR_GET("editors/3d/navigation_feel/orbit_sensitivity");
	const real_t radians_per_pixel = Math::deg_to_rad(degrees_per_pixel);
	const bool invert_y_axis = EDITOR_GET("editors/3d/navigation/invert_y_axis");
	const bool invert_x_axis = EDITOR_GET("editors/3d/navigation/invert_x_axis");
	Vector2 rotation_radians = relative * radians_per_pixel;
	if (invert_x_axis) {
		rotation_radians.x = -rotation_radians.x;
	}
	if (invert_y_axis) {
		rotation_radians.y = -rotation_radians.y;
	}
	if (_should_mouse_motion_affect_4d(p_input_event)) {
		_editor_camera_4d->orbit_rotate_ground_basis(Basis4D::from_xw(-rotation_radians.x) * Basis4D::from_zw(rotation_radians.y));
	} else {
		_editor_camera_4d->orbit_rotate_ground_basis_and_pitch(Basis4D::from_zx(-rotation_radians.x), -rotation_radians.y);
	}
	_viewport_rotation_4d->queue_redraw();
}

void EditorMainViewport4D::navigation_pan(const Ref<InputEvent> &p_input_event) {
	Vector2 relative = _get_warped_mouse_motion(p_input_event);
	Vector4 pan;
	if (_should_mouse_motion_affect_4d(p_input_event)) {
		pan = Vector4(0.0f, 0.0f, relative.y, relative.x);
	} else {
		pan = Vector4(-relative.x, relative.y, 0.0f, 0.0f);
	}
	_editor_camera_4d->pan_camera(pan / get_size().y);
}

String _viewport_4d_format_number(const double p_number) {
	const int decimals = MAX(0, 3 - log10(p_number));
	String number_text = String::num(p_number, decimals);
	if (number_text.length() < 3 && !number_text.contains(".")) {
		number_text += ".0";
	}
	return number_text;
}

void EditorMainViewport4D::navigation_change_speed(const double p_speed_change) {
	const double speed_and_zoom = _editor_camera_4d->change_speed_and_zoom(p_speed_change);
	set_information_text("Speed: " + _viewport_4d_format_number(speed_and_zoom) + "m/s");
}

void EditorMainViewport4D::navigation_change_zoom(const double p_zoom_change) {
	const double speed_and_zoom = _editor_camera_4d->change_speed_and_zoom(p_zoom_change);
	set_information_text("Zoom: " + _viewport_4d_format_number(speed_and_zoom) + "m");
}

// The "target" refers to which node should actually be selected when a given node is clicked.
// If a mesh belongs to an instantiated scene, the target should be the top-level node of that scene, not the node with the mesh.
void EditorMainViewport4D::_gather_non_empty_visible_mesh_4d_nodes(Node *p_edited_scene_root, Node *p_from_node, Node *p_target_node, Vector<Node4D *> &r_nodes, Vector<Node *> &r_targets, Vector<Rect4> &r_rect_bounds) {
	if (p_from_node == nullptr) {
		return;
	}
	// Handle self.
	Node4D *node_4d = Object::cast_to<Node4D>(p_from_node);
	if (node_4d != nullptr && node_4d->is_visible_in_tree()) {
		const Rect4 rect_bounds = node_4d->get_rect_bounds_global();
		if (rect_bounds.has_any_size()) {
			r_nodes.append(node_4d);
			r_targets.append(p_target_node);
			r_rect_bounds.append(rect_bounds);
		}
	}
	// Check if the target should propagate down to children or stay the same.
	// We only target children when the passed-in nodes match, otherwise the target has already changed.
	bool target_children;
	if (p_from_node == p_edited_scene_root) {
		// Always target children of the edited scene root, even for inherited scenes.
		target_children = true;
	} else {
		target_children = p_from_node == p_target_node;
		if (target_children) {
			const bool is_root_of_a_scene = !p_from_node->get_scene_file_path().is_empty();
			const bool is_editable = p_edited_scene_root->is_editable_instance(p_from_node);
			// We also want to stop propagating if the node is the root of a non-editable scene instance.
			target_children = !is_root_of_a_scene || is_editable;
		}
	}
	const int child_count = p_from_node->get_child_count();
	for (int i = 0; i < child_count; i++) {
		Node *child_node = p_from_node->get_child(i);
		Node *target_node = target_children ? child_node : p_target_node;
		_gather_non_empty_visible_mesh_4d_nodes(p_edited_scene_root, child_node, target_node, r_nodes, r_targets, r_rect_bounds);
	}
}

Node *EditorMainViewport4D::_raycast_from_mouse(const Vector2 &p_mouse_position, const Camera4D *p_camera) {
	const Vector4 global_ray_origin = p_camera->viewport_to_world_ray_origin(p_mouse_position);
	const Vector4 global_ray_direction = p_camera->viewport_to_world_ray_direction(p_mouse_position);
	// Gather all meshes and other visible nodes in the scene.
	Node *edited_scene_root = EditorInterface::get_singleton()->get_edited_scene_root();
	if (edited_scene_root == nullptr) {
		// Silently exit if there is no edited scene root. This can happen when the editor
		// is first opened and no scene is loaded yet, or the user closed all scenes.
		return nullptr;
	}
	Vector<Node4D *> nodes;
	Vector<Node *> targets;
	Vector<Rect4> rect_bounds; // Global space.
	_gather_non_empty_visible_mesh_4d_nodes(edited_scene_root, edited_scene_root, edited_scene_root, nodes, targets, rect_bounds);
	// First pass: Check the rect bounds of each node and exclude any that don't intersect the ray.
	// This is a quick check to avoid doing more expensive raycasts than necessary.
	for (int64_t i = nodes.size() - 1; i >= 0; i--) {
		const Rect4 &rect_bound = rect_bounds[i];
		bool raycast_intersects = rect_bound.raycast_intersects(global_ray_origin, global_ray_direction, true, nullptr, nullptr);
		if (!raycast_intersects) {
			nodes.remove_at(i);
			targets.remove_at(i);
			rect_bounds.remove_at(i);
		}
	}
	// Cache inverse global transforms for each node to avoid recalculating them.
	Vector<Transform4D> inverse_global_transforms;
	for (int64_t i = 0; i < nodes.size(); i++) {
		inverse_global_transforms.append(nodes[i]->get_global_transform().inverse());
	}
	// Second pass: For some types, perform fast checks before doing more expensive ones.
	for (int64_t i = nodes.size() - 1; i >= 0; i--) {
		Node4D *node_4d = nodes[i];
		MeshInstance4D *mesh_instance_4d = Object::cast_to<MeshInstance4D>(node_4d);
		if (mesh_instance_4d != nullptr) {
			Ref<TetraMesh4D> tetra_mesh = mesh_instance_4d->get_mesh();
			if (tetra_mesh.is_valid()) {
				tetra_mesh->populate_inverse_metric_cache(); // This will exit quickly if already populated.
				const Vector4 local_ray_origin = inverse_global_transforms[i].xform(global_ray_origin);
				const Vector4 local_ray_direction = inverse_global_transforms[i].basis.xform(global_ray_direction).normalized();
				bool hit = tetra_mesh->raycast_intersects_fast(local_ray_origin, local_ray_direction);
				if (!hit) {
					nodes.remove_at(i);
					targets.remove_at(i);
					rect_bounds.remove_at(i);
					inverse_global_transforms.remove_at(i);
				}
			}
		}
	}
	// Third pass: For nodes that intersected the ray, find the first hit along the ray.
	Node *nearest_target_node = nullptr;
	double nearest_distance = Math_INF;
	for (int64_t i = nodes.size() - 1; i >= 0; i--) {
		Node4D *node_4d = nodes[i];
		const Vector4 local_ray_origin = inverse_global_transforms[i].xform(global_ray_origin);
		const Vector4 local_ray_direction = inverse_global_transforms[i].basis.xform(global_ray_direction).normalized();
		const Dictionary raycast_result = node_4d->raycast_intersects_local(local_ray_origin, local_ray_direction, false);
		if (raycast_result.has("hit")) {
			const bool hit = raycast_result["hit"];
			if (hit && raycast_result.has("distance")) {
				// Variant's float type is double, so use double here to avoid precision loss.
				const double distance = raycast_result["distance"];
				if (nearest_distance > distance) {
					nearest_distance = distance;
					nearest_target_node = targets[i];
				}
			}
		}
	}
	return nearest_target_node;
}

// This uses InputEvent instead of InputEventMouse so that it can handle touch events as well.
void EditorMainViewport4D::viewport_mouse_input(const Ref<InputEvent> &p_input_event) {
	Ref<InputEventMouse> mouse_event = p_input_event;
	Vector2 mouse_position;
	const Camera4D *camera = _editor_camera_4d->get_camera_readonly();
	int released_button_index = -1;
	if (mouse_event.is_valid()) {
		const bool used_by_gizmo = _transform_gizmo_4d->gizmo_mouse_input(mouse_event, camera);
		if (used_by_gizmo) {
			return;
		}
		// Try to make a selection if the transform gizmo didn't use the mouse.
		Ref<InputEventMouseButton> mouse_button_event = mouse_event;
		if (mouse_button_event.is_valid()) {
			mouse_position = mouse_button_event->get_position();
			if (mouse_button_event->is_released()) {
				released_button_index = (int)mouse_button_event->get_button_index();
			}
		}
	} else {
		Ref<InputEventScreenTouch> touch_event = p_input_event;
		if (touch_event.is_valid()) {
			mouse_position = touch_event->get_position();
			if (touch_event->is_released()) {
				released_button_index = touch_event->get_index();
			}
		}
	}
	// If a mouse button was released (or a touch ended), try to select the node under the mouse.
	if (released_button_index == (int)MOUSE_BUTTON_LEFT) {
		Node *node_under_mouse = _raycast_from_mouse(mouse_position, camera);
		EditorSelection *selection = EditorInterface::get_singleton()->get_selection();
		if (node_under_mouse == nullptr) {
			selection->clear();
		} else {
			if (mouse_event.is_valid() && (mouse_event->is_shift_pressed() || mouse_event->is_command_or_control_pressed())) {
				selection->add_node(node_under_mouse);
			} else {
				selection->clear();
				selection->add_node(node_under_mouse);
			}
		}
	}
}

void EditorMainViewport4D::set_ground_view_axis(const Vector4::Axis p_axis) {
	_editor_camera_4d->set_ground_view_axis(p_axis);
	_viewport_rotation_4d->queue_redraw();
}

void EditorMainViewport4D::set_information_text(const String &p_text, const double p_auto_hide_time) {
	_information_label->set_text(p_text);
	_information_label->show();
	_information_label_auto_hide_time = p_auto_hide_time;
	set_process(true);
}

void EditorMainViewport4D::set_orthogonal_view_plane(const Vector4::Axis p_right, const Vector4::Axis p_up) {
	_editor_camera_4d->set_orthogonal_view_plane(p_right, p_up);
	_viewport_rotation_4d->queue_redraw();
}

void EditorMainViewport4D::setup(EditorMainScreen4D *p_editor_main_screen, EditorTransformGizmo4D *p_transform_gizmo_4d) {
	// Things that we should do in the constructor but can't in GDExtension
	// due to how GDExtension runs the constructor for each registered class.
	set_name(StringName("EditorMainViewport4D"));
	set_clip_children_mode(Control::CLIP_CHILDREN_AND_DRAW);
	set_v_size_flags(SIZE_EXPAND_FILL);
	set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	// Set up the viewport container and camera.
	_sub_viewport_container = memnew(SubViewportContainer);
	_sub_viewport_container->set_clip_children_mode(Control::CLIP_CHILDREN_AND_DRAW);
	_sub_viewport_container->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	_sub_viewport_container->set_stretch(true);
	add_child(_sub_viewport_container);

	_sub_viewport = memnew(SubViewport);
	_sub_viewport_container->add_child(_sub_viewport);

	_information_label = memnew(Label);
	_information_label->set_anchors_and_offsets_preset(Control::PRESET_BOTTOM_WIDE);
	_information_label->set_vertical_alignment(VerticalAlignment::VERTICAL_ALIGNMENT_BOTTOM);
	_sub_viewport_container->add_child(_information_label);

	_editor_camera_4d = memnew(EditorCamera4D);
	_editor_camera_4d->setup();
	_sub_viewport->add_child(_editor_camera_4d);

	// Set up the input surfaces.
	_input_surface_4d = memnew(EditorInputSurface4D);
	_sub_viewport_container->add_child(_input_surface_4d);

	_viewport_rotation_4d = memnew(EditorViewportRotation4D);
	_viewport_rotation_4d->setup(this);
	_input_surface_4d->add_child(_viewport_rotation_4d);

	// Set up things with the arguments (not constructor things).
	_editor_main_screen = p_editor_main_screen;
	_input_surface_4d->setup(_editor_main_screen, this);
	_transform_gizmo_4d = p_transform_gizmo_4d;
}
