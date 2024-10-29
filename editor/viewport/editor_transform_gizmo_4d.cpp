#include "editor_transform_gizmo_4d.h"

#include "editor_main_viewport_4d.h"

#include "../../math/geometry_4d.h"
#include "../../math/plane_4d.h"
#include "../../math/vector_4d.h"
#include "../../mesh/wire/array_wire_mesh_4d.h"
#include "../../mesh/wire/box_wire_mesh_4d.h"
#include "../../mesh/wire/wire_material_4d.h"
#include "../../mesh/wire/wire_mesh_builder_4d.h"
#include "../../render/rendering_server_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#elif GODOT_MODULE
#include "editor/editor_inspector.h"
#include "editor/editor_interface.h"
#endif

constexpr real_t LINE_THICKNESS = 3.0;
constexpr int MOVE_ARROW_SUBDIVISIONS = 4;
constexpr real_t MOVE_ARROW_TIP_POSITION = 0.125;
constexpr real_t MOVE_ARROW_RADIUS = 0.0625;
constexpr int ROTATION_RING_SEGMENTS = 64; // Should be a multiple of 8 for correct coloration.
constexpr int SCALE_BOX_SUBDIVISIONS = 3;
constexpr real_t SCALE_BOX_RADIUS = 0.1;
constexpr int PLANE_EDGES = 6; // Must match the data in _make_plane_wire_mesh.
constexpr real_t PLANE_OFFSET = 0.3;
constexpr real_t PLANE_RADIUS = 0.08;

static_assert(MOVE_ARROW_SUBDIVISIONS > 1);
static_assert(ROTATION_RING_SEGMENTS % 8 == 0);
static_assert(SCALE_BOX_SUBDIVISIONS > 1);

Ref<WireMaterial4D> _make_single_color_wire_material(const Color &p_color) {
	Ref<WireMaterial4D> mat;
	mat.instantiate();
	mat->set_line_thickness(LINE_THICKNESS);
	mat->set_albedo_source(WireMaterial4D::WIRE_COLOR_SOURCE_SINGLE_COLOR);
	mat->set_albedo_color(p_color);
	return mat;
}

Ref<WireMaterial4D> _make_plane_material(const Color &p_first_color, const Color &p_second_color) {
	Ref<WireMaterial4D> mat;
	mat.instantiate();
	mat->set_line_thickness(LINE_THICKNESS);
	mat->set_albedo_source(WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_ONLY);
	PackedColorArray colors;
	for (int i = 0; i < PLANE_EDGES; i++) {
		if (i < PLANE_EDGES / 2) {
			colors.push_back(p_first_color);
		} else {
			colors.push_back(p_second_color);
		}
	}
	mat->set_albedo_color_array(colors);
	return mat;
}

Ref<WireMaterial4D> _make_rotation_ring_material(const Color &p_first_color, const Color &p_second_color) {
	Ref<WireMaterial4D> mat;
	mat.instantiate();
	mat->set_albedo_source(WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_ONLY);
	PackedColorArray colors;
	for (int i = 0; i < ROTATION_RING_SEGMENTS; i++) {
		if (i < ROTATION_RING_SEGMENTS / 8) {
			colors.push_back(p_first_color);
		} else if (i < ROTATION_RING_SEGMENTS * 3 / 8) {
			colors.push_back(p_second_color);
		} else if (i < ROTATION_RING_SEGMENTS * 5 / 8) {
			colors.push_back(p_first_color);
		} else if (i < ROTATION_RING_SEGMENTS * 7 / 8) {
			colors.push_back(p_second_color);
		} else {
			colors.push_back(p_first_color);
		}
	}
	mat->set_albedo_color_array(colors);
	return mat;
}

Ref<ArrayWireMesh4D> _make_move_arrow_wire_mesh() {
	// First, create a 3D sphere for the base of the 4D arrow.
	// For symmetry between all axes, make a subdivided octahedron and normalize it.
	Ref<ArrayWireMesh4D> mesh = WireMeshBuilder4D::make_3d_orthoplex_sphere(MOVE_ARROW_RADIUS, MOVE_ARROW_SUBDIVISIONS, MOVE_ARROW_TIP_POSITION);
	PackedVector4Array vertices = mesh->get_vertices();
	PackedInt32Array edge_indices = mesh->get_edge_indices();
	const int octahedral_cone_vertex_count = vertices.size();
	vertices.append(Vector4(0.0, 0.0, 0.0, -1.0));
	edge_indices.push_back(octahedral_cone_vertex_count - 1);
	edge_indices.push_back(octahedral_cone_vertex_count);
	mesh->set_vertices(vertices);
	mesh->set_edge_indices(edge_indices);
	return mesh;
}

Ref<ArrayWireMesh4D> _make_rotation_ring_wire_mesh() {
	PackedVector4Array vertices;
	PackedInt32Array edge_indices;
	vertices.resize(ROTATION_RING_SEGMENTS);
	edge_indices.resize(ROTATION_RING_SEGMENTS * 2);
	vertices.set(0, Vector4(1.0, 0.0, 0.0, 0.0));
	edge_indices.set(0, 0);
	for (int i = 1; i < ROTATION_RING_SEGMENTS; i++) {
		const real_t angle = Math_TAU * i / ROTATION_RING_SEGMENTS;
		const real_t x = Math::cos(angle);
		const real_t y = Math::sin(angle);
		vertices.set(i, Vector4(x, y, 0.0, 0.0));
		edge_indices.set(i * 2 - 1, i);
		edge_indices.set(i * 2, i);
	}
	edge_indices.set(ROTATION_RING_SEGMENTS * 2 - 1, 0);
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(vertices);
	mesh->set_edge_indices(edge_indices);
	return mesh;
}

Ref<ArrayWireMesh4D> _make_scale_box_wire_mesh() {
	Ref<BoxWireMesh4D> box_mesh;
	box_mesh.instantiate();
	box_mesh->set_size(Vector4(SCALE_BOX_RADIUS, SCALE_BOX_RADIUS, SCALE_BOX_RADIUS, SCALE_BOX_RADIUS));
	const Vector4i subdiv_segments = Vector4i(SCALE_BOX_SUBDIVISIONS, SCALE_BOX_SUBDIVISIONS, SCALE_BOX_SUBDIVISIONS, SCALE_BOX_SUBDIVISIONS);
	Ref<ArrayWireMesh4D> mesh = box_mesh->subdivide(subdiv_segments);
	mesh->append_edge_points(Vector4(0.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0));
	return mesh;
}

Ref<ArrayWireMesh4D> _make_plane_wire_mesh() {
	// Must match constexpr int PLANE_SEGMENTS.
	PackedVector4Array vertices = {
		Vector4(-PLANE_RADIUS * 0.9, -PLANE_RADIUS, 0.0, 0.0), // First triangle lower left.
		Vector4(PLANE_RADIUS, -PLANE_RADIUS, 0.0, 0.0), // First triangle lower right.
		Vector4(PLANE_RADIUS, PLANE_RADIUS * 0.9, 0.0, 0.0), // First triangle upper right.
		Vector4(-PLANE_RADIUS, -PLANE_RADIUS * 0.9, 0.0, 0.0), // Second triangle lower left.
		Vector4(PLANE_RADIUS * 0.9, PLANE_RADIUS, 0.0, 0.0), // Second triangle upper right.
		Vector4(-PLANE_RADIUS, PLANE_RADIUS, 0.0, 0.0), // Second triangle upper left.
	};
	PackedInt32Array edge_indices = { 0, 1, 0, 2, 1, 2, 3, 4, 3, 5, 4, 5 };
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(vertices);
	mesh->set_edge_indices(edge_indices);
	return mesh;
}

