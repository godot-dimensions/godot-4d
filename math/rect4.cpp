#include "rect4.h"

// Basic math functions.
Vector4 Rect4::get_longest_axis() const {
	Vector4 longest_axis = Vector4(size.x, 0, 0, 0);
	real_t longest_size = size.x;
	if (size.y > longest_size) {
		longest_size = size.y;
		longest_axis = Vector4(0, size.y, 0, 0);
	}
	if (size.z > longest_size) {
		longest_size = size.z;
		longest_axis = Vector4(0, 0, size.z, 0);
	}
	if (size.w > longest_size) {
		longest_axis = Vector4(0, 0, 0, size.w);
	}
	return longest_axis;
}

Vector4 Rect4::get_longest_axis_direction() const {
	Vector4 longest_axis = Vector4(1, 0, 0, 0);
	real_t longest_size = size.x;
	if (size.y > longest_size) {
		longest_size = size.y;
		longest_axis = Vector4(0, 1, 0, 0);
	}
	if (size.z > longest_size) {
		longest_size = size.z;
		longest_axis = Vector4(0, 0, 1, 0);
	}
	if (size.w > longest_size) {
		longest_axis = Vector4(0, 0, 0, 1);
	}
	return longest_axis;
}

Vector4::Axis Rect4::get_longest_axis_index() const {
	Vector4::Axis longest_axis = Vector4::Axis::AXIS_X;
	real_t longest_size = size.x;
	if (size.y > longest_size) {
		longest_size = size.y;
		longest_axis = Vector4::Axis::AXIS_Y;
	}
	if (size.z > longest_size) {
		longest_size = size.z;
		longest_axis = Vector4::Axis::AXIS_Z;
	}
	if (size.w > longest_size) {
		longest_axis = Vector4::Axis::AXIS_W;
	}
	return longest_axis;
}

real_t Rect4::get_longest_axis_size() const {
	real_t longest_size = size.x;
	if (size.y > longest_size) {
		longest_size = size.y;
	}
	if (size.z > longest_size) {
		longest_size = size.z;
	}
	if (size.w > longest_size) {
		longest_size = size.w;
	}
	return longest_size;
}

Vector4 Rect4::get_shortest_axis() const {
	Vector4 shortest_axis = Vector4(size.x, 0, 0, 0);
	real_t shortest_size = size.x;
	if (size.y < shortest_size) {
		shortest_size = size.y;
		shortest_axis = Vector4(0, size.y, 0, 0);
	}
	if (size.z < shortest_size) {
		shortest_size = size.z;
		shortest_axis = Vector4(0, 0, size.z, 0);
	}
	if (size.w < shortest_size) {
		shortest_axis = Vector4(0, 0, 0, size.w);
	}
	return shortest_axis;
}

Vector4 Rect4::get_shortest_axis_direction() const {
	Vector4 shortest_axis = Vector4(1, 0, 0, 0);
	real_t shortest_size = size.x;
	if (size.y < shortest_size) {
		shortest_size = size.y;
		shortest_axis = Vector4(0, 1, 0, 0);
	}
	if (size.z < shortest_size) {
		shortest_size = size.z;
		shortest_axis = Vector4(0, 0, 1, 0);
	}
	if (size.w < shortest_size) {
		shortest_axis = Vector4(0, 0, 0, 1);
	}
	return shortest_axis;
}

Vector4::Axis Rect4::get_shortest_axis_index() const {
	Vector4::Axis shortest_axis = Vector4::Axis::AXIS_X;
	real_t shortest_size = size.x;
	if (size.y < shortest_size) {
		shortest_size = size.y;
		shortest_axis = Vector4::Axis::AXIS_Y;
	}
	if (size.z < shortest_size) {
		shortest_size = size.z;
		shortest_axis = Vector4::Axis::AXIS_Z;
	}
	if (size.w < shortest_size) {
		shortest_axis = Vector4::Axis::AXIS_W;
	}
	return shortest_axis;
}

real_t Rect4::get_shortest_axis_size() const {
	real_t shortest_size = size.x;
	if (size.y < shortest_size) {
		shortest_size = size.y;
	}
	if (size.z < shortest_size) {
		shortest_size = size.z;
	}
	if (size.w < shortest_size) {
		shortest_size = size.w;
	}
	return shortest_size;
}

// Point functions.
Vector4 Rect4::get_nearest_point(const Vector4 &p_point) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f)) {
		ERR_PRINT("Rect4 size is negative, this is not supported. Use Rect4.abs() to get a Rect4 with a positive size.");
	}
