#include "wire_mesh_builder_4d.h"

Ref<ArrayWireMesh4D> WireMeshBuilder4D::make_3d_orthoplex_sphere(const real_t p_radius, const int p_subdivisions, const real_t p_cone_tip_position) {
	PackedVector4Array vertices;
	PackedInt32Array edge_indices;
	const bool make_cone = p_cone_tip_position != 0.0;
	const int octahedral_vertex_count = 8 * (p_subdivisions * (p_subdivisions + 1)) / 2;
	vertices.resize(octahedral_vertex_count + make_cone);
	// First, create a 3D sphere for the base of the 4D arrow.
	// For symmetry between all axes, make a subdivided octahedron and normalize it.
	int vertex_index = 0;
	for (int x_sign = -1; x_sign < 2; x_sign += 2) {
		for (int y_sign = -1; y_sign < 2; y_sign += 2) {
			for (int z_sign = -1; z_sign < 2; z_sign += 2) {
				int prev_row_vertex_index = vertex_index;
				for (int x_index = 0; x_index < p_subdivisions; x_index++) {
					const int y_max = p_subdivisions - x_index;
					int x_vertex_index = vertex_index;
					for (int y_index = 0; y_index < y_max; y_index++) {
						const int z_index = y_max - y_index - 1;
						Vector3 point_3d = Vector3(x_index * x_sign, y_index * y_sign, z_index * z_sign);
						point_3d.normalize();
						point_3d *= p_radius;
						Vector4 point = Vector4(point_3d.x, point_3d.y, point_3d.z, 0.0f);
						vertices.set(vertex_index, point);
						// Append edges, deduplicating them. Note that this algorithm
						// deduplicates only edges but does not deduplicate vertices.
						if (x_index == 0) {
							if (x_sign == 1 && vertex_index > x_vertex_index) {
								edge_indices.push_back(vertex_index - 1);
								edge_indices.push_back(vertex_index);
							}
						} else { // x_index > 0
							if (y_index > 0) {
								edge_indices.push_back(vertex_index - 1);
								edge_indices.push_back(vertex_index);
							}
							if (y_sign == 1 || y_index > 0) {
								edge_indices.push_back(prev_row_vertex_index + y_index);
								edge_indices.push_back(vertex_index);
							}
							if (z_sign == 1 || z_index > 0) {
								edge_indices.push_back(prev_row_vertex_index + y_index + 1);
								edge_indices.push_back(vertex_index);
							}
						}
						vertex_index++;
					}
					prev_row_vertex_index = x_vertex_index;
				}
			}
		}
	}
	if (make_cone) {
		vertices.set(octahedral_vertex_count, Vector4(0.0, 0.0, 0.0, p_cone_tip_position));
		for (int i = 0; i < octahedral_vertex_count; i++) {
			edge_indices.push_back(i);
			edge_indices.push_back(octahedral_vertex_count);
		}
	}
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(vertices);
	mesh->set_edge_indices(edge_indices);
	return mesh;
}

Ref<ArrayWireMesh4D> WireMeshBuilder4D::make_3d_subdivided_box(const Vector3 &p_size, const Vector3i &p_subdivision_segments, const bool p_fill_cell, const bool p_breakup_edges) {
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	ERR_FAIL_COND_V_MSG(p_subdivision_segments.x < 1 || p_subdivision_segments.y < 1 || p_subdivision_segments.z < 1, wire_mesh, "Subdivision segments must be greater than 0.");
	const Vector3 start = p_size * -0.5f;
	const Vector3 step = p_size / p_subdivision_segments;
	if (p_breakup_edges) {
		for (int x_index = 0; x_index <= p_subdivision_segments.x; x_index++) {
			const bool x_boundary = x_index == 0 || x_index == p_subdivision_segments.x;
			for (int y_index = 0; y_index <= p_subdivision_segments.y; y_index++) {
				const bool y_boundary = y_index == 0 || y_index == p_subdivision_segments.y;
				for (int z_index = 0; z_index <= p_subdivision_segments.z; z_index++) {
					const bool z_boundary = z_index == 0 || z_index == p_subdivision_segments.z;
					Vector4 point = Vector4(start.x + x_index * step.x, start.x + y_index * step.y, start.x + z_index * step.z, 0);
					if (x_index < p_subdivision_segments.x && (p_fill_cell || (y_boundary + z_boundary >= 1))) {
						wire_mesh->append_edge_points(point, point + Vector4(step.x, 0, 0, 0));
					}
					if (y_index < p_subdivision_segments.y && (p_fill_cell || (x_boundary + z_boundary >= 1))) {
						wire_mesh->append_edge_points(point, point + Vector4(0, step.y, 0, 0));
					}
					if (z_index < p_subdivision_segments.z && (p_fill_cell || (x_boundary + y_boundary >= 1))) {
						wire_mesh->append_edge_points(point, point + Vector4(0, 0, step.z, 0));
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
				// Z-axis lines.
				if (p_fill_cell || (x_boundary + y_boundary >= 1)) {
					Vector4 start_point = Vector4(start.x + x_index * step.x, start.y + y_index * step.y, start.z, 0);
					wire_mesh->append_edge_points(start_point, start_point + Vector4(0, 0, p_size.z, 0));
				}
			}
			for (int z_index = 0; z_index <= p_subdivision_segments.z; z_index++) {
				const bool z_boundary = z_index == 0 || z_index == p_subdivision_segments.z;
				// Y-axis lines.
				if (p_fill_cell || (x_boundary + z_boundary >= 1)) {
					Vector4 start_point = Vector4(start.x + x_index * step.x, start.y, start.z + z_index * step.z, 0);
					wire_mesh->append_edge_points(start_point, start_point + Vector4(0, p_size.y, 0, 0));
				}
			}
		}
		for (int y_index = 0; y_index <= p_subdivision_segments.y; y_index++) {
			const bool y_boundary = y_index == 0 || y_index == p_subdivision_segments.y;
			for (int z_index = 0; z_index <= p_subdivision_segments.z; z_index++) {
				const bool z_boundary = z_index == 0 || z_index == p_subdivision_segments.z;
				// X-axis lines.
				if (p_fill_cell || (y_boundary + z_boundary >= 1)) {
					Vector4 start_point = Vector4(start.x, start.y + y_index * step.y, start.z + z_index * step.z, 0);
					wire_mesh->append_edge_points(start_point, start_point + Vector4(p_size.x, 0, 0, 0));
				}
			}
		}
	}
	return wire_mesh;
}

WireMeshBuilder4D *WireMeshBuilder4D::singleton = nullptr;

void WireMeshBuilder4D::_bind_methods() {
	ClassDB::bind_static_method("WireMeshBuilder4D", D_METHOD("make_3d_orthoplex_sphere", "radius", "subdivisions", "cone_tip_position"), &WireMeshBuilder4D::make_3d_orthoplex_sphere, DEFVAL(0.0));
	ClassDB::bind_static_method("WireMeshBuilder4D", D_METHOD("make_3d_subdivided_box", "size", "subdivision_segments", "fill_cell", "breakup_edges"), &WireMeshBuilder4D::make_3d_subdivided_box, DEFVAL(false), DEFVAL(false));
}
