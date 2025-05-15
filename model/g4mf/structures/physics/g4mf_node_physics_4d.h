#pragma once

#include "../g4mf_item_4d.h"

#include "g4mf_node_physics_motion_4d.h"

#include "../../../../physics/collision_shape_4d.h"

class G4MFNode4D;
class G4MFState4D;

class G4MFNodePhysics4D : public G4MFItem4D {
	GDCLASS(G4MFNodePhysics4D, G4MFItem4D);

	Ref<G4MFNodePhysicsMotion4D> _motion;
	int _collider_shape_index = -1;
	int _trigger_shape_index = -1;
	PackedInt32Array _trigger_node_indices;

	static const CollisionObject4D *_get_ancestor_collision_object(const Node *p_scene_parent);
	static PackedInt32Array _get_ancestor_compound_trigger_nodes(const Ref<G4MFState4D> p_state, const TypedArray<G4MFNode4D> p_state_nodes, const CollisionObject4D *p_ancestor_col_obj);
	static Node4D *_add_physics_node_to_given_node(const Ref<G4MFState4D> p_g4mf_state, const Ref<G4MFNode4D> p_g4mf_node, Node4D *p_current_node, Node4D *p_child);
	static CollisionShape4D *_generate_shape_node(const Ref<G4MFState4D> p_g4mf_state, const Ref<G4MFNode4D> p_g4mf_node, int p_shape_index);

protected:
	static void _bind_methods();

public:
	Ref<G4MFNodePhysicsMotion4D> get_motion() const { return _motion; }
	void set_motion(const Ref<G4MFNodePhysicsMotion4D> &p_motion) { _motion = p_motion; }

	int get_collider_shape_index() const { return _collider_shape_index; }
	void set_collider_shape_index(const int p_index) { _collider_shape_index = p_index; }

	int get_trigger_shape_index() const { return _trigger_shape_index; }
	void set_trigger_shape_index(const int p_index) { _trigger_shape_index = p_index; }

	PackedInt32Array get_trigger_node_indices() const { return _trigger_node_indices; }
	void set_trigger_node_indices(const PackedInt32Array &p_indices) { _trigger_node_indices = p_indices; }

	Node4D *generate_physics_node(const Ref<G4MFState4D> &p_g4mf_state, const Ref<G4MFNode4D> &p_g4mf_node, const Node *p_scene_parent) const;
	bool is_empty_of_motion_and_shapes() const; // Used to determine if this node is a compound trigger that should hold trigger node indices.
	static Ref<G4MFNodePhysics4D> from_collision_object_4d(const CollisionObject4D *p_collision_object);
	static Ref<G4MFNodePhysics4D> from_collision_shape_4d(const Ref<G4MFState4D> &p_g4mf_state, const CollisionShape4D *p_collision_shape);

	static Ref<G4MFNodePhysics4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
