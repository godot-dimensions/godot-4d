#include "g4mf_camera_4d.h"

Camera4D *G4MFCamera4D::generate_camera_4d() const {
	Camera4D *ret = memnew(Camera4D);
	ret->set_clip_far(_clip_far);
	ret->set_clip_near(_clip_near);
	ret->set_field_of_view_4d(_fov_radians);
	ret->set_orthographic_size(_size_meters);
	if (_projection_type == "orthographic") {
		ret->set_projection_type(Camera4D::PROJECTION4D_ORTHOGRAPHIC);
	} else if (_clip_near <= CMP_EPSILON) {
		ret->set_clip_near(0.05);
	}
	if (_keep_aspect == 0) {
		ret->set_keep_aspect(Camera4D::KEEP_WIDTH);
	}
	return ret;
}

Ref<G4MFCamera4D> G4MFCamera4D::convert_camera_4d(const Camera4D *p_camera) {
	Ref<G4MFCamera4D> ret;
	ret.instantiate();
	if (p_camera->get_projection_type() == Camera4D::PROJECTION4D_ORTHOGRAPHIC) {
		ret->set_projection_type("orthographic");
	}
	ret->set_keep_aspect((int)p_camera->get_keep_aspect());
	ret->set_clip_far(p_camera->get_clip_far());
	ret->set_clip_near(p_camera->get_clip_near());
	ret->set_fov_radians(p_camera->get_field_of_view_4d());
	ret->set_size_meters(p_camera->get_orthographic_size());
	return ret;
}

Ref<G4MFCamera4D> G4MFCamera4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFCamera4D> ret;
	ret.instantiate();
	ret->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("clipFar")) {
		ret->set_clip_far(p_dict["clipFar"]);
	}
	if (p_dict.has("clipNear")) {
		ret->set_clip_near(p_dict["clipNear"]);
	}
	if (p_dict.has("fov")) {
		ret->set_fov_radians(p_dict["fov"]);
	}
	if (p_dict.has("keepAspect")) {
		ret->set_keep_aspect(p_dict["keepAspect"]);
	}
	if (p_dict.has("size")) {
		ret->set_size_meters(p_dict["size"]);
	}
	if (p_dict.has("type")) {
		ret->set_projection_type(p_dict["type"]);
	}
	return ret;
}

Dictionary G4MFCamera4D::to_dictionary() const {
	Dictionary ret = write_item_entries_to_dictionary();
	if (Math::is_finite(_clip_far)) {
		ret["clipFar"] = _clip_far;
	}
	if (Math::is_finite(_clip_near)) {
		ret["clipNear"] = _clip_near;
	}
	if (_keep_aspect != 1) {
		ret["keepAspect"] = _keep_aspect;
	}
	if (_projection_type == "perspective") {
		if (_fov_radians != double(Math_TAU / 4.0)) {
			ret["fov"] = _fov_radians;
		}
	} else {
		ret["type"] = _projection_type;
		if (_size_meters != 1.0) {
			ret["size"] = _size_meters;
		}
	}
#if GODOT_MODULE && GODOT_VERSION_MAJOR >= 4 && GODOT_VERSION_MINOR >= 4
	ret.sort();
#endif
	return ret;
}

void G4MFCamera4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_projection_type"), &G4MFCamera4D::get_projection_type);
	ClassDB::bind_method(D_METHOD("set_projection_type", "projection_type"), &G4MFCamera4D::set_projection_type);
	ClassDB::bind_method(D_METHOD("get_clip_far"), &G4MFCamera4D::get_clip_far);
	ClassDB::bind_method(D_METHOD("set_clip_far", "clip_far"), &G4MFCamera4D::set_clip_far);
	ClassDB::bind_method(D_METHOD("get_clip_near"), &G4MFCamera4D::get_clip_near);
	ClassDB::bind_method(D_METHOD("set_clip_near", "clip_near"), &G4MFCamera4D::set_clip_near);
	ClassDB::bind_method(D_METHOD("get_fov_radians"), &G4MFCamera4D::get_fov_radians);
	ClassDB::bind_method(D_METHOD("set_fov_radians", "fov_radians"), &G4MFCamera4D::set_fov_radians);
	ClassDB::bind_method(D_METHOD("get_size_meters"), &G4MFCamera4D::get_size_meters);
	ClassDB::bind_method(D_METHOD("set_size_meters", "size_meters"), &G4MFCamera4D::set_size_meters);
	ClassDB::bind_method(D_METHOD("get_keep_aspect"), &G4MFCamera4D::get_keep_aspect);
	ClassDB::bind_method(D_METHOD("set_keep_aspect", "keep_aspect"), &G4MFCamera4D::set_keep_aspect);

	ClassDB::bind_method(D_METHOD("generate_camera_4d"), &G4MFCamera4D::generate_camera_4d);
	ClassDB::bind_static_method("G4MFCamera4D", D_METHOD("convert_camera_4d", "camera"), &G4MFCamera4D::convert_camera_4d);
	ClassDB::bind_static_method("G4MFCamera4D", D_METHOD("from_dictionary", "dict"), &G4MFCamera4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFCamera4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "projection_type", PROPERTY_HINT_ENUM, "orthographic,perspective"), "set_projection_type", "get_projection_type");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clip_far"), "set_clip_far", "get_clip_far");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clip_near"), "set_clip_near", "get_clip_near");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fov_radians"), "set_fov_radians", "get_fov_radians");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "size_meters"), "set_size_meters", "get_size_meters");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "keep_aspect", PROPERTY_HINT_ENUM, "width,height"), "set_keep_aspect", "get_keep_aspect");
}
