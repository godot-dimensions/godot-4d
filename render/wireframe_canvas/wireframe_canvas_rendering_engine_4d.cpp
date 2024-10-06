#include "wireframe_canvas_rendering_engine_4d.h"

#include "../../mesh/wire/wire_material_4d.h"
#include "wireframe_render_canvas.h"

Vector2 WireframeCanvasRenderingEngine4D::_project_point_3d_to_2d(const Vector3 &p_point) const {
	const Vector3 intersection_point = p_point * (get_camera()->get_focal_length_3d() / p_point.z);
	return Vector2(intersection_point.x, intersection_point.y);
}

Vector3 WireframeCanvasRenderingEngine4D::_project_point_4d_to_3d(const Vector4 &p_point) const {
	const Vector4 intersection_point = p_point * (get_camera()->get_focal_length_4d() / p_point.z);
	return Vector3(intersection_point.x, intersection_point.y, intersection_point.w);
}

Vector2 WireframeCanvasRenderingEngine4D::_project_point_4d_to_2d(const Vector4 &p_vertex) const {
	Camera4D *camera = get_camera();
	// Project from 4D to 3D.
	Vector3 projected_point_3d = Vector3();
	if (camera->is_first_pass_perspective()) {
		projected_point_3d = _project_point_4d_to_3d(p_vertex);
	} else if (camera->get_projection_type() == Camera4D::PROJECTION4D_ORTHOGRAPHIC) {
		projected_point_3d = Vector3(p_vertex.x, p_vertex.y, p_vertex.z) / get_camera()->get_orthographic_size();
	} else {
		projected_point_3d = Vector3(p_vertex.x, p_vertex.y, p_vertex.z);
	}
	// Project from 3D to 2D.
	if (camera->is_second_pass_perspective()) {
		return _project_point_3d_to_2d(projected_point_3d);
	}
	return Vector2(projected_point_3d.x, projected_point_3d.y);
}

Color _get_material_edge_color(const Ref<Material4D> &p_material, const Ref<Mesh4D> &p_mesh, int p_edge_index) {
	if (p_material.is_null()) {
		return Color(1.0f, 1.0f, 1.0f);
	}
	const int flags = p_material->get_albedo_source_flags();
	if (flags == Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR) {
		return p_material->get_albedo_color();
	}
	Color color = Color(1.0f, 1.0f, 1.0f);
	if (flags & Material4D::COLOR_SOURCE_FLAG_PER_EDGE) {
		color = p_material->get_albedo_color_array()[p_edge_index];
	} else if (flags & Material4D::COLOR_SOURCE_FLAG_PER_VERT) {
		const PackedInt32Array edge_indices = p_mesh->get_edge_indices();
		const PackedColorArray color_array = p_material->get_albedo_color_array();
		color = color_array[edge_indices[p_edge_index * 2]].lerp(color_array[edge_indices[p_edge_index * 2 + 1]], 0.5f);
	}
	if (flags & Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR) {
		color *= p_material->get_albedo_color();
	}
	return color;
}

void WireframeCanvasRenderingEngine4D::setup_for_viewport() {
	WireframeRenderCanvas *wire_canvas = memnew(WireframeRenderCanvas);
	wire_canvas->set_name("WireframeRenderCanvas");
	get_viewport()->add_child(wire_canvas);
}

