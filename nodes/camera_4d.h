#pragma once

#include "node_4d.h"

class Camera4D : public Node4D {
	GDCLASS(Camera4D, Node4D);

public:
	enum KeepAspect : int8_t {
		KEEP_WIDTH = 0,
		KEEP_HEIGHT = 1,
	};

	enum ProjectionType4D : int8_t {
		PROJECTION4D_ORTHOGRAPHIC = 0,
		PROJECTION4D_PERSPECTIVE_4D = 1,
		PROJECTION4D_PERSPECTIVE_3D = 2,
		PROJECTION4D_PERSPECTIVE_DUAL = 3,
	};

	enum ViewAngleType : int8_t {
		VIEW_ANGLE_FOCAL_LENGTH = 0,
		VIEW_ANGLE_FIELD_OF_VIEW = 1,
	};

	enum DepthFadeMode : int8_t {
		DEPTH_FADE_DISABLED = 0,
		DEPTH_FADE_DISTANCE = 1,
		DEPTH_FADE_XYZ_ONLY = 2,
		DEPTH_FADE_Z_ONLY = 3,
	};

	enum WFadeMode : int8_t {
		W_FADE_DISABLED = 0,
		W_FADE_TRANSPARENCY = 1,
		W_FADE_HUE_SHIFT = 2,
		W_FADE_TRANSPARENCY_HUE_SHIFT = 3,
	};

private:
	// Keep these default values in sync with the EditorCameraSettings4D defaults.
	String _rendering_engine_name = "";
	KeepAspect _keep_aspect = KEEP_HEIGHT;
	ProjectionType4D _projection_type = PROJECTION4D_PERSPECTIVE_4D;
	ViewAngleType _view_angle_type = VIEW_ANGLE_FOCAL_LENGTH;
	DepthFadeMode _depth_fade_mode = DEPTH_FADE_DISABLED;
	WFadeMode _w_fade_mode = W_FADE_TRANSPARENCY;

	// These have wrappers with trig functions, so let's use double to avoid precision loss.
	double _focal_length_4d = 1.25;
	double _focal_length_3d = 1.25;

	Color _w_fade_color_negative = Color(0.0f, 0.5f, 1.0f);
	Color _w_fade_color_positive = Color(1.0f, 0.5f, 0.0f);
	double _depth_fade_start = 25.0;
	double _orthographic_size = 5.0;
	double _clip_near = 0.05;
	double _clip_far = 4000.0;
	double _w_fade_distance = 5.0;
	double _w_fade_slope = 1.0;
	double _edge_falloff = 1.0;
	double _plane_sharpness = 0.3;
	double _skewness = 0.0;
	double _projection_opacity = 1.0;
	double _projection_opacity_base = 1.0;
	bool _is_current = false;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;
	void _notification(int p_what);

public:
	bool is_current() const;
	void set_current(bool p_enabled);
	void clear_current(bool p_enable_next = true);
	void make_current();

	bool is_position_behind(const Vector4 &p_global_position) const;
	Vector4 viewport_to_world_ray_origin(const Vector2 &p_viewport_position) const;
	Vector4 viewport_to_world_ray_direction(const Vector2 &p_viewport_position) const;
	Vector2 world_to_viewport_local_normal(const Vector4 &p_local_position) const;
	Vector2 world_to_viewport(const Vector4 &p_global_position) const;

	String get_rendering_engine_name() const { return _rendering_engine_name; }
	void set_rendering_engine_name(const String &p_rendering_engine_name);

	KeepAspect get_keep_aspect() const { return _keep_aspect; }
	void set_keep_aspect(const KeepAspect p_keep_aspect);

	ProjectionType4D get_projection_type() const { return _projection_type; }
	void set_projection_type(const ProjectionType4D p_projection_type_4d);
	bool is_first_pass_perspective() const;
	bool is_second_pass_perspective() const;

	ViewAngleType get_view_angle_type() const { return _view_angle_type; }
	void set_view_angle_type(const ViewAngleType p_view_angle_type);

	double get_focal_length_4d() const { return _focal_length_4d; }
	void set_focal_length_4d(const double p_focal_length_4d);

	double get_focal_length_3d() const { return _focal_length_3d; }
	void set_focal_length_3d(const double p_focal_length_3d);

	double get_field_of_view_4d() const;
	void set_field_of_view_4d(const double p_field_of_view_4d);

	double get_field_of_view_3d() const;
	void set_field_of_view_3d(const double p_field_of_view_3d);

	double get_orthographic_size() const { return _orthographic_size; }
	void set_orthographic_size(const double p_orthographic_size);

	double get_clip_near() const { return _clip_near; }
	void set_clip_near(const double p_clip_near);

	double get_clip_far() const { return _clip_far; }
	void set_clip_far(const double p_clip_far);

	DepthFadeMode get_depth_fade_mode() const { return _depth_fade_mode; }
	void set_depth_fade_mode(const DepthFadeMode p_depth_fade_mode);

	double get_depth_fade_start() const { return _depth_fade_start; }
	void set_depth_fade_start(const double p_depth_fade_start);

	WFadeMode get_w_fade_mode() const { return _w_fade_mode; }
	void set_w_fade_mode(const WFadeMode p_w_fade_mode);

	Color get_w_fade_color_negative() const { return _w_fade_color_negative; }
	void set_w_fade_color_negative(const Color &p_w_fade_color_negative);

	Color get_w_fade_color_positive() const { return _w_fade_color_positive; }
	void set_w_fade_color_positive(const Color &p_w_fade_color_positive);

	double get_w_fade_distance() const { return _w_fade_distance; }
	void set_w_fade_distance(const double p_w_fade_distance);

	double get_w_fade_slope() const { return _w_fade_slope; }
	void set_w_fade_slope(const double p_w_fade_slope);

	double get_edge_falloff() const { return _edge_falloff; }
	void set_edge_falloff(const double p_edge_falloff);

	double get_plane_sharpness() const { return _plane_sharpness; }
	void set_plane_sharpness(const double p_plane_sharpness);

	double get_skewness() const { return _skewness; }
	void set_skewness(const double p_skewness);

	double get_projection_opacity() const { return _projection_opacity; }
	void set_projection_opacity(const double p_projection_opacity);

	double get_projection_opacity_base() const { return _projection_opacity_base; }
	void set_projection_opacity_base(const double p_projection_opacity_base);
};

VARIANT_ENUM_CAST(Camera4D::KeepAspect);
VARIANT_ENUM_CAST(Camera4D::ProjectionType4D);
VARIANT_ENUM_CAST(Camera4D::ViewAngleType);
VARIANT_ENUM_CAST(Camera4D::DepthFadeMode);
VARIANT_ENUM_CAST(Camera4D::WFadeMode);
