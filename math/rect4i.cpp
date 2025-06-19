#include "rect4i.h"

// Basic math functions.
Vector4i Rect4i::get_longest_axis() const {
	Vector4i longest_axis = Vector4i(size.x, 0, 0, 0);
	int32_t longest_size = size.x;
	if (size.y > longest_size) {
		longest_size = size.y;
		longest_axis = Vector4i(0, size.y, 0, 0);
	}
	if (size.z > longest_size) {
		longest_size = size.z;
		longest_axis = Vector4i(0, 0, size.z, 0);
	}
	if (size.w > longest_size) {
		longest_axis = Vector4i(0, 0, 0, size.w);
	}
	return longest_axis;
}

Vector4i Rect4i::get_longest_axis_direction() const {
	Vector4i longest_axis = Vector4i(1, 0, 0, 0);
	int32_t longest_size = size.x;
	if (size.y > longest_size) {
		longest_size = size.y;
		longest_axis = Vector4i(0, 1, 0, 0);
	}
	if (size.z > longest_size) {
		longest_size = size.z;
		longest_axis = Vector4i(0, 0, 1, 0);
	}
	if (size.w > longest_size) {
		longest_axis = Vector4i(0, 0, 0, 1);
	}
	return longest_axis;
}

Vector4i::Axis Rect4i::get_longest_axis_index() const {
	Vector4i::Axis longest_axis = Vector4i::Axis::AXIS_X;
	int32_t longest_size = size.x;
	if (size.y > longest_size) {
		longest_size = size.y;
		longest_axis = Vector4i::Axis::AXIS_Y;
	}
	if (size.z > longest_size) {
		longest_size = size.z;
		longest_axis = Vector4i::Axis::AXIS_Z;
	}
	if (size.w > longest_size) {
		longest_axis = Vector4i::Axis::AXIS_W;
	}
	return longest_axis;
}

int32_t Rect4i::get_longest_axis_size() const {
	int32_t longest_size = size.x;
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

Vector4i Rect4i::get_shortest_axis() const {
	Vector4i shortest_axis = Vector4i(size.x, 0, 0, 0);
	int32_t shortest_size = size.x;
	if (size.y < shortest_size) {
		shortest_size = size.y;
		shortest_axis = Vector4i(0, size.y, 0, 0);
	}
	if (size.z < shortest_size) {
		shortest_size = size.z;
		shortest_axis = Vector4i(0, 0, size.z, 0);
	}
	if (size.w < shortest_size) {
		shortest_axis = Vector4i(0, 0, 0, size.w);
	}
	return shortest_axis;
}

Vector4i Rect4i::get_shortest_axis_direction() const {
	Vector4i shortest_axis = Vector4i(1, 0, 0, 0);
	int32_t shortest_size = size.x;
	if (size.y < shortest_size) {
		shortest_size = size.y;
		shortest_axis = Vector4i(0, 1, 0, 0);
	}
	if (size.z < shortest_size) {
		shortest_size = size.z;
		shortest_axis = Vector4i(0, 0, 1, 0);
	}
	if (size.w < shortest_size) {
		shortest_axis = Vector4i(0, 0, 0, 1);
	}
	return shortest_axis;
}

Vector4i::Axis Rect4i::get_shortest_axis_index() const {
	Vector4i::Axis shortest_axis = Vector4i::Axis::AXIS_X;
	int32_t shortest_size = size.x;
	if (size.y < shortest_size) {
		shortest_size = size.y;
		shortest_axis = Vector4i::Axis::AXIS_Y;
	}
	if (size.z < shortest_size) {
		shortest_size = size.z;
		shortest_axis = Vector4i::Axis::AXIS_Z;
	}
	if (size.w < shortest_size) {
		shortest_axis = Vector4i::Axis::AXIS_W;
	}
	return shortest_axis;
}

int32_t Rect4i::get_shortest_axis_size() const {
	int32_t shortest_size = size.x;
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
Vector4i Rect4i::get_nearest_point(const Vector4i &p_point) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f)) {
		ERR_PRINT("Rect4i size is negative, this is not supported. Use Rect4i.abs() to get a Rect4i with a positive size.");
	}
#endif
	Vector4i end = get_end();
	Vector4i closest = p_point;
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

Rect4i Rect4i::expand_to_point(const Vector4i &p_vector) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f)) {
		ERR_PRINT("Rect4i size is negative, this is not supported. Use Rect4i.abs() to get a Rect4i with a positive size.");
	}
#endif
	Vector4i end = get_end();
	Rect4i new_rect = *this;
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

bool Rect4i::has_point(const Vector4i &p_point) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f)) {
		ERR_PRINT("Rect4i size is negative, this is not supported. Use Rect4i.abs() to get a Rect4i with a positive size.");
	}
