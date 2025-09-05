#include "editor_transform_snap_settings_4d.h"

#if GODOT_MODULE
#define KEY_SHIFT Key::SHIFT
#define KEY_CTRL Key::CTRL
#define KEY_META Key::META
#define KEY_ALT Key::ALT
#endif

bool EditorTransformSnapSettings4D::_is_modifier_pressed() const {
	return (Input::get_singleton()->is_key_pressed(KEY_SHIFT) ||
			Input::get_singleton()->is_key_pressed(KEY_CTRL) ||
			Input::get_singleton()->is_key_pressed(KEY_META) ||
			Input::get_singleton()->is_key_pressed(KEY_ALT));
}

Vector4 EditorTransformSnapSettings4D::_snap_position(const Vector4 &p_position, const bool p_is_modifier_pressed) const {
	if (p_is_modifier_pressed == _position_snap_invert_keybind) {
		return p_position;
	}
	double active_snap_dist = _position_snap_distance_meters;
	if (_position_snap_camera_distance_mode) {
		const double cam_dist_clamped = MAX(Math::abs(_camera_distance), 0.000001);
		switch (_position_snap_camera_distance_mode) {
			case CAM_DIST_CONSTANT: {
				// Do nothing, already set above.
			} break;
			case CAM_DIST_DYNAMIC_POWER_OF_TEN: {
				const double dist_log_ten = std::log10(cam_dist_clamped);
				active_snap_dist *= Math::pow(10.0, Math::floor(dist_log_ten));
			} break;
			case CAM_DIST_DYNAMIC_POWER_OF_TWO: {
				const double dist_log_two = std::log2(cam_dist_clamped);
				active_snap_dist *= Math::pow(2.0, Math::floor(dist_log_two));
			} break;
		}
	}
	return p_position.snappedf(active_snap_dist);
}

Basis4D EditorTransformSnapSettings4D::_snap_rotation(const Basis4D &p_basis, const bool p_is_modifier_pressed) const {
	if (p_is_modifier_pressed == _rotation_snap_invert_keybind) {
		return p_basis;
	}
	Basis4D snapped_basis = p_basis;
	const Vector4 scale_abs = p_basis.get_scale_abs();
	// Rotation inherently can't account for negative scale, so just flip W if needed, and flip it back after.
	const real_t determinant = p_basis.determinant();
	if (determinant < 0.0f) {
		snapped_basis.w = -snapped_basis.w;
	}
	const Euler4D euler = Euler4D::from_basis(snapped_basis);
	snapped_basis = euler.snapped(_rotation_snap_angle_radians).to_basis();
	if (determinant < 0.0f) {
		snapped_basis.w = -snapped_basis.w;
	}
	return snapped_basis.scaled_local(scale_abs);
}

Transform4D EditorTransformSnapSettings4D::_snap_scale(const Transform4D &p_transform, const bool p_is_modifier_pressed) const {
	if (p_is_modifier_pressed == _scale_snap_invert_keybind) {
		return p_transform;
	}
	const Vector4 scale_abs = p_transform.basis.get_scale_abs();
	const Vector4 snapped_scale_abs = scale_abs.snappedf(_scale_snap_ratio);
	return p_transform.scaled_local(snapped_scale_abs / scale_abs);
}

void EditorTransformSnapSettings4D::set_snap_final_values(const bool p_snap_final_values) {
	_snap_final_values = p_snap_final_values;
	write_to_config_file();
}

void EditorTransformSnapSettings4D::set_position_snap_camera_distance_mode(const CameraDistanceMode p_position_snap_camera_distance_mode) {
	_position_snap_camera_distance_mode = p_position_snap_camera_distance_mode;
	write_to_config_file();
}

void EditorTransformSnapSettings4D::set_position_snap_distance_meters(const double p_position_snap_distance_meters) {
	_position_snap_distance_meters = p_position_snap_distance_meters;
	write_to_config_file();
}

void EditorTransformSnapSettings4D::set_position_snap_invert_keybind(const bool p_position_snap_invert_keybind) {
	_position_snap_invert_keybind = p_position_snap_invert_keybind;
	write_to_config_file();
}

void EditorTransformSnapSettings4D::set_rotation_snap_unit(const RotationSnapUnit p_rotation_snap_unit) {
	_rotation_snap_unit = p_rotation_snap_unit;
	notify_property_list_changed();
	write_to_config_file();
}

void EditorTransformSnapSettings4D::set_rotation_snap_angle_radians(const double p_rotation_snap_angle_radians) {
	_rotation_snap_angle_radians = p_rotation_snap_angle_radians;
	write_to_config_file();
}

