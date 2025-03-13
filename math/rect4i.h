#pragma once

#include "../godot_4d_defines.h"

struct _NO_DISCARD_ Rect4i {
	Vector4i position;
	Vector4i size;

	// Property getters and setters.
	const Vector4i &get_position() const { return position; }
	void set_position(const Vector4i &p_pos) { position = p_pos; }
	const Vector4i &get_size() const { return size; }
	void set_size(const Vector4i &p_size) { size = p_size; }

	Vector4i get_end() const {
		return position + size;
	}

	void set_end(const Vector4i &p_end) {
		size = p_end - position;
	}

	// Basic math functions.
	int64_t get_hypervolume() const {
		return size.x * (int64_t)size.y * size.z * size.w;
	}

	bool has_hypervolume() const {
		return size.x > 0 && size.y > 0 && size.z > 0 && size.w > 0;
	}

	int64_t get_surface_volume() const {
		return 2ll * (size.x * (int64_t)size.y * size.z + size.x * (int64_t)size.y * size.w + size.x * (int64_t)size.z * size.w + size.y * (int64_t)size.z * size.w);
	}

	bool has_surface_volume() const {
		return size.x > 0 || size.y > 0 || size.z > 0 || size.w > 0;
	}

	Rect4i abs() const {
		return Rect4i(position + size.mini(0), size.abs());
	}

	Vector4i get_longest_axis() const;
	Vector4i get_longest_axis_direction() const;
	Vector4i::Axis get_longest_axis_index() const;
	int32_t get_longest_axis_size() const;

	Vector4i get_shortest_axis() const;
	Vector4i get_shortest_axis_direction() const;
	Vector4i::Axis get_shortest_axis_index() const;
	int32_t get_shortest_axis_size() const;

	// Point functions.
	Vector4i get_nearest_point(const Vector4i &p_point) const;
	Rect4i expand_to_point(const Vector4i &p_vector) const;
	bool has_point(const Vector4i &p_point) const;
	Vector4i get_support_point(const Vector4i &p_direction) const;

	// Rect math functions.
	Rect4i grow(const int p_by) const;
	Rect4i intersection(const Rect4i &p_other) const;
	Rect4i merge(const Rect4i &p_other) const;

	// Rect comparison functions.
	bool encloses_exclusive(const Rect4i &p_other) const;
	bool encloses_inclusive(const Rect4i &p_other) const;
	bool intersects_exclusive(const Rect4i &p_other) const;
	bool intersects_inclusive(const Rect4i &p_other) const;

	// Operators.
	bool operator==(const Rect4i &p_other) const;
	bool operator!=(const Rect4i &p_other) const;
	operator String() const;

	Rect4i() {}
	Rect4i(const Vector4i &p_pos, const Vector4i &p_size) :
			position(p_pos),
			size(p_size) {
	}
	Rect4i(const int p_pos_x, const int p_pos_y, const int p_pos_z, const int p_pos_w, const int p_size_x, const int p_size_y, const int p_size_z, const int p_size_w) :
			position(p_pos_x, p_pos_y, p_pos_z, p_pos_w),
			size(p_size_x, p_size_y, p_size_z, p_size_w) {
	}
};

static_assert(sizeof(Rect4i) == sizeof(int32_t) * 8);
