#include "box_wire_mesh_4d.h"

#include "../box_mesh_data.inc"
#include "../tetra/box_tetra_mesh_4d.h"
#include "array_wire_mesh_4d.h"

Vector4 BoxWireMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void BoxWireMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	set_size(p_half_extents * 2.0f);
}

Vector4 BoxWireMesh4D::get_size() const {
	return _size;
}

void BoxWireMesh4D::set_size(const Vector4 &p_size) {
	if (p_size != _size) {
		_size = p_size;
		_vertices_cache.clear();
		wire_mesh_clear_cache();
	}
}

PackedInt32Array BoxWireMesh4D::get_edge_indices() {
	return BOX_EDGE_INDICES;
}

PackedVector4Array BoxWireMesh4D::get_edge_positions() {
	if (_edge_positions_cache.is_empty()) {
		for (const int i : BOX_EDGE_INDICES) {
			_edge_positions_cache.append(get_vertices()[i]);
		}
	}
	return _edge_positions_cache;
}

PackedVector4Array BoxWireMesh4D::get_vertices() {
	if (_vertices_cache.is_empty()) {
		const Vector4 he = get_half_extents();
		_vertices_cache.append(Vector4(-he.x, -he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, +he.z, +he.w));
	}
	return _vertices_cache;
}

