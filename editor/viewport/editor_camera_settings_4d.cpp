#include "editor_camera_settings_4d.h"

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

void EditorCameraSettings4D::set_rendering_engine(const String &p_rendering_engine) {
	_rendering_engine = p_rendering_engine;
	notify_property_list_changed();
	apply_to_cameras();
	write_to_config_file();
}

void EditorCameraSettings4D::apply_to_cameras() const {
	TypedArray<Node> cameras = _ancestor_of_cameras->find_children("*", "Camera4D", true, false);
	for (int i = 0; i < cameras.size(); i++) {
		Camera4D *camera = Object::cast_to<Camera4D>(cameras[i]);
		CRASH_COND(camera == nullptr);
		camera->set_clip_near(_clip_near);
		camera->set_clip_far(_clip_far);
		camera->set_w_fade_mode(_w_fade_mode);
		camera->set_w_fade_color_negative(_w_fade_color_negative);
		camera->set_w_fade_color_positive(_w_fade_color_positive);
		camera->set_w_fade_distance(_w_fade_distance);
		camera->set_w_fade_slope(_w_fade_slope);
		camera->set_rendering_engine(_rendering_engine);
	}
}

void EditorCameraSettings4D::setup(Node *p_ancestor_of_cameras, Ref<ConfigFile> &p_config_file, const String &p_config_file_path) {
	_ancestor_of_cameras = p_ancestor_of_cameras;
	_4d_editor_config_file = p_config_file;
	_4d_editor_config_file_path = p_config_file_path;
	_clip_near = p_config_file->get_value("camera", "clip_near", _clip_near);
	_clip_far = p_config_file->get_value("camera", "clip_far", _clip_far);
	_w_fade_mode = (Camera4D::WFadeMode)(int)p_config_file->get_value("camera", "w_fade_mode", _w_fade_mode);
	_w_fade_color_negative = p_config_file->get_value("camera", "w_fade_color_negative", _w_fade_color_negative);
	_w_fade_color_positive = p_config_file->get_value("camera", "w_fade_color_positive", _w_fade_color_positive);
	_w_fade_distance = p_config_file->get_value("camera", "w_fade_distance", _w_fade_distance);
	_w_fade_slope = p_config_file->get_value("camera", "w_fade_slope", _w_fade_slope);
	_rendering_engine = p_config_file->get_value("camera", "rendering_engine", _rendering_engine);
	apply_to_cameras();
}

void EditorCameraSettings4D::write_to_config_file() const {
	if (_4d_editor_config_file->has_section("camera")) {
		_4d_editor_config_file->erase_section("camera");
	}
	if (!Math::is_equal_approx(_clip_near, 0.05)) {
		_4d_editor_config_file->set_value("camera", "clip_near", _clip_near);
	}
	if (!Math::is_equal_approx(_clip_far, 4000.0)) {
		_4d_editor_config_file->set_value("camera", "clip_far", _clip_far);
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
	if (!_rendering_engine.is_empty()) {
		_4d_editor_config_file->set_value("camera", "rendering_engine", _rendering_engine);
	}
	_4d_editor_config_file->save(_4d_editor_config_file_path);
}

void EditorCameraSettings4D::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == StringName("clip_far")) {
		if (_rendering_engine == "Wireframe Canvas") {
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
	ClassDB::bind_method(D_METHOD("get_clip_near"), &EditorCameraSettings4D::get_clip_near);
	ClassDB::bind_method(D_METHOD("set_clip_near", "clip_near"), &EditorCameraSettings4D::set_clip_near);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clip_near", PROPERTY_HINT_RANGE, "0.001,1000,0.001,or_greater,exp,suffix:m"), "set_clip_near", "get_clip_near");

	ClassDB::bind_method(D_METHOD("get_clip_far"), &EditorCameraSettings4D::get_clip_far);
	ClassDB::bind_method(D_METHOD("set_clip_far", "clip_far"), &EditorCameraSettings4D::set_clip_far);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clip_far", PROPERTY_HINT_RANGE, "0.01,4000,0.01,or_greater,exp,suffix:m"), "set_clip_far", "get_clip_far");

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
}
