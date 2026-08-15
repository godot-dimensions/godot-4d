#include "rendering_engine_4d.h"

#include <algorithm>
#include <tuple>
#include <vector>

void RenderingEngine4D::calculate_relative_transforms() {
	const Transform4D camera_inverse_transform = _camera->get_global_transform().inverse();
	// Meshes.
	const int mesh_count = _mesh_instance_object_ids.size();
	_mesh_relative_basises.resize(mesh_count);
	_mesh_relative_positions.resize(mesh_count);
	for (int64_t i = 0; i < _mesh_instance_object_ids.size(); i++) {
		const ObjectID mesh_instance_object_id = (ObjectID)_mesh_instance_object_ids[i];
		const MeshInstance4D *mesh_instance = Object::cast_to<const MeshInstance4D>(ObjectDB::get_instance(mesh_instance_object_id));
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
	const int64_t mesh_count = _mesh_instance_object_ids.size();
	combined.reserve(mesh_count);
	for (int64_t i = 0; i < mesh_count; ++i) {
		combined.emplace_back(_mesh_instance_object_ids[i], _mesh_relative_basises[i], _mesh_relative_positions[i]);
	}
	// Sort the vector of tuples based on the Z position.
	std::sort(combined.begin(), combined.end(), [](const auto &a, const auto &b) {
		return std::get<2>(a).z < std::get<2>(b).z;
	});
	// Unpack the sorted tuples back into the original arrays
	for (size_t i = 0; i < combined.size(); ++i) {
		_mesh_instance_object_ids.set(i, std::get<0>(combined[i]));
		_mesh_relative_basises[i] = std::get<1>(combined[i]);
		_mesh_relative_positions.set(i, std::get<2>(combined[i]));
	}
}

void RenderingEngine4D::set_viewport(Viewport *p_viewport) {
	_viewport = p_viewport;
}

void RenderingEngine4D::set_camera(Camera4D *p_camera) {
	_camera = p_camera;
}

void RenderingEngine4D::set_mesh_instance_object_ids(PackedInt64Array p_mesh_instance_object_ids) {
	_mesh_instance_object_ids = p_mesh_instance_object_ids;
}

bool RenderingEngine4D::prefers_wireframe_meshes() const {
	bool prefers_wireframe = false;
	GDVIRTUAL_CALL(_prefers_wireframe_meshes, prefers_wireframe);
	return prefers_wireframe;
}

String RenderingEngine4D::get_friendly_name() const {
	String friendly_name;
	GDVIRTUAL_CALL(_get_friendly_name, friendly_name);
	return friendly_name;
}

void RenderingEngine4D::setup_for_viewport_if_needed(Viewport *p_for_viewport) {
	_viewport = p_for_viewport;
	if (_setup_viewports.has(p_for_viewport)) {
		return;
	}
	p_for_viewport->set_meta("last_rendering_engine_name_4d", get_friendly_name());
	_setup_viewports.append(p_for_viewport);
	setup_for_viewport();
}

void RenderingEngine4D::setup_for_viewport() {
	GDVIRTUAL_CALL(_setup_for_viewport);
}

void RenderingEngine4D::cleanup_for_viewport_if_needed(Viewport *p_for_viewport) {
	_viewport = p_for_viewport;
	if (!_setup_viewports.has(p_for_viewport)) {
		return;
	}
	_setup_viewports.erase(p_for_viewport);
	p_for_viewport->remove_meta("last_rendering_engine_name_4d");
	cleanup_for_viewport();
}

void RenderingEngine4D::cleanup_for_viewport() {
	GDVIRTUAL_CALL(_cleanup_for_viewport);
}

void RenderingEngine4D::render_frame() {
	GDVIRTUAL_CALL(_render_frame);
}

void RenderingEngine4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_viewport"), &RenderingEngine4D::get_viewport);
	ClassDB::bind_method(D_METHOD("get_camera"), &RenderingEngine4D::get_camera);

	ClassDB::bind_method(D_METHOD("get_mesh_instance_object_ids"), &RenderingEngine4D::get_mesh_instance_object_ids);
	ClassDB::bind_method(D_METHOD("get_mesh_relative_basises"), &RenderingEngine4D::get_mesh_relative_basises);
	ClassDB::bind_method(D_METHOD("get_mesh_relative_positions"), &RenderingEngine4D::get_mesh_relative_positions);

	GDVIRTUAL_BIND(_get_friendly_name);
	GDVIRTUAL_BIND(_prefers_wireframe_meshes);
	GDVIRTUAL_BIND(_setup_for_viewport);
	GDVIRTUAL_BIND(_cleanup_for_viewport);
	GDVIRTUAL_BIND(_render_frame);
}