#endif
	Vector4 end = get_end();
	Vector4 closest = p_point;
	if (p_point.x < position.x) {
		closest.x = position.x;
	} else if (p_point.x > end.x) {
		closest.x = end.x;
	}
	if (p_point.y < position.y) {
		closest.y = position.y;
	} else if (p_point.y > end.y) {
		closest.y = end.y;
	}
	if (p_point.z < position.z) {
		closest.z = position.z;
	} else if (p_point.z > end.z) {
		closest.z = end.z;
	}
	if (p_point.w < position.w) {
		closest.w = position.w;
	} else if (p_point.w > end.w) {
		closest.w = end.w;
	}
	return closest;
}

Rect4 Rect4::expand_to_point(const Vector4 &p_vector) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f)) {
		ERR_PRINT("Rect4 size is negative, this is not supported. Use Rect4.abs() to get a Rect4 with a positive size.");
	}
#endif // MATH_CHECKS
	Vector4 end = get_end();
	Rect4 new_rect = *this;
	if (p_vector.x < position.x) {
		new_rect.position.x = p_vector.x;
	} else if (p_vector.x > end.x) {
		end.x = p_vector.x;
	}
	if (p_vector.y < position.y) {
		new_rect.position.y = p_vector.y;
	} else if (p_vector.y > end.y) {
		end.y = p_vector.y;
	}
	if (p_vector.z < position.z) {
		new_rect.position.z = p_vector.z;
	} else if (p_vector.z > end.z) {
		end.z = p_vector.z;
	}
	if (p_vector.w < position.w) {
		new_rect.position.w = p_vector.w;
	} else if (p_vector.w > end.w) {
		end.w = p_vector.w;
	}
	new_rect.set_end(end);
	return new_rect;
}

void Rect4::expand_self_to_point(const Vector4 &p_vector) {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f)) {
		ERR_PRINT("Rect4 size is negative, this is not supported. Use Rect4.abs() to get a Rect4 with a positive size.");
	}
#endif // MATH_CHECKS
	Vector4 end = get_end();
	if (p_vector.x < position.x) {
		position.x = p_vector.x;
	} else if (p_vector.x > end.x) {
		end.x = p_vector.x;
	}
	if (p_vector.y < position.y) {
		position.y = p_vector.y;
	} else if (p_vector.y > end.y) {
		end.y = p_vector.y;
	}
	if (p_vector.z < position.z) {
		position.z = p_vector.z;
	} else if (p_vector.z > end.z) {
		end.z = p_vector.z;
	}
	if (p_vector.w < position.w) {
		position.w = p_vector.w;
	} else if (p_vector.w > end.w) {
		end.w = p_vector.w;
	}
	set_end(end);
}

bool Rect4::has_point(const Vector4 &p_point) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f)) {
		ERR_PRINT("Rect4 size is negative, this is not supported. Use Rect4.abs() to get a Rect4 with a positive size.");
	}
#endif
	return p_point.x >= position.x && p_point.x <= position.x + size.x &&
			p_point.y >= position.y && p_point.y <= position.y + size.y &&
			p_point.z >= position.z && p_point.z <= position.z + size.z &&
			p_point.w >= position.w && p_point.w <= position.w + size.w;
}

Vector4 Rect4::get_support_point(const Vector4 &p_direction) const {
	Vector4 support = position;
	if (p_direction.x > 0.0f) {
		support.x += size.x;
	}
	if (p_direction.y > 0.0f) {
		support.y += size.y;
	}
	if (p_direction.z > 0.0f) {
		support.z += size.z;
	}
	if (p_direction.w > 0.0f) {
		support.w += size.w;
	}
	return support;
}

// Rect math functions.
Rect4 Rect4::grow(const real_t p_by) const {
	Rect4 new_rect = *this;
	new_rect.position -= Vector4(p_by, p_by, p_by, p_by);
	new_rect.size += Vector4(p_by, p_by, p_by, p_by) * 2.0f;
	return new_rect;
}

Rect4 Rect4::intersection(const Rect4 &p_other) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f || p_other.size.x < 0.0f || p_other.size.y < 0.0f || p_other.size.z < 0.0f || p_other.size.w < 0.0f)) {
		ERR_PRINT("Rect4 size is negative, this is not supported. Use Rect4.abs() to get a Rect4 with a positive size.");
	}
