#pragma once

#include "g4mf_camera_4d.h"
#include "g4mf_light_4d.h"
#include "g4mf_mesh_instance_4d.h"
#include "g4mf_model_instance_4d.h"
#include "physics/g4mf_node_physics_4d.h"

#include "../../../nodes/node_4d.h"

class G4MFState4D;

class G4MFNode4D : public G4MFItem4D {
	GDCLASS(G4MFNode4D, G4MFItem4D);

	Transform4D _transform = Transform4D();
	PackedInt32Array _children_indices;
	int _parent_index = -1;

	Ref<G4MFCamera4D> _camera;
	Ref<G4MFNodePhysics4D> _physics;
	Ref<G4MFLight4D> _light;
	Ref<G4MFMeshInstance4D> _mesh_instance;
	Ref<G4MFModelInstance4D> _model_instance;
	bool _visible = true;

	NodePath _make_node_path(const Vector<StringName> &p_path) const;

protected:
	static void _bind_methods();

public:
	// Node hierarchy.
	PackedInt32Array get_children_indices() const { return _children_indices; }
	void set_children_indices(const PackedInt32Array &p_children) { _children_indices = p_children; }

	int get_parent_index() const { return _parent_index; }
	void set_parent_index(const int p_parent_index) { _parent_index = p_parent_index; }

	bool get_visible() const { return _visible; }
	void set_visible(const bool p_visible) { _visible = p_visible; }

	// Transform properties.
	Basis4D get_basis() const { return _transform.basis; }
	void set_basis(const Basis4D &p_basis) { _transform.basis = p_basis; }

	Projection get_basis_bind() const { return _transform.basis; }
	void set_basis_bind(const Projection &p_basis) { _transform.basis = p_basis; }

	Vector4 get_position() const { return _transform.origin; }
	void set_position(const Vector4 &p_position) { _transform.origin = p_position; }

	Rotor4D get_rotor() const { return Rotor4D::from_basis(_transform.basis); }
	void set_rotor(const Rotor4D &p_rotor) { _transform.basis = p_rotor.to_basis(); }

	Vector4 get_scale() const { return _transform.basis.get_scale(); }
	void set_scale(const Vector4 &p_scale) { _transform.basis.set_scale(p_scale); }

	Transform4D get_transform() const { return _transform; }
	void set_transform(const Transform4D &p_transform) { _transform = p_transform; }

	// Component properties.
	Ref<G4MFCamera4D> get_camera() const { return _camera; }
	void set_camera(const Ref<G4MFCamera4D> &p_camera) { _camera = p_camera; }

	Ref<G4MFLight4D> get_light() const { return _light; }
	void set_light(const Ref<G4MFLight4D> &p_light) { _light = p_light; }

	Ref<G4MFNodePhysics4D> get_physics() const { return _physics; }
	void set_physics(const Ref<G4MFNodePhysics4D> &p_physics) { _physics = p_physics; }

	Ref<G4MFMeshInstance4D> get_mesh_instance() const { return _mesh_instance; }
	void set_mesh_instance(const Ref<G4MFMeshInstance4D> &p_mesh_instance) { _mesh_instance = p_mesh_instance; }

	Ref<G4MFModelInstance4D> get_model_instance() const { return _model_instance; }
	void set_model_instance(const Ref<G4MFModelInstance4D> &p_model_instance) { _model_instance = p_model_instance; }

	NodePath get_scene_node_path(const Ref<G4MFState4D> &p_g4mf_state) const;
	Transform4D get_scene_global_transform(const Ref<G4MFState4D> &p_g4mf_state) const;

	// This split is needed by G4MFDocument4D for models, so we can skip the "components" for the model instance node.
	static Ref<G4MFNode4D> from_godot_node_basic(Ref<G4MFState4D> p_g4mf_state, const Node *p_godot_node);
	void from_godot_node_components(Ref<G4MFState4D> p_g4mf_state, const Node *p_godot_node);
	// However, for the exposed version, we can just make a single function that wraps both of those.
	static Ref<G4MFNode4D> from_godot_node(Ref<G4MFState4D> p_g4mf_state, const Node *p_godot_node);

	Node *import_generate_godot_node(const Ref<G4MFState4D> &p_g4mf_state, const Node *p_scene_parent) const;
	void apply_to_godot_node(Node *p_godot_node) const;
	static Ref<G4MFNode4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary(const bool p_prefer_basis = false) const;

	// Static helper functions.
	static Array basis_4d_to_json_array(const Basis4D &p_basis);
	static Basis4D json_array_to_basis_4d(const Array &p_json_array);
};
