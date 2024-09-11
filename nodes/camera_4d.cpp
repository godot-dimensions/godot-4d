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
	} else if (p_property.name == StringName("w_fade_distance")) {
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

real_t Camera4D::get_orthographic_size() const {
	return _orthographic_size;
}

void Camera4D::set_orthographic_size(const real_t p_orthographic_size) {
	_orthographic_size = p_orthographic_size;
}

real_t Camera4D::get_near() const {
	return _near;
}

void Camera4D::set_near(const real_t p_near) {
	_near = p_near;
}

real_t Camera4D::get_far() const {
	return _far;
}

void Camera4D::set_far(const real_t p_far) {
	_far = p_far;
}

Camera4D::WFadeMode Camera4D::get_w_fade_mode() const {
	return _w_fade_mode;
}

void Camera4D::set_w_fade_mode(const WFadeMode p_w_fade_mode) {
	_w_fade_mode = p_w_fade_mode;
	notify_property_list_changed();
}

real_t Camera4D::get_w_fade_distance() const {
	return _w_fade_distance;
}

void Camera4D::set_w_fade_distance(const real_t p_w_fade_distance) {
	_w_fade_distance = p_w_fade_distance;
}

void Camera4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_current"), &Camera4D::is_current);
	ClassDB::bind_method(D_METHOD("set_current", "enabled"), &Camera4D::set_current);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "current"), "set_current", "is_current");
	ClassDB::bind_method(D_METHOD("clear_current", "enable_next"), &Camera4D::clear_current);
	ClassDB::bind_method(D_METHOD("make_current"), &Camera4D::make_current);

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

	ClassDB::bind_method(D_METHOD("get_near"), &Camera4D::get_near);
	ClassDB::bind_method(D_METHOD("set_near", "near"), &Camera4D::set_near);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "near", PROPERTY_HINT_RANGE, "0.001,1000,0.001,or_greater,exp,suffix:m"), "set_near", "get_near");

	ClassDB::bind_method(D_METHOD("get_far"), &Camera4D::get_far);
	ClassDB::bind_method(D_METHOD("set_far", "far"), &Camera4D::set_far);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "far", PROPERTY_HINT_RANGE, "0.01,4000,0.01,or_greater,exp,suffix:m"), "set_far", "get_far");

	ClassDB::bind_method(D_METHOD("get_w_fade_mode"), &Camera4D::get_w_fade_mode);
	ClassDB::bind_method(D_METHOD("set_w_fade_mode", "w_fade_mode"), &Camera4D::set_w_fade_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "w_fade_mode", PROPERTY_HINT_ENUM, "Disabled,Transparency,Hue Shift,Transparency + Hue Shift"), "set_w_fade_mode", "get_w_fade_mode");

	ClassDB::bind_method(D_METHOD("get_w_fade_distance"), &Camera4D::get_w_fade_distance);
	ClassDB::bind_method(D_METHOD("set_w_fade_distance", "w_fade_distance"), &Camera4D::set_w_fade_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "w_fade_distance", PROPERTY_HINT_RANGE, "0.001,1000,0.001,suffix:m"), "set_w_fade_distance", "get_w_fade_distance");

	BIND_ENUM_CONSTANT(KEEP_WIDTH);
	BIND_ENUM_CONSTANT(KEEP_HEIGHT);

	BIND_ENUM_CONSTANT(PROJECTION4D_ORTHOGRAPHIC);
	BIND_ENUM_CONSTANT(PROJECTION4D_PERSPECTIVE_4D);
	BIND_ENUM_CONSTANT(PROJECTION4D_PERSPECTIVE_3D);
	BIND_ENUM_CONSTANT(PROJECTION4D_PERSPECTIVE_DUAL);

	BIND_ENUM_CONSTANT(VIEW_ANGLE_FOCAL_LENGTH);
	BIND_ENUM_CONSTANT(VIEW_ANGLE_FIELD_OF_VIEW);

	BIND_ENUM_CONSTANT(W_FADE_DISABLED);
	BIND_ENUM_CONSTANT(W_FADE_TRANSPARENCY);
	BIND_ENUM_CONSTANT(W_FADE_HUE_SHIFT);
	BIND_ENUM_CONSTANT(W_FADE_TRANSPARENCY_HUE_SHIFT);
}
