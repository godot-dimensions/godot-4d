#include "rendering_server_4d.h"

#include "../model/mesh/mesh_instance_4d.h"
#include "../nodes/camera_4d.h"
#include "environment/world_environment_4d.h"

#ifdef TOOLS_ENABLED
#include "../editor/viewport/editor_camera_4d.h"
#endif // TOOLS_ENABLED

#if GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#ifdef TOOLS_ENABLED
#include <godot_cpp/classes/editor_interface.hpp>
#endif // TOOLS_ENABLED
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#elif GODOT_MODULE
#include "core/config/engine.h"
#ifdef TOOLS_ENABLED
#include "editor/editor_interface.h"
#endif // TOOLS_ENABLED
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

RenderingServer4D::~RenderingServer4D() {
	for (const KeyValue<Viewport *, Vector<Camera4D *>> &E : _viewport_cameras) {
		Viewport *viewport = E.key;
		if (viewport == nullptr) {
			continue;
		}
		for (KeyValue<String, Ref<RenderingEngine4D>> &engine_entry : _rendering_engines) {
			if (engine_entry.value.is_valid()) {
				engine_entry.value->cleanup_for_viewport_if_needed(viewport);
			}
		}
	}
	_viewport_cameras.clear();
	_viewport_world_environments.clear();
	_rendering_engines.clear();
	singleton = nullptr;
}

PackedInt64Array RenderingServer4D::_get_visible_mesh_instance_object_ids() const {
	PackedInt64Array visible_mesh_instance_object_ids;
	for (int i = 0; i < _mesh_instances.size(); i++) {
		MeshInstance4D *mesh_instance = (MeshInstance4D *)(Object *)_mesh_instances[i];
		CRASH_COND_MSG(mesh_instance == nullptr, "MeshInstance4D is null. This should never happen.");
		if (mesh_instance->is_visible_in_tree()) {
			Ref<Mesh4D> mesh = mesh_instance->get_mesh();
			if (mesh.is_valid() && mesh->is_mesh_data_valid()) {
				visible_mesh_instance_object_ids.append(mesh_instance->get_instance_id());
			}
		}
	}
	return visible_mesh_instance_object_ids;
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
#ifdef TOOLS_ENABLED
		if (Engine::get_singleton()->is_editor_hint()) {
			// Only allow editor cameras to render in the editor.
			EditorCamera4D *editor_camera = Object::cast_to<EditorCamera4D>(camera0->get_parent());
			if (editor_camera == nullptr) {
				continue;
			}
		}
#endif // TOOLS_ENABLED
		ERR_FAIL_COND_MSG(_rendering_engines.is_empty(), "No 4D rendering engines registered. 4D rendering will not occur.");
		Ref<RenderingEngine4D> rendering_engine = get_rendering_engine_from_name(camera0->get_rendering_engine_name());
		if (viewport->has_meta("last_rendering_engine_name_4d")) {
			Variant last_rendering_engine_variant = viewport->get_meta("last_rendering_engine_name_4d");
			if (last_rendering_engine_variant.get_type() == Variant::STRING) {
				const String last_rendering_engine_name = last_rendering_engine_variant;
				const String next_rendering_engine_name = rendering_engine->get_friendly_name();
				if (next_rendering_engine_name != last_rendering_engine_name) {
					Ref<RenderingEngine4D> last_rendering_engine_name_4d = get_rendering_engine_from_name(last_rendering_engine_name);
					last_rendering_engine_name_4d->cleanup_for_viewport_if_needed(viewport);
				}
			}
		}
		// Now that we have a rendering engine selected, set up its properties.
		rendering_engine->setup_for_viewport_if_needed(viewport);
		rendering_engine->set_camera(camera0);
		emit_signal("pre_render", camera0, viewport, rendering_engine);
		PackedInt64Array visible_mesh_instance_object_ids = _get_visible_mesh_instance_object_ids();
		rendering_engine->set_mesh_instance_object_ids(visible_mesh_instance_object_ids);
		rendering_engine->calculate_relative_transforms();
		rendering_engine->render_frame();
	}
}