void EditorTransformSnapSettings4D::set_rotation_snap_angle_degrees(const double p_rotation_snap_angle_degrees) {
	set_rotation_snap_angle_radians(p_rotation_snap_angle_degrees * (Math_TAU / 360.0));
}

void EditorTransformSnapSettings4D::set_rotation_snap_angle_inverse_turns(const double p_rotation_snap_angle_inverse_turns) {
	set_rotation_snap_angle_radians(Math_TAU / p_rotation_snap_angle_inverse_turns);
}

void EditorTransformSnapSettings4D::set_rotation_snap_invert_keybind(const bool p_rotation_snap_invert_keybind) {
	_rotation_snap_invert_keybind = p_rotation_snap_invert_keybind;
	write_to_config_file();
}

void EditorTransformSnapSettings4D::set_scale_snap_ratio(const double p_scale_snap_ratio) {
	_scale_snap_ratio = p_scale_snap_ratio;
	write_to_config_file();
}

void EditorTransformSnapSettings4D::set_scale_snap_invert_keybind(const bool p_scale_snap_invert_keybind) {
	_scale_snap_invert_keybind = p_scale_snap_invert_keybind;
	write_to_config_file();
}

Transform4D EditorTransformSnapSettings4D::snap_transform_change(const Transform4D &p_old_transform, const Transform4D &p_transform_change) const {
	if (_snap_final_values) {
		// Snap the final values after applying the change.
		return snap_single_transform(p_old_transform * p_transform_change);
	}
	// Snap the change itself before applying it. We need to avoid scaling the value output by the snap function,
	// so always snap the final scale value, but snap the change in position and rotation.
	const Vector4 old_local_scale_abs = p_old_transform.basis.get_scale_abs();
	const bool is_modifier_pressed = _is_modifier_pressed();
	Transform4D composed = Transform4D(
			_snap_rotation(p_transform_change.basis, is_modifier_pressed),
			_snap_position(old_local_scale_abs * p_transform_change.origin, is_modifier_pressed) / old_local_scale_abs);
	// Always snap scale after composing.
	return _snap_scale(p_old_transform * composed, is_modifier_pressed);
}

Transform4D EditorTransformSnapSettings4D::snap_single_transform(const Transform4D &p_transform) const {
	const bool is_modifier_pressed = _is_modifier_pressed();
	const Transform4D pos_rot_snapped = Transform4D(
			_snap_rotation(p_transform.basis, is_modifier_pressed),
			_snap_position(p_transform.origin, is_modifier_pressed));
	return _snap_scale(pos_rot_snapped, is_modifier_pressed);
}

void EditorTransformSnapSettings4D::setup(Ref<ConfigFile> &p_config_file, const String &p_config_file_path) {
	_4d_editor_config_file = p_config_file;
	_4d_editor_config_file_path = p_config_file_path;
	_snap_final_values = p_config_file->get_value("snap", "snap_final_values", _snap_final_values);
	const String position_snap_camera_distance_string = p_config_file->get_value("snap", "position_snap_camera_distance_mode", "power_of_10");
	if (position_snap_camera_distance_string == "constant") {
		_position_snap_camera_distance_mode = CAM_DIST_CONSTANT;
	} else if (position_snap_camera_distance_string == "power_of_2") {
		_position_snap_camera_distance_mode = CAM_DIST_DYNAMIC_POWER_OF_TWO;
	}
	_position_snap_distance_meters = p_config_file->get_value("snap", "position_snap_distance_meters", _position_snap_distance_meters);
	_position_snap_invert_keybind = p_config_file->get_value("snap", "position_snap_invert_keybind", _position_snap_invert_keybind);
	const String rotation_snap_unit_string = p_config_file->get_value("snap", "rotation_snap_unit", "degrees");
	if (rotation_snap_unit_string == "radians") {
		_rotation_snap_unit = ROTATION_SNAP_RADIANS;
	} else if (rotation_snap_unit_string == "inverse_turns") {
		_rotation_snap_unit = ROTATION_SNAP_INVERSE_TURNS;
	}
	_rotation_snap_angle_radians = p_config_file->get_value("snap", "rotation_snap_angle_radians", _rotation_snap_angle_radians);
	_rotation_snap_invert_keybind = p_config_file->get_value("snap", "rotation_snap_invert_keybind", _rotation_snap_invert_keybind);
	_scale_snap_ratio = p_config_file->get_value("snap", "scale_snap_ratio", _scale_snap_ratio);
	_scale_snap_invert_keybind = p_config_file->get_value("snap", "scale_snap_invert_keybind", _scale_snap_invert_keybind);
}