#endif
	return p_point.x >= position.x && p_point.x <= position.x + size.x &&
			p_point.y >= position.y && p_point.y <= position.y + size.y &&
			p_point.z >= position.z && p_point.z <= position.z + size.z &&
			p_point.w >= position.w && p_point.w <= position.w + size.w;
}

Vector4i Rect4i::get_support_point(const Vector4i &p_direction) const {
	Vector4i support = position;
	if (p_direction.x > 0) {
		support.x += size.x;
	}
	if (p_direction.y > 0) {
		support.y += size.y;
	}
	if (p_direction.z > 0) {
		support.z += size.z;
	}
	if (p_direction.w > 0) {
		support.w += size.w;
	}
	return support;
}

// Rect math functions.
Rect4i Rect4i::grow(const int32_t p_by) const {
	Rect4i new_rect = *this;
	new_rect.position -= Vector4i(p_by, p_by, p_by, p_by);
	new_rect.size += Vector4i(p_by, p_by, p_by, p_by) * 2.0f;
	return new_rect;
}

Rect4i Rect4i::intersection(const Rect4i &p_other) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f || p_other.size.x < 0.0f || p_other.size.y < 0.0f || p_other.size.z < 0.0f || p_other.size.w < 0.0f)) {
		ERR_PRINT("Rect4i size is negative, this is not supported. Use Rect4i.abs() to get a Rect4i with a positive size.");
	}
#endif
	Vector4i end = get_end();
	Vector4i other_end = p_other.get_end();
	Rect4i new_rect = *this;
	if (position.x > other_end.x || end.x < p_other.position.x ||
			position.y > other_end.y || end.y < p_other.position.y ||
			position.z > other_end.z || end.z < p_other.position.z ||
			position.w > other_end.w || end.w < p_other.position.w) {
		// No intersection.
		return Rect4i();
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

Rect4i Rect4i::merge(const Rect4i &p_other) const {
#ifdef MATH_CHECKS
	if (unlikely(size.x < 0.0f || size.y < 0.0f || size.z < 0.0f || size.w < 0.0f || p_other.size.x < 0.0f || p_other.size.y < 0.0f || p_other.size.z < 0.0f || p_other.size.w < 0.0f)) {
		ERR_PRINT("Rect4i size is negative, this is not supported. Use Rect4i.abs() to get a Rect4i with a positive size.");
	}
#endif
	Vector4i end = get_end();
	Vector4i other_end = p_other.get_end();
	Rect4i new_rect = *this;
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
bool Rect4i::encloses_exclusive(const Rect4i &p_other) const {
	Vector4i end = get_end();
	Vector4i other_end = p_other.get_end();
	return position.x < p_other.position.x && end.x > other_end.x &&
			position.y < p_other.position.y && end.y > other_end.y &&
			position.z < p_other.position.z && end.z > other_end.z &&
			position.w < p_other.position.w && end.w > other_end.w;
}

bool Rect4i::encloses_inclusive(const Rect4i &p_other) const {
	Vector4i end = get_end();
	Vector4i other_end = p_other.get_end();
	return position.x <= p_other.position.x && end.x >= other_end.x &&
			position.y <= p_other.position.y && end.y >= other_end.y &&
			position.z <= p_other.position.z && end.z >= other_end.z &&
			position.w <= p_other.position.w && end.w >= other_end.w;
}

bool Rect4i::intersects_exclusive(const Rect4i &p_other) const {
	Vector4i end = get_end();
	Vector4i other_end = p_other.get_end();
	return position.x < other_end.x && end.x > p_other.position.x &&
			position.y < other_end.y && end.y > p_other.position.y &&
			position.z < other_end.z && end.z > p_other.position.z &&
			position.w < other_end.w && end.w > p_other.position.w;
}

bool Rect4i::intersects_inclusive(const Rect4i &p_other) const {
	Vector4i end = get_end();
	Vector4i other_end = p_other.get_end();
	return position.x <= other_end.x && end.x >= p_other.position.x &&
			position.y <= other_end.y && end.y >= p_other.position.y &&
			position.z <= other_end.z && end.z >= p_other.position.z &&
			position.w <= other_end.w && end.w >= p_other.position.w;
}

// Operators.
bool Rect4i::operator==(const Rect4i &p_other) const {
	return position == p_other.position && size == p_other.size;
}

bool Rect4i::operator!=(const Rect4i &p_other) const {
	return position != p_other.position || size != p_other.size;
}

Rect4i::operator String() const {
	return "[P: " + String(position) + ", S: " + String(size) + "]";
}

static_assert(sizeof(Rect4i) == 8 * sizeof(int32_t));
