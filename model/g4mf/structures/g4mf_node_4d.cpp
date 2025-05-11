#include "g4mf_node_4d.h"

#include "../../../math/vector_4d.h"
#include "../../../model/mesh_instance_4d.h"
#include "../g4mf_state_4d.h"

NodePath G4MFNode4D::_make_node_path(const Vector<StringName> &p_path) const {
#if GDEXTENSION
	PackedStringArray path_array;
	for (const StringName &name : p_path) {
		path_array.push_back(name);
	}
	String path_str = String("/").join(path_array);
	return NodePath(path_str);
#elif GODOT_MODULE
	return NodePath(p_path, false);
#endif
}

NodePath G4MFNode4D::get_scene_node_path(const Ref<G4MFState4D> &p_g4mf_state) const {
	Vector<StringName> path;
	if (unlikely(_parent_index < 0)) {
		path.append(".");
		return _make_node_path(path);
	}
	const TypedArray<G4MFNode4D> all_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	const int g4mf_node_count = all_g4mf_nodes.size();
	path.insert(0, get_name());
	int current_ancestor = _parent_index;
	// Don't include the root node (node 0) in the path.
	// Ex: A/B/C, A is root, we want B/C only.
	while (current_ancestor > 0) {
		ERR_FAIL_INDEX_V(current_ancestor, g4mf_node_count, NodePath());
		const Ref<G4MFNode4D> current_g4mf_node = all_g4mf_nodes[current_ancestor];
		path.insert(0, current_g4mf_node->get_name());
		current_ancestor = current_g4mf_node->get_parent_index();
	}
	return _make_node_path(path);
}

// Returns the global transform of this node relative to the root node of the scene.
Transform4D G4MFNode4D::get_scene_global_transform(const Ref<G4MFState4D> &p_g4mf_state) const {
	Transform4D global_transform = _transform;
	if (_parent_index < 0) {
		return global_transform;
	}
	const TypedArray<G4MFNode4D> all_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	const int g4mf_node_count = all_g4mf_nodes.size();
	int current_ancestor = _parent_index;
	while (current_ancestor > 0) {
		ERR_FAIL_INDEX_V(current_ancestor, g4mf_node_count, global_transform);
		const Ref<G4MFNode4D> current_g4mf_node = all_g4mf_nodes[current_ancestor];
		global_transform = current_g4mf_node->get_transform() * global_transform;
		current_ancestor = current_g4mf_node->get_parent_index();
	}
	return global_transform;
}

Ref<G4MFNode4D> G4MFNode4D::from_godot_node(Ref<G4MFState4D> p_g4mf_state, const Node *p_godot_node) {
	Ref<G4MFNode4D> g4mf_node;
	g4mf_node.instantiate();
	ERR_FAIL_NULL_V(p_godot_node, g4mf_node);
	g4mf_node->set_name(p_godot_node->get_name());
	if (p_godot_node->has_meta(StringName("extras"))) {
		g4mf_node->set_meta(StringName("extras"), p_godot_node->get_meta(StringName("extras")));
	}
	const Node4D *godot_node_4d = Object::cast_to<Node4D>(p_godot_node);
	if (godot_node_4d) {
		g4mf_node->set_transform(godot_node_4d->get_transform());
		g4mf_node->set_visible(godot_node_4d->is_visible());
		const CollisionObject4D *col_obj = Object::cast_to<CollisionObject4D>(p_godot_node);
		if (col_obj) {
			g4mf_node->set_physics(G4MFNodePhysics4D::from_collision_object_4d(col_obj));
			return g4mf_node;
		}
		const CollisionShape4D *col_shape = Object::cast_to<CollisionShape4D>(p_godot_node);
		if (col_shape) {
			g4mf_node->set_physics(G4MFNodePhysics4D::from_collision_shape_4d(p_g4mf_state, col_shape));
			return g4mf_node;
		}
		const MeshInstance4D *mesh_instance = Object::cast_to<MeshInstance4D>(p_godot_node);
		if (mesh_instance) {
			const Ref<Mesh4D> mesh = mesh_instance->get_mesh();
			if (mesh.is_valid()) {
				Ref<Material4D> material = mesh_instance->get_material_override();
				if (material.is_null()) {
					material = mesh->get_material();
				}
				const int mesh_index = G4MFMesh4D::convert_mesh_into_state(p_g4mf_state, mesh, material, true);
				g4mf_node->set_mesh_index(mesh_index);
			}
			return g4mf_node;
		}
		const Camera4D *godot_camera_4d = Object::cast_to<Camera4D>(godot_node_4d);
		if (godot_camera_4d) {
			g4mf_node->set_camera(G4MFCamera4D::convert_camera_4d(godot_camera_4d));
			return g4mf_node;
		}
	}
	return g4mf_node;
}

