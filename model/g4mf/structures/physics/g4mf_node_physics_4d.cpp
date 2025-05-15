#include "g4mf_node_physics_4d.h"

#include "../../g4mf_state_4d.h"
#include "../g4mf_node_4d.h"

#include "../../../../physics/bodies/area_4d.h"
#include "../../../../physics/bodies/static_body_4d.h"

const CollisionObject4D *G4MFNodePhysics4D::_get_ancestor_collision_object(const Node *p_scene_parent) {
	const Node *ancestor = p_scene_parent;
	while (ancestor) {
		const Node4D *ancestor_4d = Object::cast_to<Node4D>(ancestor);
		if (unlikely(!ancestor_4d)) {
			return nullptr;
		}
		const CollisionObject4D *co = Object::cast_to<CollisionObject4D>(ancestor);
		if (likely(co)) {
			return co;
		}
		ancestor = ancestor->get_parent();
	}
	return nullptr;
}

PackedInt32Array G4MFNodePhysics4D::_get_ancestor_compound_trigger_nodes(Ref<G4MFState4D> p_state, TypedArray<G4MFNode4D> p_state_nodes, const CollisionObject4D *p_ancestor_col_obj) {
	PackedInt32Array ret;
	if (p_ancestor_col_obj == nullptr) {
		return ret;
	}
	const int ancestor_index = p_state->get_node_index(p_ancestor_col_obj);
	ERR_FAIL_INDEX_V(ancestor_index, p_state_nodes.size(), ret);
	const Ref<G4MFNode4D> ancestor_g4mf_node = p_state_nodes[ancestor_index];
	ERR_FAIL_COND_V(ancestor_g4mf_node.is_null(), ret);
	const Ref<G4MFNodePhysics4D> physics = ancestor_g4mf_node->get_physics();
	if (physics.is_null()) {
		return ret;
	}
	ret = physics->get_trigger_node_indices();
	return ret;
}

// Either add the child to the parent, or return the child if there is no parent.
Node4D *G4MFNodePhysics4D::_add_physics_node_to_given_node(const Ref<G4MFState4D> p_g4mf_state, Ref<G4MFNode4D> p_g4mf_node, Node4D *p_current_node, Node4D *p_child) {
	if (!p_current_node) {
		return p_child;
	}
	String suffix;
	if (Object::cast_to<CollisionShape4D>(p_child)) {
		suffix = "Shape";
	} else if (Object::cast_to<Area4D>(p_child)) {
		suffix = "Trigger";
	} else {
		suffix = "Collider";
	}
	p_child->set_name(p_g4mf_state->reserve_unique_name(p_g4mf_node->get_name() + suffix));
	p_current_node->add_child(p_child);
	return p_current_node;
}

CollisionShape4D *G4MFNodePhysics4D::_generate_shape_node(const Ref<G4MFState4D> p_g4mf_state, const Ref<G4MFNode4D> p_g4mf_node, int p_shape_index) {
	CollisionShape4D *col_shape = memnew(CollisionShape4D);
	const TypedArray<G4MFShape4D> shapes = p_g4mf_state->get_g4mf_shapes();
	ERR_FAIL_INDEX_V(p_shape_index, shapes.size(), col_shape);
	const Ref<G4MFShape4D> shape = shapes[p_shape_index];
	col_shape->set_shape(shape->generate_shape(p_g4mf_state));
	return col_shape;
}

