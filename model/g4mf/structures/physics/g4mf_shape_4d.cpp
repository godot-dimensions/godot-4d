#include "g4mf_shape_4d.h"

#include "../../../../math/vector_4d.h"
#include "../../../../physics/shapes/box_shape_4d.h"
#include "../../../../physics/shapes/capsule_shape_4d.h"
#include "../../../../physics/shapes/concave_mesh_shape_4d.h"
#include "../../../../physics/shapes/convex_hull_shape_4d.h"
#include "../../../../physics/shapes/cubinder_shape_4d.h"
#include "../../../../physics/shapes/cylinder_shape_4d.h"
#include "../../../../physics/shapes/duocylinder_shape_4d.h"
#include "../../../../physics/shapes/height_map_shape_4d.h"
#include "../../../../physics/shapes/orthoplex_shape_4d.h"
#include "../../../../physics/shapes/plane_shape_4d.h"
#include "../../../../physics/shapes/ray_shape_4d.h"
#include "../../../../physics/shapes/sphere_shape_4d.h"
#include "../../g4mf_state_4d.h"

bool G4MFShape4D::is_equal_exact(const Ref<G4MFShape4D> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	if (_shape_type != p_other->_shape_type) {
		return false;
	}
	if (_base_size != p_other->_base_size) {
		return false;
	}
	if (_curves.size() != p_other->_curves.size()) {
		return false;
	}
	for (int i = 0; i < _curves.size(); ++i) {
		Ref<GeneralShapeCurve4D> this_curve = _curves[i];
		Ref<GeneralShapeCurve4D> other_curve = p_other->_curves[i];
		if (!this_curve->is_equal_exact(other_curve)) {
			return false;
		}
	}
	if (_heights_accessor_index != p_other->_heights_accessor_index) {
		return false;
	}
	if (_mesh_index != p_other->_mesh_index) {
		return false;
	}
	if (_ray_length != p_other->_ray_length) {
		return false;
	}
	return true;
}

PackedFloat64Array G4MFShape4D::load_heights(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_accessors();
	ERR_FAIL_INDEX_V(_heights_accessor_index, state_accessors.size(), PackedFloat64Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_heights_accessor_index];
	return accessor->decode_floats_from_primitives(p_g4mf_state);
}

