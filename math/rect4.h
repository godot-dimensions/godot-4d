#pragma once

#include "../godot_4d_defines.h"

struct _NO_DISCARD_ Rect4 {
	Vector4 position = Vector4();
	Vector4 size = Vector4();

	// Property getters and setters.
	const Vector4 &get_position() const { return position; }
	void set_position(const Vector4 &p_pos) { position = p_pos; }
	const Vector4 &get_size() const { return size; }
	void set_size(const Vector4 &p_size) { size = p_size; }

	Vector4 get_center() const {
		return position + (size * 0.5f);
	}

	void set_center(const Vector4 &p_center) {
		position = p_center - (size * 0.5f);
	}

	Vector4 get_end() const {
		return position + size;
	}

	void set_end(const Vector4 &p_end) {
		size = p_end - position;
	}

	// Basic math functions.
	real_t get_hypervolume() const {
		return size.x * size.y * size.z * size.w;
	}

	bool has_hypervolume() const {
		return size.x > 0.0f && size.y > 0.0f && size.z > 0.0f && size.w > 0.0f;
	}

	real_t get_surface_volume() const {
		return 2.0f * (size.x * size.y * size.z + size.x * size.y * size.w + size.x * size.z * size.w + size.y * size.z * size.w);
	}

	bool has_surface_volume() const {
		return size.x > 0.0f || size.y > 0.0f || size.z > 0.0f || size.w > 0.0f;
	}

	Rect4 abs() const {
		return Rect4(position + size.minf(0.0f), size.abs());
	}

	Vector4 get_longest_axis() const;
	Vector4 get_longest_axis_direction() const;
	Vector4::Axis get_longest_axis_index() const;
	real_t get_longest_axis_size() const;

	Vector4 get_shortest_axis() const;
	Vector4 get_shortest_axis_direction() const;
	Vector4::Axis get_shortest_axis_index() const;
	real_t get_shortest_axis_size() const;

	// Point functions.
	Vector4 get_nearest_point(const Vector4 &p_point) const;
	Rect4 expand_to_point(const Vector4 &p_vector) const;
	void expand_self_to_point(const Vector4 &p_vector);
	bool has_point(const Vector4 &p_point) const;
	Vector4 get_support_point(const Vector4 &p_direction) const;

	// Rect math functions.
	Rect4 grow(const real_t p_by) const;
	Rect4 intersection(const Rect4 &p_other) const;
	Rect4 merge(const Rect4 &p_other) const;

	// Rect collision functions.
	real_t continuous_collision_depth(const Vector4 &p_relative_velocity, const Rect4 &p_obstacle, Vector4 *r_out_normal = nullptr) const;
	bool continuous_collision_overlaps(const Vector4 &p_relative_velocity, const Rect4 &p_obstacle) const;

	// Rect comparison functions.
	bool encloses_exclusive(const Rect4 &p_other) const;
	bool encloses_inclusive(const Rect4 &p_other) const;
	bool intersects_exclusive(const Rect4 &p_other) const;
	bool intersects_inclusive(const Rect4 &p_other) const;
	bool is_equal_approx(const Rect4 &p_other) const;
	bool is_finite() const;

	// Operators.
	bool operator==(const Rect4 &p_other) const;
	bool operator!=(const Rect4 &p_other) const;
	operator String() const;

	Rect4() {}
	Rect4(const Vector4 &p_pos, const Vector4 &p_size) :
			position(p_pos),
			size(p_size) {
	}
	Rect4(const real_t p_pos_x, const real_t p_pos_y, const real_t p_pos_z, const real_t p_pos_w, const real_t p_size_x, const real_t p_size_y, const real_t p_size_z, const real_t p_size_w) :
			position(p_pos_x, p_pos_y, p_pos_z, p_pos_w),
			size(p_size_x, p_size_y, p_size_z, p_size_w) {
	}
};

static_assert(sizeof(Rect4) == sizeof(real_t) * 8);
