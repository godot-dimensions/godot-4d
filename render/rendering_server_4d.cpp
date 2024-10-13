#include "rendering_server_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#elif GODOT_MODULE
#include "scene/main/scene_tree.h"
#include "servers/rendering_server.h"
#endif

Ref<RenderingEngine4D> RenderingServer4D::_get_rendering_engine(const String &p_name) const {
	if (_rendering_engines.has(p_name)) {
		return _rendering_engines[p_name];
	}
	// Fallback to the first registered rendering engine. If the name is empty,
	// treat it as "auto" and do not print a warning. Else, print a warning.
	if (!p_name.is_empty()) {
		WARN_PRINT("Rendering engine '" + p_name + "' not registered. Using the first registered engine.");
	}
	return _rendering_engines.begin()->value;
}

TypedArray<MeshInstance4D> RenderingServer4D::_get_visible_mesh_instances() const {
	TypedArray<MeshInstance4D> visible_mesh_instances;
	for (int i = 0; i < _mesh_instances.size(); i++) {
		MeshInstance4D *mesh_instance = (MeshInstance4D *)(Object *)_mesh_instances[i];
		if (mesh_instance->is_visible_in_tree()) {
			Ref<Mesh4D> mesh = mesh_instance->get_mesh();
			if (mesh.is_valid() && mesh->is_mesh_data_valid()) {
				visible_mesh_instances.append(mesh_instance);
			}
		}
	}
	return visible_mesh_instances;
}

void RenderingServer4D::_render_frame() {
	for (const KeyValue<Viewport *, Vector<Camera4D *>> &E : _viewport_cameras) {
		Viewport *viewport = E.key;
		const Vector<Camera4D *> &cameras = E.value;
		if (cameras.is_empty()) {
			continue; // Viewport has no 4D cameras.
		}
		Camera4D *camera0 = cameras[0];
		if (!camera0->is_current()) {
			continue; // No 4D cameras are currently rendering.
		}
		ERR_FAIL_COND_MSG(_rendering_engines.is_empty(), "No 4D rendering engines registered. 4D rendering will not occur.");
		const String &rendering_engine_name = camera0->get_rendering_engine();
		Ref<RenderingEngine4D> rendering_engine = _get_rendering_engine(rendering_engine_name);
		// Now that we have a rendering engine selected, set up its properties.
		rendering_engine->setup_for_viewport_if_needed(viewport);
		rendering_engine->set_camera(camera0);
		TypedArray<MeshInstance4D> visible_mesh_instances = _get_visible_mesh_instances();
		rendering_engine->set_mesh_instances(visible_mesh_instances);
		rendering_engine->calculate_relative_transforms();
		rendering_engine->render_frame();
	}
}

void RenderingServer4D::register_camera(Camera4D *p_camera) {
	ERR_FAIL_NULL(p_camera);
	Viewport *viewport = p_camera->get_viewport();
	ERR_FAIL_NULL(viewport);
	if (_viewport_cameras.has(viewport)) {
		Vector<Camera4D *> &cameras = _viewport_cameras[viewport];
		ERR_FAIL_COND_MSG(cameras.has(p_camera), "Camera4D is already registered to this Viewport.");
		cameras.append(p_camera);
		if (p_camera->is_current()) {
			RenderingServer4D::make_camera_current(p_camera);
		}
		return;
	}
	// This is the first time a Camera4D has been registered to this Viewport.
	Vector<Camera4D *> cameras;
	cameras.append(p_camera);
	_viewport_cameras[viewport] = cameras;
	p_camera->make_current();
	// Is this also the first time any Camera4D has been registered? If so, connect to the SceneTree's frame signal.
	if (_is_render_frame_connected) {
		return;
	}
	RenderingServer *godot_rendering_server = RenderingServer::get_singleton();
	godot_rendering_server->connect(StringName("frame_pre_draw"), callable_mp(this, &RenderingServer4D::_render_frame));
	_is_render_frame_connected = true;
}