Ref<Shape4D> G4MFShape4D::_generate_shape_from_general() const {
	Ref<Shape4D> ret;
	if (_curves.is_empty()) {
		// No curves? This is just a box shape.
		Ref<BoxShape4D> box_shape;
		box_shape.instantiate();
		box_shape->set_size(_base_size);
		return box_shape;
	} else if (_curves.size() == 1) {
		const Ref<GeneralShapeCurve4D> curve = _curves[0];
		const Vector4 curve_radii = curve->get_radii();
		const int curve_dimension = curve->get_radii_dimension();
		const real_t curve_radius = curve->get_radii_sum() / curve_dimension;
		if (Math::is_zero_approx(_base_size.x) && Math::is_zero_approx(_base_size.z)) {
			// A single curve with no X or Z base size? Could be one of many 4D shapes: sphere, capsule, cylinder, cubinder, orthoplex.
			if (curve->get_exponent() == 1.0 && Math::is_zero_approx(_base_size.y) && Math::is_zero_approx(_base_size.w)) {
				// If the exponent is 1.0 and there is no base size, it is an orthoplex.
				Ref<OrthoplexShape4D> orthoplex_shape;
				orthoplex_shape.instantiate();
				orthoplex_shape->set_half_extents(curve_radii);
				return orthoplex_shape;
			} else if (curve_radii.x == curve_radius && curve_radii.z == curve_radius) {
				// The curve is uniform on X and Z? Could be one of many 4D shapes: sphere, capsule, cylinder, cubinder.
				if (curve_dimension == 4) {
					// A shape with one curve of dimension 4 can be a 4D sphere or a 4D capsule (or not one of our special shapes).
					if (Math::is_zero_approx(_base_size.w) && curve_radii.y == curve_radius && curve_radii.w == curve_radius) {
						// It passed! Now, one last check. The only difference between these two is that a sphere has no mid-height.
						if (Math::is_zero_approx(_base_size.y)) {
							Ref<SphereShape4D> sphere_shape;
							sphere_shape.instantiate();
							sphere_shape->set_radius(curve_radius);
							return sphere_shape;
						} else {
							Ref<CapsuleShape4D> capsule_shape;
							capsule_shape.instantiate();
							capsule_shape->set_radius(curve_radius);
							capsule_shape->set_height(_base_size.y);
							return capsule_shape;
						}
					}
				} else if (curve_dimension == 3) {
					// A shape with one curve of dimension 3 can be a 4D cylinder (or not one of our special shapes).
					if (Math::is_zero_approx(_base_size.w) && curve_radii.w == curve_radius) {
						Ref<CylinderShape4D> cylinder_shape;
						cylinder_shape.instantiate();
						cylinder_shape->set_radius(curve_radius);
						cylinder_shape->set_height(_base_size.y);
						return cylinder_shape;
					}
				} else if (curve_dimension == 2) {
					// A shape with one curve of dimension 2 can be a 4D cubinder (or not one of our special shapes).
					// All the previous checks are actually sufficient to narrow it down to a cubinder.
					Ref<CubinderShape4D> cubinder_shape;
					cubinder_shape.instantiate();
					cubinder_shape->set_radius(curve_radius);
					cubinder_shape->set_height(_base_size.y);
					cubinder_shape->set_thickness(_base_size.w);
					return cubinder_shape;
				}
			}
		}
	} else if (_curves.size() == 2 && _base_size.is_zero_approx()) {
		// The only special case shape with two curves is a duocylinder.
		const Ref<GeneralShapeCurve4D> curve0 = _curves[0];
		const Ref<GeneralShapeCurve4D> curve1 = _curves[1];
		const Vector4 curve0_radii = curve0->get_radii();
		const Vector4 curve1_radii = curve1->get_radii();
		if (curve0_radii.x == curve0_radii.y && curve0_radii.z == 0.0f && curve0_radii.w == 0.0f && curve1_radii.x == 0.0f && curve1_radii.y == 0.0f && curve1_radii.z == curve1_radii.w) {
			Ref<DuocylinderShape4D> duocylinder_shape;
			duocylinder_shape.instantiate();
			duocylinder_shape->set_radius_xy(curve0_radii.x);
			duocylinder_shape->set_radius_zw(curve1_radii.z);
			return duocylinder_shape;
		}
	}
	// If none of the special case shape types match, return a GeneralShape4D.
	Ref<GeneralShape4D> general_shape;
	general_shape.instantiate();
	general_shape->set_base_size(_base_size);
	general_shape->set_curves(_curves.duplicate());
	return general_shape;
}

Ref<Shape4D> G4MFShape4D::generate_shape(const Ref<G4MFState4D> &p_g4mf_state) const {
	Ref<Shape4D> ret;
	if (_shape_type == "concave") {
		Ref<ConcaveMeshShape4D> concave_shape;
		concave_shape.instantiate();
		ERR_PRINT("G4MFShape4D.generate_shape: Godot 4D does not support concave mesh shapes yet.");
		ret = concave_shape;
	} else if (_shape_type == "convex") {
		Ref<ConvexHullShape4D> convex_shape;
		convex_shape.instantiate();
		ERR_PRINT("G4MFShape4D.generate_shape: Godot 4D does not support convex hull mesh shapes yet.");
		ret = convex_shape;
	} else if (_shape_type == "heightmap") {
		Ref<HeightMapShape4D> heightmap_shape;
		heightmap_shape.instantiate();
		heightmap_shape->set_size(Vector3i(_base_size.x, _base_size.z, _base_size.w));
		heightmap_shape->set_height_data(load_heights(p_g4mf_state));
		ret = heightmap_shape;
	} else if (_shape_type == "plane") {
		Ref<PlaneShape4D> plane_shape;
		plane_shape.instantiate();
		// PlaneShape4D has no data to set.
		ret = plane_shape;
	} else if (_shape_type == "ray") {
		Ref<RayShape4D> ray_shape;
		ray_shape.instantiate();
		ray_shape->set_ray_length(_ray_length);
		ret = ray_shape;
	} else {
		// Either this is a general shape or we don't know what it is, so generate a general shape.
		ret = _generate_shape_from_general();
	}
	ERR_FAIL_COND_V_MSG(ret.is_null(), ret, "Failed to generate shape from G4MFShape4D.");
	ret->set_name(get_name());
	return ret;
}