void EditorTransformSnapSettings4D::write_to_config_file() const {
	if (_4d_editor_config_file->has_section("snap")) {
		_4d_editor_config_file->erase_section("snap");
	}
	// Keep these in sync with the EditorTransformSnapSettings4D defaults.
	if (_snap_final_values != false) {
		_4d_editor_config_file->set_value("snap", "snap_final_values", _snap_final_values);
	}
	if (_position_snap_camera_distance_mode == CAM_DIST_CONSTANT) {
		_4d_editor_config_file->set_value("snap", "position_snap_camera_distance_mode", "constant");
	} else if (_position_snap_camera_distance_mode == CAM_DIST_DYNAMIC_POWER_OF_TWO) {
		_4d_editor_config_file->set_value("snap", "position_snap_camera_distance_mode", "power_of_2");
	}
	if (!Math::is_equal_approx(_position_snap_distance_meters, 0.1)) {
		_4d_editor_config_file->set_value("snap", "position_snap_distance_meters", _position_snap_distance_meters);
	}
	if (_position_snap_invert_keybind != false) {
		_4d_editor_config_file->set_value("snap", "position_snap_invert_keybind", _position_snap_invert_keybind);
	}
	if (_rotation_snap_unit == ROTATION_SNAP_RADIANS) {
		_4d_editor_config_file->set_value("snap", "rotation_snap_unit", "radians");
	} else if (_rotation_snap_unit == ROTATION_SNAP_INVERSE_TURNS) {
		_4d_editor_config_file->set_value("snap", "rotation_snap_unit", "inverse_turns");
	}
	// Always serialize the underlying radians value, even if degrees or inverse turns is selected.
	if (!Math::is_equal_approx(_rotation_snap_angle_radians, Math_TAU * (15.0 / 360.0))) {
		_4d_editor_config_file->set_value("snap", "rotation_snap_angle_radians", _rotation_snap_angle_radians);
	}
	if (_rotation_snap_invert_keybind != false) {
		_4d_editor_config_file->set_value("snap", "rotation_snap_invert_keybind", _rotation_snap_invert_keybind);
	}
	if (!Math::is_equal_approx(_scale_snap_ratio, 0.1)) {
		_4d_editor_config_file->set_value("snap", "scale_snap_ratio", _scale_snap_ratio);
	}
	if (_scale_snap_invert_keybind != false) {
		_4d_editor_config_file->set_value("snap", "scale_snap_invert_keybind", _scale_snap_invert_keybind);
	}
	_4d_editor_config_file->save(_4d_editor_config_file_path);
}

void EditorTransformSnapSettings4D::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == StringName("rotation_snap_angle_radians")) {
		if (_rotation_snap_unit != ROTATION_SNAP_RADIANS) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("rotation_snap_angle_degrees")) {
		if (_rotation_snap_unit != ROTATION_SNAP_DEGREES) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else if (p_property.name == StringName("rotation_snap_angle_inverse_turns")) {
		if (_rotation_snap_unit != ROTATION_SNAP_INVERSE_TURNS) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	}
}

