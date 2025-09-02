#include "camera_4d.h"

#include "../render/rendering_server_4d.h"

void Camera4D::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == StringName("rendering_engine")) {
		PackedStringArray rendering_engine_names = RenderingServer4D::get_singleton()->get_rendering_engine_names();
		p_property.hint_string = String(",").join(rendering_engine_names);
	} else if (p_property.name == StringName("view_angle_type")) {
		if (!(_projection_type & PROJECTION4D_PERSPECTIVE_DUAL)) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("focal_length_3d")) {
		if (_view_angle_type != VIEW_ANGLE_FOCAL_LENGTH || !is_second_pass_perspective()) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("focal_length_4d")) {
		if (_view_angle_type != VIEW_ANGLE_FOCAL_LENGTH || !is_first_pass_perspective()) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("field_of_view_3d")) {
		if (_view_angle_type != VIEW_ANGLE_FIELD_OF_VIEW || !is_second_pass_perspective()) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("field_of_view_4d")) {
		if (_view_angle_type != VIEW_ANGLE_FIELD_OF_VIEW || !is_first_pass_perspective()) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("orthographic_size")) {
		if (_projection_type != PROJECTION4D_ORTHOGRAPHIC) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("w_fade_color_negative")) {
		if (!(_w_fade_mode & W_FADE_HUE_SHIFT)) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("w_fade_color_positive")) {
		if (!(_w_fade_mode & W_FADE_HUE_SHIFT)) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("w_fade_distance")) {
		if (_w_fade_mode == W_FADE_DISABLED) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("w_fade_slope")) {
		if (_w_fade_mode == W_FADE_DISABLED) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	}
}

void Camera4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			RenderingServer4D::get_singleton()->register_camera(this);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			RenderingServer4D::get_singleton()->unregister_camera(this);
		} break;
	}
}

bool Camera4D::is_current() const {
	return _is_current;
}

void Camera4D::set_current(bool p_enabled) {
	_is_current = p_enabled;
	if (is_inside_tree()) {
		if (p_enabled) {
			RenderingServer4D::get_singleton()->make_camera_current(this);
		} else {
			RenderingServer4D::get_singleton()->clear_camera_current(this);
		}
	}
}

void Camera4D::clear_current(bool p_enable_next) {
	_is_current = false;
	if (p_enable_next && is_inside_tree()) {
		RenderingServer4D::get_singleton()->clear_camera_current(this);
	}
}

void Camera4D::make_current() {
	_is_current = true;
	if (is_inside_tree()) {
		RenderingServer4D::get_singleton()->make_camera_current(this);
	}
}

void Camera4D::set_depth_fade_mode(const DepthFadeMode p_depth_fade_mode) {
	_depth_fade_mode = p_depth_fade_mode;
}

void Camera4D::set_depth_fade_start(const double p_depth_fade_start) {
	_depth_fade_start = p_depth_fade_start;
}

double Camera4D::get_depth_fade_start() const {
	return _depth_fade_start;
}

bool Camera4D::is_position_behind(const Vector4 &p_global_position) const {
	const Transform4D global_xform = get_global_transform();
	return global_xform.basis.z.dot(p_global_position - global_xform.origin) > -_clip_near;
}

Vector4 Camera4D::viewport_to_world_ray_origin(const Vector2 &p_viewport_position) const {
	Viewport *viewport = get_viewport();
	ERR_FAIL_COND_V_MSG(viewport == nullptr, Vector4(), "Camera4D must be in the scene tree to convert viewport coordinates to world coordinates.");
	const Transform4D global_xform = get_global_transform();
	// Perspective cameras always have their ray origin at the camera's position.
	if (_projection_type != PROJECTION4D_ORTHOGRAPHIC) {
		return global_xform.origin;
	}
	// Orthographic cameras have ray origins offset by the orthographic size.
	const Vector2 viewport_size = viewport->call(StringName("get_size"));
	const double pixel_size = _keep_aspect == KEEP_WIDTH ? viewport_size.x : viewport_size.y;
	const Vector2 scaled_position = (p_viewport_position * 2.0f - viewport_size) * (_orthographic_size / pixel_size);
	return global_xform.origin + global_xform.basis.x * scaled_position.x + global_xform.basis.y * -scaled_position.y;
}

