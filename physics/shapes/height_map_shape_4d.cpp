#include "height_map_shape_4d.h"

#include "../../model/mesh/wire/array_wire_mesh_4d.h"

void HeightMapShape4D::set_height_data(const PackedFloat64Array &p_height_data) {
	const int64_t old_size = _height_data.size();
	const int64_t input_size = p_height_data.size();
	_height_data = p_height_data;
	if (unlikely(input_size != old_size)) {
		_height_data.resize(old_size);
		for (int64_t i = input_size; i < old_size; i++) {
			_height_data.set(i, 0.0);
		}
	}
}

void HeightMapShape4D::set_size(const Vector3i &p_size) {
	ERR_FAIL_COND_MSG(p_size.x < 1 || p_size.y < 1 || p_size.z < 1, "HeightMapShape4D: Size must be positive in all dimensions.");
	ERR_FAIL_COND_MSG(p_size.x > MAX_SIZE || p_size.y > MAX_SIZE || p_size.z > MAX_SIZE, "HeightMapShape4D: Size exceeds maximum size.");
	_size = p_size;
	const int64_t old_size = _height_data.size();
	const int64_t new_size = _size.x * _size.y * _size.z;
	_height_data.resize(new_size);
	for (int64_t i = old_size; i < new_size; i++) {
		_height_data.set(i, 0.0);
	}
}

void HeightMapShape4D::set_size_width(int32_t p_width) {
	ERR_FAIL_COND_MSG(p_width < 1, "HeightMapShape4D: Width must be positive.");
	set_size(Vector3i(p_width, _size.y, _size.z));
}

void HeightMapShape4D::set_size_depth(int32_t p_depth) {
	ERR_FAIL_COND_MSG(p_depth < 1, "HeightMapShape4D: Depth must be positive.");
	set_size(Vector3i(_size.x, p_depth, _size.z));
}

void HeightMapShape4D::set_size_thickness(int32_t p_thickness) {
	ERR_FAIL_COND_MSG(p_thickness < 1, "HeightMapShape4D: Thickness must be positive.");
	set_size(Vector3i(_size.x, _size.y, p_thickness));
}

int64_t HeightMapShape4D::get_height_index(const int32_t p_x, const int32_t p_z, const int32_t p_w) const {
	ERR_FAIL_INDEX_V(p_x, _size.x, -1);
	ERR_FAIL_INDEX_V(p_z, _size.y, -1);
	ERR_FAIL_INDEX_V(p_w, _size.z, -1);
	return _get_height_index_nocheck(p_x, p_z, p_w);
}

int64_t HeightMapShape4D::get_height_index_vec3i(const Vector3i &p_pos) const {
	ERR_FAIL_INDEX_V(p_pos.x, _size.x, -1);
	ERR_FAIL_INDEX_V(p_pos.y, _size.y, -1);
	ERR_FAIL_INDEX_V(p_pos.z, _size.z, -1);
	return _get_height_index_nocheck(p_pos.x, p_pos.y, p_pos.z);
}

double HeightMapShape4D::get_height_vec3(const Vector3 &p_pos) const {
	const Vector3 pos_rel_to_start = p_pos + _get_start_offset();
	const Vector3 pos_floored = pos_rel_to_start.floor();
	const Vector3i pos_floored_int = Vector3i(pos_floored);
	const Vector3 pos_frac = pos_rel_to_start - pos_floored;
	const Vector3 pos_frac_inv = Vector3(1.0f, 1.0f, 1.0f) - pos_frac;
	// Perform trilinear interpolation. First get the indices of the 6 axes.
	const int32_t x0 = CLAMP(pos_floored_int.x, int32_t(0), int32_t(_size.x - int32_t(1)));
	const int32_t x1 = CLAMP(pos_floored_int.x + 1, int32_t(0), int32_t(_size.x - int32_t(1)));
	const int32_t y0 = CLAMP(pos_floored_int.y, int32_t(0), int32_t(_size.y - int32_t(1)));
	const int32_t y1 = CLAMP(pos_floored_int.y + 1, int32_t(0), int32_t(_size.y - int32_t(1)));
	const int32_t z0 = CLAMP(pos_floored_int.z, int32_t(0), int32_t(_size.z - int32_t(1)));
	const int32_t z1 = CLAMP(pos_floored_int.z + 1, int32_t(0), int32_t(_size.z - int32_t(1)));
	// Next, get the height values of the 8 corners.
	const double height_x0y0z0 = _height_data[_get_height_index_nocheck(x0, y0, z0)];
	const double height_x1y0z0 = _height_data[_get_height_index_nocheck(x1, y0, z0)];
	const double height_x0y1z0 = _height_data[_get_height_index_nocheck(x0, y1, z0)];
	const double height_x1y1z0 = _height_data[_get_height_index_nocheck(x1, y1, z0)];
	const double height_x0y0z1 = _height_data[_get_height_index_nocheck(x0, y0, z1)];
	const double height_x1y0z1 = _height_data[_get_height_index_nocheck(x1, y0, z1)];
	const double height_x0y1z1 = _height_data[_get_height_index_nocheck(x0, y1, z1)];
	const double height_x1y1z1 = _height_data[_get_height_index_nocheck(x1, y1, z1)];
	// Now, we can perform trilinear interpolation. The order of axes does not matter in this calculation.
	/* clang-format off */
	return (
		pos_frac_inv.x * (
			pos_frac_inv.y * (
				pos_frac_inv.z * height_x0y0z0 +
				pos_frac.z * height_x0y0z1
			) +
			pos_frac.y * (
				pos_frac_inv.z * height_x0y1z0 +
				pos_frac.z * height_x0y1z1
			)
		) +
		pos_frac.x * (
			pos_frac_inv.y * (
				pos_frac_inv.z * height_x1y0z0 +
				pos_frac.z * height_x1y0z1
			) +
			pos_frac.y * (
				pos_frac_inv.z * height_x1y1z0 +
				pos_frac.z * height_x1y1z1
			)
		)
	);
	/* clang-format on */
}