Ref<G4MFShape4D> G4MFShape4D::convert_shape(Ref<G4MFState4D> p_g4mf_state, const Ref<Shape4D> &p_shape, const bool p_deduplicate) {
	Ref<G4MFShape4D> ret;
	ret.instantiate();
	ERR_FAIL_COND_V_MSG(p_shape.is_null(), ret, "The given Shape4D was null, cannot convert to G4MFShape4D.");
	ret->set_name(p_shape->get_name());
	const Ref<BoxShape4D> box_shape = p_shape;
	if (box_shape.is_valid()) {
		ret->set_base_size(box_shape->get_size());
		return ret;
	}
	const Ref<ConcaveMeshShape4D> concave_shape = p_shape;
	if (concave_shape.is_valid()) {
		ret->set_shape_type("concave");
		return ret;
	}
	const Ref<ConvexHullShape4D> convex_shape = p_shape;
	if (convex_shape.is_valid()) {
		ret->set_shape_type("convex");
		return ret;
	}
	const Ref<HeightMapShape4D> heightmap_shape = p_shape;
	if (heightmap_shape.is_valid()) {
		ret->set_shape_type("heightmap");
		const Vector3i heightmap_size = heightmap_shape->get_size();
		ret->set_base_size(Vector4(heightmap_size.x, 1.0, heightmap_size.y, heightmap_size.z));
		const PackedFloat64Array heightmap_data = heightmap_shape->get_height_data();
		const String prim_type = G4MFAccessor4D::minimal_primitive_type_for_floats(heightmap_shape->get_height_data());
		Ref<G4MFAccessor4D> accessor = G4MFAccessor4D::make_new_accessor_without_data(prim_type);
		const PackedByteArray encoded_heightmap_data = accessor->encode_floats_as_primitives(heightmap_data);
		int heights_accessor_index = accessor->store_accessor_data_into_state(p_g4mf_state, encoded_heightmap_data, p_deduplicate);
		ret->set_heights_accessor_index(heights_accessor_index);
		return ret;
	}
	const Ref<PlaneShape4D> plane_shape = p_shape;
	if (plane_shape.is_valid()) {
		ret->set_shape_type("plane");
		return ret;
	}
	const Ref<RayShape4D> ray_shape = p_shape;
	if (ray_shape.is_valid()) {
		ret->set_shape_type("ray");
		ret->set_ray_length(ray_shape->get_ray_length());
		return ret;
	}
	// If it's not any of those, it probably has a curve.
	Ref<GeneralShapeCurve4D> curve;
	curve.instantiate();
	const Ref<SphereShape4D> sphere_shape = p_shape;
	if (sphere_shape.is_valid()) {
		const real_t sphere_radius = sphere_shape->get_radius();
		curve->set_radii(Vector4(sphere_radius, sphere_radius, sphere_radius, sphere_radius));
		ret->_curves.append(curve);
		return ret;
	}
	const Ref<CapsuleShape4D> capsule_shape = p_shape;
	if (capsule_shape.is_valid()) {
		const real_t capsule_height = capsule_shape->get_height();
		ret->set_base_size(Vector4(0.0, capsule_height, 0.0, 0.0));
		const real_t capsule_radius = capsule_shape->get_radius();
		curve->set_radii(Vector4(capsule_radius, capsule_radius, capsule_radius, capsule_radius));
		ret->_curves.append(curve);
		return ret;
	}
	const Ref<CylinderShape4D> cylinder_shape = p_shape;
	if (cylinder_shape.is_valid()) {
		const real_t cylinder_height = cylinder_shape->get_height();
		ret->set_base_size(Vector4(0.0, cylinder_height, 0.0, 0.0));
		const real_t cylinder_radius = cylinder_shape->get_radius();
		curve->set_radii(Vector4(cylinder_radius, 0.0, cylinder_radius, cylinder_radius));
		ret->_curves.append(curve);
		return ret;
	}
	const Ref<CubinderShape4D> cubinder_shape = p_shape;
	if (cubinder_shape.is_valid()) {
		const real_t cubinder_height = cubinder_shape->get_height();
		const real_t cubinder_thickness = cubinder_shape->get_thickness();
		ret->set_base_size(Vector4(0.0, cubinder_height, 0.0, cubinder_thickness));
		const real_t cubinder_radius = cubinder_shape->get_radius();
		curve->set_radii(Vector4(cubinder_radius, 0.0, cubinder_radius, 0.0));
		ret->_curves.append(curve);
		return ret;
	}
	const Ref<DuocylinderShape4D> duocylinder_shape = p_shape;
	if (duocylinder_shape.is_valid()) {
		const real_t duocylinder_radius_xy = duocylinder_shape->get_radius_xy();
		const real_t duocylinder_radius_zw = duocylinder_shape->get_radius_zw();
		curve->set_radii(Vector4(duocylinder_radius_xy, duocylinder_radius_xy, 0.0, 0.0));
		ret->_curves.append(curve);
		Ref<GeneralShapeCurve4D> curve1;
		curve1.instantiate();
		curve1->set_radii(Vector4(0.0, 0.0, duocylinder_radius_zw, duocylinder_radius_zw));
		ret->_curves.append(curve1);
		return ret;
	}
	const Ref<OrthoplexShape4D> orthoplex_shape = p_shape;
	if (orthoplex_shape.is_valid()) {
		curve->set_radii(orthoplex_shape->get_half_extents());
		curve->set_exponent(1.0);
		ret->_curves.append(curve);
		return ret;
	}
	const Ref<GeneralShape4D> general_shape = p_shape;
	if (general_shape.is_valid()) {
		const Vector4 general_shape_base_size = general_shape->get_base_size();
		ret->set_base_size(general_shape_base_size);
		const TypedArray<GeneralShapeCurve4D> general_shape_curves = general_shape->get_curves();
		ret->set_curves(general_shape_curves);
		return ret;
	}
	WARN_PRINT("G4MFShape4D.convert_shape: Unknown shape type: " + String(Variant(p_shape)));
	return ret;
}

