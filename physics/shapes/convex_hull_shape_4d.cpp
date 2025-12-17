#include "convex_hull_shape_4d.h"

#include "../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../model/mesh/wire/array_wire_mesh_4d.h"

void ConvexHullShape4D::set_points(const PackedVector4Array &p_points) {
	_points = p_points;
}

bool ConvexHullShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<ConvexHullShape4D> other_shape = p_shape;
	if (other_shape.is_null()) {
		return false;
	}
	const PackedVector4Array other_points = other_shape->get_points();
	return _points == other_points;
}

Ref<TetraMesh4D> ConvexHullShape4D::to_tetra_mesh() const {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(_points);
	ERR_PRINT("ConvexHullShape4D.convert_to_tetra_mesh: Calculating the tetrahedral mesh cells is not implemented yet.");
	return mesh;
}

Ref<WireMesh4D> ConvexHullShape4D::to_wire_mesh() const {
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(_points);
	ERR_PRINT("ConvexHullShape4D.convert_to_wire_mesh: Calculating the wire mesh edge indices is not implemented yet.");
	return mesh;
}

Ref<ConvexHullShape4D> ConvexHullShape4D::create_from_mesh(const Ref<Mesh4D> &p_mesh) {
	Ref<ConvexHullShape4D> shape;
	shape.instantiate();
	shape->set_points(p_mesh->get_vertices());
	ERR_PRINT("ConvexHullShape4D.create_from_mesh: Calculating the convex hull from mesh vertices is not implemented yet.");
	return shape;
}

void ConvexHullShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_points"), &ConvexHullShape4D::get_points);
	ClassDB::bind_method(D_METHOD("set_points", "points"), &ConvexHullShape4D::set_points);

	ClassDB::bind_static_method("ConvexHullShape4D", D_METHOD("create_from_mesh", "mesh"), &ConvexHullShape4D::create_from_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "points", PROPERTY_HINT_NONE, "suffix:m"), "set_points", "get_points");
}