Ref<ArrayWireMesh4D> _make_stretch_triplane_wire_mesh() {
	Ref<ArrayWireMesh4D> mesh = WireMeshBuilder4D::make_3d_subdivided_box(Vector3(SCALE_BOX_RADIUS, SCALE_BOX_RADIUS, SCALE_BOX_RADIUS), Vector3i(SCALE_BOX_SUBDIVISIONS, SCALE_BOX_SUBDIVISIONS, SCALE_BOX_SUBDIVISIONS));
	return mesh;
}

MeshInstance4D *EditorTransformGizmo4D::_make_mesh_instance(const StringName &p_name, const Ref<ArrayWireMesh4D> &p_mesh, const Ref<WireMaterial4D> &p_material) {
	MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
	mesh_instance->set_name(p_name);
	mesh_instance->set_mesh(p_mesh);
	mesh_instance->set_material_override(p_material);
	mesh_instance->set_meta(StringName("original_material"), p_material);
	_mesh_holder->add_child(mesh_instance);
	return mesh_instance;
}

void EditorTransformGizmo4D::_generate_gizmo_meshes(const PackedColorArray &p_axis_colors) {
	// TODO: Support tetrahedral gizmo meshes (we need a rendering engine for that first to test with).
	_are_generated_meshes_wireframes = true;
	Ref<WireMaterial4D> x_material = _make_single_color_wire_material(p_axis_colors[0]);
	Ref<WireMaterial4D> y_material = _make_single_color_wire_material(p_axis_colors[1]);
	Ref<WireMaterial4D> z_material = _make_single_color_wire_material(p_axis_colors[2]);
	Ref<WireMaterial4D> w_material = _make_single_color_wire_material(p_axis_colors[3]);
	// Create the move arrow meshes.
	Ref<ArrayWireMesh4D> move_arrow_mesh = _make_move_arrow_wire_mesh();
	_meshes[TRANSFORM_MOVE_X] = _make_mesh_instance(StringName("MoveArrowX"), move_arrow_mesh, x_material);
	_meshes[TRANSFORM_MOVE_X]->set_basis(Basis4D(Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0)));
	_meshes[TRANSFORM_MOVE_X]->set_position(Vector4(1.0, 0.0, 0.0, 0.0));
	_meshes[TRANSFORM_MOVE_Y] = _make_mesh_instance(StringName("MoveArrowY"), move_arrow_mesh, y_material);
	_meshes[TRANSFORM_MOVE_Y]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0)));
	_meshes[TRANSFORM_MOVE_Y]->set_position(Vector4(0.0, 1.0, 0.0, 0.0));
	_meshes[TRANSFORM_MOVE_Z] = _make_mesh_instance(StringName("MoveArrowZ"), move_arrow_mesh, z_material);
	_meshes[TRANSFORM_MOVE_Z]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 0.0, 1.0, 0.0)));
	_meshes[TRANSFORM_MOVE_Z]->set_position(Vector4(0.0, 0.0, 1.0, 0.0));
	_meshes[TRANSFORM_MOVE_W] = _make_mesh_instance(StringName("MoveArrowW"), move_arrow_mesh, w_material);
	_meshes[TRANSFORM_MOVE_W]->set_position(Vector4(0.0, 0.0, 0.0, 1.0));
	// Create the plane meshes.
	Ref<ArrayWireMesh4D> plane_mesh = _make_plane_wire_mesh();
	_meshes[TRANSFORM_MOVE_XY] = _make_mesh_instance(StringName("PlaneXY"), plane_mesh, _make_plane_material(p_axis_colors[0], p_axis_colors[1]));
	_meshes[TRANSFORM_MOVE_XY]->set_position(Vector4(PLANE_OFFSET, PLANE_OFFSET, 0.0, 0.0));
	_meshes[TRANSFORM_MOVE_XZ] = _make_mesh_instance(StringName("PlaneXZ"), plane_mesh, _make_plane_material(p_axis_colors[0], p_axis_colors[2]));
	_meshes[TRANSFORM_MOVE_XZ]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, -1.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0)));
	_meshes[TRANSFORM_MOVE_XZ]->set_position(Vector4(PLANE_OFFSET, 0.0, PLANE_OFFSET, 0.0));
	_meshes[TRANSFORM_MOVE_XW] = _make_mesh_instance(StringName("PlaneXW"), plane_mesh, _make_plane_material(p_axis_colors[0], p_axis_colors[3]));
	_meshes[TRANSFORM_MOVE_XW]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, -1.0, 0.0, 0.0)));
	_meshes[TRANSFORM_MOVE_XW]->set_position(Vector4(PLANE_OFFSET, 0.0, 0.0, PLANE_OFFSET));
	_meshes[TRANSFORM_MOVE_YZ] = _make_mesh_instance(StringName("PlaneYZ"), plane_mesh, _make_plane_material(p_axis_colors[2], p_axis_colors[1]));
	_meshes[TRANSFORM_MOVE_YZ]->set_basis(Basis4D(Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(-1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0)));
	_meshes[TRANSFORM_MOVE_YZ]->set_position(Vector4(0.0, PLANE_OFFSET, PLANE_OFFSET, 0.0));
	_meshes[TRANSFORM_MOVE_YW] = _make_mesh_instance(StringName("PlaneYW"), plane_mesh, _make_plane_material(p_axis_colors[3], p_axis_colors[1]));
	_meshes[TRANSFORM_MOVE_YW]->set_basis(Basis4D(Vector4(0.0, 0.0, 0.0, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(-1.0, 0.0, 0.0, 0.0)));
	_meshes[TRANSFORM_MOVE_YW]->set_position(Vector4(0.0, PLANE_OFFSET, 0.0, PLANE_OFFSET));
	_meshes[TRANSFORM_MOVE_ZW] = _make_mesh_instance(StringName("PlaneZW"), plane_mesh, _make_plane_material(p_axis_colors[2], p_axis_colors[3]));
	_meshes[TRANSFORM_MOVE_ZW]->set_basis(Basis4D(Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0), Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0)));
	_meshes[TRANSFORM_MOVE_ZW]->set_position(Vector4(0.0, 0.0, PLANE_OFFSET, PLANE_OFFSET));
	// Create the rotation ring meshes.
	Ref<ArrayWireMesh4D> rotation_ring_mesh = _make_rotation_ring_wire_mesh();
	_meshes[TRANSFORM_ROTATE_XY] = _make_mesh_instance(StringName("RotationRingXY"), rotation_ring_mesh, _make_rotation_ring_material(p_axis_colors[1], p_axis_colors[0]));
	_meshes[TRANSFORM_ROTATE_XZ] = _make_mesh_instance(StringName("RotationRingXZ"), rotation_ring_mesh, _make_rotation_ring_material(p_axis_colors[2], p_axis_colors[0]));
	_meshes[TRANSFORM_ROTATE_XZ]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, -1.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0)));
	_meshes[TRANSFORM_ROTATE_XW] = _make_mesh_instance(StringName("RotationRingXW"), rotation_ring_mesh, _make_rotation_ring_material(p_axis_colors[3], p_axis_colors[0]));
	_meshes[TRANSFORM_ROTATE_XW]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, -1.0, 0.0, 0.0)));
	_meshes[TRANSFORM_ROTATE_YZ] = _make_mesh_instance(StringName("RotationRingYZ"), rotation_ring_mesh, _make_rotation_ring_material(p_axis_colors[1], p_axis_colors[2]));
	_meshes[TRANSFORM_ROTATE_YZ]->set_basis(Basis4D(Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(-1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0)));
	_meshes[TRANSFORM_ROTATE_YW] = _make_mesh_instance(StringName("RotationRingYW"), rotation_ring_mesh, _make_rotation_ring_material(p_axis_colors[1], p_axis_colors[3]));
	_meshes[TRANSFORM_ROTATE_YW]->set_basis(Basis4D(Vector4(0.0, 0.0, 0.0, 1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(-1.0, 0.0, 0.0, 0.0)));
	_meshes[TRANSFORM_ROTATE_ZW] = _make_mesh_instance(StringName("RotationRingZW"), rotation_ring_mesh, _make_rotation_ring_material(p_axis_colors[3], p_axis_colors[2]));
	_meshes[TRANSFORM_ROTATE_ZW]->set_basis(Basis4D(Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0), Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0)));
	// Create the scale box meshes.
	Ref<ArrayWireMesh4D> scale_box_mesh = _make_scale_box_wire_mesh();
	_meshes[TRANSFORM_SCALE_X] = _make_mesh_instance(StringName("ScaleBoxX"), scale_box_mesh, x_material);
	_meshes[TRANSFORM_SCALE_X]->set_basis(Basis4D(Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0)));
	_meshes[TRANSFORM_SCALE_X]->set_position(Vector4(1.0, 0.0, 0.0, 0.0));
	_meshes[TRANSFORM_SCALE_Y] = _make_mesh_instance(StringName("ScaleBoxY"), scale_box_mesh, y_material);
	_meshes[TRANSFORM_SCALE_Y]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0)));
	_meshes[TRANSFORM_SCALE_Y]->set_position(Vector4(0.0, 1.0, 0.0, 0.0));
	_meshes[TRANSFORM_SCALE_Z] = _make_mesh_instance(StringName("ScaleBoxZ"), scale_box_mesh, z_material);
	_meshes[TRANSFORM_SCALE_Z]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 0.0, 1.0, 0.0)));
	_meshes[TRANSFORM_SCALE_Z]->set_position(Vector4(0.0, 0.0, 1.0, 0.0));
	_meshes[TRANSFORM_SCALE_W] = _make_mesh_instance(StringName("ScaleBoxW"), scale_box_mesh, w_material);
	_meshes[TRANSFORM_SCALE_W]->set_position(Vector4(0.0, 0.0, 0.0, 1.0));
	// The scale plane meshes are the same as the move plane meshes.
	_meshes[TRANSFORM_SCALE_XY] = _meshes[TRANSFORM_MOVE_XY];
	_meshes[TRANSFORM_SCALE_XZ] = _meshes[TRANSFORM_MOVE_XZ];
	_meshes[TRANSFORM_SCALE_XW] = _meshes[TRANSFORM_MOVE_XW];
	_meshes[TRANSFORM_SCALE_YZ] = _meshes[TRANSFORM_MOVE_YZ];
	_meshes[TRANSFORM_SCALE_YW] = _meshes[TRANSFORM_MOVE_YW];
	_meshes[TRANSFORM_SCALE_ZW] = _meshes[TRANSFORM_MOVE_ZW];
	// Create the stretch triplane meshes.
	Ref<ArrayWireMesh4D> stretch_triplane_mesh = _make_stretch_triplane_wire_mesh();
	_meshes[TRANSFORM_STRETCH_POS_X] = _make_mesh_instance(StringName("StretchTriplanePosX"), stretch_triplane_mesh, x_material);
	_meshes[TRANSFORM_STRETCH_POS_X]->set_basis(Basis4D(Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0)));
	_meshes[TRANSFORM_STRETCH_POS_X]->set_position(Vector4(1.0, 0.0, 0.0, 0.0));
	_meshes[TRANSFORM_STRETCH_NEG_X] = _make_mesh_instance(StringName("StretchTriplaneNegX"), stretch_triplane_mesh, x_material);
	_meshes[TRANSFORM_STRETCH_NEG_X]->set_basis(Basis4D(Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0)));
	_meshes[TRANSFORM_STRETCH_NEG_X]->set_position(Vector4(-1.0, 0.0, 0.0, 0.0));
	_meshes[TRANSFORM_STRETCH_POS_Y] = _make_mesh_instance(StringName("StretchTriplanePosY"), stretch_triplane_mesh, y_material);
	_meshes[TRANSFORM_STRETCH_POS_Y]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0)));
	_meshes[TRANSFORM_STRETCH_POS_Y]->set_position(Vector4(0.0, 1.0, 0.0, 0.0));
	_meshes[TRANSFORM_STRETCH_NEG_Y] = _make_mesh_instance(StringName("StretchTriplaneNegY"), stretch_triplane_mesh, y_material);
	_meshes[TRANSFORM_STRETCH_NEG_Y]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0)));
	_meshes[TRANSFORM_STRETCH_NEG_Y]->set_position(Vector4(0.0, -1.0, 0.0, 0.0));
	_meshes[TRANSFORM_STRETCH_POS_Z] = _make_mesh_instance(StringName("StretchTriplanePosZ"), stretch_triplane_mesh, z_material);
	_meshes[TRANSFORM_STRETCH_POS_Z]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 0.0, 1.0, 0.0)));
	_meshes[TRANSFORM_STRETCH_POS_Z]->set_position(Vector4(0.0, 0.0, 1.0, 0.0));
	_meshes[TRANSFORM_STRETCH_NEG_Z] = _make_mesh_instance(StringName("StretchTriplaneNegZ"), stretch_triplane_mesh, z_material);
	_meshes[TRANSFORM_STRETCH_NEG_Z]->set_basis(Basis4D(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, -1.0), Vector4(0.0, 0.0, 1.0, 0.0)));
	_meshes[TRANSFORM_STRETCH_NEG_Z]->set_position(Vector4(0.0, 0.0, -1.0, 0.0));
	_meshes[TRANSFORM_STRETCH_POS_W] = _make_mesh_instance(StringName("StretchTriplanePosW"), stretch_triplane_mesh, w_material);
	_meshes[TRANSFORM_STRETCH_POS_W]->set_position(Vector4(0.0, 0.0, 0.0, 1.0));
	_meshes[TRANSFORM_STRETCH_NEG_W] = _make_mesh_instance(StringName("StretchTriplaneNegW"), stretch_triplane_mesh, w_material);
	_meshes[TRANSFORM_STRETCH_NEG_W]->set_position(Vector4(0.0, 0.0, 0.0, -1.0));
	// Set the gizmo mode to set the visibility of these new meshes.
	set_gizmo_mode(GizmoMode::SELECT);
}