void RenderingServer4D::unregister_camera(Camera4D *p_camera) {
	ERR_FAIL_NULL(p_camera);
	Viewport *viewport = p_camera->get_viewport();
	Vector<Camera4D *> &cameras = _viewport_cameras[viewport];
	ERR_FAIL_COND(!cameras.has(p_camera));
	if (p_camera == cameras[0]) {
		p_camera->clear_current();
	}
	cameras.erase(p_camera);
}

void RenderingServer4D::make_camera_current(Camera4D *p_camera) {
	ERR_FAIL_NULL(p_camera);
	if (unlikely(!p_camera->is_current())) {
		// Will set the camera's variable and call RenderingServer4D::make_camera_current again.
		p_camera->make_current();
		return;
	}
	Viewport *viewport = p_camera->get_viewport();
	Vector<Camera4D *> &cameras = _viewport_cameras[viewport];
	ERR_FAIL_COND(!cameras.has(p_camera));
	Camera4D *camera0 = cameras[0];
	if (p_camera != camera0) {
		if (camera0->is_current()) {
			camera0->clear_current(false);
		}
		cameras.erase(p_camera);
		cameras.insert(0, p_camera);
	}
}

void RenderingServer4D::clear_camera_current(Camera4D *p_camera) {
	ERR_FAIL_NULL(p_camera);
	if (unlikely(p_camera->is_current())) {
		p_camera->clear_current(false);
	}
	Viewport *viewport = p_camera->get_viewport();
	Vector<Camera4D *> &cameras = _viewport_cameras[viewport];
	ERR_FAIL_COND(!cameras.has(p_camera));
	if (p_camera == cameras[0] && cameras.size() > 1) {
		cameras.remove_at(0);
		cameras.append(p_camera);
		Camera4D *camera0 = cameras[0];
		camera0->make_current();
	}
}

Camera4D *RenderingServer4D::get_current_camera(Viewport *p_viewport) const {
	if (!_viewport_cameras.has(p_viewport)) {
		return nullptr;
	}
	const Vector<Camera4D *> &cameras = _viewport_cameras[p_viewport];
	if (cameras.is_empty()) {
		return nullptr;
	}
	Camera4D *camera0 = cameras[0];
	if (camera0->is_current()) {
		return camera0;
	}
	return nullptr;
}

void RenderingServer4D::register_mesh_instance(MeshInstance4D *p_mesh_instance) {
	_mesh_instances.append(p_mesh_instance);
}

void RenderingServer4D::unregister_mesh_instance(MeshInstance4D *p_mesh_instance) {
	_mesh_instances.erase(p_mesh_instance);
}

void RenderingServer4D::register_rendering_engine(const String &p_name, const Ref<RenderingEngine4D> &p_engine) {
	if (_rendering_engines.has(p_name)) {
		WARN_PRINT("Rendering engine '" + p_name + "' already registered. The existing engine will be replaced.");
	}
	_rendering_engines[p_name] = p_engine;
}

void RenderingServer4D::unregister_rendering_engine(const String &p_name) {
	_rendering_engines.erase(p_name);
}

PackedStringArray RenderingServer4D::get_rendering_engine_names() const {
	// HashMap doesn't have a keys() method, so we have to do this manually.
	PackedStringArray engine_names;
	if (_rendering_engines.is_empty()) {
		return engine_names;
	}
	engine_names.resize(_rendering_engines.size());
	int i = 0;
	for (const KeyValue<String, Ref<RenderingEngine4D>> &E : _rendering_engines) {
		engine_names.set(i, String(E.key));
		i++;
	}
	return engine_names;
}

RenderingServer4D *RenderingServer4D::singleton = nullptr;

void RenderingServer4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_current_camera", "viewport"), &RenderingServer4D::get_current_camera);
	ClassDB::bind_method(D_METHOD("register_rendering_engine", "name", "engine"), &RenderingServer4D::register_rendering_engine);
	ClassDB::bind_method(D_METHOD("unregister_rendering_engine", "name"), &RenderingServer4D::unregister_rendering_engine);
	ClassDB::bind_method(D_METHOD("get_rendering_engine_names"), &RenderingServer4D::get_rendering_engine_names);
}