Vector4 Camera4D::viewport_to_world_ray_direction(const Vector2 &p_viewport_position) const {
	Viewport *viewport = get_viewport();
	ERR_FAIL_COND_V_MSG(viewport == nullptr, Vector4(), "Camera4D must be in the scene tree to convert viewport coordinates to world coordinates.");
	const Transform4D global_xform = get_global_transform();
	// Orthographic cameras always have their ray direction pointing straight down the negative Z-axis.
	if (_projection_type == PROJECTION4D_ORTHOGRAPHIC) {
		return -global_xform.basis.z;
	}
	// Perspective cameras have ray directions pointing more to the side when near the sides of the viewport.
	const Vector2 viewport_size = viewport->call(StringName("get_size"));
	const double pixel_size = _keep_aspect == KEEP_WIDTH ? viewport_size.x : viewport_size.y;
	const double focal_length = _projection_type == PROJECTION4D_PERSPECTIVE_4D ? _focal_length_4d : _focal_length_3d;
	const Vector2 scaled_position = (p_viewport_position * 2.0f - viewport_size) / pixel_size;
	const Vector4 ray_direction = global_xform.basis.x * scaled_position.x + global_xform.basis.y * -scaled_position.y + global_xform.basis.z * -focal_length;
	return ray_direction.normalized();
}

Vector2 Camera4D::world_to_viewport_local_normal(const Vector4 &p_local_position) const {
	if (_projection_type == Camera4D::PROJECTION4D_ORTHOGRAPHIC) {
		return Vector2(p_local_position.x, -p_local_position.y) / _orthographic_size;
	}
	// Project from 4D to 3D.
	Vector3 projected_point_3d;
	if (bool(_projection_type & PROJECTION4D_PERSPECTIVE_4D)) {
		projected_point_3d = Vector3(p_local_position.x, p_local_position.y, p_local_position.w) * (_focal_length_4d / p_local_position.z);
	} else {
		projected_point_3d = Vector3(p_local_position.x, p_local_position.y, p_local_position.z);
	}
	// Project from 3D to 2D.
	if (bool(_projection_type & PROJECTION4D_PERSPECTIVE_3D)) {
		return Vector2(-projected_point_3d.x, projected_point_3d.y) * (_focal_length_3d / projected_point_3d.z);
	}
	return Vector2(-projected_point_3d.x, projected_point_3d.y);
}

Vector2 Camera4D::world_to_viewport(const Vector4 &p_global_position) const {
	Viewport *viewport = get_viewport();
	ERR_FAIL_COND_V_MSG(viewport == nullptr, Vector2(), "Camera4D must be in the scene tree to convert world coordinates to viewport coordinates.");
	const Vector4 local_position = get_global_transform().xform_transposed(p_global_position);
	const Vector2 viewport_size = viewport->call(StringName("get_size"));
	const double pixel_size = _keep_aspect == KEEP_WIDTH ? viewport_size.x : viewport_size.y;
	const Vector2 projected = world_to_viewport_local_normal(local_position);
	return (projected * pixel_size + viewport_size) * 0.5f;
}

String Camera4D::get_rendering_engine() const {
	return _rendering_engine;
}

void Camera4D::set_rendering_engine(const String &p_rendering_engine) {
	_rendering_engine = p_rendering_engine;
}

Camera4D::KeepAspect Camera4D::get_keep_aspect() const {
	return _keep_aspect;
}

void Camera4D::set_keep_aspect(const KeepAspect p_keep_aspect) {
	_keep_aspect = p_keep_aspect;
}

Camera4D::ProjectionType4D Camera4D::get_projection_type() const {
	return _projection_type;
}

void Camera4D::set_projection_type(const ProjectionType4D p_projection_type_4d) {
	_projection_type = p_projection_type_4d;
	notify_property_list_changed();
}

bool Camera4D::is_first_pass_perspective() const {
	return bool(_projection_type & PROJECTION4D_PERSPECTIVE_4D);
}

bool Camera4D::is_second_pass_perspective() const {
	return bool(_projection_type & PROJECTION4D_PERSPECTIVE_3D);
}

Camera4D::ViewAngleType Camera4D::get_view_angle_type() const {
	return _view_angle_type;
}

void Camera4D::set_view_angle_type(const ViewAngleType p_view_angle_type) {
	_view_angle_type = p_view_angle_type;
	notify_property_list_changed();
}

double Camera4D::get_focal_length_4d() const {
	return _focal_length_4d;
}

void Camera4D::set_focal_length_4d(const double p_focal_length_4d) {
	_focal_length_4d = p_focal_length_4d;
}