void EditorTransformGizmo4D::_on_rendering_server_pre_render(Camera4D *p_camera, Viewport *p_viewport, RenderingEngine4D *p_rendering_engine) {
	_update_gizmo_mesh_transform(p_camera);
}

void EditorTransformGizmo4D::_on_editor_inspector_property_edited(const String &p_prop) {
	_update_gizmo_transform();
}

void EditorTransformGizmo4D::_on_undo_redo_version_changed() {
	_update_gizmo_transform();
}

void EditorTransformGizmo4D::_update_gizmo_transform() {
	int transform_count = 0;
	Transform4D sum_transform = Transform4D(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	const int size = _selected_top_nodes.size();
	for (int i = 0; i < size; i++) {
		Node4D *node_4d = Object::cast_to<Node4D>(_selected_top_nodes[i]);
		if (node_4d != nullptr) {
			Transform4D global_transform = node_4d->get_global_transform();
			_selected_top_node_old_transforms.set(i, global_transform);
			sum_transform += global_transform;
			transform_count++;
		}
	}
	if (transform_count == 0) {
		set_visible(false);
	} else {
		set_visible(true);
		set_transform(sum_transform / real_t(transform_count));
	}
}

void EditorTransformGizmo4D::_update_gizmo_mesh_transform(const Camera4D *p_camera) {
	// We want to keep the gizmo the same size on the screen regardless of the camera's position.
	const Transform4D camera_transform = p_camera->get_global_transform();
	Vector4 global_position = get_position();
	Basis4D global_basis;
	if (_is_use_local_rotation) {
		global_basis = get_basis().normalized();
	}
	if (_is_stretch_enabled) {
		const Rect4 bounds = _get_rect_bounds_of_selection(Transform4D(global_basis, global_position).inverse());
		global_position += global_basis * bounds.get_center();
		global_basis.scale_local(bounds.size * 0.5f);
		const real_t determinant = bounds.size.x * bounds.size.y * bounds.size.z * bounds.size.w;
		set_visible(!Math::is_zero_approx(determinant));
	} else {
		real_t scale;
		if (p_camera->get_projection_type() == Camera4D::PROJECTION4D_ORTHOGRAPHIC) {
			scale = p_camera->get_orthographic_size() * 0.4f;
		} else {
			scale = global_position.distance_to(camera_transform.origin) * 0.4f;
		}
		global_basis *= scale;
	}
	_mesh_holder->set_global_transform(Transform4D(global_basis, global_position));
}

Rect4 EditorTransformGizmo4D::_get_rect_bounds_of_selection(const Transform4D &p_inv_relative_to) const {
	Rect4 bounds;
	const int size = _selected_top_nodes.size();
	for (int i = 0; i < size; i++) {
		Node4D *node_4d = Object::cast_to<Node4D>(_selected_top_nodes[i]);
		if (node_4d != nullptr) {
			bounds = bounds.merge(node_4d->get_rect_bounds_recursive(p_inv_relative_to));
		}
	}
	return bounds;
}

Vector4 _origin_axis_aligned_biplane_raycast(const Vector4 &p_ray_origin, const Vector4 &p_ray_direction, const Vector4 &p_axis1, const Vector4 &p_axis2, const Vector4 &p_perp, const bool correct_for_ring) {
	const Plane4D plane = Plane4D::from_coplanar_directions(p_axis1, p_axis2, p_perp);
	if (plane.normal == Vector4()) {
		return Vector4();
	}
	const Variant intersection = plane.intersect_ray(p_ray_origin, p_ray_direction);
	if (intersection.get_type() != Variant::VECTOR4) {
		return Vector4();
	}
	Vector4 intersection_adjusted = intersection;
	// I don't know why this works, but it does. It was found by trial and error.
	if (correct_for_ring) {
		const Vector4 biplane_axes = p_axis1 + p_axis2;
		if (biplane_axes.x == 0.0f && !Math::is_zero_approx(p_perp.x)) {
			intersection_adjusted.x /= p_perp.x;
		}
		if (biplane_axes.y == 0.0f && !Math::is_zero_approx(p_perp.y)) {
			intersection_adjusted.y /= p_perp.y;
		}
		if (biplane_axes.z == 0.0f && !Math::is_zero_approx(p_perp.z)) {
			intersection_adjusted.z /= p_perp.z;
		}
		if (biplane_axes.w == 0.0f && !Math::is_zero_approx(p_perp.w)) {
			intersection_adjusted.w /= p_perp.w;
		}
	}
	return intersection_adjusted;
}

EditorTransformGizmo4D::TransformPart EditorTransformGizmo4D::_check_for_best_hit(const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction) const {
	Vector4 aligned;
	real_t current_distance;
	real_t closest_distance = 0.1;
	TransformPart closest_part = TRANSFORM_NONE;
	// Check arrows.
	if (_is_move_linear_enabled || _is_scale_linear_enabled) {
		PackedVector4Array current_points;
#define CHECK_ARROW(m_axis, m_part)                                                                                                                        \
	aligned = Vector4D::slide(m_axis, p_local_perp_direction);                                                                                             \
	current_points = Geometry4D::closest_points_between_line_and_segment(p_local_ray_origin, p_local_ray_direction, Vector4(0.0, 0.0, 0.0, 0.0), aligned); \
	current_distance = current_points[0].distance_to(current_points[1]);                                                                                   \
	if (current_distance < closest_distance) {                                                                                                             \
		closest_distance = current_distance;                                                                                                               \
		closest_part = m_part;                                                                                                                             \
	}
		CHECK_ARROW(Vector4(1.0, 0.0, 0.0, 0.0), _is_move_linear_enabled ? TRANSFORM_MOVE_X : TRANSFORM_SCALE_X);
		CHECK_ARROW(Vector4(0.0, 1.0, 0.0, 0.0), _is_move_linear_enabled ? TRANSFORM_MOVE_Y : TRANSFORM_SCALE_Y);
		CHECK_ARROW(Vector4(0.0, 0.0, 1.0, 0.0), _is_move_linear_enabled ? TRANSFORM_MOVE_Z : TRANSFORM_SCALE_Z);
		CHECK_ARROW(Vector4(0.0, 0.0, 0.0, 1.0), _is_move_linear_enabled ? TRANSFORM_MOVE_W : TRANSFORM_SCALE_W);
#undef CHECK_ARROW
	}
	Vector4 current_point;
#define CHECK_POINT(m_point, m_part)                                                                       \
	aligned = Vector4D::slide(m_point, p_local_perp_direction);                                            \
	current_point = Geometry4D::closest_point_on_line(p_local_ray_origin, p_local_ray_direction, aligned); \
	current_distance = current_point.distance_to(aligned);                                                 \
	if (current_distance < closest_distance) {                                                             \
		closest_distance = current_distance;                                                               \
		closest_part = m_part;                                                                             \
	}
	// Check planes.
	if (_is_move_planar_enabled || _is_scale_planar_enabled) {
		CHECK_POINT(Vector4(PLANE_OFFSET, PLANE_OFFSET, 0.0, 0.0), _is_move_planar_enabled ? TRANSFORM_MOVE_XY : TRANSFORM_SCALE_XY);
		CHECK_POINT(Vector4(PLANE_OFFSET, 0.0, PLANE_OFFSET, 0.0), _is_move_planar_enabled ? TRANSFORM_MOVE_XZ : TRANSFORM_SCALE_XZ);
		CHECK_POINT(Vector4(PLANE_OFFSET, 0.0, 0.0, PLANE_OFFSET), _is_move_planar_enabled ? TRANSFORM_MOVE_XW : TRANSFORM_SCALE_XW);
		CHECK_POINT(Vector4(0.0, PLANE_OFFSET, PLANE_OFFSET, 0.0), _is_move_planar_enabled ? TRANSFORM_MOVE_YZ : TRANSFORM_SCALE_YZ);
		CHECK_POINT(Vector4(0.0, PLANE_OFFSET, 0.0, PLANE_OFFSET), _is_move_planar_enabled ? TRANSFORM_MOVE_YW : TRANSFORM_SCALE_YW);
		CHECK_POINT(Vector4(0.0, 0.0, PLANE_OFFSET, PLANE_OFFSET), _is_move_planar_enabled ? TRANSFORM_MOVE_ZW : TRANSFORM_SCALE_ZW);
	}
	// Check stretch.
	if (_is_stretch_enabled) {
		CHECK_POINT(Vector4(+1.0, 0.0, 0.0, 0.0), TRANSFORM_STRETCH_POS_X);
		CHECK_POINT(Vector4(-1.0, 0.0, 0.0, 0.0), TRANSFORM_STRETCH_NEG_X);
		CHECK_POINT(Vector4(0.0, +1.0, 0.0, 0.0), TRANSFORM_STRETCH_POS_Y);
		CHECK_POINT(Vector4(0.0, -1.0, 0.0, 0.0), TRANSFORM_STRETCH_NEG_Y);
		CHECK_POINT(Vector4(0.0, 0.0, +1.0, 0.0), TRANSFORM_STRETCH_POS_Z);
		CHECK_POINT(Vector4(0.0, 0.0, -1.0, 0.0), TRANSFORM_STRETCH_NEG_Z);
		CHECK_POINT(Vector4(0.0, 0.0, 0.0, +1.0), TRANSFORM_STRETCH_POS_W);
		CHECK_POINT(Vector4(0.0, 0.0, 0.0, -1.0), TRANSFORM_STRETCH_NEG_W);
	}
#undef CHECK_POINT
	// Check rotation rings.
	if (_is_rotation_enabled) {
#define CHECK_RING(m_axis1, m_axis2, m_part)                                                                                                         \
	current_point = _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, m_axis1, m_axis2, p_local_perp_direction, true); \
	current_distance = Math::abs(current_point.length() - 1.0);                                                                                      \
	if (current_distance < closest_distance) {                                                                                                       \
		closest_distance = current_distance;                                                                                                         \
		closest_part = m_part;                                                                                                                       \
	}
		CHECK_RING(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 1.0, 0.0, 0.0), TRANSFORM_ROTATE_XY);
		CHECK_RING(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), TRANSFORM_ROTATE_XZ);
		CHECK_RING(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0), TRANSFORM_ROTATE_XW);
		CHECK_RING(Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), TRANSFORM_ROTATE_YZ);
		CHECK_RING(Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0), TRANSFORM_ROTATE_YW);
		CHECK_RING(Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0), TRANSFORM_ROTATE_ZW);
	}
