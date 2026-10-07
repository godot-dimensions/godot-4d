#include "convex_hull_shape_4d.h"

#include "../../model/mesh/multi_surface_mesh_4d.h"
#include "../../model/mesh/single_surface_mesh_4d.h"
#include "../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../model/mesh/wire/array_wire_mesh_4d.h"

void ConvexHullShape4D::set_points(const PackedVector4Array &p_points) {
	_points = p_points;
}

Rect4 ConvexHullShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	const int64_t point_count = _points.size();
	if (point_count == 0) {
		return Rect4(p_to_target.origin, Vector4());
	}
	// The bounds of the hull are the bounds of its points. Unlike the default implementation, this does not include
	// the origin, since the points do not need to surround it, and this is cheaper than 8 support point queries.
	const Vector4 *points_ptr = _points.ptr();
	Rect4 bounds = Rect4(p_to_target * points_ptr[0], Vector4());
	for (int64_t i = 1; i < point_count; i++) {
		bounds.expand_self_to_point(p_to_target * points_ptr[i]);
	}
	return bounds;
}

Vector4 ConvexHullShape4D::get_support_point(const Vector4 &p_local_direction) const {
	const int64_t point_count = _points.size();
	if (point_count == 0) {
		return Vector4();
	}
	// The farthest point of a convex hull along any direction is always one of its points, so the hull itself
	// does not need to be calculated. Any interior points are never farther along the direction than the hull.
	const Vector4 *points_ptr = _points.ptr();
	Vector4 support = points_ptr[0];
	real_t support_distance = support.dot(p_local_direction);
	for (int64_t i = 1; i < point_count; i++) {
		const real_t distance = points_ptr[i].dot(p_local_direction);
		if (distance > support_distance) {
			support = points_ptr[i];
			support_distance = distance;
		}
	}
	return support;
}

bool ConvexHullShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<ConvexHullShape4D> other_shape = p_shape;
	if (other_shape.is_null()) {
		return false;
	}
	const PackedVector4Array other_points = other_shape->get_points();
	return _points == other_points;
}

Ref<TetraMesh4D> ConvexHullShape4D::to_tetra_mesh(const Dictionary &p_options) const {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertex_positions(_points);
	ERR_PRINT("ConvexHullShape4D.convert_to_tetra_mesh: Calculating the tetrahedral mesh cells is not implemented yet.");
	return mesh;
}

Ref<WireMesh4D> ConvexHullShape4D::to_wire_mesh(const Dictionary &p_options) const {
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertex_positions(_points);
	ERR_PRINT("ConvexHullShape4D.convert_to_wire_mesh: Calculating the wire mesh edge indices is not implemented yet.");
	return mesh;
}

Ref<ConvexHullShape4D> ConvexHullShape4D::create_from_mesh(const Ref<Mesh4D> &p_mesh) {
	Ref<ConvexHullShape4D> shape;
	shape.instantiate();
	PackedVector4Array points;
	const Ref<SingleSurfaceMesh4D> single_surface_mesh_4d = p_mesh;
	if (single_surface_mesh_4d.is_valid()) {
		points = single_surface_mesh_4d->get_vertex_positions();
	} else {
		const Ref<MultiSurfaceMesh4D> multi_surface_mesh_4d = p_mesh;
		if (multi_surface_mesh_4d.is_valid()) {
			const Vector<Ref<SingleSurfaceMesh4D>> &surface_meshes = multi_surface_mesh_4d->get_surface_meshes();
			for (int64_t surface_index = 0; surface_index < surface_meshes.size(); surface_index++) {
				const Ref<SingleSurfaceMesh4D> &surface_mesh_4d = surface_meshes[surface_index];
				if (surface_mesh_4d.is_valid()) {
					points.append_array(surface_mesh_4d->get_vertex_positions());
				}
			}
		} else {
			ERR_FAIL_V_MSG(shape, "ConvexHullShape4D.create_from_mesh: Unhandled mesh type.");
		}
	}
	shape->set_points(points);
	ERR_PRINT("ConvexHullShape4D.create_from_mesh: Calculating the convex hull from mesh vertices is not implemented yet.");
	return shape;
}

void ConvexHullShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_points"), &ConvexHullShape4D::get_points);
	ClassDB::bind_method(D_METHOD("set_points", "points"), &ConvexHullShape4D::set_points);

	ClassDB::bind_static_method("ConvexHullShape4D", D_METHOD("create_from_mesh", "mesh"), &ConvexHullShape4D::create_from_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "points", PROPERTY_HINT_NONE, "suffix:m"), "set_points", "get_points");
}