Node4D *G4MFNodePhysics4D::generate_physics_node(const Ref<G4MFState4D> &p_g4mf_state, const Ref<G4MFNode4D> &p_g4mf_node, const Node *p_scene_parent) const {
	const CollisionObject4D *col_obj = nullptr;
	Node4D *ret = nullptr;
	if (_motion.is_valid()) {
		CollisionObject4D *from_motion = _motion->to_physics_body();
		col_obj = from_motion;
		ret = from_motion;
	} else if (!_trigger_node_indices.is_empty() || is_empty_of_motion_and_shapes()) {
		Area4D *area_node = memnew(Area4D);
		col_obj = area_node;
		ret = area_node;
	} else {
		col_obj = _get_ancestor_collision_object(p_scene_parent);
		if (Object::cast_to<Area4D>(col_obj) && _trigger_shape_index > -1) {
			// At this point, we found an ancestor Area3D node. But do we want to use it for this trigger shape?
			const TypedArray<G4MFNode4D> state_nodes = p_g4mf_state->get_g4mf_nodes();
			const int32_t self_index = state_nodes.find(p_g4mf_node);
			const PackedInt32Array compound_trigger_nodes = _get_ancestor_compound_trigger_nodes(p_g4mf_state, state_nodes, col_obj);
			// Check if the ancestor specifies compound trigger nodes, and if this node is in there.
			if (compound_trigger_nodes.size() > 0 && !compound_trigger_nodes.has(self_index)) {
				// If the compound trigger we found is not the intended user of
				// this shape node, then we need to create a new Area4D node.
				Area4D *area_node = memnew(Area4D);
				col_obj = area_node;
				ret = area_node;
			}
		}
	}
	const bool is_col_obj_trigger = Object::cast_to<Area4D>(col_obj) != nullptr;
	const bool is_col_obj_solid = Object::cast_to<PhysicsBody4D>(col_obj) != nullptr;
	if (_collider_shape_index > -1 && is_col_obj_solid) {
		CollisionShape4D *col_shape_node = _generate_shape_node(p_g4mf_state, p_g4mf_node, _collider_shape_index);
		ret = _add_physics_node_to_given_node(p_g4mf_state, p_g4mf_node, ret, col_shape_node);
	}
	if (_trigger_shape_index > -1) {
		CollisionShape4D *trigger_shape_node = _generate_shape_node(p_g4mf_state, p_g4mf_node, _trigger_shape_index);
		if (is_col_obj_trigger) {
			// The ancestor collision object is already an Area4D node, so we just add the shape to it.
			ret = _add_physics_node_to_given_node(p_g4mf_state, p_g4mf_node, ret, trigger_shape_node);
		} else {
			// There isn't an ancestor Area4D node, so we need to create one, and add the shape to it.
			Area4D *area_node = memnew(Area4D);
			ret = _add_physics_node_to_given_node(p_g4mf_state, p_g4mf_node, ret, area_node);
			_add_physics_node_to_given_node(p_g4mf_state, p_g4mf_node, area_node, trigger_shape_node);
		}
	}
	if (_collider_shape_index > -1 && !is_col_obj_solid) {
		CollisionShape4D *col_shape_node = _generate_shape_node(p_g4mf_state, p_g4mf_node, _collider_shape_index);
		if (is_col_obj_trigger || Object::cast_to<Area4D>(ret) != nullptr) {
			// The ancestor collision object is an Area4D node, so we need to create a StaticBody4D node.
			StaticBody4D *static_body_node = memnew(StaticBody4D);
			ret = _add_physics_node_to_given_node(p_g4mf_state, p_g4mf_node, ret, static_body_node);
			_add_physics_node_to_given_node(p_g4mf_state, p_g4mf_node, static_body_node, col_shape_node);
		} else {
			// There isn't an ancestor Area4D node, so the ancestor is either a physics body or nothing.
			ret = _add_physics_node_to_given_node(p_g4mf_state, p_g4mf_node, ret, col_shape_node);
		}
	}
	return ret;
}

bool G4MFNodePhysics4D::is_empty_of_motion_and_shapes() const {
	return _motion.is_null() && _collider_shape_index == -1 && _trigger_shape_index == -1;
}

Ref<G4MFNodePhysics4D> G4MFNodePhysics4D::from_collision_object_4d(const CollisionObject4D *p_collision_object) {
	Ref<G4MFNodePhysics4D> node_physics;
	node_physics.instantiate();
	const PhysicsBody4D *physics_body = Object::cast_to<PhysicsBody4D>(p_collision_object);
	if (physics_body == nullptr) {
		return node_physics;
	}
	Ref<G4MFNodePhysicsMotion4D> motion = G4MFNodePhysicsMotion4D::from_physics_body(physics_body);
	node_physics->set_motion(motion);
	return node_physics;
}

Ref<G4MFNodePhysics4D> G4MFNodePhysics4D::from_collision_shape_4d(const Ref<G4MFState4D> &p_g4mf_state, const CollisionShape4D *p_collision_shape) {
	// Put the shape into the state's shapes array.
	int shape_index = G4MFShape4D::convert_shape_into_state(p_g4mf_state, p_collision_shape->get_shape());
	// Reference the shape in this node's physics properties. But which one? We need to check the collision object type.
	Ref<G4MFNodePhysics4D> node_physics;
	node_physics.instantiate();
	const CollisionObject4D *col_obj = _get_ancestor_collision_object(p_collision_shape);
	if (Object::cast_to<Area4D>(col_obj) == nullptr) {
		node_physics->set_collider_shape_index(shape_index);
		return node_physics;
	}
	node_physics->set_trigger_shape_index(shape_index);
	// We also need to inform a parent compound trigger node about this shape.
	const TypedArray<G4MFNode4D> state_nodes = p_g4mf_state->get_g4mf_nodes();
	const int32_t col_obj_index = p_g4mf_state->get_node_index(col_obj);
	ERR_FAIL_INDEX_V(col_obj_index, state_nodes.size(), node_physics);
	Ref<G4MFNode4D> col_obj_g4mf_node = state_nodes[col_obj_index];
	ERR_FAIL_COND_V(col_obj_g4mf_node.is_null(), node_physics);
	Ref<G4MFNodePhysics4D> col_obj_physics = col_obj_g4mf_node->get_physics();
	ERR_FAIL_COND_V(col_obj_physics.is_null(), node_physics);
	ERR_FAIL_COND_V(!col_obj_physics->is_empty_of_motion_and_shapes(), node_physics);
	PackedInt32Array trigger_node_indices = col_obj_physics->get_trigger_node_indices();
	// Assumes that this function is called right before the node is added to the G4MF state.
	const int new_node_index = p_g4mf_state->get_g4mf_nodes().size();
	trigger_node_indices.append(new_node_index);
	col_obj_physics->set_trigger_node_indices(trigger_node_indices);
	return node_physics;
}

