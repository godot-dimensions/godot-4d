#include "shape_4d.h"

void Shape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("has_point", "point"), &Shape4D::has_point);
	GDVIRTUAL_BIND(_has_point, "point");
}

bool Shape4D::has_point(const Vector4 &p_point) const {
	bool has = false;
	GDVIRTUAL_CALL(_has_point, p_point, has);
	return has;
}