Node4D *G4MFNode4D::generate_godot_node(const Ref<G4MFState4D> &p_g4mf_state, const Node *p_scene_parent, const bool p_force_wireframe) const {
	Node4D *ret_node = nullptr;
	if (_physics.is_valid()) {
		ret_node = _physics->generate_physics_node(p_g4mf_state, Ref<G4MFNode4D>(this), p_scene_parent);
	}
	if (_mesh_index >= 0) {
		TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
		ERR_FAIL_INDEX_V(_mesh_index, state_g4mf_meshes.size(), nullptr);
		Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[_mesh_index];
		Ref<Mesh4D> mesh = g4mf_mesh->generate_mesh(p_g4mf_state, p_force_wireframe);
		if (mesh.is_valid()) {
			MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
			mesh_instance->set_mesh(mesh);
			if (ret_node == nullptr) {
				ret_node = mesh_instance;
			} else {
				mesh_instance->set_name(p_g4mf_state->reserve_unique_name(get_name() + "Mesh"));
				ret_node->add_child(mesh_instance);
			}
		}
	}
	if (_camera.is_valid()) {
		Camera4D *camera = _camera->generate_camera_4d();
		if (ret_node == nullptr) {
			ret_node = camera;
		} else {
			camera->set_name(p_g4mf_state->reserve_unique_name(get_name() + "Camera"));
			ret_node->add_child(camera);
		}
	}
	if (ret_node == nullptr) {
		ret_node = memnew(Node4D);
	}
	apply_to_godot_node_4d(ret_node);
	return ret_node;
}

void G4MFNode4D::apply_to_godot_node_4d(Node4D *p_godot_node) const {
	ERR_FAIL_NULL(p_godot_node);
	p_godot_node->set_name(get_name());
	p_godot_node->set_transform(_transform);
	p_godot_node->set_visible(_visible);
	if (has_meta(StringName("extras"))) {
		p_godot_node->set_meta(StringName("extras"), get_meta(StringName("extras")));
	}
}

Ref<G4MFNode4D> G4MFNode4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFNode4D> node;
	node.instantiate();
	node->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("basis")) {
		node->_transform.basis = json_array_to_basis_4d(p_dict["basis"]);
	} else {
		if (p_dict.has("rotor")) {
			node->_transform.basis = json_array_to_rotor_4d(p_dict["rotor"]).to_basis();
		}
		if (p_dict.has("scale")) {
			const Array scale_array = p_dict["scale"];
			if (scale_array.size() == 1) {
				node->_transform.basis.scale_uniform(scale_array[0]);
			} else {
				node->_transform.basis.scale_local(Vector4D::from_json_array(p_dict["scale"]));
			}
		}
	}
	if (p_dict.has("children")) {
		node->_children_indices = json_array_to_int32_array(p_dict["children"]);
	}
	if (p_dict.has("parent")) {
		// Not a part of the G4MF spec but useful if an extension wants to
		// compose external G4MF nodes on top of a G4MF scene. (G4MFX?)
		node->_parent_index = p_dict["parent"];
	}
	if (p_dict.has("position")) {
		node->_transform.origin = Vector4D::from_json_array(p_dict["position"]);
	}
	if (p_dict.has("visible")) {
		node->_visible = p_dict["visible"];
	}
	// Component properties.
	if (p_dict.has("camera")) {
		node->_camera = G4MFCamera4D::from_dictionary(p_dict["camera"]);
	}
	if (p_dict.has("mesh")) {
		node->_mesh_index = p_dict["mesh"];
	}
	if (p_dict.has("physics")) {
		node->_physics = G4MFNodePhysics4D::from_dictionary(p_dict["physics"]);
	}
	return node;
}

