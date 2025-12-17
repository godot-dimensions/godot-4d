#pragma once

#include "editor_viewport_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/config_file.hpp>
#elif GODOT_MODULE
#include "core/io/config_file.h"
#endif

class EditorTransformSnapSettings4D : public Object {
	GDCLASS(EditorTransformSnapSettings4D, Object);

public:
	enum CameraDistanceMode {
		CAM_DIST_CONSTANT,
		CAM_DIST_DYNAMIC_POWER_OF_TEN,
		CAM_DIST_DYNAMIC_POWER_OF_TWO,
	};

	enum RotationSnapUnit {
		ROTATION_SNAP_DEGREES,
		ROTATION_SNAP_RADIANS,
		ROTATION_SNAP_INVERSE_TURNS,
	};

private:
	Ref<ConfigFile> _4d_editor_config_file;
	String _4d_editor_config_file_path = "";

	double _position_snap_distance_meters = 0.1;
	double _rotation_snap_angle_radians = Math_TAU * (15.0 / 360.0); // 15 degrees in radians.
	double _scale_snap_ratio = 0.1;
	real_t _camera_distance = 4.0;
	CameraDistanceMode _position_snap_camera_distance_mode = CAM_DIST_DYNAMIC_POWER_OF_TEN;
	RotationSnapUnit _rotation_snap_unit = ROTATION_SNAP_DEGREES;
	bool _snap_final_values = false;
	bool _position_snap_invert_keybind = false;
	bool _rotation_snap_invert_keybind = false;
	bool _scale_snap_invert_keybind = false;

	bool _is_modifier_pressed() const;
	Vector4 _snap_position(const Vector4 &p_position, const bool p_is_modifier_pressed) const;
	Basis4D _snap_rotation(const Basis4D &p_basis, const bool p_is_modifier_pressed) const;
	Transform4D _snap_scale(const Transform4D &p_transform, const bool p_is_modifier_pressed) const;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	bool get_snap_final_values() const { return _snap_final_values; }
	void set_snap_final_values(const bool p_snap_final_values);

	CameraDistanceMode get_position_snap_camera_distance_mode() const { return _position_snap_camera_distance_mode; }
	void set_position_snap_camera_distance_mode(const CameraDistanceMode p_position_snap_camera_distance_mode);
	double get_position_snap_distance_meters() const { return _position_snap_distance_meters; }
	void set_position_snap_distance_meters(const double p_position_snap_distance_meters);
	bool get_position_snap_invert_keybind() const { return _position_snap_invert_keybind; }
	void set_position_snap_invert_keybind(const bool p_position_snap_invert_keybind);

	RotationSnapUnit get_rotation_snap_unit() const { return _rotation_snap_unit; }
	void set_rotation_snap_unit(const RotationSnapUnit p_rotation_snap_unit);
	double get_rotation_snap_angle_radians() const { return _rotation_snap_angle_radians; }
	void set_rotation_snap_angle_radians(const double p_rotation_snap_angle_radians);
	double get_rotation_snap_angle_degrees() const { return _rotation_snap_angle_radians * (360.0 / Math_TAU); }
	void set_rotation_snap_angle_degrees(const double p_rotation_snap_angle_degrees);
	double get_rotation_snap_angle_inverse_turns() const { return Math_TAU / _rotation_snap_angle_radians; }
	void set_rotation_snap_angle_inverse_turns(const double p_rotation_snap_angle_inverse_turns);
	bool get_rotation_snap_invert_keybind() const { return _rotation_snap_invert_keybind; }
	void set_rotation_snap_invert_keybind(const bool p_rotation_snap_invert_keybind);

	double get_scale_snap_ratio() const { return _scale_snap_ratio; }
	void set_scale_snap_ratio(const double p_scale_snap_ratio);
	bool get_scale_snap_invert_keybind() const { return _scale_snap_invert_keybind; }
	void set_scale_snap_invert_keybind(const bool p_scale_snap_invert_keybind);

	Transform4D snap_single_transform(const Transform4D &p_transform) const;
	Transform4D snap_transform_change(const Transform4D &p_old_transform, const Transform4D &p_transform_change) const;

	void setup(Ref<ConfigFile> &p_config_file, const String &p_config_file_path);
	void set_camera_distance(const real_t p_camera_distance) { _camera_distance = p_camera_distance; }
	void write_to_config_file() const;
};

VARIANT_ENUM_CAST(EditorTransformSnapSettings4D::CameraDistanceMode);
VARIANT_ENUM_CAST(EditorTransformSnapSettings4D::RotationSnapUnit);