double Camera4D::get_focal_length_3d() const {
	return _focal_length_3d;
}

void Camera4D::set_focal_length_3d(const double p_focal_length_3d) {
	_focal_length_3d = p_focal_length_3d;
}

double Camera4D::get_field_of_view_4d() const {
	return Math_PI - 2.0 * Math::atan(_focal_length_4d);
}

void Camera4D::set_field_of_view_4d(const double p_field_of_view_4d) {
	_focal_length_4d = Math::tan((Math_PI - p_field_of_view_4d) * 0.5);
}

double Camera4D::get_field_of_view_3d() const {
	return Math_PI - 2.0 * Math::atan(_focal_length_3d);
}

void Camera4D::set_field_of_view_3d(const double p_field_of_view_3d) {
	_focal_length_3d = Math::tan((Math_PI - p_field_of_view_3d) * 0.5);
}

double Camera4D::get_orthographic_size() const {
	return _orthographic_size;
}

void Camera4D::set_orthographic_size(const double p_orthographic_size) {
	_orthographic_size = p_orthographic_size;
}

double Camera4D::get_clip_near() const {
	return _clip_near;
}

void Camera4D::set_clip_near(const double p_clip_near) {
	_clip_near = p_clip_near;
}

double Camera4D::get_clip_far() const {
	return _clip_far;
}

void Camera4D::set_clip_far(const double p_clip_far) {
	_clip_far = p_clip_far;
}

Camera4D::WFadeMode Camera4D::get_w_fade_mode() const {
	return _w_fade_mode;
}

void Camera4D::set_w_fade_mode(const WFadeMode p_w_fade_mode) {
	_w_fade_mode = p_w_fade_mode;
	notify_property_list_changed();
}

Color Camera4D::get_w_fade_color_negative() const {
	return _w_fade_color_negative;
}

void Camera4D::set_w_fade_color_negative(const Color &p_w_fade_color_negative) {
	_w_fade_color_negative = p_w_fade_color_negative;
}

Color Camera4D::get_w_fade_color_positive() const {
	return _w_fade_color_positive;
}

void Camera4D::set_w_fade_color_positive(const Color &p_w_fade_color_positive) {
	_w_fade_color_positive = p_w_fade_color_positive;
}

double Camera4D::get_w_fade_distance() const {
	return _w_fade_distance;
}

void Camera4D::set_w_fade_distance(const double p_w_fade_distance) {
	_w_fade_distance = p_w_fade_distance;
}

double Camera4D::get_w_fade_slope() const {
	return _w_fade_slope;
}

void Camera4D::set_w_fade_slope(const double p_w_fade_slope) {
	_w_fade_slope = p_w_fade_slope;
}

