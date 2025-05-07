#include "general_shape_curve_4d.h"

void GeneralShapeCurve4D::set_radii(const Vector4 &p_radii) {
	_radii = p_radii.abs();
}

void GeneralShapeCurve4D::set_exponent(double p_exponent) {
	ERR_FAIL_COND_MSG(!(p_exponent > 0.0), "GeneralShapeCurve4D: Exponent must be a finite positive number.");
	_exponent = p_exponent;
}

int GeneralShapeCurve4D::get_radii_dimension() const {
	int dimension = 0;
	for (int i = 0; i < 4; i++) {
		if (_radii[i] > 0.0) {
			dimension++;
		}
	}
	return dimension;
}

real_t GeneralShapeCurve4D::get_radii_sum() const {
	return _radii.x + _radii.y + _radii.z + _radii.w;
}

PackedInt32Array GeneralShapeCurve4D::get_used_axes() const {
	PackedInt32Array used_axes;
	for (int i = 0; i < 4; i++) {
		if (_radii[i] > 0.0) {
			used_axes.push_back(i);
		}
	}
	return used_axes;
}

PackedInt32Array GeneralShapeCurve4D::get_zero_axes() const {
	PackedInt32Array zero_axes;
	for (int i = 0; i < 4; i++) {
		if (_radii[i] == 0.0) {
			zero_axes.push_back(i);
		}
	}
	return zero_axes;
}

bool GeneralShapeCurveTaperPoint4D::is_equal_exact(const GeneralShapeCurveTaperPoint4D &p_other) const {
	if (position != p_other.position) {
		return false;
	}
	if (radii != p_other.radii) {
		return false;
	}
	if (exponent != p_other.exponent) {
		return false;
	}
	return true;
}

bool GeneralShapeCurve4D::is_equal_exact(const Ref<GeneralShapeCurve4D> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	if (_radii != p_other->_radii) {
		return false;
	}
	if (_exponent != p_other->_exponent) {
		return false;
	}
	for (int i = 0; i < _taper.size(); i++) {
		if (!_taper[i].is_equal_exact(p_other->_taper[i])) {
			return false;
		}
	}
	return true;
}

void GeneralShapeCurve4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radii"), &GeneralShapeCurve4D::get_radii);
	ClassDB::bind_method(D_METHOD("set_radii", "radii"), &GeneralShapeCurve4D::set_radii);
	ClassDB::bind_method(D_METHOD("get_exponent"), &GeneralShapeCurve4D::get_exponent);
	ClassDB::bind_method(D_METHOD("set_exponent", "exponent"), &GeneralShapeCurve4D::set_exponent);

	ClassDB::bind_method(D_METHOD("get_radii_dimension"), &GeneralShapeCurve4D::get_radii_dimension);
	ClassDB::bind_method(D_METHOD("get_radii_sum"), &GeneralShapeCurve4D::get_radii_sum);
	ClassDB::bind_method(D_METHOD("get_used_axes"), &GeneralShapeCurve4D::get_used_axes);
	ClassDB::bind_method(D_METHOD("get_zero_axes"), &GeneralShapeCurve4D::get_zero_axes);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "radii", PROPERTY_HINT_RANGE, "0.001,1000,0.001,or_greater"), "set_radii", "get_radii");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "exponent", PROPERTY_HINT_RANGE, "0.001,1000,0.001"), "set_exponent", "get_exponent");
}