Ref<ArrayWireMesh4D> BoxWireMesh4D::subdivide_box(const Vector4i &p_subdivision_segments, const bool p_fill_cells, const bool p_breakup_edges) const {
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	ERR_FAIL_COND_V_MSG(p_subdivision_segments.x < 1 || p_subdivision_segments.y < 1 || p_subdivision_segments.z < 1 || p_subdivision_segments.w < 1, wire_mesh, "Subdivision segments must be greater than 0.");
	const Vector4 start = _size * -0.5f;
	const Vector4 step = _size / p_subdivision_segments;
	if (p_breakup_edges) {
		for (int x_index = 0; x_index <= p_subdivision_segments.x; x_index++) {
			const bool x_boundary = x_index == 0 || x_index == p_subdivision_segments.x;
			for (int y_index = 0; y_index <= p_subdivision_segments.y; y_index++) {
				const bool y_boundary = y_index == 0 || y_index == p_subdivision_segments.y;
				for (int z_index = 0; z_index <= p_subdivision_segments.z; z_index++) {
					const bool z_boundary = z_index == 0 || z_index == p_subdivision_segments.z;
					for (int w_index = 0; w_index <= p_subdivision_segments.w; w_index++) {
						const bool w_boundary = w_index == 0 || w_index == p_subdivision_segments.w;
						Vector4 point = start + Vector4(x_index, y_index, z_index, w_index) * step;
						if (x_index < p_subdivision_segments.x && (p_fill_cells || (y_boundary + z_boundary + w_boundary >= 2))) {
							wire_mesh->append_edge_points(point, point + Vector4(step.x, 0, 0, 0));
						}
						if (y_index < p_subdivision_segments.y && (p_fill_cells || (x_boundary + z_boundary + w_boundary >= 2))) {
							wire_mesh->append_edge_points(point, point + Vector4(0, step.y, 0, 0));
						}
						if (z_index < p_subdivision_segments.z && (p_fill_cells || (x_boundary + y_boundary + w_boundary >= 2))) {
							wire_mesh->append_edge_points(point, point + Vector4(0, 0, step.z, 0));
						}
						if (w_index < p_subdivision_segments.w && (p_fill_cells || (x_boundary + y_boundary + z_boundary >= 2))) {
							wire_mesh->append_edge_points(point, point + Vector4(0, 0, 0, step.w));
						}
					}
				}
			}
		}
	} else {
		// Loop over each axis and create continuous edges for the entire subdivided box
		for (int x_index = 0; x_index <= p_subdivision_segments.x; x_index++) {
			const bool x_boundary = x_index == 0 || x_index == p_subdivision_segments.x;
			for (int y_index = 0; y_index <= p_subdivision_segments.y; y_index++) {
				const bool y_boundary = y_index == 0 || y_index == p_subdivision_segments.y;
				for (int z_index = 0; z_index <= p_subdivision_segments.z; z_index++) {
					const bool z_boundary = z_index == 0 || z_index == p_subdivision_segments.z;
					// W-axis lines.
					if (p_fill_cells || (x_boundary + y_boundary + z_boundary >= 2)) {
						Vector4 start_point = start + Vector4(x_index * step.x, y_index * step.y, z_index * step.z, 0);
						wire_mesh->append_edge_points(start_point, start_point + Vector4(0, 0, 0, _size.w));
					}
				}
				for (int w_index = 0; w_index <= p_subdivision_segments.w; w_index++) {
					const bool w_boundary = w_index == 0 || w_index == p_subdivision_segments.w;
					// Z-axis lines.
					if (p_fill_cells || (x_boundary + y_boundary + w_boundary >= 2)) {
						Vector4 start_point = start + Vector4(x_index * step.x, y_index * step.y, 0, w_index * step.w);
						wire_mesh->append_edge_points(start_point, start_point + Vector4(0, 0, _size.z, 0));
					}
				}
			}
			for (int z_index = 0; z_index <= p_subdivision_segments.z; z_index++) {
				const bool z_boundary = z_index == 0 || z_index == p_subdivision_segments.z;
				for (int w_index = 0; w_index <= p_subdivision_segments.w; w_index++) {
					const bool w_boundary = w_index == 0 || w_index == p_subdivision_segments.w;
					// Y-axis lines.
					if (p_fill_cells || (x_boundary + z_boundary + w_boundary >= 2)) {
						Vector4 start_point = start + Vector4(x_index * step.x, 0, z_index * step.z, w_index * step.w);
						wire_mesh->append_edge_points(start_point, start_point + Vector4(0, _size.y, 0, 0));
					}
				}
			}
		}
		for (int y_index = 0; y_index <= p_subdivision_segments.y; y_index++) {
			const bool y_boundary = y_index == 0 || y_index == p_subdivision_segments.y;
			for (int z_index = 0; z_index <= p_subdivision_segments.z; z_index++) {
				const bool z_boundary = z_index == 0 || z_index == p_subdivision_segments.z;
				for (int w_index = 0; w_index <= p_subdivision_segments.w; w_index++) {
					const bool w_boundary = w_index == 0 || w_index == p_subdivision_segments.w;
					// X-axis lines.
					if (p_fill_cells || (y_boundary + z_boundary + w_boundary >= 2)) {
						Vector4 start_point = start + Vector4(0, y_index * step.y, z_index * step.z, w_index * step.w);
						wire_mesh->append_edge_points(start_point, start_point + Vector4(_size.x, 0, 0, 0));
					}
				}
			}
		}
	}
	return wire_mesh;
}

Ref<BoxWireMesh4D> BoxWireMesh4D::from_tetra_mesh(const Ref<BoxTetraMesh4D> &p_tetra_mesh) {
	Ref<BoxWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(p_tetra_mesh->get_size());
	wire_mesh->set_material(p_tetra_mesh->get_material());
	return wire_mesh;
}

Ref<BoxTetraMesh4D> BoxWireMesh4D::to_tetra_mesh() const {
	Ref<BoxTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	tetra_mesh->set_material(get_material());
	return tetra_mesh;
}

Ref<WireMesh4D> BoxWireMesh4D::to_wire_mesh() {
	Ref<BoxWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

void BoxWireMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("subdivide_box", "subdivision_segments", "fill_cells", "breakup_edges"), &BoxWireMesh4D::subdivide_box, DEFVAL(false), DEFVAL(false));

	ClassDB::bind_method(D_METHOD("get_half_extents"), &BoxWireMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &BoxWireMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxWireMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxWireMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_static_method("BoxWireMesh4D", D_METHOD("from_tetra_mesh", "tetra_mesh"), &BoxWireMesh4D::from_tetra_mesh);
	ClassDB::bind_method(D_METHOD("to_tetra_mesh"), &BoxWireMesh4D::to_tetra_mesh);
}