Dictionary G4MFNode4D::to_dictionary(const bool p_prefer_basis) const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (!_transform.basis.is_equal_approx(Basis4D())) {
		if (!p_prefer_basis && _transform.basis.is_orthogonal() && _transform.basis.determinant() > 0) {
			const Rotor4D rotor = Rotor4D::from_basis(_transform.basis);
			// TODO: Double rotations in Rotor4D are still buggy, so export as basis for now.
			if (Math::is_zero_approx(rotor.get_pseudoscalar())) {
				if (!_transform.basis.is_diagonal()) {
					dict["rotor"] = rotor_4d_to_json_array(rotor);
				}
				Vector4 scale = _transform.basis.get_scale();
				if (!scale.is_equal_approx(Vector4(1, 1, 1, 1))) {
					if (Vector4D::is_uniform(scale)) {
						Array uniform_scale_array;
						uniform_scale_array.push_back(scale.x);
						dict["scale"] = uniform_scale_array;
					} else {
						dict["scale"] = Vector4D::to_json_array(scale);
					}
				}
			} else {
				dict["basis"] = basis_4d_to_json_array(_transform.basis);
			}
		} else {
			dict["basis"] = basis_4d_to_json_array(_transform.basis);
		}
	}
	if (!_children_indices.is_empty()) {
		dict["children"] = int32_array_to_json_array(_children_indices);
	}
	if (!_transform.origin.is_zero_approx()) {
		dict["position"] = Vector4D::to_json_array(_transform.origin);
	}
	if (!_visible) {
		dict["visible"] = _visible;
	}
	// Component properties.
	if (_camera.is_valid()) {
		dict["camera"] = _camera->to_dictionary();
	}
	if (_mesh_index >= 0) {
		dict["mesh"] = _mesh_index;
	}
	if (_physics.is_valid()) {
		dict["physics"] = _physics->to_dictionary();
	}
	return dict;
}

// Static helper functions.

Array G4MFNode4D::basis_4d_to_json_array(const Basis4D &p_basis) {
	Array json_array;
	json_array.resize(16);
	json_array[0] = p_basis.x.x;
	json_array[1] = p_basis.x.y;
	json_array[2] = p_basis.x.z;
	json_array[3] = p_basis.x.w;
	json_array[4] = p_basis.y.x;
	json_array[5] = p_basis.y.y;
	json_array[6] = p_basis.y.z;
	json_array[7] = p_basis.y.w;
	json_array[8] = p_basis.z.x;
	json_array[9] = p_basis.z.y;
	json_array[10] = p_basis.z.z;
	json_array[11] = p_basis.z.w;
	json_array[12] = p_basis.w.x;
	json_array[13] = p_basis.w.y;
	json_array[14] = p_basis.w.z;
	json_array[15] = p_basis.w.w;
	return json_array;
}

Basis4D G4MFNode4D::json_array_to_basis_4d(const Array &p_json_array) {
	if (likely(p_json_array.size() == 16)) {
		return Basis4D(
				p_json_array[0], p_json_array[1], p_json_array[2], p_json_array[3],
				p_json_array[4], p_json_array[5], p_json_array[6], p_json_array[7],
				p_json_array[8], p_json_array[9], p_json_array[10], p_json_array[11],
				p_json_array[12], p_json_array[13], p_json_array[14], p_json_array[15]);
	}
	if (p_json_array.size() > 16) {
		const int dimension = (int)Math::sqrt((double)p_json_array.size());
		return Basis4D(
				p_json_array[0], p_json_array[1], p_json_array[2], p_json_array[3],
				p_json_array[dimension], p_json_array[dimension + 1], p_json_array[dimension + 2], p_json_array[dimension + 3],
				p_json_array[dimension * 2], p_json_array[dimension * 2 + 1], p_json_array[dimension * 2 + 2], p_json_array[dimension * 2 + 3],
				p_json_array[dimension * 3], p_json_array[dimension * 3 + 1], p_json_array[dimension * 3 + 2], p_json_array[dimension * 3 + 3]);
	}
	if (p_json_array.size() >= 9) {
		return Basis4D(
				p_json_array[0], p_json_array[1], p_json_array[2], 0.0,
				p_json_array[3], p_json_array[4], p_json_array[5], 0.0,
				p_json_array[6], p_json_array[7], p_json_array[8], 0.0,
				0.0, 0.0, 0.0, 1.0);
	}
	if (p_json_array.size() >= 4) {
		return Basis4D(
				p_json_array[0], p_json_array[1], 0.0, 0.0,
				p_json_array[2], p_json_array[3], 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0);
	}
	// Interpret a 1D basis as a uniform scale.
	if (p_json_array.size() >= 1) {
		return Basis4D(
				p_json_array[0], 0.0, 0.0, 0.0,
				0.0, p_json_array[0], 0.0, 0.0,
				0.0, 0.0, p_json_array[0], 0.0,
				0.0, 0.0, 0.0, p_json_array[0]);
	}
	return Basis4D();
}

