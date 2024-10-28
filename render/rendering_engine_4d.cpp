#include "rendering_engine_4d.h"

#include <algorithm>
#include <tuple>
#include <vector>

void RenderingEngine4D::calculate_relative_transforms() {
	const int mesh_count = _mesh_instances.size();
	_mesh_relative_basises.resize(mesh_count);
	_mesh_relative_positions.resize(mesh_count);
	const Transform4D camera_inverse_transform = _camera->get_global_transform().inverse();
	for (int i = 0; i < _mesh_instances.size(); i++) {
		const MeshInstance4D *mesh_instance = (const MeshInstance4D *)(const Object *)_mesh_instances[i];
		const Transform4D relative_transform = camera_inverse_transform * mesh_instance->get_global_transform();
		_mesh_relative_basises[i] = relative_transform.basis.operator Projection();
		_mesh_relative_positions.set(i, relative_transform.origin);
	}
	_sort_meshes_by_relative_z();
}

void RenderingEngine4D::_sort_meshes_by_relative_z() {
	// Can't use Godot's types to do this operation easily, so we'll use the standard library instead.
	// See https://github.com/godotengine/godot/pull/77213 for a discussion on adding sort to Dictionary and HashMap.
	std::vector<std::tuple<Variant, Variant, Vector4>> combined;
	combined.reserve(_mesh_instances.size());
	for (int i = 0; i < _mesh_instances.size(); ++i) {
		combined.emplace_back(_mesh_instances[i], _mesh_relative_basises[i], _mesh_relative_positions[i]);
	}
	// Sort the vector of tuples based on the Z position.
	std::sort(combined.begin(), combined.end(), [](const auto &a, const auto &b) {
		return std::get<2>(a).z < std::get<2>(b).z;
	});
	// Unpack the sorted tuples back into the original arrays
	for (size_t i = 0; i < combined.size(); ++i) {
		_mesh_instances[i] = std::get<0>(combined[i]);
		_mesh_relative_basises[i] = std::get<1>(combined[i]);
		_mesh_relative_positions.set(i, std::get<2>(combined[i]));
	}
}

Viewport *RenderingEngine4D::get_viewport() const {
	return _viewport;
}

void RenderingEngine4D::set_viewport(Viewport *p_viewport) {
	_viewport = p_viewport;
}

Camera4D *RenderingEngine4D::get_camera() const {
	return _camera;
}

void RenderingEngine4D::set_camera(Camera4D *p_camera) {
	_camera = p_camera;
}

TypedArray<MeshInstance4D> RenderingEngine4D::get_mesh_instances() const {
	return _mesh_instances;
}

void RenderingEngine4D::set_mesh_instances(TypedArray<MeshInstance4D> p_mesh_instances) {
	_mesh_instances = p_mesh_instances;
}

TypedArray<Projection> RenderingEngine4D::get_mesh_relative_basises() const {
	return _mesh_relative_basises;
}

void RenderingEngine4D::set_mesh_relative_basises(TypedArray<Projection> p_mesh_relative_basises) {
	_mesh_relative_basises = p_mesh_relative_basises;
}

PackedVector4Array RenderingEngine4D::get_mesh_relative_positions() const {
	return _mesh_relative_positions;
}

void RenderingEngine4D::set_mesh_relative_positions(PackedVector4Array p_mesh_relative_positions) {
	_mesh_relative_positions = p_mesh_relative_positions;
}

bool RenderingEngine4D::prefers_wireframe_meshes() {
	bool prefers_wireframe = false;
	GDVIRTUAL_CALL(_prefers_wireframe_meshes, prefers_wireframe);
	return prefers_wireframe;
}

void RenderingEngine4D::setup_for_viewport_if_needed(Viewport *p_for_viewport) {
	_viewport = p_for_viewport;
	if (_setup_viewports.has(p_for_viewport)) {
		return;
	}
	_setup_viewports.append(p_for_viewport);
	setup_for_viewport();
}

void RenderingEngine4D::setup_for_viewport() {
	GDVIRTUAL_CALL(_setup_for_viewport);
}

void RenderingEngine4D::render_frame() {
	GDVIRTUAL_CALL(_render_frame);
}

void RenderingEngine4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_viewport"), &RenderingEngine4D::get_viewport);
	ClassDB::bind_method(D_METHOD("set_viewport", "viewport"), &RenderingEngine4D::set_viewport);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "viewport", PROPERTY_HINT_RESOURCE_TYPE, "Viewport"), "set_viewport", "get_viewport");

	ClassDB::bind_method(D_METHOD("get_camera"), &RenderingEngine4D::get_camera);
	ClassDB::bind_method(D_METHOD("set_camera", "camera"), &RenderingEngine4D::set_camera);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "camera", PROPERTY_HINT_RESOURCE_TYPE, "Camera4D"), "set_camera", "get_camera");

	ClassDB::bind_method(D_METHOD("get_mesh_instances"), &RenderingEngine4D::get_mesh_instances);
	ClassDB::bind_method(D_METHOD("set_mesh_instances", "mesh_instances"), &RenderingEngine4D::set_mesh_instances);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "mesh_instances"), "set_mesh_instances", "get_mesh_instances");

	ClassDB::bind_method(D_METHOD("get_mesh_relative_basises"), &RenderingEngine4D::get_mesh_relative_basises);
	ClassDB::bind_method(D_METHOD("set_mesh_relative_basises", "mesh_relative_basises"), &RenderingEngine4D::set_mesh_relative_basises);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "mesh_relative_basises"), "set_mesh_relative_basises", "get_mesh_relative_basises");

	ClassDB::bind_method(D_METHOD("get_mesh_relative_positions"), &RenderingEngine4D::get_mesh_relative_positions);
	ClassDB::bind_method(D_METHOD("set_mesh_relative_positions", "mesh_relative_positions"), &RenderingEngine4D::set_mesh_relative_positions);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "mesh_relative_positions"), "set_mesh_relative_positions", "get_mesh_relative_positions");

	GDVIRTUAL_BIND(_prefers_wireframe_meshes);
	GDVIRTUAL_BIND(_setup_for_viewport);
	GDVIRTUAL_BIND(_render_frame);
}
