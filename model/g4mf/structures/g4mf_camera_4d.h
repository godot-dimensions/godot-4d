#pragma once

#include "g4mf_item_4d.h"

#include "../../../nodes/camera_4d.h"

class G4MFCamera4D : public G4MFItem4D {
	GDCLASS(G4MFCamera4D, G4MFItem4D);

	String _projection_type = "perspective";
	double _clip_far = INFINITY;
	double _clip_near = -INFINITY;
	double _fov_radians = Math_TAU / 4.0;
	double _size_meters = 1.0;
	int _keep_aspect = 1;

protected:
	static void _bind_methods();

public:
	String get_projection_type() const { return _projection_type; }
	void set_projection_type(const String &p_projection_type) { _projection_type = p_projection_type; }

	double get_clip_far() const { return _clip_far; }
	void set_clip_far(const double p_clip_far) { _clip_far = p_clip_far; }

	double get_clip_near() const { return _clip_near; }
	void set_clip_near(const double p_clip_near) { _clip_near = p_clip_near; }

	double get_fov_radians() const { return _fov_radians; }
	void set_fov_radians(const double p_fov_radians) { _fov_radians = p_fov_radians; }

	double get_size_meters() const { return _size_meters; }
	void set_size_meters(const double p_size_meters) { _size_meters = p_size_meters; }

	int get_keep_aspect() const { return _keep_aspect; }
	void set_keep_aspect(const int p_keep_aspect) { _keep_aspect = p_keep_aspect; }

	Camera4D *generate_camera_4d() const;
	static Ref<G4MFCamera4D> convert_camera_4d(const Camera4D *p_camera);

	static Ref<G4MFCamera4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