int G4MFShape4D::convert_shape_into_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Shape4D> &p_shape, const bool p_deduplicate) {
	Ref<G4MFShape4D> g4mf_shape = convert_shape(p_g4mf_state, p_shape);
	// Add the G4MFShape4D to the G4MFState4D, but check for duplicates first.
	TypedArray<G4MFShape4D> state_shapes = p_g4mf_state->get_g4mf_shapes();
	const int state_shape_count = state_shapes.size();
	if (p_deduplicate) {
		for (int i = 0; i < state_shape_count; i++) {
			const Ref<G4MFShape4D> state_shape = state_shapes[i];
			if (g4mf_shape->is_equal_exact(state_shape)) {
				// An identical shape already exists in state, we can just use that.
				return i;
			}
		}
	}
	state_shapes.append(g4mf_shape);
	p_g4mf_state->set_g4mf_shapes(state_shapes);
	return state_shape_count;
}

Ref<G4MFShape4D> G4MFShape4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFShape4D> ret;
	ret.instantiate();
	ret->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("type")) {
		ret->set_shape_type(p_dict["type"]);
	}
	if (p_dict.has("size")) {
		Array base_size_array = p_dict["size"];
		ret->set_base_size(Vector4D::from_json_array(base_size_array));
	}
	if (p_dict.has("length")) {
		ret->set_ray_length(p_dict["length"]);
	}
	if (p_dict.has("heights")) {
		ret->set_heights_accessor_index(p_dict["heights"]);
	}
	if (p_dict.has("mesh")) {
		ret->set_mesh_index(p_dict["mesh"]);
	}
	if (p_dict.has("curves")) {
		Array curves_array = p_dict["curves"];
		TypedArray<GeneralShapeCurve4D> curves;
		for (int curve_index = 0; curve_index < curves_array.size(); curve_index++) {
			Dictionary curve_dict = curves_array[curve_index];
			Ref<GeneralShapeCurve4D> curve;
			curve.instantiate();
			if (curve_dict.has("radii")) {
				curve->set_radii(Vector4D::from_json_array(curve_dict["radii"]));
			}
			if (curve_dict.has("exponent")) {
				curve->set_exponent(curve_dict["exponent"]);
			}
			if (curve_dict.has("taper")) {
				Array taper_array = curve_dict["taper"];
				Vector<GeneralShapeCurveTaperPoint4D> taper;
				for (int taper_index = 0; taper_index < taper_array.size(); ++taper_index) {
					Dictionary taper_dict = taper_array[taper_index];
					GeneralShapeCurveTaperPoint4D taper_point;
					if (taper_dict.has("position")) {
						taper_point.position = Vector4D::from_json_array(taper_dict["position"]);
					}
					if (taper_dict.has("radii")) {
						taper_point.radii = Vector4D::from_json_array(taper_dict["radii"]);
					}
					if (taper_dict.has("exponent")) {
						taper_point.exponent = taper_dict["exponent"];
					}
					taper.append(taper_point);
				}
				curve->set_taper(taper);
			}
			curves.append(curve);
		}
		ret->set_curves(curves);
	}
	return ret;
}

