#include "sphere_shape_4d.h"

#include "../../model/mesh/wire/array_wire_mesh_4d.h"

real_t SphereShape4D::get_radius() const {
	return _radius;
}

void SphereShape4D::set_radius(const real_t p_radius) {
	_radius = p_radius;
}

real_t SphereShape4D::get_hypervolume() const {
	return (0.125 * Math_TAU * Math_TAU) * (_radius * _radius * _radius * _radius);
}

Vector4 SphereShape4D::get_nearest_point(const Vector4 &p_point) const {
	const real_t length_sq = p_point.length_squared();
	if (length_sq <= _radius * _radius) {
		return p_point;
	}
	return p_point * (_radius / Math::sqrt(length_sq));
}

Vector4 SphereShape4D::get_support_point(const Vector4 &p_direction) const {
	return p_direction.normalized() * _radius;
}

real_t SphereShape4D::get_surface_volume() const {
	return (0.5 * Math_TAU * Math_TAU) * (_radius * _radius * _radius);
}

bool SphereShape4D::has_point(const Vector4 &p_point) const {
	return p_point.length_squared() <= _radius * _radius;
}

bool SphereShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<SphereShape4D> sphere_shape = p_shape;
	if (sphere_shape.is_null()) {
		return false;
	}
	return _radius == sphere_shape->get_radius();
}

Ref<WireMesh4D> SphereShape4D::to_wire_mesh(const Dictionary &p_options) const {
	const int64_t segments = p_options.has("segments") ? (int64_t)p_options["segments"] : (int64_t)16;
	const bool deduplicate = p_options.has("deduplicate") ? (bool)p_options["deduplicate"] : true;
	// Create 6 rings on the 6 bivector planes.
	PackedVector4Array vertices;
	PackedInt32Array edge_indices;
	vertices.resize(6 * segments);
	edge_indices.resize(6 * 2 * segments);
	int vert_iter = 0;
	for (int a = 0; a < 3; a++) {
		for (int b = a + 1; b < 4; b++) {
			// Create a ring on the a-b bivector plane.
			Vector4 point = Vector4(0.0, 0.0, 0.0, 0.0);
			const int ring_start_vert = vert_iter;
			point[a] = _radius;
			vertices.set(vert_iter, point);
			for (int i = 1; i < segments; i++) {
				const double angle = (double)i * (Math_TAU / (double)segments);
				point[a] = _radius * Math::cos(angle);
				point[b] = _radius * Math::sin(angle);
				edge_indices.set(vert_iter * 2, vert_iter);
				edge_indices.set(vert_iter * 2 + 1, vert_iter + 1);
				vert_iter++;
				vertices.set(vert_iter, point);
			}
			edge_indices.set(vert_iter * 2, ring_start_vert);
			edge_indices.set(vert_iter * 2 + 1, vert_iter);
			vert_iter++;
		}
	}
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertices(vertices);
	wire_mesh->set_edge_indices(edge_indices);
	if (deduplicate) {
		wire_mesh->deduplicate_all_elements();
	}
	CRASH_COND(!wire_mesh->is_mesh_data_valid());
	return wire_mesh;
}

void SphereShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &SphereShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SphereShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");
}
