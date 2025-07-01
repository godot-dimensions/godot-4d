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

	double _clip_near = 0.05;
	double _clip_far = 4000.0;

	Camera4D::WFadeMode _w_fade_mode = Camera4D::W_FADE_TRANSPARENCY;
	Color _w_fade_color_negative = Color(0.0f, 0.5f, 1.0f);
	Color _w_fade_color_positive = Color(1.0f, 0.5f, 0.0f);

	Ref<ConfigFile> _4d_editor_config_file;
	String _4d_editor_config_file_path;
	String _rendering_engine = "";

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	double get_clip_near() const { return _clip_near; }
	void set_clip_near(const double p_clip_near);

	double get_clip_far() const { return _clip_far; }
	void set_clip_far(const double p_clip_far);

	Camera4D::WFadeMode get_w_fade_mode() const { return _w_fade_mode; }
	void set_w_fade_mode(const Camera4D::WFadeMode p_w_fade_mode);

	Color get_w_fade_color_negative() const { return _w_fade_color_negative; }
	void set_w_fade_color_negative(const Color &p_w_fade_color_negative);

	Color get_w_fade_color_positive() const { return _w_fade_color_positive; }
	void set_w_fade_color_positive(const Color &p_w_fade_color_positive);

	// Only a setter because the source of truth for this should be the rendering engine menu.
	void set_rendering_engine(const String &p_rendering_engine);

	void apply_to_cameras() const;
	void setup(Node *p_ancestor_of_cameras, Ref<ConfigFile> &p_config_file, const String &p_config_file_path);
	void write_to_config_file() const;
};
