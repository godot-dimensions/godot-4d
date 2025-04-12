#pragma once

#include "structures/g4mf_node_4d.h"

#include "../../nodes/node_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class G4MFState4D : public G4MFItem4D {
	GDCLASS(G4MFState4D, G4MFItem4D);

	Dictionary _g4mf_json;
	TypedArray<G4MFNode4D> _g4mf_nodes;
	TypedArray<Node4D> _godot_nodes;
	String _base_path;
	String _filename;
	HashSet<String> _unique_names;

protected:
	static void _bind_methods();

public:
	Dictionary get_g4mf_json() const { return _g4mf_json; }
	void set_g4mf_json(const Dictionary &p_g4mf_json) { _g4mf_json = p_g4mf_json; }

	TypedArray<G4MFNode4D> get_g4mf_nodes() const { return _g4mf_nodes; }
	void set_g4mf_nodes(const TypedArray<G4MFNode4D> &p_g4mf_nodes) { _g4mf_nodes = p_g4mf_nodes; }

	TypedArray<Node4D> get_godot_nodes() const { return _godot_nodes; }
	void set_godot_nodes(const TypedArray<Node4D> &p_godot_nodes) { _godot_nodes = p_godot_nodes; }

	String get_base_path() const { return _base_path; }
	void set_base_path(const String &p_base_path) { _base_path = p_base_path; }

	String get_filename() const { return _filename; }
	void set_filename(const String &p_filename) { _filename = p_filename; }

	void append_g4mf_node(Ref<G4MFNode4D> p_node);
	String reserve_unique_name(const String &p_requested_name);
};
