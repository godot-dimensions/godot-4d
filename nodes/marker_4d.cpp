#include "marker_4d.h"

#include "../math/vector_4d.h"
#include "../model/mesh/wire/array_wire_mesh_4d.h"
#include "../model/mesh/wire/wire_material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#elif GODOT_MODULE
#include "core/config/engine.h"
#endif

void Marker4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
#ifdef TOOLS_ENABLED
			if (!Engine::get_singleton()->is_editor_hint())
#endif // TOOLS_ENABLED
			{
				switch (_runtime_behavior) {
					case RUNTIME_BEHAVIOR_SHOW:
						break;
					case RUNTIME_BEHAVIOR_HIDE:
						set_visible(false);
						return;
					case RUNTIME_BEHAVIOR_DELETE:
						queue_free();
						return;
				}
			}
			generate_marker_mesh();
		} break;
	}
	MeshInstance4D::_notification(p_what);
}

void Marker4D::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name != StringName("marker_extents") &&
			p_property.name != StringName("runtime_behavior") &&
			p_property.name != StringName("darken_negative_amount") &&
			p_property.name != StringName("subdivisions") &&
			p_property.name != StringName("editor_description")) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

PackedStringArray Marker4D::GDEXTMOD_GET_CONFIGURATION_WARNINGS() const {
	PackedStringArray warnings;
	if (_runtime_behavior == RUNTIME_BEHAVIOR_DELETE) {
		if (get_child_count() != 0) {
			warnings.append("Marker4D will be deleted at runtime, but it has children. This may cause unexpected behavior.");
		}
	}
	return warnings;
}

void Marker4D::set_marker_extents(const real_t p_marker_extents) {
	_marker_extents = p_marker_extents;
	generate_marker_mesh();
}

void Marker4D::set_darken_negative_amount(const float p_darken_negative_amount) {
	_darken_negative_amount = p_darken_negative_amount;
	generate_marker_mesh();
}

void Marker4D::set_subdivisions(const int p_subdivisions) {
	ERR_FAIL_COND(p_subdivisions < 1);
	ERR_FAIL_COND(p_subdivisions > 63);
	_subdivisions = p_subdivisions;
	generate_marker_mesh();
}

void Marker4D::generate_marker_mesh() {
	// Splitting the line helps with precision issues when zooming.
	const int EDGES_PER_DIRECTION = _subdivisions;
	constexpr int DIRECTIONS = 8; // Twice the number of axes.
	constexpr int DIRECTION_INDICES = DIRECTIONS * 2;
	const int LAST_EDGES = EDGES_PER_DIRECTION - 1;
	const int LAST_VERTEX_INDEX = EDGES_PER_DIRECTION * DIRECTIONS;
	// Build the mesh data arrays for the marker.
	PackedVector4Array vertices;
	vertices.resize(LAST_VERTEX_INDEX + 1);
	int vertex_index = 0;
	for (int edge = 0; edge < EDGES_PER_DIRECTION; edge++) {
		const real_t extent = _marker_extents / (1ull << uint64_t(edge));
		for (int axis = 0; axis < 4; axis++) {
			Vector4 vertex = Vector4(0, 0, 0, 0);
			vertex[axis] = extent;
			vertices.set(vertex_index++, vertex);
			vertex[axis] = -extent;
			vertices.set(vertex_index++, vertex);
		}
	}
	vertices.set(LAST_VERTEX_INDEX, Vector4(0, 0, 0, 0));
	PackedInt32Array edge_indices;
	edge_indices.resize(EDGES_PER_DIRECTION * DIRECTION_INDICES);
	int edge_index = 0;
	for (int edge = 0; edge < EDGES_PER_DIRECTION; edge++) {
		for (int dir = 0; dir < DIRECTIONS; dir++) {
			edge_indices.set(edge_index++, dir + DIRECTIONS * edge);
			if (edge == LAST_EDGES) {
				edge_indices.set(edge_index++, LAST_VERTEX_INDEX);
			} else {
				edge_indices.set(edge_index++, dir + DIRECTIONS * (edge + 1));
			}
		}
	}
	PackedColorArray albedo_colors;
	albedo_colors.resize(EDGES_PER_DIRECTION * DIRECTIONS);
	// Marker4D can be used at runtime, so it can't grab colors from the editor theme.
	const Color axis_x_color = Vector4D::axis_color(0);
	const Color axis_y_color = Vector4D::axis_color(1);
	const Color axis_z_color = Vector4D::axis_color(2);
	const Color axis_w_color = Vector4D::axis_color(3);
	const Color neg_x_color = axis_x_color.darkened(_darken_negative_amount);
	const Color neg_y_color = axis_y_color.darkened(_darken_negative_amount);
	const Color neg_z_color = axis_z_color.darkened(_darken_negative_amount);
	const Color neg_w_color = axis_w_color.darkened(_darken_negative_amount);
	int color_index = 0;
	for (int edge = 0; edge < EDGES_PER_DIRECTION; edge++) {
		albedo_colors.set(color_index++, axis_x_color);
		albedo_colors.set(color_index++, neg_x_color);
		albedo_colors.set(color_index++, axis_y_color);
		albedo_colors.set(color_index++, neg_y_color);
		albedo_colors.set(color_index++, axis_z_color);
		albedo_colors.set(color_index++, neg_z_color);
		albedo_colors.set(color_index++, axis_w_color);
		albedo_colors.set(color_index++, neg_w_color);
	}
	// Set up the mesh and material with the generated data.
	Ref<WireMaterial4D> material;
	material.instantiate();
	material->set_albedo_source(WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_ONLY);
	material->set_albedo_color_array(albedo_colors);
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(vertices);
	mesh->set_edge_indices(edge_indices);
	mesh->set_material(material);
	set_mesh(mesh);
}

void Marker4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_marker_extents"), &Marker4D::get_marker_extents);
	ClassDB::bind_method(D_METHOD("set_marker_extents", "extents"), &Marker4D::set_marker_extents);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "marker_extents"), "set_marker_extents", "get_marker_extents");

	ClassDB::bind_method(D_METHOD("get_darken_negative_amount"), &Marker4D::get_darken_negative_amount);
	ClassDB::bind_method(D_METHOD("set_darken_negative_amount", "darken_amount"), &Marker4D::set_darken_negative_amount);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "darken_negative_amount", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_darken_negative_amount", "get_darken_negative_amount");

	ClassDB::bind_method(D_METHOD("get_runtime_behavior"), &Marker4D::get_runtime_behavior);
	ClassDB::bind_method(D_METHOD("set_runtime_behavior", "behavior"), &Marker4D::set_runtime_behavior);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "runtime_behavior", PROPERTY_HINT_ENUM, "Show,Hide,Delete"), "set_runtime_behavior", "get_runtime_behavior");

	ClassDB::bind_method(D_METHOD("get_subdivisions"), &Marker4D::get_subdivisions);
	ClassDB::bind_method(D_METHOD("set_subdivisions", "subdivisions"), &Marker4D::set_subdivisions);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "subdivisions", PROPERTY_HINT_RANGE, "1,32,1"), "set_subdivisions", "get_subdivisions");

	BIND_ENUM_CONSTANT(RUNTIME_BEHAVIOR_SHOW);
	BIND_ENUM_CONSTANT(RUNTIME_BEHAVIOR_HIDE);
	BIND_ENUM_CONSTANT(RUNTIME_BEHAVIOR_DELETE);
}