double HeightMapShape4D::get_height_vec4(const Vector4 &p_pos) const {
	return get_height_vec3(Vector3(p_pos.x, p_pos.z, p_pos.w));
}

double HeightMapShape4D::get_height_on_grid_vec3i(const Vector3i &p_grid_pos) const {
	ERR_FAIL_INDEX_V(p_grid_pos.x, _size.x, 0.0f);
	ERR_FAIL_INDEX_V(p_grid_pos.y, _size.y, 0.0f);
	ERR_FAIL_INDEX_V(p_grid_pos.z, _size.z, 0.0f);
	const int64_t index = _get_height_index_nocheck(p_grid_pos.x, p_grid_pos.y, p_grid_pos.z);
	return _height_data[index];
}

double HeightMapShape4D::get_height_on_grid_vec4i(const Vector4i &p_grid_pos) const {
	ERR_FAIL_INDEX_V(p_grid_pos.x, _size.x, 0.0f);
	ERR_FAIL_INDEX_V(p_grid_pos.z, _size.y, 0.0f);
	ERR_FAIL_INDEX_V(p_grid_pos.w, _size.z, 0.0f);
	const int64_t index = _get_height_index_nocheck(p_grid_pos.x, p_grid_pos.z, p_grid_pos.w);
	return _height_data[index];
}

real_t HeightMapShape4D::get_hypervolume() const {
	// Since the heightmap does not have a bottom, just treat it as 1 meter thick.
	return _size.x * _size.y * _size.z;
}

real_t HeightMapShape4D::get_surface_volume() const {
	// Note: This does not account for the height data.
	return _size.x * _size.y * _size.z;
}

Rect4 HeightMapShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	const Vector3 start_flat_pos = -_get_start_offset();
	const Vector4 start_point = Vector4(start_flat_pos.x, _height_data[0], start_flat_pos.y, start_flat_pos.z);
	Rect4 rect_bounds = Rect4(p_to_target.xform(start_point), Vector4(0, 0, 0, 0));
	for (int32_t i = 0; i < _size.x; i++) {
		for (int32_t j = 0; j < _size.y; j++) {
			for (int32_t k = 0; k < _size.z; k++) {
				const Vector3 data_flat_pos = Vector3(i, j, k) + start_flat_pos;
				const Vector4 local_point = Vector4(data_flat_pos.x, _height_data[_get_height_index_nocheck(i, j, k)], data_flat_pos.y, data_flat_pos.z);
				rect_bounds.expand_self_to_point(p_to_target.xform(local_point));
			}
		}
	}
	return rect_bounds;
}

Vector4 HeightMapShape4D::get_nearest_point(const Vector4 &p_point) const {
	// TODO: This is not optimal, but it gives good results for mostly flat heightmaps.
	return Vector4(p_point.x, get_height_vec4(p_point), p_point.z, p_point.w);
}

Vector4 HeightMapShape4D::get_support_point(const Vector4 &p_direction) const {
	const Vector3 start_flat_pos = -_get_start_offset();
	Vector4 best_point = Vector4(0, 0, 0, 0);
	real_t best_dot = -1e20;
	for (int32_t i = 0; i < _size.x; i++) {
		for (int32_t j = 0; j < _size.y; j++) {
			for (int32_t k = 0; k < _size.z; k++) {
				const Vector3 data_flat_pos = Vector3(i, j, k) + start_flat_pos;
				const int64_t index = _get_height_index_nocheck(i, j, k);
				const Vector4 local_point = Vector4(data_flat_pos.x, _height_data[index], data_flat_pos.y, data_flat_pos.z);
				const real_t dot = p_direction.dot(local_point);
				if (dot > best_dot) {
					best_dot = dot;
					best_point = local_point;
				}
			}
		}
	}
	return best_point;
}

