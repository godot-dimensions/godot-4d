#include "height_map_shape_4d.h"

#include "../../math/math_4d.h"
#include "../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../model/mesh/wire/array_wire_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_map.hpp>
#elif GODOT_MODULE
#include "core/templates/hash_map.h"
#endif

void HeightMapShape4D::set_height_data(const PackedFloat64Array &p_height_data) {
	const int64_t old_size = _height_data.size();
	const int64_t input_size = p_height_data.size();
	_height_data = p_height_data;
	// Force the data to have the same length as the product of the size dimensions.
	// Users are expected to use `set_size` before setting height data.
	if (unlikely(input_size != old_size)) {
		WARN_PRINT("HeightMapShape4D: The given height data has a different array length than the product of the grid size dimensions. Set the size of the height map using set_size() before setting height data to ensure correct behavior.");
		_height_data.resize_initialized(old_size);
		for (int64_t i = input_size; i < old_size; i++) {
			_height_data.set(i, 0.0);
		}
	}
}

void HeightMapShape4D::set_grid_spacing(const Vector3 &p_grid_spacing) {
	ERR_FAIL_COND_MSG(p_grid_spacing.x <= 0.0 || p_grid_spacing.y <= 0.0 || p_grid_spacing.z <= 0.0, "HeightMapShape4D: Spacing must be positive in all dimensions.");
	_grid_spacing = p_grid_spacing;
}

void HeightMapShape4D::set_grid_size(const Vector3i &p_grid_size) {
	ERR_FAIL_COND_MSG(p_grid_size.x < 1 || p_grid_size.y < 1 || p_grid_size.z < 1, "HeightMapShape4D: Size must be positive in all dimensions.");
	ERR_FAIL_COND_MSG(p_grid_size.x > MAX_SIZE || p_grid_size.y > MAX_SIZE || p_grid_size.z > MAX_SIZE, "HeightMapShape4D: Size exceeds maximum size.");
	const int64_t old_size = _height_data.size();
	const int64_t new_size = p_grid_size.x * p_grid_size.y * p_grid_size.z;
	_grid_size = p_grid_size;
	// Resize the height data to match the new size, preserving existing data where possible.
	_height_data.resize_initialized(new_size);
	for (int64_t i = old_size; i < new_size; i++) {
		_height_data.set(i, 0.0);
	}
}

void HeightMapShape4D::set_grid_size_width(int32_t p_width) {
	ERR_FAIL_COND_MSG(p_width < 1, "HeightMapShape4D: Width must be positive.");
	set_grid_size(Vector3i(p_width, _grid_size.y, _grid_size.z));
}

void HeightMapShape4D::set_grid_size_depth(int32_t p_depth) {
	ERR_FAIL_COND_MSG(p_depth < 1, "HeightMapShape4D: Depth must be positive.");
	set_grid_size(Vector3i(_grid_size.x, p_depth, _grid_size.z));
}

void HeightMapShape4D::set_grid_size_thickness(int32_t p_thickness) {
	ERR_FAIL_COND_MSG(p_thickness < 1, "HeightMapShape4D: Thickness must be positive.");
	set_grid_size(Vector3i(_grid_size.x, _grid_size.y, p_thickness));
}

int64_t HeightMapShape4D::get_height_index(const int32_t p_x, const int32_t p_z, const int32_t p_w) const {
	ERR_FAIL_INDEX_V(p_x, _grid_size.x, -1);
	ERR_FAIL_INDEX_V(p_z, _grid_size.y, -1);
	ERR_FAIL_INDEX_V(p_w, _grid_size.z, -1);
	return _get_height_index_nocheck(p_x, p_z, p_w);
}

int64_t HeightMapShape4D::get_height_index_vec3i(const Vector3i &p_pos) const {
	ERR_FAIL_INDEX_V(p_pos.x, _grid_size.x, -1);
	ERR_FAIL_INDEX_V(p_pos.y, _grid_size.y, -1);
	ERR_FAIL_INDEX_V(p_pos.z, _grid_size.z, -1);
	return _get_height_index_nocheck(p_pos.x, p_pos.y, p_pos.z);
}

