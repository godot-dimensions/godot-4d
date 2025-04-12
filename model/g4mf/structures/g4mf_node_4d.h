#pragma once

#include "g4mf_item_4d.h"

#include "../../../nodes/node_4d.h"

class G4MFState4D;

class G4MFNode4D : public G4MFItem4D {
	GDCLASS(G4MFNode4D, G4MFItem4D);

	PackedInt32Array _children_indices;
	int _parent_index = -1;
	Transform4D _transform = Transform4D();
	bool _visible = true;

	NodePath _make_node_path(const Vector<StringName> &p_path) const;

protected:
	static void _bind_methods();

public:
	PackedInt32Array get_children_indices() const { return _children_indices; }
	void set_children_indices(const PackedInt32Array &p_children) { _children_indices = p_children; }

	int get_parent_index() const { return _parent_index; }
	void set_parent_index(const int p_parent_index) { _parent_index = p_parent_index; }

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

	bool get_visible() const { return _visible; }
	void set_visible(const bool p_visible) { _visible = p_visible; }

	NodePath get_scene_node_path(const Ref<G4MFState4D> &p_g4mf_state) const;

	static Ref<G4MFNode4D> from_godot_node(const Node *p_godot_node);
	void apply_to_godot_node_4d(Node4D *p_godot_node) const;
	static Ref<G4MFNode4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary(const bool p_prefer_basis = false) const;

	// Static helper functions.
	static Array basis_4d_to_json_array(const Basis4D &p_basis);
	static Array rotor_4d_to_json_array(const Rotor4D &p_rotor);
	static Basis4D json_array_to_basis_4d(const Array &p_json_array);
	static Rotor4D json_array_to_rotor_4d(const Array &p_json_array);
};