#undef CHECK_RING
	return closest_part;
}

void EditorTransformGizmo4D::_unhiglight_mesh(TransformPart p_transformation) {
	if (p_transformation == TRANSFORM_NONE) {
		return;
	}
	MeshInstance4D *mesh_instance = _meshes[p_transformation];
	Ref<WireMaterial4D> material = mesh_instance->get_meta(StringName("original_material"));
	mesh_instance->set_material_override(material);
}

void EditorTransformGizmo4D::_highlight_mesh(TransformPart p_transformation) {
	if (p_transformation == TRANSFORM_NONE) {
		return;
	}
	MeshInstance4D *mesh_instance = _meshes[p_transformation];
	Ref<WireMaterial4D> material = mesh_instance->get_meta(StringName("original_material"));
	Ref<WireMaterial4D> highlight_material = material->duplicate(true);
	WireMaterial4D::WireColorSource source = highlight_material->get_albedo_source();
	if (source == WireMaterial4D::WIRE_COLOR_SOURCE_SINGLE_COLOR) {
		highlight_material->set_albedo_color(highlight_material->get_albedo_color().lightened(0.5));
	} else if (source == WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_ONLY) {
		PackedColorArray colors = highlight_material->get_albedo_color_array();
		for (int i = 0; i < colors.size(); i++) {
			colors.set(i, colors[i].lightened(0.5));
		}
		highlight_material->set_albedo_color_array(colors);
	}
	mesh_instance->set_material_override(highlight_material);
}