Dictionary G4MFShape4D::to_dictionary() const {
	Dictionary ret = write_item_entries_to_dictionary();
	if (_shape_type != "general") {
		ret["type"] = _shape_type;
	}
	if (!_base_size.is_zero_approx()) {
		ret["size"] = Vector4D::to_json_array(_base_size);
	}
	if (_ray_length != 1.0) {
		ret["length"] = _ray_length;
	}
	if (_heights_accessor_index > -1) {
		ret["heights"] = _heights_accessor_index;
	}
	if (_mesh_index > -1) {
		ret["mesh"] = _mesh_index;
	}
	if (!_curves.is_empty()) {
		Array curves_array;
		curves_array.resize(_curves.size());
		for (int curve_index = 0; curve_index < _curves.size(); curve_index++) {
			Ref<GeneralShapeCurve4D> curve = _curves[curve_index];
			Dictionary curve_dict;
			if (!curve->get_radii().is_zero_approx()) {
				curve_dict["radii"] = Vector4D::to_json_array(curve->get_radii());
			}
			if (curve->get_exponent() != 2.0) {
				curve_dict["exponent"] = curve->get_exponent();
			}
			if (curve->get_taper_count() > 0) {
				const Vector<GeneralShapeCurveTaperPoint4D> taper = curve->get_taper();
				Array taper_array;
				taper_array.resize(taper.size());
				for (int taper_index = 0; taper_index < taper.size(); ++taper_index) {
					Dictionary taper_dict;
					const GeneralShapeCurveTaperPoint4D &taper_point = taper[taper_index];
					if (!taper_point.position.is_zero_approx()) {
						taper_dict["position"] = Vector4D::to_json_array(taper_point.position);
					}
					if (!taper_point.radii.is_zero_approx()) {
						taper_dict["radii"] = Vector4D::to_json_array(taper_point.radii);
					}
					if (taper_point.exponent != 2.0) {
						taper_dict["exponent"] = taper_point.exponent;
					}
					taper_array[taper_index] = taper_dict;
				}
				curve_dict["taper"] = taper_array;
			}
			curves_array[curve_index] = curve_dict;
		}
		ret["curves"] = curves_array;
	}
	return ret;
}

void G4MFShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_base_size"), &G4MFShape4D::get_base_size);
	ClassDB::bind_method(D_METHOD("set_base_size", "base_size"), &G4MFShape4D::set_base_size);
	ClassDB::bind_method(D_METHOD("get_curves"), &G4MFShape4D::get_curves);
	ClassDB::bind_method(D_METHOD("set_curves", "curves"), &G4MFShape4D::set_curves);
	ClassDB::bind_method(D_METHOD("get_shape_type"), &G4MFShape4D::get_shape_type);
	ClassDB::bind_method(D_METHOD("set_shape_type", "shape_type"), &G4MFShape4D::set_shape_type);
	ClassDB::bind_method(D_METHOD("get_ray_length"), &G4MFShape4D::get_ray_length);
	ClassDB::bind_method(D_METHOD("set_ray_length", "ray_length"), &G4MFShape4D::set_ray_length);
	ClassDB::bind_method(D_METHOD("get_heights_accessor_index"), &G4MFShape4D::get_heights_accessor_index);
	ClassDB::bind_method(D_METHOD("set_heights_accessor_index", "heights_accessor_index"), &G4MFShape4D::set_heights_accessor_index);
	ClassDB::bind_method(D_METHOD("get_mesh_index"), &G4MFShape4D::get_mesh_index);
	ClassDB::bind_method(D_METHOD("set_mesh_index", "mesh_index"), &G4MFShape4D::set_mesh_index);

	ClassDB::bind_method(D_METHOD("load_heights", "g4mf_state"), &G4MFShape4D::load_heights);
	ClassDB::bind_method(D_METHOD("generate_shape", "g4mf_state"), &G4MFShape4D::generate_shape);
	ClassDB::bind_static_method("G4MFShape4D", D_METHOD("convert_shape", "g4mf_state", "shape", "deduplicate"), &G4MFShape4D::convert_shape, DEFVAL(true));
	ClassDB::bind_static_method("G4MFShape4D", D_METHOD("convert_shape_into_state", "g4mf_state", "shape", "deduplicate"), &G4MFShape4D::convert_shape_into_state, DEFVAL(true));
	ClassDB::bind_static_method("G4MFShape4D", D_METHOD("from_dictionary", "dict"), &G4MFShape4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFShape4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "base_size"), "set_base_size", "get_base_size");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "curves"), "set_curves", "get_curves");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "shape_type"), "set_shape_type", "get_shape_type");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ray_length"), "set_ray_length", "get_ray_length");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "heights_accessor_index"), "set_heights_accessor_index", "get_heights_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_index"), "set_mesh_index", "get_mesh_index");
}