Ref<G4MFNodePhysics4D> G4MFNodePhysics4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFNodePhysics4D> g4mf_physics;
	g4mf_physics.instantiate();
	g4mf_physics->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("motion")) {
		Ref<G4MFNodePhysicsMotion4D> motion = G4MFNodePhysicsMotion4D::from_dictionary(p_dict["motion"]);
		g4mf_physics->set_motion(motion);
	}
	if (p_dict.has("collider")) {
		Dictionary collider = p_dict["collider"];
		if (collider.has("shape")) {
			g4mf_physics->set_collider_shape_index(collider["shape"]);
		}
	}
	if (p_dict.has("trigger")) {
		Dictionary trigger = p_dict["trigger"];
		if (trigger.has("shape")) {
			g4mf_physics->set_trigger_shape_index(trigger["shape"]);
		}
		if (trigger.has("nodes")) {
			g4mf_physics->set_trigger_node_indices(G4MFItem4D::json_array_to_int32_array(trigger["nodes"]));
		}
	}
	return g4mf_physics;
}

Dictionary G4MFNodePhysics4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_motion.is_valid()) {
		dict["motion"] = _motion->to_dictionary();
	}
	if (_collider_shape_index > -1) {
		Dictionary collider;
		collider["shape"] = _collider_shape_index;
		dict["collider"] = collider;
	}
	const bool use_compound_trigger_nodes = !_trigger_node_indices.is_empty() || is_empty_of_motion_and_shapes();
	if (_trigger_shape_index > -1 || use_compound_trigger_nodes) {
		Dictionary trigger;
		if (_trigger_shape_index > -1) {
			trigger["shape"] = _trigger_shape_index;
		}
		if (use_compound_trigger_nodes) {
			trigger["nodes"] = G4MFItem4D::int32_array_to_json_array(_trigger_node_indices);
		}
		dict["trigger"] = trigger;
	}
	return dict;
}

void G4MFNodePhysics4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_motion"), &G4MFNodePhysics4D::get_motion);
	ClassDB::bind_method(D_METHOD("set_motion", "motion"), &G4MFNodePhysics4D::set_motion);
	ClassDB::bind_method(D_METHOD("get_collider_shape_index"), &G4MFNodePhysics4D::get_collider_shape_index);
	ClassDB::bind_method(D_METHOD("set_collider_shape_index", "index"), &G4MFNodePhysics4D::set_collider_shape_index);
	ClassDB::bind_method(D_METHOD("get_trigger_shape_index"), &G4MFNodePhysics4D::get_trigger_shape_index);
	ClassDB::bind_method(D_METHOD("set_trigger_shape_index", "index"), &G4MFNodePhysics4D::set_trigger_shape_index);
	ClassDB::bind_method(D_METHOD("get_trigger_node_indices"), &G4MFNodePhysics4D::get_trigger_node_indices);
	ClassDB::bind_method(D_METHOD("set_trigger_node_indices", "indices"), &G4MFNodePhysics4D::set_trigger_node_indices);

	ClassDB::bind_method(D_METHOD("generate_physics_node", "g4mf_state", "g4mf_node", "scene_parent"), &G4MFNodePhysics4D::generate_physics_node);
	ClassDB::bind_static_method("G4MFNodePhysics4D", D_METHOD("from_collision_object_4d", "collision_object"), &G4MFNodePhysics4D::from_collision_object_4d);
	ClassDB::bind_static_method("G4MFNodePhysics4D", D_METHOD("from_collision_shape_4d", "g4mf_state", "collision_shape"), &G4MFNodePhysics4D::from_collision_shape_4d);

	ClassDB::bind_static_method("G4MFNodePhysics4D", D_METHOD("from_dictionary", "dict"), &G4MFNodePhysics4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFNodePhysics4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "motion", PROPERTY_HINT_RESOURCE_TYPE, "G4MFNodePhysicsMotion4D"), "set_motion", "get_motion");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collider_shape_index"), "set_collider_shape_index", "get_collider_shape_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "trigger_shape_index"), "set_trigger_shape_index", "get_trigger_shape_index");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "trigger_node_indices"), "set_trigger_node_indices", "get_trigger_node_indices");
}