void Camera4D::_bind_methods() {
	// Be sure to keep the relevant properties in sync with EditorCameraSettings4D.
	ClassDB::bind_method(D_METHOD("is_current"), &Camera4D::is_current);
	ClassDB::bind_method(D_METHOD("set_current", "enabled"), &Camera4D::set_current);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "current"), "set_current", "is_current");
	ClassDB::bind_method(D_METHOD("clear_current", "enable_next"), &Camera4D::clear_current);
	ClassDB::bind_method(D_METHOD("make_current"), &Camera4D::make_current);

	ClassDB::bind_method(D_METHOD("is_position_behind", "global_position"), &Camera4D::is_position_behind);
	ClassDB::bind_method(D_METHOD("viewport_to_world_ray_origin", "viewport_position"), &Camera4D::viewport_to_world_ray_origin);
	ClassDB::bind_method(D_METHOD("viewport_to_world_ray_direction", "viewport_position"), &Camera4D::viewport_to_world_ray_direction);
	ClassDB::bind_method(D_METHOD("world_to_viewport_local_normal", "local_position"), &Camera4D::world_to_viewport_local_normal);
	ClassDB::bind_method(D_METHOD("world_to_viewport", "global_position"), &Camera4D::world_to_viewport);

	ClassDB::bind_method(D_METHOD("get_rendering_engine"), &Camera4D::get_rendering_engine);
	ClassDB::bind_method(D_METHOD("set_rendering_engine", "rendering_engine"), &Camera4D::set_rendering_engine);
	// The property hint string will be filled out when _validate_property is called.
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "rendering_engine", PROPERTY_HINT_ENUM, ""), "set_rendering_engine", "get_rendering_engine");

	ClassDB::bind_method(D_METHOD("get_keep_aspect"), &Camera4D::get_keep_aspect);
	ClassDB::bind_method(D_METHOD("set_keep_aspect", "keep_aspect"), &Camera4D::set_keep_aspect);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "keep_aspect", PROPERTY_HINT_ENUM, "Keep Width,Keep Height"), "set_keep_aspect", "get_keep_aspect");

	ClassDB::bind_method(D_METHOD("get_projection_type"), &Camera4D::get_projection_type);
	ClassDB::bind_method(D_METHOD("set_projection_type", "projection_type_4d"), &Camera4D::set_projection_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "projection_type", PROPERTY_HINT_ENUM, "Orthographic,Perspective 4D (recommended),Orthographic 4D + Perspective 3D,Dual Perspective"), "set_projection_type", "get_projection_type");
	ClassDB::bind_method(D_METHOD("is_first_pass_perspective"), &Camera4D::is_first_pass_perspective);
	ClassDB::bind_method(D_METHOD("is_second_pass_perspective"), &Camera4D::is_second_pass_perspective);

	ClassDB::bind_method(D_METHOD("get_view_angle_type"), &Camera4D::get_view_angle_type);
	ClassDB::bind_method(D_METHOD("set_view_angle_type", "view_angle_type"), &Camera4D::set_view_angle_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "view_angle_type", PROPERTY_HINT_ENUM, "Focal Length,Field of View"), "set_view_angle_type", "get_view_angle_type");

	ClassDB::bind_method(D_METHOD("get_focal_length_4d"), &Camera4D::get_focal_length_4d);
	ClassDB::bind_method(D_METHOD("set_focal_length_4d", "focal_length_4d"), &Camera4D::set_focal_length_4d);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "focal_length_4d", PROPERTY_HINT_NONE, "suffix:m"), "set_focal_length_4d", "get_focal_length_4d");

	ClassDB::bind_method(D_METHOD("get_focal_length_3d"), &Camera4D::get_focal_length_3d);
	ClassDB::bind_method(D_METHOD("set_focal_length_3d", "focal_length_3d"), &Camera4D::set_focal_length_3d);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "focal_length_3d", PROPERTY_HINT_NONE, "suffix:m"), "set_focal_length_3d", "get_focal_length_3d");

	ClassDB::bind_method(D_METHOD("get_field_of_view_4d"), &Camera4D::get_field_of_view_4d);
	ClassDB::bind_method(D_METHOD("set_field_of_view_4d", "field_of_view_4d"), &Camera4D::set_field_of_view_4d);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "field_of_view_4d", PROPERTY_HINT_RANGE, "1,179,0.1,radians_as_degrees"), "set_field_of_view_4d", "get_field_of_view_4d");

	ClassDB::bind_method(D_METHOD("get_field_of_view_3d"), &Camera4D::get_field_of_view_3d);
	ClassDB::bind_method(D_METHOD("set_field_of_view_3d", "field_of_view_3d"), &Camera4D::set_field_of_view_3d);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "field_of_view_3d", PROPERTY_HINT_RANGE, "1,179,0.1,radians_as_degrees"), "set_field_of_view_3d", "get_field_of_view_3d");

	ClassDB::bind_method(D_METHOD("get_orthographic_size"), &Camera4D::get_orthographic_size);
	ClassDB::bind_method(D_METHOD("set_orthographic_size", "orthographic_size"), &Camera4D::set_orthographic_size);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "orthographic_size", PROPERTY_HINT_NONE, "suffix:m"), "set_orthographic_size", "get_orthographic_size");

	ClassDB::bind_method(D_METHOD("get_clip_near"), &Camera4D::get_clip_near);
	ClassDB::bind_method(D_METHOD("set_clip_near", "clip_near"), &Camera4D::set_clip_near);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clip_near", PROPERTY_HINT_RANGE, "0.001,1000,0.001,or_greater,exp,suffix:m"), "set_clip_near", "get_clip_near");

	ClassDB::bind_method(D_METHOD("get_clip_far"), &Camera4D::get_clip_far);
	ClassDB::bind_method(D_METHOD("set_clip_far", "clip_far"), &Camera4D::set_clip_far);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clip_far", PROPERTY_HINT_RANGE, "0.01,4000,0.01,or_greater,exp,suffix:m"), "set_clip_far", "get_clip_far");

	ClassDB::bind_method(D_METHOD("get_w_fade_mode"), &Camera4D::get_w_fade_mode);
	ClassDB::bind_method(D_METHOD("set_w_fade_mode", "w_fade_mode"), &Camera4D::set_w_fade_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "w_fade_mode", PROPERTY_HINT_ENUM, "Disabled,Transparency,Hue Shift,Transparency + Hue Shift"), "set_w_fade_mode", "get_w_fade_mode");

	ClassDB::bind_method(D_METHOD("get_w_fade_color_negative"), &Camera4D::get_w_fade_color_negative);
	ClassDB::bind_method(D_METHOD("set_w_fade_color_negative", "w_fade_color_negative"), &Camera4D::set_w_fade_color_negative);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "w_fade_color_negative"), "set_w_fade_color_negative", "get_w_fade_color_negative");

	ClassDB::bind_method(D_METHOD("get_w_fade_color_positive"), &Camera4D::get_w_fade_color_positive);
	ClassDB::bind_method(D_METHOD("set_w_fade_color_positive", "w_fade_color_positive"), &Camera4D::set_w_fade_color_positive);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "w_fade_color_positive"), "set_w_fade_color_positive", "get_w_fade_color_positive");

	ClassDB::bind_method(D_METHOD("get_w_fade_distance"), &Camera4D::get_w_fade_distance);
	ClassDB::bind_method(D_METHOD("set_w_fade_distance", "w_fade_distance"), &Camera4D::set_w_fade_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "w_fade_distance", PROPERTY_HINT_RANGE, "0.001,100,0.001,or_greater,or_less,exp,suffix:m"), "set_w_fade_distance", "get_w_fade_distance");

	ClassDB::bind_method(D_METHOD("get_w_fade_slope"), &Camera4D::get_w_fade_slope);
	ClassDB::bind_method(D_METHOD("set_w_fade_slope", "w_fade_slope"), &Camera4D::set_w_fade_slope);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "w_fade_slope", PROPERTY_HINT_RANGE, "0.001,10,0.001,or_greater,or_less,exp"), "set_w_fade_slope", "get_w_fade_slope");

	ClassDB::bind_method(D_METHOD("get_depth_fade_mode"), &Camera4D::get_depth_fade_mode);
	ClassDB::bind_method(D_METHOD("set_depth_fade_mode", "depth_fade_mode"), &Camera4D::set_depth_fade_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "depth_fade_mode", PROPERTY_HINT_ENUM, "Disabled,Distance,XYZ Only,Z Only"), "set_depth_fade_mode", "get_depth_fade_mode");

	ClassDB::bind_method(D_METHOD("get_depth_fade_start"), &Camera4D::get_depth_fade_start);
	ClassDB::bind_method(D_METHOD("set_depth_fade_start", "depth_fade_start"), &Camera4D::set_depth_fade_start);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "depth_fade_start", PROPERTY_HINT_RANGE, "1,100,0.01,or_greater,or_less,exp"), "set_depth_fade_start", "get_depth_fade_start");

	BIND_ENUM_CONSTANT(KEEP_WIDTH);
	BIND_ENUM_CONSTANT(KEEP_HEIGHT);

	BIND_ENUM_CONSTANT(PROJECTION4D_ORTHOGRAPHIC);
	BIND_ENUM_CONSTANT(PROJECTION4D_PERSPECTIVE_4D);
	BIND_ENUM_CONSTANT(PROJECTION4D_PERSPECTIVE_3D);
	BIND_ENUM_CONSTANT(PROJECTION4D_PERSPECTIVE_DUAL);

	BIND_ENUM_CONSTANT(VIEW_ANGLE_FOCAL_LENGTH);
	BIND_ENUM_CONSTANT(VIEW_ANGLE_FIELD_OF_VIEW);

	BIND_ENUM_CONSTANT(DEPTH_FADE_DISABLED);
	BIND_ENUM_CONSTANT(DEPTH_FADE_DISTANCE);
	BIND_ENUM_CONSTANT(DEPTH_FADE_XYZ_ONLY);
	BIND_ENUM_CONSTANT(DEPTH_FADE_Z_ONLY);

	BIND_ENUM_CONSTANT(W_FADE_DISABLED);
	BIND_ENUM_CONSTANT(W_FADE_TRANSPARENCY);
	BIND_ENUM_CONSTANT(W_FADE_HUE_SHIFT);
	BIND_ENUM_CONSTANT(W_FADE_TRANSPARENCY_HUE_SHIFT);
}