Variant EditorTransformGizmo4D::_get_transform_raycast_value(const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction) {
	switch (_current_transformation) {
		case TRANSFORM_NONE: {
			return Variant();
		} break;
		case TRANSFORM_MOVE_X:
		case TRANSFORM_SCALE_X:
		case TRANSFORM_STRETCH_POS_X:
		case TRANSFORM_STRETCH_NEG_X: {
			return Geometry4D::closest_point_between_lines(Vector4(), Vector4(1, 0, 0, 0), p_local_ray_origin, p_local_ray_direction);
		} break;
		case TRANSFORM_MOVE_Y:
		case TRANSFORM_SCALE_Y:
		case TRANSFORM_STRETCH_POS_Y:
		case TRANSFORM_STRETCH_NEG_Y: {
			return Geometry4D::closest_point_between_lines(Vector4(), Vector4(0, 1, 0, 0), p_local_ray_origin, p_local_ray_direction);
		} break;
		case TRANSFORM_MOVE_Z:
		case TRANSFORM_SCALE_Z:
		case TRANSFORM_STRETCH_POS_Z:
		case TRANSFORM_STRETCH_NEG_Z: {
			return Geometry4D::closest_point_between_lines(Vector4(), Vector4(0, 0, 1, 0), p_local_ray_origin, p_local_ray_direction);
		} break;
		case TRANSFORM_MOVE_W:
		case TRANSFORM_SCALE_W:
		case TRANSFORM_STRETCH_POS_W:
		case TRANSFORM_STRETCH_NEG_W: {
			return Geometry4D::closest_point_between_lines(Vector4(), Vector4(0, 0, 0, 1), p_local_ray_origin, p_local_ray_direction);
		} break;
		case TRANSFORM_MOVE_XY:
		case TRANSFORM_SCALE_XY: {
			return _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), p_local_perp_direction, false);
		} break;
		case TRANSFORM_MOVE_XZ:
		case TRANSFORM_SCALE_XZ: {
			return _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(1, 0, 0, 0), Vector4(0, 0, 1, 0), p_local_perp_direction, false);
		} break;
		case TRANSFORM_MOVE_XW:
		case TRANSFORM_SCALE_XW: {
			return _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 1), p_local_perp_direction, false);
		} break;
		case TRANSFORM_MOVE_YZ:
		case TRANSFORM_SCALE_YZ: {
			return _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), p_local_perp_direction, false);
		} break;
		case TRANSFORM_MOVE_YW:
		case TRANSFORM_SCALE_YW: {
			return _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 1), p_local_perp_direction, false);
		} break;
		case TRANSFORM_MOVE_ZW:
		case TRANSFORM_SCALE_ZW: {
			return _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1), p_local_perp_direction, false);
		} break;
		case TRANSFORM_ROTATE_XY: {
			Vector4 casted = _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), p_local_perp_direction, true);
			return Math::atan2(casted.y, casted.x);
		} break;
		case TRANSFORM_ROTATE_XZ: {
			Vector4 casted = _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(1, 0, 0, 0), Vector4(0, 0, 1, 0), p_local_perp_direction, true);
			return Math::atan2(casted.x, casted.z);
		} break;
		case TRANSFORM_ROTATE_XW: {
			Vector4 casted = _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 1), p_local_perp_direction, true);
			return Math::atan2(casted.w, casted.x);
		} break;
		case TRANSFORM_ROTATE_YZ: {
			Vector4 casted = _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), p_local_perp_direction, true);
			return Math::atan2(casted.z, casted.y);
		} break;
		case TRANSFORM_ROTATE_YW: {
			Vector4 casted = _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 1), p_local_perp_direction, true);
			return Math::atan2(casted.y, casted.w);
		} break;
		case TRANSFORM_ROTATE_ZW: {
			Vector4 casted = _origin_axis_aligned_biplane_raycast(p_local_ray_origin, p_local_ray_direction, Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1), p_local_perp_direction, true);
			return Math::atan2(casted.w, casted.z);
		} break;
		case TRANSFORM_MAX: {
			return Variant();
		} break;
	}
	return Variant();
}

