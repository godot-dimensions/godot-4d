#pragma once

#include "shape_4d.h"

class HeightMapShape4D : public Shape4D {
	GDCLASS(HeightMapShape4D, Shape4D);

	static constexpr int32_t MAX_SIZE = 1 << 20;

	PackedFloat64Array _height_data = { 0, 0, 0, 0, 0, 0, 0, 0 };
	Vector3i _size = Vector3i(2, 2, 2);

	inline Vector3 _get_start_offset() const {
		return Vector3(_size - Vector3i(1, 1, 1)) * 0.5;
	}

	inline int64_t _get_height_index_nocheck(const int32_t p_x, const int32_t p_z, const int32_t p_w) const {
		return p_x + (p_z * _size.x) + (p_w * _size.x * _size.y);
	}

protected:
	static void _bind_methods();

public:
	PackedFloat64Array get_height_data() const { return _height_data; }
	void set_height_data(const PackedFloat64Array &p_height_data);

	Vector3i get_size() const { return _size; }
	void set_size(const Vector3i &p_size);

	int32_t get_size_width() const { return _size.x; }
	void set_size_width(int32_t p_width);

	int32_t get_size_depth() const { return _size.y; }
	void set_size_depth(int32_t p_depth);

	int32_t get_size_thickness() const { return _size.z; }
	void set_size_thickness(int32_t p_thickness);

	int64_t get_height_index(const int32_t p_x, const int32_t p_z, const int32_t p_w) const;
	int64_t get_height_index_vec3i(const Vector3i &p_pos) const;

	double get_height_vec3(const Vector3 &p_pos) const;
	double get_height_vec4(const Vector4 &p_pos) const;
	double get_height_on_grid_vec3i(const Vector3i &p_grid_pos) const;
	double get_height_on_grid_vec4i(const Vector4i &p_grid_pos) const;

	virtual real_t get_hypervolume() const override;
	virtual real_t get_surface_volume() const override;
	virtual Rect4 get_rect_bounds(const Transform4D &p_to_target = Transform4D()) const override;

	virtual Vector4 get_nearest_point(const Vector4 &p_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const override;
	virtual bool has_point(const Vector4 &p_point) const override;

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;

	virtual Ref<WireMesh4D> to_wire_mesh() const override;
};
