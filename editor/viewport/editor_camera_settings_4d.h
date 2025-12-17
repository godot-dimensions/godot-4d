#pragma once

#include "editor_viewport_4d_defines.h"

#include "../../nodes/camera_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/config_file.hpp>
#elif GODOT_MODULE
#include "core/io/config_file.h"
#endif

class EditorCameraSettings4D : public Object {
	GDCLASS(EditorCameraSettings4D, Object);

	Node *_ancestor_of_cameras = nullptr;

	// Keep these default values in sync with the Camera4D defaults and the values in `write_to_config_file()`.
	double _clip_near = 0.05;
	double _clip_far = 4000.0;

	Camera4D::DepthFadeMode _depth_fade_mode = Camera4D::DEPTH_FADE_DISABLED;
	double _depth_fade_start = 25.0;

	Camera4D::WFadeMode _w_fade_mode = Camera4D::W_FADE_TRANSPARENCY;
	Color _w_fade_color_negative = Color(0.0f, 0.5f, 1.0f);
	Color _w_fade_color_positive = Color(1.0f, 0.5f, 0.0f);
	double _w_fade_distance = 5.0;
	double _w_fade_slope = 1.0;

	Ref<ConfigFile> _4d_editor_config_file;
	String _4d_editor_config_file_path = "";
	String _rendering_engine = "";

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	double get_clip_near() const { return _clip_near; }
	void set_clip_near(const double p_clip_near);

	double get_clip_far() const { return _clip_far; }
	void set_clip_far(const double p_clip_far);

	Camera4D::DepthFadeMode get_depth_fade_mode() const { return _depth_fade_mode; }
	void set_depth_fade_mode(const Camera4D::DepthFadeMode p_depth_fade_mode);

	double get_depth_fade_start() const { return _depth_fade_start; }
	void set_depth_fade_start(const double p_depth_fade_start);

	Camera4D::WFadeMode get_w_fade_mode() const { return _w_fade_mode; }
	void set_w_fade_mode(const Camera4D::WFadeMode p_w_fade_mode);

	Color get_w_fade_color_negative() const { return _w_fade_color_negative; }
	void set_w_fade_color_negative(const Color &p_w_fade_color_negative);

	Color get_w_fade_color_positive() const { return _w_fade_color_positive; }
	void set_w_fade_color_positive(const Color &p_w_fade_color_positive);

	double get_w_fade_distance() const { return _w_fade_distance; }
	void set_w_fade_distance(const double p_w_fade_distance);

	double get_w_fade_slope() const { return _w_fade_slope; }
	void set_w_fade_slope(const double p_w_fade_slope);

	// Only a setter because the source of truth for this should be the rendering engine menu.
	void set_rendering_engine(const String &p_rendering_engine);

	void apply_to_cameras() const;
	void setup(Node *p_ancestor_of_cameras, Ref<ConfigFile> &p_config_file, const String &p_config_file_path);
	void write_to_config_file() const;
};