void EditorTransformGizmo4D::_begin_transformation(const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction) {
	_old_transform = _mesh_holder->get_global_transform();
	_transform_reference_value = _get_transform_raycast_value(p_local_ray_origin, p_local_ray_direction, p_local_perp_direction);
	_selected_top_node_old_transforms.resize(_selected_top_nodes.size());
	for (int i = 0; i < _selected_top_nodes.size(); i++) {
		Node4D *node_4d = Object::cast_to<Node4D>(_selected_top_nodes[i]);
		if (node_4d != nullptr) {
			_selected_top_node_old_transforms.set(i, node_4d->get_global_transform());
		}
	}
}

void EditorTransformGizmo4D::_end_transformation() {
	if (_current_transformation == TRANSFORM_NONE) {
		return;
	}
	// Create an undo/redo action for the transformation.
	const bool is_move_only = _current_transformation >= TRANSFORM_MOVE_X && _current_transformation <= TRANSFORM_MOVE_ZW;
	undo_redo->create_action(is_move_only ? String("Move 4D nodes with gizmo") : String("Transform 4D nodes with gizmo"));
	const int size = _selected_top_nodes.size();
	for (int i = 0; i < size; i++) {
		Node4D *node_4d = Object::cast_to<Node4D>(_selected_top_nodes[i]);
		if (node_4d != nullptr) {
			undo_redo->add_do_property(node_4d, StringName("global_position"), node_4d->get_global_position());
			undo_redo->add_undo_property(node_4d, StringName("global_position"), _selected_top_node_old_transforms[i].origin);
			if (!is_move_only) {
				undo_redo->add_do_property(node_4d, StringName("global_basis"), (Projection)node_4d->get_global_basis());
				undo_redo->add_undo_property(node_4d, StringName("global_basis"), (Projection)_selected_top_node_old_transforms[i].basis);
			}
		}
	}
	undo_redo->commit_action(false);
	// Clear out the transformation data and mark the scene as unsaved.
	_old_transform = Transform4D();
	_transform_reference_value = Variant();
	_current_transformation = TRANSFORM_NONE;
	EditorInterface::get_singleton()->mark_scene_as_unsaved();
}

