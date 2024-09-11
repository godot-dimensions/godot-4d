#pragma once

#include "node_4d.h"

class Camera4D : public Node4D {
	GDCLASS(Camera4D, Node4D);

public:
	enum KeepAspect {
		KEEP_WIDTH,
		KEEP_HEIGHT
	};

	enum ProjectionType4D {
		PROJECTION4D_ORTHOGRAPHIC = 0,
		PROJECTION4D_PERSPECTIVE_4D = 1,
		PROJECTION4D_PERSPECTIVE_3D = 2,
		PROJECTION4D_PERSPECTIVE_DUAL = 3,
	};

	enum ViewAngleType {
		VIEW_ANGLE_FOCAL_LENGTH = 0,
		VIEW_ANGLE_FIELD_OF_VIEW = 1,
	};

	enum WFadeMode {
		W_FADE_DISABLED = 0,
		W_FADE_TRANSPARENCY = 1,
		W_FADE_HUE_SHIFT = 2,
		W_FADE_TRANSPARENCY_HUE_SHIFT = 3,
	};

private:
	String _rendering_engine = "";
	KeepAspect _keep_aspect = KEEP_HEIGHT;
	ProjectionType4D _projection_type = PROJECTION4D_PERSPECTIVE_4D;
	ViewAngleType _view_angle_type = VIEW_ANGLE_FOCAL_LENGTH;
	WFadeMode _w_fade_mode = W_FADE_TRANSPARENCY;

	// These have wrappers with trig functions, so let's use double to avoid precision loss.
	double _focal_length_4d = 1.0;
	double _focal_length_3d = 1.0;

	real_t _orthographic_size = 5.0;
	real_t _near = 0.05;
	real_t _far = 4000.0;
	real_t _w_fade_distance = 5.0;
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

	String get_rendering_engine() const;
	void set_rendering_engine(const String &p_rendering_engine);

	KeepAspect get_keep_aspect() const;
	void set_keep_aspect(const KeepAspect p_keep_aspect);

	ProjectionType4D get_projection_type() const;
	void set_projection_type(const ProjectionType4D p_projection_type_4d);
	bool is_first_pass_perspective() const;
	bool is_second_pass_perspective() const;

	ViewAngleType get_view_angle_type() const;
	void set_view_angle_type(const ViewAngleType p_view_angle_type);

	double get_focal_length_4d() const;
	void set_focal_length_4d(const double p_focal_length_4d);

	double get_focal_length_3d() const;
	void set_focal_length_3d(const double p_focal_length_3d);

	double get_field_of_view_4d() const;
	void set_field_of_view_4d(const double p_field_of_view_4d);

	double get_field_of_view_3d() const;
	void set_field_of_view_3d(const double p_field_of_view_3d);

	real_t get_orthographic_size() const;
	void set_orthographic_size(const real_t p_orthographic_size);

	real_t get_near() const;
	void set_near(const real_t p_near);

	real_t get_far() const;
	void set_far(const real_t p_far);

	WFadeMode get_w_fade_mode() const;
	void set_w_fade_mode(const WFadeMode p_w_fade_mode);

	real_t get_w_fade_distance() const;
	void set_w_fade_distance(const real_t p_w_fade_distance);
};

VARIANT_ENUM_CAST(Camera4D::KeepAspect);
VARIANT_ENUM_CAST(Camera4D::ProjectionType4D);
VARIANT_ENUM_CAST(Camera4D::ViewAngleType);
VARIANT_ENUM_CAST(Camera4D::WFadeMode);