void RenderingServer4D::_request_godot_redraw() {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	for (const KeyValue<Viewport *, Vector<Camera4D *>> &E : _viewport_cameras) {
		if (E.key == nullptr) {
			continue;
		}
		const RID viewport_rid = E.key->get_viewport_rid();
		// This seems like it should be a no-op, since it sets the update mode to its current value.
		// However, this actually marks a change, forcing the RenderingServer to redraw everything
		// on the next frame, bypassing low processor mode, so long as the update mode is not disabled
		// or a value that would otherwise cause it to not redraw when low processor mode is disabled.
		// Note that this means that a call on ANY viewport fixes the problem for the whole app.
		// Don't bother looking for the definition of `viewport_set_update_mode`, it is generated by a macro.
		const RSE::ViewportUpdateMode update_mode = rendering_server->viewport_get_update_mode(viewport_rid);
		rendering_server->viewport_set_update_mode(viewport_rid, update_mode);
		return;
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
	// Is this also the first time any Camera4D has been registered? If so, connect to the RenderingServer's frame signal.
	if (_are_render_frame_and_process_frame_connected) {
		return;
	}
	RenderingServer *godot_rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(godot_rendering_server);
	SceneTree *godot_scene_tree = p_camera->get_tree();
	ERR_FAIL_NULL(godot_scene_tree);
	godot_rendering_server->connect(StringName("frame_pre_draw"), callable_mp(this, &RenderingServer4D::_render_frame));
	// Connect to the SceneTree's process_frame signal, so we can request a redraw every tick.
	// This is necessary because the RenderingServer will not redraw the viewport if it thinks nothing has changed.
	// There are a ton of things that will need a redraw, to the point that it is easier to just request a redraw
	// every tick, rather than slow each tick down with a thousand calls to `_request_godot_redraw()` each tick.
	// This will result in increased resource usage when idle, but more FPS when actively running, it's a tradeoff.
	godot_scene_tree->connect(StringName("process_frame"), callable_mp(this, &RenderingServer4D::_request_godot_redraw));
	_are_render_frame_and_process_frame_connected = true;
}

void RenderingServer4D::unregister_camera(Camera4D *p_camera) {
	ERR_FAIL_NULL(p_camera);
	Viewport *viewport = p_camera->get_viewport();
	ERR_FAIL_NULL(viewport);
	ERR_FAIL_COND(!_viewport_cameras.has(viewport));
	Vector<Camera4D *> &cameras = _viewport_cameras[viewport];
	ERR_FAIL_COND(!cameras.has(p_camera));
	if (p_camera == cameras[0]) {
		p_camera->clear_current();
	}
	cameras.erase(p_camera);
	if (cameras.is_empty()) {
		for (KeyValue<String, Ref<RenderingEngine4D>> &E : _rendering_engines) {
			if (E.value.is_valid()) {
				E.value->cleanup_for_viewport_if_needed(viewport);
			}
		}
		_viewport_cameras.erase(viewport);
		if (_are_render_frame_and_process_frame_connected && _viewport_cameras.is_empty()) {
			// If there are no more 4D cameras on any Viewport, we can disconnect the signals to put RenderingServer4D to sleep.
			RenderingServer *godot_rendering_server = RenderingServer::get_singleton();
			if (godot_rendering_server != nullptr) {
				const Callable render_callable = callable_mp(this, &RenderingServer4D::_render_frame);
				if (godot_rendering_server->is_connected(StringName("frame_pre_draw"), render_callable)) {
					godot_rendering_server->disconnect(StringName("frame_pre_draw"), render_callable);
				}
			}
			SceneTree *godot_scene_tree = p_camera->get_tree();
			if (godot_scene_tree != nullptr) {
				const Callable redraw_callable = callable_mp(this, &RenderingServer4D::_request_godot_redraw);
				if (godot_scene_tree->is_connected(StringName("process_frame"), redraw_callable)) {
					godot_scene_tree->disconnect(StringName("process_frame"), redraw_callable);
				}
			}
			_are_render_frame_and_process_frame_connected = false;
		}
	}
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
	ERR_FAIL_NULL(p_mesh_instance);
	ERR_FAIL_COND_MSG(_mesh_instances.has(p_mesh_instance), "MeshInstance4D is already registered.");
	_mesh_instances.append(p_mesh_instance);
}

void RenderingServer4D::unregister_mesh_instance(MeshInstance4D *p_mesh_instance) {
	ERR_FAIL_NULL(p_mesh_instance);
	ERR_FAIL_COND_MSG(!_mesh_instances.has(p_mesh_instance), "MeshInstance4D is not registered.");
	_mesh_instances.erase(p_mesh_instance);
}

void RenderingServer4D::register_rendering_engine(const Ref<RenderingEngine4D> &p_engine) {
	const String friendly_name = p_engine->get_friendly_name();
	if (_rendering_engines.has(friendly_name)) {
		WARN_PRINT("Rendering engine '" + friendly_name + "' already registered. The existing engine will be replaced.");
	}
	_rendering_engines[friendly_name] = p_engine;
}

void RenderingServer4D::unregister_rendering_engine(const String &p_friendly_name) {
	_rendering_engines.erase(p_friendly_name);
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

Ref<RenderingEngine4D> RenderingServer4D::get_rendering_engine_from_name(const String &p_friendly_name) const {
	if (_rendering_engines.has(p_friendly_name)) {
		return _rendering_engines[p_friendly_name];
	}
	// Fallback to the first registered rendering engine. If the name is empty,
	// treat it as "auto" and do not print a warning. Else, print a warning.
	if (!p_friendly_name.is_empty()) {
		WARN_PRINT("Rendering engine '" + p_friendly_name + "' not registered. Using the first registered engine.");
	}
	return _rendering_engines.begin()->value;
}

void RenderingServer4D::register_world_environment(WorldEnvironment4D *p_world_environment) {
	ERR_FAIL_NULL(p_world_environment);
	Viewport *viewport = p_world_environment->get_viewport();
	ERR_FAIL_NULL(viewport);
	if (_viewport_world_environments.has(viewport)) {
		Vector<WorldEnvironment4D *> &world_environments = _viewport_world_environments[viewport];
		ERR_FAIL_COND_MSG(world_environments.has(p_world_environment), "WorldEnvironment4D is already registered to this Viewport.");
		world_environments.append(p_world_environment);
		if (p_world_environment->is_current()) {
			RenderingServer4D::make_world_environment_current(p_world_environment);
		}
		return;
	}
	// This is the first time a WorldEnvironment4D has been registered to this Viewport.
	Vector<WorldEnvironment4D *> world_environments;
	world_environments.append(p_world_environment);
	_viewport_world_environments[viewport] = world_environments;
	p_world_environment->make_current();
}

void RenderingServer4D::unregister_world_environment(WorldEnvironment4D *p_world_environment) {
	ERR_FAIL_NULL(p_world_environment);
	Viewport *viewport = p_world_environment->get_viewport();
	ERR_FAIL_NULL(viewport);
	ERR_FAIL_COND(!_viewport_world_environments.has(viewport));
	Vector<WorldEnvironment4D *> &world_environments = _viewport_world_environments[viewport];
	ERR_FAIL_COND(!world_environments.has(p_world_environment));
	if (p_world_environment == world_environments[0]) {
		p_world_environment->clear_current();
	}
	world_environments.erase(p_world_environment);
	if (world_environments.is_empty()) {
		_viewport_world_environments.erase(viewport);
	}
}

void RenderingServer4D::clear_world_environment_current(WorldEnvironment4D *p_world_environment) {
	ERR_FAIL_NULL(p_world_environment);
	if (unlikely(p_world_environment->is_current())) {
		p_world_environment->clear_current(false);
	}
	Viewport *viewport = p_world_environment->get_viewport();
	Vector<WorldEnvironment4D *> &world_environments = _viewport_world_environments[viewport];
	ERR_FAIL_COND(!world_environments.has(p_world_environment));
	if (p_world_environment == world_environments[0] && world_environments.size() > 1) {
		world_environments.remove_at(0);
		world_environments.append(p_world_environment);
		WorldEnvironment4D *world_environment0 = world_environments[0];
		world_environment0->make_current();
	}
}

void RenderingServer4D::make_world_environment_current(WorldEnvironment4D *p_world_environment) {
	ERR_FAIL_NULL(p_world_environment);
	if (unlikely(!p_world_environment->is_current())) {
		// Will set the environment's variable and call RenderingServer4D::make_world_environment_current again.
		p_world_environment->make_current();
		return;
	}
	Viewport *viewport = p_world_environment->get_viewport();
	Vector<WorldEnvironment4D *> &world_environments = _viewport_world_environments[viewport];
	ERR_FAIL_COND(!world_environments.has(p_world_environment));
	WorldEnvironment4D *world_environment0 = world_environments[0];
	if (p_world_environment != world_environment0) {
		if (world_environment0->is_current()) {
			world_environment0->clear_current(false);
		}
		world_environments.erase(p_world_environment);
		world_environments.insert(0, p_world_environment);
	}
}

WorldEnvironment4D *RenderingServer4D::get_current_world_environment(Viewport *p_viewport) const {
	if (!_viewport_world_environments.has(p_viewport)) {
		return nullptr;
	}
	const Vector<WorldEnvironment4D *> &world_environments = _viewport_world_environments[p_viewport];
	if (world_environments.is_empty()) {
		return nullptr;
	}
	WorldEnvironment4D *world_environment0 = world_environments[0];
	if (world_environment0->is_current()) {
		return world_environment0;
	}
	return nullptr;
}

WorldEnvironment4D *RenderingServer4D::get_current_world_environment_for_camera(Camera4D *p_camera) const {
	ERR_FAIL_NULL_V(p_camera, nullptr);
	Viewport *environment_viewport = p_camera->get_viewport();
#ifdef TOOLS_ENABLED
	// The edited scene and preview environment belong to different editor
	// viewports, while each 4D editor camera belongs to another SubViewport.
	if (Engine::get_singleton()->is_editor_hint() && Object::cast_to<EditorCamera4D>(p_camera->get_parent()) != nullptr) {
		Node *edited_scene_root = EditorInterface::get_singleton()->get_edited_scene_root();
		if (edited_scene_root != nullptr) {
			WorldEnvironment4D *world_environment = get_current_world_environment(edited_scene_root->get_viewport());
			if (world_environment != nullptr) {
				return world_environment;
			}
		}
		// The preview environment belongs to the editor's root viewport.
		SceneTree *scene_tree = p_camera->get_tree();
		ERR_FAIL_NULL_V(scene_tree, nullptr);
		environment_viewport = scene_tree->get_root();
	}
#endif // TOOLS_ENABLED
	return get_current_world_environment(environment_viewport);
}

RenderingServer4D *RenderingServer4D::singleton = nullptr;

void RenderingServer4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_current_camera", "viewport"), &RenderingServer4D::get_current_camera);
	ClassDB::bind_method(D_METHOD("get_current_world_environment", "viewport"), &RenderingServer4D::get_current_world_environment);
	ClassDB::bind_method(D_METHOD("register_rendering_engine", "engine"), &RenderingServer4D::register_rendering_engine);
	ClassDB::bind_method(D_METHOD("unregister_rendering_engine", "name"), &RenderingServer4D::unregister_rendering_engine);
	ClassDB::bind_method(D_METHOD("get_rendering_engine_names"), &RenderingServer4D::get_rendering_engine_names);

	ADD_SIGNAL(MethodInfo("pre_render", PropertyInfo(Variant::OBJECT, "camera", PROPERTY_HINT_RESOURCE_TYPE, "Camera4D"), PropertyInfo(Variant::OBJECT, "viewport", PROPERTY_HINT_RESOURCE_TYPE, "Viewport"), PropertyInfo(Variant::OBJECT, "rendering_engine", PROPERTY_HINT_RESOURCE_TYPE, "RenderingEngine4D")));
}