void EditorTransformGizmo4D::_process_transform(const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction) {
	Transform4D transform_change;
	Variant casted = _get_transform_raycast_value(p_local_ray_origin, p_local_ray_direction, p_local_perp_direction);
	switch (_current_transformation) {
		case TRANSFORM_NONE: {
			// Do nothing.
		} break;
		case TRANSFORM_MOVE_X:
		case TRANSFORM_MOVE_Y:
		case TRANSFORM_MOVE_Z:
		case TRANSFORM_MOVE_W:
		case TRANSFORM_MOVE_XY:
		case TRANSFORM_MOVE_XZ:
		case TRANSFORM_MOVE_XW:
		case TRANSFORM_MOVE_YZ:
		case TRANSFORM_MOVE_YW:
		case TRANSFORM_MOVE_ZW: {
			transform_change.origin = Vector4(casted) - Vector4(_transform_reference_value);
		} break;
		case TRANSFORM_ROTATE_XY: {
			transform_change.basis = Basis4D::from_xy(real_t(casted) - real_t(_transform_reference_value));
		} break;
		case TRANSFORM_ROTATE_XZ: {
			transform_change.basis = Basis4D::from_zx(real_t(casted) - real_t(_transform_reference_value));
		} break;
		case TRANSFORM_ROTATE_XW: {
			transform_change.basis = Basis4D::from_xw(real_t(casted) - real_t(_transform_reference_value));
		} break;
		case TRANSFORM_ROTATE_YZ: {
			transform_change.basis = Basis4D::from_yz(real_t(casted) - real_t(_transform_reference_value));
		} break;
		case TRANSFORM_ROTATE_YW: {
			transform_change.basis = Basis4D::from_wy(real_t(casted) - real_t(_transform_reference_value));
		} break;
		case TRANSFORM_ROTATE_ZW: {
			transform_change.basis = Basis4D::from_zw(real_t(casted) - real_t(_transform_reference_value));
		} break;
		case TRANSFORM_SCALE_X: {
			transform_change.basis.x.x = Vector4(casted).x / Vector4(_transform_reference_value).x;
		} break;
		case TRANSFORM_SCALE_Y: {
			transform_change.basis.y.y = Vector4(casted).y / Vector4(_transform_reference_value).y;
		} break;
		case TRANSFORM_SCALE_Z: {
			transform_change.basis.z.z = Vector4(casted).z / Vector4(_transform_reference_value).z;
		} break;
		case TRANSFORM_SCALE_W: {
			transform_change.basis.w.w = Vector4(casted).w / Vector4(_transform_reference_value).w;
		} break;
		case TRANSFORM_SCALE_XY: {
			transform_change.basis.x.x = Vector4(casted).x / Vector4(_transform_reference_value).x;
			transform_change.basis.y.y = Vector4(casted).y / Vector4(_transform_reference_value).y;
		} break;
		case TRANSFORM_SCALE_XZ: {
			transform_change.basis.x.x = Vector4(casted).x / Vector4(_transform_reference_value).x;
			transform_change.basis.z.z = Vector4(casted).z / Vector4(_transform_reference_value).z;
		} break;
		case TRANSFORM_SCALE_XW: {
			transform_change.basis.x.x = Vector4(casted).x / Vector4(_transform_reference_value).x;
			transform_change.basis.w.w = Vector4(casted).w / Vector4(_transform_reference_value).w;
		} break;
		case TRANSFORM_SCALE_YZ: {
			transform_change.basis.y.y = Vector4(casted).y / Vector4(_transform_reference_value).y;
			transform_change.basis.z.z = Vector4(casted).z / Vector4(_transform_reference_value).z;
		} break;
		case TRANSFORM_SCALE_YW: {
			transform_change.basis.y.y = Vector4(casted).y / Vector4(_transform_reference_value).y;
			transform_change.basis.w.w = Vector4(casted).w / Vector4(_transform_reference_value).w;
		} break;
		case TRANSFORM_SCALE_ZW: {
			transform_change.basis.z.z = Vector4(casted).z / Vector4(_transform_reference_value).z;
			transform_change.basis.w.w = Vector4(casted).w / Vector4(_transform_reference_value).w;
		} break;
		case TRANSFORM_STRETCH_POS_X: {
			const real_t stretch = Vector4(casted).x * 0.5f;
			transform_change.basis.x.x = stretch + 0.5f;
			transform_change.origin.x = stretch - 0.5f;
		} break;
		case TRANSFORM_STRETCH_NEG_X: {
			const real_t stretch = Vector4(casted).x * 0.5f;
			transform_change.basis.x.x = -stretch + 0.5f;
			transform_change.origin.x = stretch + 0.5f;
		} break;
		case TRANSFORM_STRETCH_POS_Y: {
			const real_t stretch = Vector4(casted).y * 0.5f;
			transform_change.basis.y.y = stretch + 0.5f;
			transform_change.origin.y = stretch - 0.5f;
		} break;
		case TRANSFORM_STRETCH_NEG_Y: {
			const real_t stretch = Vector4(casted).y * 0.5f;
			transform_change.basis.y.y = -stretch + 0.5f;
			transform_change.origin.y = stretch + 0.5f;
		} break;
		case TRANSFORM_STRETCH_POS_Z: {
			const real_t stretch = Vector4(casted).z * 0.5f;
			transform_change.basis.z.z = stretch + 0.5f;
			transform_change.origin.z = stretch - 0.5f;
		} break;
		case TRANSFORM_STRETCH_NEG_Z: {
			const real_t stretch = Vector4(casted).z * 0.5f;
			transform_change.basis.z.z = -stretch + 0.5f;
			transform_change.origin.z = stretch + 0.5f;
		} break;
		case TRANSFORM_STRETCH_POS_W: {
			const real_t stretch = Vector4(casted).w * 0.5f;
			transform_change.basis.w.w = stretch + 0.5f;
			transform_change.origin.w = stretch - 0.5f;
		} break;
		case TRANSFORM_STRETCH_NEG_W: {
			const real_t stretch = Vector4(casted).w * 0.5f;
			transform_change.basis.w.w = -stretch + 0.5f;
			transform_change.origin.w = stretch + 0.5f;
		} break;
		case TRANSFORM_MAX: {
			ERR_FAIL_MSG("Invalid transformation.");
		} break;
	}
	if (_keep_mode == KeepMode::CONFORMAL) {
		switch (_current_transformation) {
			case TRANSFORM_SCALE_X:
			case TRANSFORM_STRETCH_POS_X:
			case TRANSFORM_STRETCH_NEG_X: {
				transform_change.basis.y.y = transform_change.basis.x.x;
				transform_change.basis.z.z = transform_change.basis.x.x;
				transform_change.basis.w.w = transform_change.basis.x.x;
			} break;
			case TRANSFORM_SCALE_Y:
			case TRANSFORM_STRETCH_POS_Y:
			case TRANSFORM_STRETCH_NEG_Y: {
				transform_change.basis.x.x = transform_change.basis.y.y;
				transform_change.basis.z.z = transform_change.basis.y.y;
				transform_change.basis.w.w = transform_change.basis.y.y;
			} break;
			case TRANSFORM_SCALE_Z:
			case TRANSFORM_STRETCH_POS_Z:
			case TRANSFORM_STRETCH_NEG_Z: {
				transform_change.basis.x.x = transform_change.basis.z.z;
				transform_change.basis.y.y = transform_change.basis.z.z;
				transform_change.basis.w.w = transform_change.basis.z.z;
			} break;
			case TRANSFORM_SCALE_W:
			case TRANSFORM_STRETCH_POS_W:
			case TRANSFORM_STRETCH_NEG_W: {
				transform_change.basis.x.x = transform_change.basis.w.w;
				transform_change.basis.y.y = transform_change.basis.w.w;
				transform_change.basis.z.z = transform_change.basis.w.w;
			} break;
			default: {
				// Do nothing.
			} break;
		}
	}
	Transform4D new_transform = _old_transform * transform_change;
	set_transform(new_transform);
	// We want the global diff so we can apply it from the left on the global transform of all selected nodes.
	// Without this, the transforms would be relative to each node (ex: moving on X moves on each node's X axis).
	transform_change = _old_transform.transform_to(new_transform);
	for (int i = 0; i < _selected_top_nodes.size(); i++) {
		Node4D *node_4d = Object::cast_to<Node4D>(_selected_top_nodes[i]);
		if (node_4d != nullptr) {
			Transform4D new_node_transform = transform_change * _selected_top_node_old_transforms[i];
			switch (_keep_mode) {
				case KeepMode::FREEFORM: {
					// Do nothing.
				} break;
				case KeepMode::ORTHOGONAL: {
					new_node_transform.orthogonalize();
				} break;
				case KeepMode::CONFORMAL: {
					new_node_transform.conformalize();
				} break;
				case KeepMode::ORTHONORMAL: {
					new_node_transform.orthonormalize();
				} break;
			}
			node_4d->set_global_transform(new_node_transform);
		}
	}
}

void EditorTransformGizmo4D::selected_nodes_changed(const TypedArray<Node> &p_top_nodes) {
	_end_transformation();
	_selected_top_nodes = p_top_nodes;
	const int size = _selected_top_nodes.size();
	if (_selected_top_node_old_transforms.size() < size) {
		_selected_top_node_old_transforms.resize(size);
	}
	_update_gizmo_transform();
}

void EditorTransformGizmo4D::theme_changed(const PackedColorArray &p_axis_colors) {
	if (_meshes[TRANSFORM_MOVE_X] == nullptr) {
		_generate_gizmo_meshes(p_axis_colors);
	}
}

void EditorTransformGizmo4D::set_keep_mode(const KeepMode p_mode) {
	_keep_mode = p_mode;
	if (_keep_mode == KeepMode::CONFORMAL) {
		_is_scale_planar_enabled = false;
	} else if (_is_scale_linear_enabled) {
		_is_scale_planar_enabled = true;
	}
	const bool is_plane_visible = _is_move_planar_enabled || _is_scale_planar_enabled;
	_meshes[TRANSFORM_SCALE_XY]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_SCALE_XZ]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_SCALE_XW]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_SCALE_YZ]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_SCALE_YW]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_SCALE_ZW]->set_visible(is_plane_visible);
}