void WireframeCanvasRenderingEngine4D::render_frame() {
#if GDEXTENSION
	WireframeRenderCanvas *wire_canvas = get_viewport()->get_node<WireframeRenderCanvas>(NodePath("WireframeRenderCanvas"));
#elif GODOT_MODULE
	WireframeRenderCanvas *wire_canvas = Object::cast_to<WireframeRenderCanvas>(get_viewport()->get_node(NodePath("WireframeRenderCanvas")));
#endif
	ERR_FAIL_NULL_MSG(wire_canvas, "WireframeCanvasRenderingEngine4D: Canvas was null.");
	const Camera4D *camera = get_camera();
	Vector<PackedColorArray> edge_colors_to_draw;
	PackedFloat32Array edge_thicknesses_to_draw;
	Vector<PackedVector2Array> edge_vertices_to_draw;
	TypedArray<MeshInstance4D> mesh_instances = get_mesh_instances();
	TypedArray<Projection> mesh_relative_basises = get_mesh_relative_basises();
	PackedVector4Array mesh_relative_positions = get_mesh_relative_positions();
	for (int mesh_index = 0; mesh_index < mesh_instances.size(); mesh_index++) {
		MeshInstance4D *mesh_inst = Object::cast_to<MeshInstance4D>(mesh_instances[mesh_index]);
		ERR_CONTINUE(mesh_inst == nullptr);
		Ref<Material4D> material = mesh_inst->get_active_material();
		Projection mesh_relative_basis = mesh_relative_basises[mesh_index];
		Vector4 mesh_relative_position = mesh_relative_positions[mesh_index];
		const PackedVector4Array camera_relative_vertices = Transform4D(mesh_relative_basis, mesh_relative_position).xform_many(mesh_inst->get_mesh()->get_vertices());
		PackedVector2Array projected_vertices;
		for (int vertex = 0; vertex < camera_relative_vertices.size(); vertex++) {
			projected_vertices.push_back(_project_point_4d_to_2d(camera_relative_vertices[vertex]));
		}
		PackedColorArray edge_colors;
		PackedVector2Array edge_vertices;
		const PackedInt32Array edge_indices = mesh_inst->get_mesh()->get_edge_indices();
		for (int edge_index = 0; edge_index < edge_indices.size() / 2; edge_index++) {
			const int a_index = edge_indices[edge_index * 2];
			const int b_index = edge_indices[edge_index * 2 + 1];
			const Vector4 a_vert_4d = camera_relative_vertices[a_index];
			const Vector4 b_vert_4d = camera_relative_vertices[b_index];
			if (a_vert_4d.z > -camera->get_near()) {
				if (b_vert_4d.z > -camera->get_near()) {
					// Both points are behind the camera, so we skip this edge.
					continue;
				} else {
					// A is behind the camera, while B is in front of the camera.
					const real_t factor = (a_vert_4d.z + camera->get_near()) / (a_vert_4d.z - b_vert_4d.z);
					const Vector4 clipped = a_vert_4d.lerp(b_vert_4d, factor);
					edge_vertices.push_back(_project_point_4d_to_2d(clipped));
					edge_vertices.push_back(projected_vertices[b_index]);
				}
			} else {
				edge_vertices.push_back(projected_vertices[a_index]);
				if (b_vert_4d.z > -camera->get_near()) {
					// B is behind the camera, while A is in front of the camera.
					const real_t factor = (b_vert_4d.z + camera->get_near()) / (b_vert_4d.z - a_vert_4d.z);
					const Vector4 clipped = b_vert_4d.lerp(a_vert_4d, factor);
					edge_vertices.push_back(_project_point_4d_to_2d(clipped));
				} else {
					// Both points are in front of the camera, so render the edge as-is.
					edge_vertices.push_back(projected_vertices[b_index]);
				}
			}
			Color edge_color = _get_material_edge_color(material, mesh_inst->get_mesh(), edge_index);
			if (camera->get_w_fade_mode() != Camera4D::W_FADE_DISABLED) {
				const real_t fade_denom = camera->get_w_fade_slope() * -0.5f * (a_vert_4d.z + b_vert_4d.z) + camera->get_w_fade_distance();
				const real_t fade_factor = (a_vert_4d.w + b_vert_4d.w) * (0.5f / fade_denom);
				if (camera->get_w_fade_mode() & Camera4D::W_FADE_HUE_SHIFT) {
					if (fade_factor < 0.0f) {
						edge_color = edge_color.lerp(camera->get_w_fade_color_negative() * edge_color.get_v(), MIN(1.0f, -fade_factor));
					} else {
						edge_color = edge_color.lerp(camera->get_w_fade_color_positive() * edge_color.get_v(), MIN(1.0f, fade_factor));
					}
				}
				if (camera->get_w_fade_mode() & Camera4D::W_FADE_TRANSPARENCY) {
					edge_color.a = 1.0f - MIN(1.0f, ABS(fade_factor));
				}
			}
			edge_colors.push_back(edge_color);
		}
		if (edge_vertices.is_empty()) {
			continue;
		}
		edge_colors_to_draw.push_back(edge_colors);
		Ref<WireMaterial4D> wire_material = material;
		if (wire_material.is_valid()) {
			edge_thicknesses_to_draw.push_back(wire_material->get_line_thickness() > 0.0f ? wire_material->get_line_thickness() : -1.0f);
		} else {
			edge_thicknesses_to_draw.push_back(-1.0f);
		}
		edge_vertices_to_draw.push_back(edge_vertices);
	}
	wire_canvas->set_camera_aspect(camera->get_keep_aspect());
	wire_canvas->set_edge_colors_to_draw(edge_colors_to_draw);
	wire_canvas->set_edge_thicknesses_to_draw(edge_thicknesses_to_draw);
	wire_canvas->set_edge_vertices_to_draw(edge_vertices_to_draw);
	wire_canvas->queue_redraw();
}