bool HeightMapShape4D::has_point(const Vector4 &p_point) const {
	const real_t height = get_height_vec4(p_point);
	return p_point.y <= height;
}

bool HeightMapShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<HeightMapShape4D> other = p_shape;
	if (other.is_null()) {
		return false;
	}
	if (_size != other->_size) {
		return false;
	}
	return _height_data == other->_height_data;
}

Ref<WireMesh4D> HeightMapShape4D::to_wire_mesh() const {
	const Vector3 start_flat_pos = -_get_start_offset();
	PackedVector4Array vertices;
	PackedInt32Array edge_indices;
	vertices.resize(_size.x * _size.y * _size.z);
	edge_indices.resize(2 * ((_size.x - 1) * _size.y * _size.z + (_size.y - 1) * _size.x * _size.z + (_size.z - 1) * _size.x * _size.y));
	int64_t edge_iter = 0;
	for (int32_t i = 0; i < _size.x; i++) {
		for (int32_t j = 0; j < _size.y; j++) {
			for (int32_t k = 0; k < _size.z; k++) {
				const Vector3 data_flat_pos = Vector3(i, j, k) + start_flat_pos;
				const int64_t index = _get_height_index_nocheck(i, j, k);
				const Vector4 local_point = Vector4(data_flat_pos.x, _height_data[index], data_flat_pos.y, data_flat_pos.z);
				vertices.set(index, local_point);
				if (i > 0) {
					edge_indices.set(edge_iter++, _get_height_index_nocheck(i - 1, j, k));
					edge_indices.set(edge_iter++, index);
				}
				if (j > 0) {
					edge_indices.set(edge_iter++, _get_height_index_nocheck(i, j - 1, k));
					edge_indices.set(edge_iter++, index);
				}
				if (k > 0) {
					edge_indices.set(edge_iter++, _get_height_index_nocheck(i, j, k - 1));
					edge_indices.set(edge_iter++, index);
				}
			}
		}
	}
	CRASH_COND(edge_iter != edge_indices.size());
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertices(vertices);
	wire_mesh->set_edge_indices(edge_indices);
	return wire_mesh;
}

void HeightMapShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_height_data", "height_data"), &HeightMapShape4D::set_height_data);
	ClassDB::bind_method(D_METHOD("get_height_data"), &HeightMapShape4D::get_height_data);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &HeightMapShape4D::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &HeightMapShape4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size_width", "width"), &HeightMapShape4D::set_size_width);
	ClassDB::bind_method(D_METHOD("get_size_width"), &HeightMapShape4D::get_size_width);
	ClassDB::bind_method(D_METHOD("set_size_depth", "depth"), &HeightMapShape4D::set_size_depth);
	ClassDB::bind_method(D_METHOD("get_size_depth"), &HeightMapShape4D::get_size_depth);
	ClassDB::bind_method(D_METHOD("set_size_thickness", "thickness"), &HeightMapShape4D::set_size_thickness);
	ClassDB::bind_method(D_METHOD("get_size_thickness"), &HeightMapShape4D::get_size_thickness);

	ClassDB::bind_method(D_METHOD("get_height_index", "x", "z", "w"), &HeightMapShape4D::_get_height_index_nocheck);
	ClassDB::bind_method(D_METHOD("get_height_index_vec3i", "vector"), &HeightMapShape4D::get_height_index_vec3i);
	ClassDB::bind_method(D_METHOD("get_height_vec3", "pos"), &HeightMapShape4D::get_height_vec3);
	ClassDB::bind_method(D_METHOD("get_height_vec4", "pos"), &HeightMapShape4D::get_height_vec4);
	ClassDB::bind_method(D_METHOD("get_height_on_grid_vec3i", "grid_pos"), &HeightMapShape4D::get_height_on_grid_vec3i);
	ClassDB::bind_method(D_METHOD("get_height_on_grid_vec4i", "grid_pos"), &HeightMapShape4D::get_height_on_grid_vec4i);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "height_data"), "set_height_data", "get_height_data");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "size", PROPERTY_HINT_RANGE, "1,1000,or_greater", PROPERTY_USAGE_STORAGE), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size_width", PROPERTY_HINT_RANGE, "1,1000,or_greater", PROPERTY_USAGE_EDITOR), "set_size_width", "get_size_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size_depth", PROPERTY_HINT_RANGE, "1,1000,or_greater", PROPERTY_USAGE_EDITOR), "set_size_depth", "get_size_depth");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size_thickness", PROPERTY_HINT_RANGE, "1,1000,or_greater", PROPERTY_USAGE_EDITOR), "set_size_thickness", "get_size_thickness");
}