void EditorTransformGizmo4D::set_gizmo_mode(const GizmoMode p_mode) {
	switch (p_mode) {
		case GizmoMode::SELECT: {
			_is_move_linear_enabled = true;
			_is_move_planar_enabled = false;
			_is_rotation_enabled = false;
			_is_scale_linear_enabled = false;
			_is_scale_planar_enabled = false;
			_is_stretch_enabled = false;
		} break;
		case GizmoMode::MOVE: {
			_is_move_linear_enabled = true;
			_is_move_planar_enabled = true;
			_is_rotation_enabled = false;
			_is_scale_linear_enabled = false;
			_is_scale_planar_enabled = false;
			_is_stretch_enabled = false;
		} break;
		case GizmoMode::ROTATE: {
			_is_move_linear_enabled = false;
			_is_move_planar_enabled = false;
			_is_rotation_enabled = true;
			_is_scale_linear_enabled = false;
			_is_scale_planar_enabled = false;
			_is_stretch_enabled = false;
		} break;
		case GizmoMode::SCALE: {
			_is_move_linear_enabled = false;
			_is_move_planar_enabled = false;
			_is_rotation_enabled = false;
			_is_scale_linear_enabled = true;
			_is_scale_planar_enabled = _keep_mode != KeepMode::CONFORMAL;
			_is_stretch_enabled = false;
		} break;
		case GizmoMode::STRETCH: {
			_is_move_linear_enabled = false;
			_is_move_planar_enabled = false;
			_is_rotation_enabled = false;
			_is_scale_linear_enabled = false;
			_is_scale_planar_enabled = false;
			_is_stretch_enabled = true;
		} break;
	}
	// Update mesh instance visibility.
	_meshes[TRANSFORM_MOVE_X]->set_visible(_is_move_linear_enabled);
	_meshes[TRANSFORM_MOVE_Y]->set_visible(_is_move_linear_enabled);
	_meshes[TRANSFORM_MOVE_Z]->set_visible(_is_move_linear_enabled);
	_meshes[TRANSFORM_MOVE_W]->set_visible(_is_move_linear_enabled);
	_meshes[TRANSFORM_ROTATE_XY]->set_visible(_is_rotation_enabled);
	_meshes[TRANSFORM_ROTATE_XZ]->set_visible(_is_rotation_enabled);
	_meshes[TRANSFORM_ROTATE_XW]->set_visible(_is_rotation_enabled);
	_meshes[TRANSFORM_ROTATE_YZ]->set_visible(_is_rotation_enabled);
	_meshes[TRANSFORM_ROTATE_YW]->set_visible(_is_rotation_enabled);
	_meshes[TRANSFORM_ROTATE_ZW]->set_visible(_is_rotation_enabled);
	_meshes[TRANSFORM_SCALE_X]->set_visible(_is_scale_linear_enabled);
	_meshes[TRANSFORM_SCALE_Y]->set_visible(_is_scale_linear_enabled);
	_meshes[TRANSFORM_SCALE_Z]->set_visible(_is_scale_linear_enabled);
	_meshes[TRANSFORM_SCALE_W]->set_visible(_is_scale_linear_enabled);
	_meshes[TRANSFORM_STRETCH_POS_X]->set_visible(_is_stretch_enabled);
	_meshes[TRANSFORM_STRETCH_NEG_X]->set_visible(_is_stretch_enabled);
	_meshes[TRANSFORM_STRETCH_POS_Y]->set_visible(_is_stretch_enabled);
	_meshes[TRANSFORM_STRETCH_NEG_Y]->set_visible(_is_stretch_enabled);
	_meshes[TRANSFORM_STRETCH_POS_Z]->set_visible(_is_stretch_enabled);
	_meshes[TRANSFORM_STRETCH_NEG_Z]->set_visible(_is_stretch_enabled);
	_meshes[TRANSFORM_STRETCH_POS_W]->set_visible(_is_stretch_enabled);
	_meshes[TRANSFORM_STRETCH_NEG_W]->set_visible(_is_stretch_enabled);
	const bool is_plane_visible = _is_move_planar_enabled || _is_scale_planar_enabled;
	_meshes[TRANSFORM_MOVE_XY]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_MOVE_XZ]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_MOVE_XW]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_MOVE_YZ]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_MOVE_YW]->set_visible(is_plane_visible);
	_meshes[TRANSFORM_MOVE_ZW]->set_visible(is_plane_visible);
	set_visible(true);
}

bool EditorTransformGizmo4D::gizmo_mouse_input(const Ref<InputEventMouse> &p_mouse_event, const Camera4D *p_camera) {
	_update_gizmo_mesh_transform(p_camera);
	if (!is_visible()) {
		return false;
	}
	const Vector2 mouse_position = p_mouse_event->get_position();
	const Vector4 ray_origin = p_camera->viewport_to_world_ray_origin(mouse_position);
	const Vector4 ray_direction = p_camera->viewport_to_world_ray_direction(mouse_position);
	const Vector4 perpendicular_direction = p_camera->get_global_basis().w;
	if (_current_transformation == TRANSFORM_NONE) {
		_old_transform = _mesh_holder->get_global_transform();
	}
	const Transform4D global_to_local = _old_transform.inverse();
	const Vector4 local_perp_direction = global_to_local.basis.xform(perpendicular_direction).normalized();
	const Vector4 local_ray_origin = Vector4D::slide(global_to_local.xform(ray_origin), local_perp_direction);
	const Vector4 local_ray_direction = Vector4D::slide(global_to_local.basis.xform(ray_direction), local_perp_direction);
	return gizmo_mouse_raycast(p_mouse_event, local_ray_origin, local_ray_direction, local_perp_direction);
}

bool EditorTransformGizmo4D::gizmo_mouse_raycast(const Ref<InputEventMouse> &p_mouse_event, const Vector4 &p_local_ray_origin, const Vector4 &p_local_ray_direction, const Vector4 &p_local_perp_direction) {
	Ref<InputEventMouseButton> mouse_button = p_mouse_event;
	if (mouse_button.is_valid()) {
		if (mouse_button->get_button_index() != MOUSE_BUTTON_LEFT) {
			return false;
		}
		if (mouse_button->is_pressed() && _current_transformation == TRANSFORM_NONE) {
			// If we are not transforming anything and the user clicks, try to start a transformation.
			_current_transformation = _check_for_best_hit(p_local_ray_origin, p_local_ray_direction, p_local_perp_direction);
			if (_current_transformation != TRANSFORM_NONE) {
				_begin_transformation(p_local_ray_origin, p_local_ray_direction, p_local_perp_direction);
			}
		} else if (!mouse_button->is_pressed() && _current_transformation != TRANSFORM_NONE) {
			// If we are transforming something and the user releases the click, end the transformation.
			_end_transformation();
		}
		return false;
	}
	Ref<InputEventMouseMotion> mouse_motion = p_mouse_event;
	if (mouse_motion.is_valid()) {
		if (_current_transformation == TRANSFORM_NONE) {
			// If we receive motion but no transformation is happening, it means
			// we need to highlight the closest part of the gizmo (if any).
			TransformPart hit = _check_for_best_hit(p_local_ray_origin, p_local_ray_direction, p_local_perp_direction);
			if (hit == _highlighted_transformation) {
				return false;
			}
			_unhiglight_mesh(_highlighted_transformation);
			_highlight_mesh(hit);
			_highlighted_transformation = hit;
			return true;
		} else {
			// If we are transforming something, process the transformation.
			_process_transform(p_local_ray_origin, p_local_ray_direction, p_local_perp_direction);
			return true;
		}
	}
	return false;
}

bool EditorTransformGizmo4D::get_use_local_rotation() const {
	return _is_use_local_rotation;
}

void EditorTransformGizmo4D::set_use_local_rotation(const bool p_use_local_transform) {
	_is_use_local_rotation = p_use_local_transform;
}

void EditorTransformGizmo4D::setup(EditorUndoRedoManager *p_undo_redo_manager) {
	undo_redo = p_undo_redo_manager;
	p_undo_redo_manager->connect(StringName("version_changed"), callable_mp(this, &EditorTransformGizmo4D::_on_undo_redo_version_changed));
}

EditorTransformGizmo4D::EditorTransformGizmo4D() {
	_mesh_holder = memnew(Node4D);
	_mesh_holder->set_name(StringName("GizmoMeshHolder4D"));
	add_child(_mesh_holder);
	RenderingServer4D::get_singleton()->connect(StringName("pre_render"), callable_mp(this, &EditorTransformGizmo4D::_on_rendering_server_pre_render));
	EditorInterface::get_singleton()->get_inspector()->connect(StringName("property_edited"), callable_mp(this, &EditorTransformGizmo4D::_on_editor_inspector_property_edited));
}