void EditorTransformSnapSettings4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_snap_final_values"), &EditorTransformSnapSettings4D::get_snap_final_values);
	ClassDB::bind_method(D_METHOD("set_snap_final_values", "snap_final_values"), &EditorTransformSnapSettings4D::set_snap_final_values);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_final_values"), "set_snap_final_values", "get_snap_final_values");

	ADD_GROUP("Position Snap", "position_snap_");
	ClassDB::bind_method(D_METHOD("get_position_snap_camera_distance_mode"), &EditorTransformSnapSettings4D::get_position_snap_camera_distance_mode);
	ClassDB::bind_method(D_METHOD("set_position_snap_camera_distance_mode", "position_snap_camera_distance_mode"), &EditorTransformSnapSettings4D::set_position_snap_camera_distance_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "position_snap_camera_distance_mode", PROPERTY_HINT_ENUM, "Constant,Dynamic Power of 10,Dynamic Power of 2"), "set_position_snap_camera_distance_mode", "get_position_snap_camera_distance_mode");
	ClassDB::bind_method(D_METHOD("get_position_snap_distance_meters"), &EditorTransformSnapSettings4D::get_position_snap_distance_meters);
	ClassDB::bind_method(D_METHOD("set_position_snap_distance_meters", "position_snap_distance_meters"), &EditorTransformSnapSettings4D::set_position_snap_distance_meters);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "position_snap_distance_meters", PROPERTY_HINT_RANGE, "0.001,1000,0.001,or_greater,exp,suffix:m"), "set_position_snap_distance_meters", "get_position_snap_distance_meters");
	ClassDB::bind_method(D_METHOD("get_position_snap_invert_keybind"), &EditorTransformSnapSettings4D::get_position_snap_invert_keybind);
	ClassDB::bind_method(D_METHOD("set_position_snap_invert_keybind", "position_snap_invert_keybind"), &EditorTransformSnapSettings4D::set_position_snap_invert_keybind);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "position_snap_invert_keybind"), "set_position_snap_invert_keybind", "get_position_snap_invert_keybind");

	ADD_GROUP("Rotation Angle Snap", "rotation_snap_");
	ClassDB::bind_method(D_METHOD("get_rotation_snap_unit"), &EditorTransformSnapSettings4D::get_rotation_snap_unit);
	ClassDB::bind_method(D_METHOD("set_rotation_snap_unit", "rotation_snap_unit"), &EditorTransformSnapSettings4D::set_rotation_snap_unit);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rotation_snap_unit", PROPERTY_HINT_ENUM, "Degrees,Radians,Inverse Turns"), "set_rotation_snap_unit", "get_rotation_snap_unit");
	ClassDB::bind_method(D_METHOD("get_rotation_snap_angle_radians"), &EditorTransformSnapSettings4D::get_rotation_snap_angle_radians);
	ClassDB::bind_method(D_METHOD("set_rotation_snap_angle_radians", "rotation_snap_angle_radians"), &EditorTransformSnapSettings4D::set_rotation_snap_angle_radians);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rotation_snap_angle_radians", PROPERTY_HINT_RANGE, "0.00000000000001,6.28318530717958,0.00000000000001,or_greater,suffix:rad"), "set_rotation_snap_angle_radians", "get_rotation_snap_angle_radians");
	ClassDB::bind_method(D_METHOD("get_rotation_snap_angle_degrees"), &EditorTransformSnapSettings4D::get_rotation_snap_angle_degrees);
	ClassDB::bind_method(D_METHOD("set_rotation_snap_angle_degrees", "rotation_snap_angle_degrees"), &EditorTransformSnapSettings4D::set_rotation_snap_angle_degrees);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rotation_snap_angle_degrees", PROPERTY_HINT_RANGE, U"0.01,360,0.01,or_greater,suffix:\u00B0"), "set_rotation_snap_angle_degrees", "get_rotation_snap_angle_degrees");
	ClassDB::bind_method(D_METHOD("get_rotation_snap_angle_inverse_turns"), &EditorTransformSnapSettings4D::get_rotation_snap_angle_inverse_turns);
	ClassDB::bind_method(D_METHOD("set_rotation_snap_angle_inverse_turns", "rotation_snap_angle_inverse_turns"), &EditorTransformSnapSettings4D::set_rotation_snap_angle_inverse_turns);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rotation_snap_angle_inverse_turns", PROPERTY_HINT_RANGE, "1,360,1,or_greater"), "set_rotation_snap_angle_inverse_turns", "get_rotation_snap_angle_inverse_turns");
	ClassDB::bind_method(D_METHOD("get_rotation_snap_invert_keybind"), &EditorTransformSnapSettings4D::get_rotation_snap_invert_keybind);
	ClassDB::bind_method(D_METHOD("set_rotation_snap_invert_keybind", "rotation_snap_invert_keybind"), &EditorTransformSnapSettings4D::set_rotation_snap_invert_keybind);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rotation_snap_invert_keybind"), "set_rotation_snap_invert_keybind", "get_rotation_snap_invert_keybind");

	ADD_GROUP("Scale and Stretch Snap", "scale_snap_");
	ClassDB::bind_method(D_METHOD("get_scale_snap_ratio"), &EditorTransformSnapSettings4D::get_scale_snap_ratio);
	ClassDB::bind_method(D_METHOD("set_scale_snap_ratio", "scale_snap_ratio"), &EditorTransformSnapSettings4D::set_scale_snap_ratio);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_snap_ratio", PROPERTY_HINT_NONE, "0.001,1,0.001"), "set_scale_snap_ratio", "get_scale_snap_ratio");
	ClassDB::bind_method(D_METHOD("get_scale_snap_invert_keybind"), &EditorTransformSnapSettings4D::get_scale_snap_invert_keybind);
	ClassDB::bind_method(D_METHOD("set_scale_snap_invert_keybind", "scale_snap_invert_keybind"), &EditorTransformSnapSettings4D::set_scale_snap_invert_keybind);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "scale_snap_invert_keybind"), "set_scale_snap_invert_keybind", "get_scale_snap_invert_keybind");
}