#endif
	Vector4 end = get_end();
	Vector4 other_end = p_other.get_end();
	Rect4 new_rect = *this;
	if (position.x > other_end.x || end.x < p_other.position.x ||
			position.y > other_end.y || end.y < p_other.position.y ||
			position.z > other_end.z || end.z < p_other.position.z ||
			position.w > other_end.w || end.w < p_other.position.w) {
		// No intersection.
		return Rect4();
	}
	if (p_other.position.x > position.x) {
		new_rect.position.x = p_other.position.x;
	}
	if (p_other.position.y > position.y) {
		new_rect.position.y = p_other.position.y;
	}
	if (p_other.position.z > position.z) {
		new_rect.position.z = p_other.position.z;
	}
	if (p_other.position.w > position.w) {
		new_rect.position.w = p_other.position.w;
	}
	if (other_end.x < end.x) {
		end.x = other_end.x;
	}
	if (other_end.y < end.y) {
		end.y = other_end.y;
	}
	if (other_end.z < end.z) {
		end.z = other_end.z;
	}
	if (other_end.w < end.w) {
		end.w = other_end.w;
	}
	new_rect.set_end(end);
	return new_rect;
}

Rect4 Rect4::merge(const Rect4 &p_other) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f || p_other.size.x < 0.0f || p_other.size.y < 0.0f || p_other.size.z < 0.0f || p_other.size.w < 0.0f)) {
		ERR_PRINT("Rect4 size is negative, this is not supported. Use Rect4.abs() to get a Rect4 with a positive size.");
	}
#endif
	Vector4 end = get_end();
	Vector4 other_end = p_other.get_end();
	Rect4 new_rect = *this;
	if (p_other.position.x < position.x) {
		new_rect.position.x = p_other.position.x;
	}
	if (p_other.position.y < position.y) {
		new_rect.position.y = p_other.position.y;
	}
	if (p_other.position.z < position.z) {
		new_rect.position.z = p_other.position.z;
	}
	if (p_other.position.w < position.w) {
		new_rect.position.w = p_other.position.w;
	}
	if (other_end.x > end.x) {
		end.x = other_end.x;
	}
	if (other_end.y > end.y) {
		end.y = other_end.y;
	}
	if (other_end.z > end.z) {
		end.z = other_end.z;
	}
	if (other_end.w > end.w) {
		end.w = other_end.w;
	}
	new_rect.set_end(end);
	return new_rect;
}

// Rect comparison functions.
bool Rect4::encloses_exclusive(const Rect4 &p_other) const {
	Vector4 end = get_end();
	Vector4 other_end = p_other.get_end();
	return position.x < p_other.position.x && end.x > other_end.x &&
			position.y < p_other.position.y && end.y > other_end.y &&
			position.z < p_other.position.z && end.z > other_end.z &&
			position.w < p_other.position.w && end.w > other_end.w;
}

bool Rect4::encloses_inclusive(const Rect4 &p_other) const {
	Vector4 end = get_end();
	Vector4 other_end = p_other.get_end();
	return position.x <= p_other.position.x && end.x >= other_end.x &&
			position.y <= p_other.position.y && end.y >= other_end.y &&
			position.z <= p_other.position.z && end.z >= other_end.z &&
			position.w <= p_other.position.w && end.w >= other_end.w;
}

bool Rect4::intersects_exclusive(const Rect4 &p_other) const {
	Vector4 end = get_end();
	Vector4 other_end = p_other.get_end();
	return position.x < other_end.x && end.x > p_other.position.x &&
			position.y < other_end.y && end.y > p_other.position.y &&
			position.z < other_end.z && end.z > p_other.position.z &&
			position.w < other_end.w && end.w > p_other.position.w;
}

bool Rect4::intersects_inclusive(const Rect4 &p_other) const {
	Vector4 end = get_end();
	Vector4 other_end = p_other.get_end();
	return position.x <= other_end.x && end.x >= p_other.position.x &&
			position.y <= other_end.y && end.y >= p_other.position.y &&
			position.z <= other_end.z && end.z >= p_other.position.z &&
			position.w <= other_end.w && end.w >= p_other.position.w;
}

bool Rect4::is_equal_approx(const Rect4 &p_other) const {
	return position.is_equal_approx(p_other.position) && size.is_equal_approx(p_other.size);
}

bool Rect4::is_finite() const {
	return position.is_finite() && size.is_finite();
}

// Operators.
bool Rect4::operator==(const Rect4 &p_other) const {
	return position == p_other.position && size == p_other.size;
}

bool Rect4::operator!=(const Rect4 &p_other) const {
	return position != p_other.position || size != p_other.size;
}

Rect4::operator String() const {
	return "[P: " + position.operator String() + ", S: " + size + "]";
}

static_assert(sizeof(Rect4) == 8 * sizeof(real_t));
