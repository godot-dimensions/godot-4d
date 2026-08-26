#include "editor_camera_settings_4d.h"

#include "editor_main_screen_4d.h"

void EditorCameraSettings4D::set_view_angle_type(const Camera4D::ViewAngleType p_view_angle_type) {
	_view_angle_type = p_view_angle_type;
	notify_property_list_changed();
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_focal_length(const double p_focal_length) {
	_focal_length = p_focal_length;
	apply_to_cameras();
	write_to_config_file();
}

double EditorCameraSettings4D::get_field_of_view() const {
	return Math_PI - 2.0 * Math::atan(_focal_length);
}

void EditorCameraSettings4D::set_field_of_view(const double p_field_of_view) {
	_focal_length = Math::tan((Math_PI - p_field_of_view) * 0.5);
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_clip_near(const double p_clip_near) {
	_clip_near = p_clip_near;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_clip_far(const double p_clip_far) {
	_clip_far = p_clip_far;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_rotation_axis_lock(const int p_rotation_axis_lock) {
	_rotation_axis_lock = (EditorViewportCameraRotationAxisLock4D)p_rotation_axis_lock;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_depth_fade_mode(const Camera4D::DepthFadeMode p_depth_fade_mode) {
	_depth_fade_mode = p_depth_fade_mode;
	notify_property_list_changed();
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_depth_fade_start(const double p_depth_fade_start) {
	_depth_fade_start = p_depth_fade_start;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_w_fade_mode(const Camera4D::WFadeMode p_w_fade_mode) {
	_w_fade_mode = p_w_fade_mode;
	notify_property_list_changed();
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_w_fade_color_negative(const Color &p_w_fade_color_negative) {
	_w_fade_color_negative = p_w_fade_color_negative;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_w_fade_color_positive(const Color &p_w_fade_color_positive) {
	_w_fade_color_positive = p_w_fade_color_positive;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_w_fade_distance(const double p_w_fade_distance) {
	_w_fade_distance = p_w_fade_distance;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_w_fade_slope(const double p_w_fade_slope) {
	_w_fade_slope = p_w_fade_slope;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_edge_falloff(const double p_edge_falloff) {
	_edge_falloff = p_edge_falloff;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_plane_sharpness(const double p_plane_sharpness) {
	_plane_sharpness = p_plane_sharpness;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_skewness(const double p_skewness) {
	_skewness = p_skewness;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_projection_opacity(const double p_projection_opacity) {
	_projection_opacity = p_projection_opacity;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_projection_opacity_base(const double p_projection_opacity_base) {
	_projection_opacity_base = p_projection_opacity_base;
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::set_rendering_engine_name(const String &p_rendering_engine_name) {
	_rendering_engine_name = p_rendering_engine_name;
	notify_property_list_changed();
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::apply_to_cameras() const {
	_editor_main_screen->set_camera_rotation_axis_lock_policy(_rotation_axis_lock);
	TypedArray<Node> cameras = _editor_main_screen->find_children("*", "Camera4D", true, false);
	for (int i = 0; i < cameras.size(); i++) {
		Camera4D *camera = Object::cast_to<Camera4D>(cameras[i]);
		CRASH_COND(camera == nullptr);
		camera->set_view_angle_type(_view_angle_type);
		camera->set_focal_length_4d(_focal_length);
		camera->set_clip_near(_clip_near);
		camera->set_clip_far(_clip_far);
		camera->set_depth_fade_mode(_depth_fade_mode);
		camera->set_depth_fade_start(_depth_fade_start);
		camera->set_w_fade_mode(_w_fade_mode);
		camera->set_w_fade_color_negative(_w_fade_color_negative);
		camera->set_w_fade_color_positive(_w_fade_color_positive);
		camera->set_w_fade_distance(_w_fade_distance);
		camera->set_w_fade_slope(_w_fade_slope);
		camera->set_edge_falloff(_edge_falloff);
		camera->set_plane_sharpness(_plane_sharpness);
		camera->set_skewness(_skewness);
		camera->set_projection_opacity(_projection_opacity);
		camera->set_projection_opacity_base(_projection_opacity_base);
		camera->set_rendering_engine_name(_rendering_engine_name);
	}
}

void EditorCameraSettings4D::setup(EditorMainScreen4D *p_editor_main_screen, Ref<ConfigFile> &p_config_file, const String &p_config_file_path) {
	_editor_main_screen = p_editor_main_screen;
	_4d_editor_config_file = p_config_file;
	_4d_editor_config_file_path = p_config_file_path;
	_view_angle_type = (Camera4D::ViewAngleType)(int)p_config_file->get_value("camera", "view_angle_type", _view_angle_type);
	_focal_length = p_config_file->get_value("camera", "focal_length", _focal_length);
	_clip_near = p_config_file->get_value("camera", "clip_near", _clip_near);
	_clip_far = p_config_file->get_value("camera", "clip_far", _clip_far);
	_rotation_axis_lock = (EditorViewportCameraRotationAxisLock4D)(int)p_config_file->get_value("camera", "rotation_axis_lock", (int)_rotation_axis_lock);
	_depth_fade_mode = (Camera4D::DepthFadeMode)(int)p_config_file->get_value("camera", "depth_fade_mode", _depth_fade_mode);
	_w_fade_mode = (Camera4D::WFadeMode)(int)p_config_file->get_value("camera", "w_fade_mode", _w_fade_mode);
	_w_fade_color_negative = p_config_file->get_value("camera", "w_fade_color_negative", _w_fade_color_negative);
	_w_fade_color_positive = p_config_file->get_value("camera", "w_fade_color_positive", _w_fade_color_positive);
	_w_fade_distance = p_config_file->get_value("camera", "w_fade_distance", _w_fade_distance);
	_w_fade_slope = p_config_file->get_value("camera", "w_fade_slope", _w_fade_slope);
	_edge_falloff = p_config_file->get_value("camera", "edge_falloff", _edge_falloff);
	_plane_sharpness = p_config_file->get_value("camera", "plane_sharpness", _plane_sharpness);
	_skewness = p_config_file->get_value("camera", "skewness", _skewness);
	_projection_opacity = p_config_file->get_value("camera", "projection_opacity", _projection_opacity);
	_projection_opacity_base = p_config_file->get_value("camera", "projection_opacity_base", _projection_opacity_base);
	// Keep this in sync with `EditorMainScreen4D::_update_rendering_engine_menu()`.
	_rendering_engine_name = p_config_file->get_value("camera", "rendering_engine_name", _rendering_engine_name);
	apply_to_cameras();
}

void EditorCameraSettings4D::write_to_config_file() const {
	if (_4d_editor_config_file->has_section("camera")) {
		_4d_editor_config_file->erase_section("camera");
	}
	// Keep these in sync with the Camera4D and EditorCameraSettings4D defaults.
	if (_view_angle_type != Camera4D::VIEW_ANGLE_FOCAL_LENGTH) {
		_4d_editor_config_file->set_value("camera", "view_angle_type", (int)_view_angle_type);
	}
	if (!Math::is_equal_approx(_focal_length, 1.25)) {
		_4d_editor_config_file->set_value("camera", "focal_length", _focal_length);
	}
	if (!Math::is_equal_approx(_clip_near, 0.05)) {
		_4d_editor_config_file->set_value("camera", "clip_near", _clip_near);
	}
	if (!Math::is_equal_approx(_clip_far, 4000.0)) {
		_4d_editor_config_file->set_value("camera", "clip_far", _clip_far);
	}
	if (_rotation_axis_lock != EditorViewportCameraRotationAxisLock4D::FULLY_LOCKED) {
		_4d_editor_config_file->set_value("camera", "rotation_axis_lock", (int)_rotation_axis_lock);
	}
	if (_depth_fade_mode != Camera4D::DEPTH_FADE_DISABLED) {
		_4d_editor_config_file->set_value("camera", "depth_fade_mode", (int)_depth_fade_mode);
	}
	if (!Math::is_equal_approx(_depth_fade_start, 25.0)) {
		_4d_editor_config_file->set_value("camera", "depth_fade_start", _depth_fade_start);
	}
	if (_w_fade_mode != Camera4D::W_FADE_TRANSPARENCY) {
		_4d_editor_config_file->set_value("camera", "w_fade_mode", (int)_w_fade_mode);
	}
	if (!_w_fade_color_negative.is_equal_approx(Color(0.0f, 0.5f, 1.0f))) {
		_4d_editor_config_file->set_value("camera", "w_fade_color_negative", _w_fade_color_negative);
	}
	if (!_w_fade_color_positive.is_equal_approx(Color(1.0f, 0.5f, 0.0f))) {
		_4d_editor_config_file->set_value("camera", "w_fade_color_positive", _w_fade_color_positive);
	}
	if (!Math::is_equal_approx(_w_fade_distance, 5.0)) {
		_4d_editor_config_file->set_value("camera", "w_fade_distance", _w_fade_distance);
	}
	if (!Math::is_equal_approx(_w_fade_slope, 1.0)) {
		_4d_editor_config_file->set_value("camera", "w_fade_slope", _w_fade_slope);
	}
	if (!Math::is_equal_approx(_edge_falloff, 1.0)) {
		_4d_editor_config_file->set_value("camera", "edge_falloff", _edge_falloff);
	}
	if (!Math::is_equal_approx(_plane_sharpness, 0.3)) {
		_4d_editor_config_file->set_value("camera", "plane_sharpness", _plane_sharpness);
	}
	if (!Math::is_equal_approx(_skewness, 0.0)) {
		_4d_editor_config_file->set_value("camera", "skewness", _skewness);
	}
	if (!Math::is_equal_approx(_projection_opacity, 1.0)) {
		_4d_editor_config_file->set_value("camera", "projection_opacity", _projection_opacity);
	}
	if (!Math::is_equal_approx(_projection_opacity_base, 1.0)) {
		_4d_editor_config_file->set_value("camera", "projection_opacity_base", _projection_opacity_base);
	}
	if (!_rendering_engine_name.is_empty()) {
		_4d_editor_config_file->set_value("camera", "rendering_engine_name", _rendering_engine_name);
	}
	_4d_editor_config_file->save(_4d_editor_config_file_path);
}

void EditorCameraSettings4D::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == StringName("focal_length")) {
		if (_view_angle_type != Camera4D::VIEW_ANGLE_FOCAL_LENGTH) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("field_of_view")) {
		if (_view_angle_type != Camera4D::VIEW_ANGLE_FIELD_OF_VIEW) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("clip_far")) {
		if (_rendering_engine_name == "Wireframe Canvas" && _depth_fade_mode == Camera4D::DEPTH_FADE_DISABLED) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("depth_fade_start")) {
		if (_depth_fade_mode == Camera4D::DEPTH_FADE_DISABLED) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("w_fade_color_negative")) {
		if (!(_w_fade_mode & Camera4D::W_FADE_HUE_SHIFT)) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("w_fade_color_positive")) {
		if (!(_w_fade_mode & Camera4D::W_FADE_HUE_SHIFT)) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("w_fade_distance")) {
		if (_w_fade_mode == Camera4D::W_FADE_DISABLED) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("w_fade_slope")) {
		if (_w_fade_mode == Camera4D::W_FADE_DISABLED) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	}
}

void EditorCameraSettings4D::_bind_methods() {
	// These are copies of the Camera4D properties relevant for the editor camera.
	// Be sure to keep these in sync with Camera4D.
	ClassDB::bind_method(D_METHOD("get_view_angle_type"), &EditorCameraSettings4D::get_view_angle_type);
	ClassDB::bind_method(D_METHOD("set_view_angle_type", "view_angle_type"), &EditorCameraSettings4D::set_view_angle_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "view_angle_type", PROPERTY_HINT_ENUM, "Focal Length,Field of View"), "set_view_angle_type", "get_view_angle_type");

	ClassDB::bind_method(D_METHOD("get_focal_length"), &EditorCameraSettings4D::get_focal_length);
	ClassDB::bind_method(D_METHOD("set_focal_length", "focal_length"), &EditorCameraSettings4D::set_focal_length);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "focal_length", PROPERTY_HINT_NONE, "suffix:m"), "set_focal_length", "get_focal_length");

	ClassDB::bind_method(D_METHOD("get_field_of_view"), &EditorCameraSettings4D::get_field_of_view);
	ClassDB::bind_method(D_METHOD("set_field_of_view", "field_of_view"), &EditorCameraSettings4D::set_field_of_view);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "field_of_view", PROPERTY_HINT_RANGE, "1,179,0.1,radians_as_degrees"), "set_field_of_view", "get_field_of_view");

	ClassDB::bind_method(D_METHOD("get_clip_near"), &EditorCameraSettings4D::get_clip_near);
	ClassDB::bind_method(D_METHOD("set_clip_near", "clip_near"), &EditorCameraSettings4D::set_clip_near);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clip_near", PROPERTY_HINT_RANGE, "0.001,1000,0.001,or_greater,exp,suffix:m"), "set_clip_near", "get_clip_near");

	ClassDB::bind_method(D_METHOD("get_clip_far"), &EditorCameraSettings4D::get_clip_far);
	ClassDB::bind_method(D_METHOD("set_clip_far", "clip_far"), &EditorCameraSettings4D::set_clip_far);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clip_far", PROPERTY_HINT_RANGE, "0.01,4000,0.01,or_greater,exp,suffix:m"), "set_clip_far", "get_clip_far");

	ClassDB::bind_method(D_METHOD("get_rotation_axis_lock"), &EditorCameraSettings4D::get_rotation_axis_lock);
	ClassDB::bind_method(D_METHOD("set_rotation_axis_lock", "rotation_axis_lock"), &EditorCameraSettings4D::set_rotation_axis_lock);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rotation_axis_lock", PROPERTY_HINT_ENUM, "Fully Locked,Free Ground View,Fully Free"), "set_rotation_axis_lock", "get_rotation_axis_lock");

	ClassDB::bind_method(D_METHOD("get_depth_fade_mode"), &EditorCameraSettings4D::get_depth_fade_mode);
	ClassDB::bind_method(D_METHOD("set_depth_fade_mode", "depth_fade_mode"), &EditorCameraSettings4D::set_depth_fade_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "depth_fade_mode", PROPERTY_HINT_ENUM, "Disabled,Distance,XYZ Only,Z Only"), "set_depth_fade_mode", "get_depth_fade_mode");

	ClassDB::bind_method(D_METHOD("get_depth_fade_start"), &EditorCameraSettings4D::get_depth_fade_start);
	ClassDB::bind_method(D_METHOD("set_depth_fade_start", "depth_fade_start"), &EditorCameraSettings4D::set_depth_fade_start);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "depth_fade_start", PROPERTY_HINT_RANGE, "1,100,0.01,or_greater,or_less,exp,suffix:m"), "set_depth_fade_start", "get_depth_fade_start");

	ClassDB::bind_method(D_METHOD("get_w_fade_mode"), &EditorCameraSettings4D::get_w_fade_mode);
	ClassDB::bind_method(D_METHOD("set_w_fade_mode", "w_fade_mode"), &EditorCameraSettings4D::set_w_fade_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "w_fade_mode", PROPERTY_HINT_ENUM, "Disabled,Transparency,Hue Shift,Transparency + Hue Shift"), "set_w_fade_mode", "get_w_fade_mode");

	ClassDB::bind_method(D_METHOD("get_w_fade_color_negative"), &EditorCameraSettings4D::get_w_fade_color_negative);
	ClassDB::bind_method(D_METHOD("set_w_fade_color_negative", "w_fade_color_negative"), &EditorCameraSettings4D::set_w_fade_color_negative);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "w_fade_color_negative"), "set_w_fade_color_negative", "get_w_fade_color_negative");

	ClassDB::bind_method(D_METHOD("get_w_fade_color_positive"), &EditorCameraSettings4D::get_w_fade_color_positive);
	ClassDB::bind_method(D_METHOD("set_w_fade_color_positive", "w_fade_color_positive"), &EditorCameraSettings4D::set_w_fade_color_positive);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "w_fade_color_positive"), "set_w_fade_color_positive", "get_w_fade_color_positive");

	ClassDB::bind_method(D_METHOD("get_w_fade_distance"), &EditorCameraSettings4D::get_w_fade_distance);
	ClassDB::bind_method(D_METHOD("set_w_fade_distance", "w_fade_distance"), &EditorCameraSettings4D::set_w_fade_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "w_fade_distance", PROPERTY_HINT_RANGE, "0.01,10,0.001,or_greater,or_less,exp,suffix:m"), "set_w_fade_distance", "get_w_fade_distance");

	ClassDB::bind_method(D_METHOD("get_w_fade_slope"), &EditorCameraSettings4D::get_w_fade_slope);
	ClassDB::bind_method(D_METHOD("set_w_fade_slope", "w_fade_slope"), &EditorCameraSettings4D::set_w_fade_slope);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "w_fade_slope", PROPERTY_HINT_RANGE, "0.01,10,0.001,or_greater,or_less,exp"), "set_w_fade_slope", "get_w_fade_slope");

	ClassDB::bind_method(D_METHOD("get_edge_falloff"), &EditorCameraSettings4D::get_edge_falloff);
	ClassDB::bind_method(D_METHOD("set_edge_falloff", "edge_falloff"), &EditorCameraSettings4D::set_edge_falloff);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "edge_falloff", PROPERTY_HINT_RANGE, "0,5,0.01,or_greater"), "set_edge_falloff", "get_edge_falloff");

	ClassDB::bind_method(D_METHOD("get_plane_sharpness"), &EditorCameraSettings4D::get_plane_sharpness);
	ClassDB::bind_method(D_METHOD("set_plane_sharpness", "plane_sharpness"), &EditorCameraSettings4D::set_plane_sharpness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "plane_sharpness", PROPERTY_HINT_RANGE, "0,0.99,0.001"), "set_plane_sharpness", "get_plane_sharpness");

	ClassDB::bind_method(D_METHOD("get_skewness"), &EditorCameraSettings4D::get_skewness);
	ClassDB::bind_method(D_METHOD("set_skewness", "skewness"), &EditorCameraSettings4D::set_skewness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "skewness", PROPERTY_HINT_RANGE, "-1,1,0.001"), "set_skewness", "get_skewness");

	ClassDB::bind_method(D_METHOD("get_projection_opacity"), &EditorCameraSettings4D::get_projection_opacity);
	ClassDB::bind_method(D_METHOD("set_projection_opacity", "projection_opacity"), &EditorCameraSettings4D::set_projection_opacity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "projection_opacity", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_projection_opacity", "get_projection_opacity");

	ClassDB::bind_method(D_METHOD("get_projection_opacity_base"), &EditorCameraSettings4D::get_projection_opacity_base);
	ClassDB::bind_method(D_METHOD("set_projection_opacity_base", "projection_opacity_base"), &EditorCameraSettings4D::set_projection_opacity_base);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "projection_opacity_base", PROPERTY_HINT_RANGE, "0.01,10,0.001,or_greater,exp"), "set_projection_opacity_base", "get_projection_opacity_base");
}