void G4MFNode4D::_bind_methods() {
	// Node hierarchy.
	ClassDB::bind_method(D_METHOD("get_children_indices"), &G4MFNode4D::get_children_indices);
	ClassDB::bind_method(D_METHOD("set_children_indices", "children"), &G4MFNode4D::set_children_indices);
	ClassDB::bind_method(D_METHOD("get_parent_index"), &G4MFNode4D::get_parent_index);
	ClassDB::bind_method(D_METHOD("set_parent_index", "parent_index"), &G4MFNode4D::set_parent_index);
	ClassDB::bind_method(D_METHOD("get_visible"), &G4MFNode4D::get_visible);
	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &G4MFNode4D::set_visible);
	// Transform properties.
	ClassDB::bind_method(D_METHOD("get_basis"), &G4MFNode4D::get_basis_bind);
	ClassDB::bind_method(D_METHOD("set_basis", "basis"), &G4MFNode4D::set_basis_bind);
	ClassDB::bind_method(D_METHOD("get_position"), &G4MFNode4D::get_position);
	ClassDB::bind_method(D_METHOD("set_position", "position"), &G4MFNode4D::set_position);
	ClassDB::bind_method(D_METHOD("get_scale"), &G4MFNode4D::get_scale);
	ClassDB::bind_method(D_METHOD("set_scale", "scale"), &G4MFNode4D::set_scale);
	// Component properties.
	ClassDB::bind_method(D_METHOD("get_camera"), &G4MFNode4D::get_camera);
	ClassDB::bind_method(D_METHOD("set_camera", "camera"), &G4MFNode4D::set_camera);
	ClassDB::bind_method(D_METHOD("get_mesh_index"), &G4MFNode4D::get_mesh_index);
	ClassDB::bind_method(D_METHOD("set_mesh_index", "mesh_index"), &G4MFNode4D::set_mesh_index);

	ClassDB::bind_method(D_METHOD("get_scene_node_path", "g4mf_state"), &G4MFNode4D::get_scene_node_path);

	ClassDB::bind_static_method("G4MFNode4D", D_METHOD("from_godot_node", "g4mf_state", "godot_node"), &G4MFNode4D::from_godot_node);
	ClassDB::bind_method(D_METHOD("generate_godot_node", "g4mf_state", "scene_parent", "force_wireframe"), &G4MFNode4D::generate_godot_node);
	ClassDB::bind_method(D_METHOD("apply_to_godot_node_4d", "node"), &G4MFNode4D::apply_to_godot_node_4d);

	ClassDB::bind_static_method("G4MFNode4D", D_METHOD("from_dictionary", "dict"), &G4MFNode4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary", "prefer_basis"), &G4MFNode4D::to_dictionary);

	// Node hierarchy.
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "children_indices"), "set_children_indices", "get_children_indices");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "parent_index"), "set_parent_index", "get_parent_index");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible"), "set_visible", "get_visible");
	// Transform properties.
	ADD_PROPERTY(PropertyInfo(Variant::PROJECTION, "basis"), "set_basis", "get_basis");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "position", PROPERTY_HINT_NONE, "suffix:m"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "scale", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_scale", "get_scale");
	// Component properties.
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "camera", PROPERTY_HINT_RESOURCE_TYPE, "G4MFCamera4D"), "set_camera", "get_camera");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_index"), "set_mesh_index", "get_mesh_index");
}