double HeightMapShape4D::get_height_vec3(const Vector3 &p_physical_pos) const {
	const Vector3 grid_pos_rel_to_start = (p_physical_pos - _get_start_physical_offset()) / _grid_spacing;
	const Vector3 pos_floored_float = grid_pos_rel_to_start.floor();
	const Vector3i pos_floored_int = Vector3i(pos_floored_float);
	const Vector3 pos_frac = grid_pos_rel_to_start - pos_floored_float;
	const Vector3 pos_frac_inv = Vector3(1.0f, 1.0f, 1.0f) - pos_frac;
	// Perform trilinear interpolation. First get the indices of the 6 bounds on the 3 ground axes.
	const int32_t x0 = CLAMP(pos_floored_int.x, int32_t(0), int32_t(_grid_size.x - int32_t(1)));
	const int32_t x1 = CLAMP(pos_floored_int.x + 1, int32_t(0), int32_t(_grid_size.x - int32_t(1)));
	const int32_t y0 = CLAMP(pos_floored_int.y, int32_t(0), int32_t(_grid_size.y - int32_t(1)));
	const int32_t y1 = CLAMP(pos_floored_int.y + 1, int32_t(0), int32_t(_grid_size.y - int32_t(1)));
	const int32_t z0 = CLAMP(pos_floored_int.z, int32_t(0), int32_t(_grid_size.z - int32_t(1)));
	const int32_t z1 = CLAMP(pos_floored_int.z + 1, int32_t(0), int32_t(_grid_size.z - int32_t(1)));
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

double HeightMapShape4D::get_height_vec4(const Vector4 &p_physical_pos) const {
	return get_height_vec3(Vector3(p_physical_pos.x, p_physical_pos.z, p_physical_pos.w));
}

double HeightMapShape4D::get_height_on_grid_vec3i(const Vector3i &p_grid_pos) const {
	ERR_FAIL_INDEX_V(p_grid_pos.x, _grid_size.x, 0.0f);
	ERR_FAIL_INDEX_V(p_grid_pos.y, _grid_size.y, 0.0f);
	ERR_FAIL_INDEX_V(p_grid_pos.z, _grid_size.z, 0.0f);
	const int64_t index = _get_height_index_nocheck(p_grid_pos.x, p_grid_pos.y, p_grid_pos.z);
	return _height_data[index];
}

double HeightMapShape4D::get_height_on_grid_vec4i(const Vector4i &p_grid_pos) const {
	ERR_FAIL_INDEX_V(p_grid_pos.x, _grid_size.x, 0.0f);
	ERR_FAIL_INDEX_V(p_grid_pos.z, _grid_size.y, 0.0f);
	ERR_FAIL_INDEX_V(p_grid_pos.w, _grid_size.z, 0.0f);
	const int64_t index = _get_height_index_nocheck(p_grid_pos.x, p_grid_pos.z, p_grid_pos.w);
	return _height_data[index];
}

void HeightMapShape4D::quantize_to_float8() {
	for (int64_t i = 0; i < _height_data.size(); i++) {
		_height_data.set(i, Math4D::float8_to_double(Math4D::double_to_float8(_height_data[i])));
	}
	_grid_spacing.x = Math4D::float8_to_double(Math4D::double_to_float8(_grid_spacing.x));
	_grid_spacing.y = Math4D::float8_to_double(Math4D::double_to_float8(_grid_spacing.y));
	_grid_spacing.z = Math4D::float8_to_double(Math4D::double_to_float8(_grid_spacing.z));
}

void HeightMapShape4D::quantize_to_float16() {
	for (int64_t i = 0; i < _height_data.size(); i++) {
		_height_data.set(i, Math4D::float16_to_double(Math4D::double_to_float16(_height_data[i])));
	}
	_grid_spacing.x = Math4D::float16_to_double(Math4D::double_to_float16(_grid_spacing.x));
	_grid_spacing.y = Math4D::float16_to_double(Math4D::double_to_float16(_grid_spacing.y));
	_grid_spacing.z = Math4D::float16_to_double(Math4D::double_to_float16(_grid_spacing.z));
}

void HeightMapShape4D::quantize_to_float32() {
	for (int64_t i = 0; i < _height_data.size(); i++) {
		_height_data.set(i, (float)_height_data[i]);
	}
	_grid_spacing.x = (float)_grid_spacing.x;
	_grid_spacing.y = (float)_grid_spacing.y;
	_grid_spacing.z = (float)_grid_spacing.z;
}

void HeightMapShape4D::fill_from_mesh_3d(const Ref<Mesh> &p_mesh_3d, const double p_exponent, const real_t p_max_height, const real_t p_min_height) {
	ERR_FAIL_COND(p_mesh_3d.is_null());
#if GDEXTENSION
	ERR_FAIL_MSG("HeightMapShape4D::fill_from_mesh_3d is not available in GDExtension builds of Godot 4D.");
#elif GODOT_MODULE
	const Vector<Face3> faces = p_mesh_3d->get_faces();
	const Vector3 start_flat_physical_pos = _get_start_physical_offset();
	// Iterate through all grid points.
	for (int32_t i = 0; i < _grid_size.x; i++) {
		for (int32_t j = 0; j < _grid_size.y; j++) {
			for (int32_t k = 0; k < _grid_size.z; k++) {
				const Vector3 data_flat_physical_pos = Vector3(i, j, k) * _grid_spacing + start_flat_physical_pos;
				Face3 closest_face = Face3();
				double min_distance_squared = Math_INF;
				for (const Face3 &face : faces) {
					const Vector3 point_on_face = face.get_closest_point_to(data_flat_physical_pos);
					const double distance_squared = point_on_face.distance_squared_to(data_flat_physical_pos);
					if (distance_squared < min_distance_squared) {
						min_distance_squared = distance_squared;
						closest_face = face;
					}
				}
				// The 0.5 factor applies the sqrt to give us the distance rather than distance squared.
				double height = Math::pow(min_distance_squared, p_exponent * 0.5);
				const bool over = closest_face.get_plane().is_point_over(data_flat_physical_pos);
				if (!over) {
					height = -height;
				}
				height = CLAMP(height, p_min_height, p_max_height);
				_height_data.set(_get_height_index_nocheck(i, j, k), height);
			}
		}
	}
#endif
}

void HeightMapShape4D::gaussian_blur(const Vector3 &p_blur_radius) {
	// p_blur_radius is also known as "sigma" in the context of Gaussian blur.
	ERR_FAIL_COND_MSG(p_blur_radius.x < 0.0, "Gaussian blur X radius (Vector3 X component) must not be negative.");
	ERR_FAIL_COND_MSG(p_blur_radius.y < 0.0, "Gaussian blur Z radius (Vector3 Y component) must not be negative.");
	ERR_FAIL_COND_MSG(p_blur_radius.z < 0.0, "Gaussian blur W radius (Vector3 Z component) must not be negative.");
	PackedFloat64Array new_height_data;
	HashMap<real_t, PackedFloat64Array> kernel_cache;
	// Gaussian blur is an inherently separable operation. A 3D Gaussian blur is the same as three 1D Gaussian blurs along each axis.
	for (uint8_t axis = 0; axis < 3; axis++) {
		// Generate the Gaussian kernel for this axis, caching it for future iterations if the same blur radius is used on another axis.
		const real_t axis_blur_radius = p_blur_radius[axis];
		if (axis_blur_radius == 0.0) {
			continue;
		}
		PackedFloat64Array kernel;
		if (kernel_cache.has(axis_blur_radius)) {
			kernel = kernel_cache[axis_blur_radius];
		} else {
			// 3 sigma captures ~99.7% of the Gaussian curve. This is the usual practical
			// cutoff for converting the infinite Gaussian into a finite sampled kernel.
			constexpr double GAUSSIAN_CUTOFF_MULTIPLIER = 3.0;
			const int32_t cutoff_radius = (int32_t)Math::ceil(axis_blur_radius * GAUSSIAN_CUTOFF_MULTIPLIER);
			const int32_t kernel_size = cutoff_radius * 2 + 1;
			kernel.resize(kernel_size);
			const double gaussian_denominator = 2.0 * axis_blur_radius * axis_blur_radius;
			double total = 0.0;
			for (int32_t i = -cutoff_radius; i <= cutoff_radius; i++) {
				const double weight = Math::exp(-(double)(i * i) / gaussian_denominator);
				kernel.set(i + cutoff_radius, weight);
				total += weight;
			}
			for (int32_t i = 0; i < kernel_size; i++) {
				kernel.set(i, kernel[i] / total);
			}
			kernel_cache[axis_blur_radius] = kernel;
		}
		const int32_t kernel_radius = kernel.size() / 2;
		new_height_data.resize(_height_data.size());
		// Iterate over every point in the height map.
		for (int32_t w = 0; w < _grid_size.z; w++) {
			for (int32_t z = 0; z < _grid_size.y; z++) {
				for (int32_t x = 0; x < _grid_size.x; x++) {
					// Perform a 1D Gaussian blur along the current axis.
					double blurred_value = 0.0;
					for (int32_t kernel_index = 0; kernel_index < kernel.size(); kernel_index++) {
						const int32_t offset = kernel_index - kernel_radius;
						int32_t sample_x = x;
						int32_t sample_z = z;
						int32_t sample_w = w;
						if (axis == 0) {
							sample_x = CLAMP(x + offset, 0, _grid_size.x - 1);
						} else if (axis == 1) {
							sample_z = CLAMP(z + offset, 0, _grid_size.y - 1);
						} else {
							sample_w = CLAMP(w + offset, 0, _grid_size.z - 1);
						}
						const int64_t source_index = _get_height_index_nocheck(sample_x, sample_z, sample_w);
						blurred_value += _height_data[source_index] * kernel[kernel_index];
					}
					const int64_t destination_index = _get_height_index_nocheck(x, z, w);
					new_height_data.set(destination_index, blurred_value);
				}
			}
		}
		// Each iteration uses the previously blurred data as the source for the next axis blur,
		// and at the end we want to write back into the height data, so this does both at once.
		_height_data = new_height_data;
	}
}

real_t HeightMapShape4D::get_hypervolume() const {
	// Since the heightmap does not have a bottom, just treat it as 1 meter thick.
	return _grid_size.x * _grid_size.y * _grid_size.z * _grid_spacing.x * _grid_spacing.y * _grid_spacing.z;
}

real_t HeightMapShape4D::get_surface_volume() const {
	// Note: This does not account for the height data.
	return _grid_size.x * _grid_size.y * _grid_size.z * _grid_spacing.x * _grid_spacing.y * _grid_spacing.z;
}

Rect4 HeightMapShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	const Vector3 start_flat_physical_pos = _get_start_physical_offset();
	const Vector4 start_physical_point = Vector4(start_flat_physical_pos.x, _height_data[0], start_flat_physical_pos.y, start_flat_physical_pos.z);
	Rect4 rect_bounds = Rect4(p_to_target.xform(start_physical_point), Vector4(0, 0, 0, 0));
	for (int32_t i = 0; i < _grid_size.x; i++) {
		for (int32_t j = 0; j < _grid_size.y; j++) {
			for (int32_t k = 0; k < _grid_size.z; k++) {
				const Vector3 data_flat_physical_pos = Vector3(i, j, k) * _grid_spacing + start_flat_physical_pos;
				const Vector4 local_point = Vector4(data_flat_physical_pos.x, _height_data[_get_height_index_nocheck(i, j, k)], data_flat_physical_pos.y, data_flat_physical_pos.z);
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
	const Vector3 start_flat_physical_pos = _get_start_physical_offset();
	Vector4 best_point = Vector4(0, 0, 0, 0);
	real_t best_dot = -1e20;
	for (int32_t i = 0; i < _grid_size.x; i++) {
		for (int32_t j = 0; j < _grid_size.y; j++) {
			for (int32_t k = 0; k < _grid_size.z; k++) {
				const Vector3 data_flat_physical_pos = Vector3(i, j, k) * _grid_spacing + start_flat_physical_pos;
				const int64_t index = _get_height_index_nocheck(i, j, k);
				const Vector4 local_point = Vector4(data_flat_physical_pos.x, _height_data[index], data_flat_physical_pos.y, data_flat_physical_pos.z);
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
	if (!Math::is_finite(height)) {
		// If the height is not finite, treat it as if the point is not contained.
		return false;
	}
	return p_point.y <= height;
}

bool HeightMapShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<HeightMapShape4D> other = p_shape;
	if (other.is_null()) {
		return false;
	}
	if (_grid_spacing != other->_grid_spacing) {
		return false;
	}
	if (_grid_size != other->_grid_size) {
		return false;
	}
	return _height_data == other->_height_data;
}

Ref<TetraMesh4D> HeightMapShape4D::to_tetra_mesh() const {
	const Vector3 start_flat_physical_pos = _get_start_physical_offset();
	PackedVector4Array vertices;
	PackedVector4Array vert_normals;
	PackedInt32Array simplex_cell_indices;
	// This is the maximum possible size, but the actual size may be smaller if the heightmap has any holes (non-finite height values).
	const int64_t grid_cell_max_count = (_grid_size.x - 1) * (_grid_size.y - 1) * (_grid_size.z - 1);
	// 5 tetrahedra per cube, 4 vertex indices per tetrahedron.
	simplex_cell_indices.resize(4 * 5 * grid_cell_max_count);
	// However, the vertices are always the same size as the height data, matching 1:1 for easy addressability.
	const int64_t vertex_count = _grid_size.x * _grid_size.y * _grid_size.z;
	vertices.resize(vertex_count);
	vert_normals.resize(vertex_count);
	int64_t simplex_cell_iter = 0;
	for (int32_t i = 0; i < _grid_size.x; i++) {
		for (int32_t j = 0; j < _grid_size.y; j++) {
			for (int32_t k = 0; k < _grid_size.z; k++) {
				const Vector3 data_flat_physical_pos = Vector3(i, j, k) * _grid_spacing + start_flat_physical_pos;
				const int64_t index = _get_height_index_nocheck(i, j, k);
				const Vector4 local_point = Vector4(data_flat_physical_pos.x, _height_data[index], data_flat_physical_pos.y, data_flat_physical_pos.z);
				vertices.set(index, local_point);
				if (i > 0 && j > 0 && k > 0) {
					// The corners of the cube use the same vertex indices as the height data indices.
					int32_t corner_x0y0z0 = _get_height_index_nocheck(i - 1, j - 1, k - 1);
					int32_t corner_x1y0z0 = _get_height_index_nocheck(i, j - 1, k - 1);
					int32_t corner_x0y1z0 = _get_height_index_nocheck(i - 1, j, k - 1);
					int32_t corner_x1y1z0 = _get_height_index_nocheck(i, j, k - 1);
					int32_t corner_x0y0z1 = _get_height_index_nocheck(i - 1, j - 1, k);
					int32_t corner_x1y0z1 = _get_height_index_nocheck(i, j - 1, k);
					int32_t corner_x0y1z1 = _get_height_index_nocheck(i - 1, j, k);
					int32_t corner_x1y1z1 = index;
					/* clang-format off */
					// If any of the corners have non-finite height data, skip this cube.
					if (!Math::is_finite(_height_data[corner_x0y0z0])
							|| !Math::is_finite(_height_data[corner_x1y0z0])
							|| !Math::is_finite(_height_data[corner_x0y1z0])
							|| !Math::is_finite(_height_data[corner_x1y1z0])
							|| !Math::is_finite(_height_data[corner_x0y0z1])
							|| !Math::is_finite(_height_data[corner_x1y0z1])
							|| !Math::is_finite(_height_data[corner_x0y1z1])
							|| !Math::is_finite(_height_data[corner_x1y1z1])) {
						continue;
					}
					// Add a normal for this cube to the vertex normals array (must be done before swapping the corners).
					Vector3 changes = Vector3(
						+ (_height_data[corner_x0y0z0] + _height_data[corner_x0y1z0] + _height_data[corner_x0y0z1] + _height_data[corner_x0y1z1])
						- (_height_data[corner_x1y0z0] + _height_data[corner_x1y1z0] + _height_data[corner_x1y0z1] + _height_data[corner_x1y1z1]),
						+ (_height_data[corner_x0y0z0] + _height_data[corner_x1y0z0] + _height_data[corner_x0y0z1] + _height_data[corner_x1y0z1])
						- (_height_data[corner_x0y1z0] + _height_data[corner_x1y1z0] + _height_data[corner_x0y1z1] + _height_data[corner_x1y1z1]),
						+ (_height_data[corner_x0y0z0] + _height_data[corner_x0y1z0] + _height_data[corner_x1y0z0] + _height_data[corner_x1y1z0])
						- (_height_data[corner_x0y0z1] + _height_data[corner_x0y1z1] + _height_data[corner_x1y0z1] + _height_data[corner_x1y1z1])
					);
					// Average the changes from the 4 corners on each side (0.25x), then rise over run.
					changes *= 0.25 * _grid_spacing.inverse();
					const Vector4 grid_cell_normal = Vector4(changes.x, 1.0, changes.y, changes.z).normalized();
					vert_normals.set(corner_x0y0z0, vert_normals[corner_x0y0z0] + grid_cell_normal);
					vert_normals.set(corner_x1y0z0, vert_normals[corner_x1y0z0] + grid_cell_normal);
					vert_normals.set(corner_x0y1z0, vert_normals[corner_x0y1z0] + grid_cell_normal);
					vert_normals.set(corner_x1y1z0, vert_normals[corner_x1y1z0] + grid_cell_normal);
					vert_normals.set(corner_x0y0z1, vert_normals[corner_x0y0z1] + grid_cell_normal);
					vert_normals.set(corner_x1y0z1, vert_normals[corner_x1y0z1] + grid_cell_normal);
					vert_normals.set(corner_x0y1z1, vert_normals[corner_x0y1z1] + grid_cell_normal);
					vert_normals.set(corner_x1y1z1, vert_normals[corner_x1y1z1] + grid_cell_normal);
					// Swap these around to ensure that neighboring cubes have aligned triangulations.
					if (i % 2 == 0) {
						SWAP(corner_x0y0z0, corner_x1y0z0);
						SWAP(corner_x0y1z0, corner_x1y1z0);
						SWAP(corner_x0y0z1, corner_x1y0z1);
						SWAP(corner_x0y1z1, corner_x1y1z1);
					}
					if (j % 2 == 0) {
						SWAP(corner_x0y0z0, corner_x0y1z0);
						SWAP(corner_x1y0z0, corner_x1y1z0);
						SWAP(corner_x0y0z1, corner_x0y1z1);
						SWAP(corner_x1y0z1, corner_x1y1z1);
					}
					if (k % 2 == 0) {
						SWAP(corner_x0y0z0, corner_x0y0z1);
						SWAP(corner_x1y0z0, corner_x1y0z1);
						SWAP(corner_x0y1z0, corner_x0y1z1);
						SWAP(corner_x1y1z0, corner_x1y1z1);
					}
					// Create 5 tetrahedra for this cube.
					int32_t tet_corners[20] = {
						corner_x0y1z1, corner_x1y1z0, corner_x1y0z1, corner_x0y0z0,
						corner_x0y1z1, corner_x0y0z1, corner_x0y0z0, corner_x1y0z1,
						corner_x0y1z1, corner_x0y1z0, corner_x1y1z0, corner_x0y0z0,
						corner_x0y1z1, corner_x1y1z1, corner_x1y0z1, corner_x1y1z0,
						corner_x1y0z0, corner_x1y1z0, corner_x0y0z0, corner_x1y0z1,
					};
					/* clang-format on */
					if ((i + j + k) % 2 == 0) {
						// Swap the last two corners of each tetrahedron to flip its orientation.
						SWAP(tet_corners[2], tet_corners[3]);
						SWAP(tet_corners[6], tet_corners[7]);
						SWAP(tet_corners[10], tet_corners[11]);
						SWAP(tet_corners[14], tet_corners[15]);
						SWAP(tet_corners[18], tet_corners[19]);
					}
					for (uint8_t t = 0; t < 20; t++) {
						simplex_cell_indices.set(simplex_cell_iter++, tet_corners[t]);
					}
				}
			}
		}
	}
	simplex_cell_indices.resize(simplex_cell_iter);
	// Fill a texture map using the horizontal coordinates of the vertices.
	PackedVector3Array simplex_cell_texture_map;
	simplex_cell_texture_map.resize(simplex_cell_iter);
	const Vector3 vert_min_vec3 = Vector3(vertices[0].x, vertices[0].z, vertices[0].w);
	const Vector3 vert_max_vec3 = Vector3(vertices[vertices.size() - 1].x, vertices[vertices.size() - 1].z, vertices[vertices.size() - 1].w);
	const Vector3 vert_size_vec3 = vert_max_vec3 - vert_min_vec3;
	for (int64_t i = 0; i < simplex_cell_iter; i++) {
		const int32_t simplex_vert = simplex_cell_indices[i];
		const Vector4 vertex_vec4 = vertices[simplex_vert];
		const Vector3 vertex_vec3 = Vector3(vertex_vec4.x, vertex_vec4.z, vertex_vec4.w);
		simplex_cell_texture_map.set(i, (vertex_vec3 - vert_min_vec3) / vert_size_vec3);
	}
	// Normalize the normals and sample them for the tetrahedra.
	for (int64_t i = 0; i < vert_normals.size(); i++) {
		vert_normals.set(i, vert_normals[i].normalized());
	}
	PackedVector4Array simplex_cell_vertex_normals;
	simplex_cell_vertex_normals.resize(simplex_cell_iter);
	for (int64_t i = 0; i < simplex_cell_iter; i++) {
		simplex_cell_vertex_normals.set(i, vert_normals[simplex_cell_indices[i]]);
	}
	// Insert the final data into a new mesh.
	Ref<ArrayTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_vertices(vertices);
	tetra_mesh->set_simplex_cell_indices(simplex_cell_indices);
	tetra_mesh->set_simplex_cell_texture_map(simplex_cell_texture_map);
	tetra_mesh->set_simplex_cell_vertex_normals(simplex_cell_vertex_normals);
	CRASH_COND(!tetra_mesh->is_mesh_data_valid());
	return tetra_mesh;
}

Ref<WireMesh4D> HeightMapShape4D::to_wire_mesh() const {
	const Vector3 start_flat_physical_pos = _get_start_physical_offset();
	PackedVector4Array vertices;
	PackedInt32Array edge_indices;
	// This is the maximum possible size, but the actual size may be smaller if the heightmap has any holes (non-finite height values).
	edge_indices.resize(2 * ((_grid_size.x - 1) * _grid_size.y * _grid_size.z + (_grid_size.y - 1) * _grid_size.x * _grid_size.z + (_grid_size.z - 1) * _grid_size.x * _grid_size.y));
	// However, the vertices are always the same size as the height data, matching 1:1 for easy addressability.
	vertices.resize(_grid_size.x * _grid_size.y * _grid_size.z);
	int64_t edge_iter = 0;
	for (int32_t i = 0; i < _grid_size.x; i++) {
		for (int32_t j = 0; j < _grid_size.y; j++) {
			for (int32_t k = 0; k < _grid_size.z; k++) {
				const Vector3 data_flat_physical_pos = Vector3(i, j, k) * _grid_spacing + start_flat_physical_pos;
				const int64_t index = _get_height_index_nocheck(i, j, k);
				const Vector4 local_point = Vector4(data_flat_physical_pos.x, _height_data[index], data_flat_physical_pos.y, data_flat_physical_pos.z);
				vertices.set(index, local_point);
				// If the height data is not finite, skip creating edges for this vertex.
				if (!Math::is_finite(_height_data[index])) {
					continue;
				}
				if (i > 0) {
					const int64_t other_index = _get_height_index_nocheck(i - 1, j, k);
					if (Math::is_finite(_height_data[other_index])) {
						edge_indices.set(edge_iter++, other_index);
						edge_indices.set(edge_iter++, index);
					}
				}
				if (j > 0) {
					const int64_t other_index = _get_height_index_nocheck(i, j - 1, k);
					if (Math::is_finite(_height_data[other_index])) {
						edge_indices.set(edge_iter++, other_index);
						edge_indices.set(edge_iter++, index);
					}
				}
				if (k > 0) {
					const int64_t other_index = _get_height_index_nocheck(i, j, k - 1);
					if (Math::is_finite(_height_data[other_index])) {
						edge_indices.set(edge_iter++, other_index);
						edge_indices.set(edge_iter++, index);
					}
				}
			}
		}
	}
	edge_indices.resize(edge_iter);
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertices(vertices);
	wire_mesh->set_edge_indices(edge_indices);
	return wire_mesh;
}

void HeightMapShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_height_data", "height_data"), &HeightMapShape4D::set_height_data);
	ClassDB::bind_method(D_METHOD("get_height_data"), &HeightMapShape4D::get_height_data);
	ClassDB::bind_method(D_METHOD("set_grid_spacing", "spacing"), &HeightMapShape4D::set_grid_spacing);
	ClassDB::bind_method(D_METHOD("get_grid_spacing"), &HeightMapShape4D::get_grid_spacing);
	ClassDB::bind_method(D_METHOD("set_grid_size", "grid_size"), &HeightMapShape4D::set_grid_size);
	ClassDB::bind_method(D_METHOD("get_grid_size"), &HeightMapShape4D::get_grid_size);
	ClassDB::bind_method(D_METHOD("set_grid_size_width", "width"), &HeightMapShape4D::set_grid_size_width);
	ClassDB::bind_method(D_METHOD("get_grid_size_width"), &HeightMapShape4D::get_grid_size_width);
	ClassDB::bind_method(D_METHOD("set_grid_size_depth", "depth"), &HeightMapShape4D::set_grid_size_depth);
	ClassDB::bind_method(D_METHOD("get_grid_size_depth"), &HeightMapShape4D::get_grid_size_depth);
	ClassDB::bind_method(D_METHOD("set_grid_size_thickness", "thickness"), &HeightMapShape4D::set_grid_size_thickness);
	ClassDB::bind_method(D_METHOD("get_grid_size_thickness"), &HeightMapShape4D::get_grid_size_thickness);

	ClassDB::bind_method(D_METHOD("get_height_index", "x", "z", "w"), &HeightMapShape4D::get_height_index);
	ClassDB::bind_method(D_METHOD("get_height_index_vec3i", "vector"), &HeightMapShape4D::get_height_index_vec3i);
	ClassDB::bind_method(D_METHOD("get_height_vec3", "physical_pos"), &HeightMapShape4D::get_height_vec3);
	ClassDB::bind_method(D_METHOD("get_height_vec4", "physical_pos"), &HeightMapShape4D::get_height_vec4);
	ClassDB::bind_method(D_METHOD("get_height_on_grid_vec3i", "grid_pos"), &HeightMapShape4D::get_height_on_grid_vec3i);
	ClassDB::bind_method(D_METHOD("get_height_on_grid_vec4i", "grid_pos"), &HeightMapShape4D::get_height_on_grid_vec4i);

	ClassDB::bind_method(D_METHOD("quantize_to_float8"), &HeightMapShape4D::quantize_to_float8);
	ClassDB::bind_method(D_METHOD("quantize_to_float16"), &HeightMapShape4D::quantize_to_float16);
	ClassDB::bind_method(D_METHOD("quantize_to_float32"), &HeightMapShape4D::quantize_to_float32);

	ClassDB::bind_method(D_METHOD("fill_from_mesh_3d", "mesh_3d", "exponent", "max_height", "min_height"), &HeightMapShape4D::fill_from_mesh_3d);
	ClassDB::bind_method(D_METHOD("gaussian_blur", "blur_radius"), &HeightMapShape4D::gaussian_blur);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "grid_spacing"), "set_grid_spacing", "get_grid_spacing");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "grid_size", PROPERTY_HINT_RANGE, "1,1000,or_greater", PROPERTY_USAGE_STORAGE), "set_grid_size", "get_grid_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "grid_size_width", PROPERTY_HINT_RANGE, "1,1000,or_greater", PROPERTY_USAGE_EDITOR), "set_grid_size_width", "get_grid_size_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "grid_size_depth", PROPERTY_HINT_RANGE, "1,1000,or_greater", PROPERTY_USAGE_EDITOR), "set_grid_size_depth", "get_grid_size_depth");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "grid_size_thickness", PROPERTY_HINT_RANGE, "1,1000,or_greater", PROPERTY_USAGE_EDITOR), "set_grid_size_thickness", "get_grid_size_thickness");
	// This MUST be the last property, since it depends on the size being set, and Godot populates properties in this order.
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "height_data"), "set_height_data", "get_height_data");
}
