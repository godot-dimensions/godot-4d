#pragma once

#include "structures/g4mf_accessor_4d.h"
#include "structures/g4mf_buffer_view_4d.h"
#include "structures/g4mf_light_4d.h"
#include "structures/g4mf_material_4d.h"
#include "structures/g4mf_mesh_4d.h"
#include "structures/g4mf_node_4d.h"
#include "structures/g4mf_texture_4d.h"
#include "structures/physics/g4mf_shape_4d.h"

#include "../../nodes/node_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class G4MFState4D : public G4MFItem4D {
	GDCLASS(G4MFState4D, G4MFItem4D);

public:
	enum ExternalDataMode {
		EXTERNAL_DATA_MODE_AUTOMATIC,
		EXTERNAL_DATA_MODE_EMBED_EVERYTHING,
		EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES,
		EXTERNAL_DATA_MODE_SEPARATE_BINARY_BLOBS,
		EXTERNAL_DATA_MODE_SEPARATE_RESOURCE_FILES,
	};

private:
	// Data for the file itself.
	TypedArray<G4MFAccessor4D> _accessors;
	TypedArray<PackedByteArray> _buffers;
	TypedArray<G4MFBufferView4D> _buffer_views;
	TypedArray<G4MFLight4D> _g4mf_lights;
	TypedArray<G4MFTexture4D> _g4mf_textures;
	TypedArray<G4MFMaterial4D> _g4mf_materials;
	TypedArray<G4MFMesh4D> _g4mf_meshes;
	TypedArray<G4MFShape4D> _g4mf_shapes;
	TypedArray<G4MFNode4D> _g4mf_nodes;
	TypedArray<Node4D> _godot_nodes;
	HashSet<String> _unique_names;
	Dictionary _g4mf_json;
	String _base_path;
	String _filename;

	// Settings for handling the file.
	ExternalDataMode _external_data_mode = ExternalDataMode::EXTERNAL_DATA_MODE_AUTOMATIC;

protected:
	static void _bind_methods();

public:
	TypedArray<G4MFAccessor4D> get_accessors() const { return _accessors; }
	void set_accessors(const TypedArray<G4MFAccessor4D> &p_accessors) { _accessors = p_accessors; }

	String get_base_path() const { return _base_path; }
	void set_base_path(const String &p_base_path) { _base_path = p_base_path; }

	TypedArray<PackedByteArray> get_buffers() const { return _buffers; }
	void set_buffers(const TypedArray<PackedByteArray> &p_buffers) { _buffers = p_buffers; }

	TypedArray<G4MFBufferView4D> get_buffer_views() const { return _buffer_views; }
	void set_buffer_views(const TypedArray<G4MFBufferView4D> &p_buffer_views) { _buffer_views = p_buffer_views; }

	String get_filename() const { return _filename; }
	void set_filename(const String &p_filename) { _filename = p_filename; }

	Dictionary get_g4mf_json() const { return _g4mf_json; }
	void set_g4mf_json(const Dictionary &p_g4mf_json) { _g4mf_json = p_g4mf_json; }

	TypedArray<G4MFTexture4D> get_g4mf_textures() const { return _g4mf_textures; }
	void set_g4mf_textures(const TypedArray<G4MFTexture4D> &p_g4mf_textures) { _g4mf_textures = p_g4mf_textures; }

	TypedArray<G4MFMaterial4D> get_g4mf_materials() const { return _g4mf_materials; }
	void set_g4mf_materials(const TypedArray<G4MFMaterial4D> &p_g4mf_materials) { _g4mf_materials = p_g4mf_materials; }

	TypedArray<G4MFMesh4D> get_g4mf_meshes() const { return _g4mf_meshes; }
	void set_g4mf_meshes(const TypedArray<G4MFMesh4D> &p_g4mf_meshes) { _g4mf_meshes = p_g4mf_meshes; }

	TypedArray<G4MFShape4D> get_g4mf_shapes() const { return _g4mf_shapes; }
	void set_g4mf_shapes(const TypedArray<G4MFShape4D> &p_g4mf_shapes) { _g4mf_shapes = p_g4mf_shapes; }

	TypedArray<G4MFLight4D> get_g4mf_lights() const { return _g4mf_lights; }
	void set_g4mf_lights(const TypedArray<G4MFLight4D> &p_g4mf_lights) { _g4mf_lights = p_g4mf_lights; }

	TypedArray<G4MFNode4D> get_g4mf_nodes() const { return _g4mf_nodes; }
	void set_g4mf_nodes(const TypedArray<G4MFNode4D> &p_g4mf_nodes) { _g4mf_nodes = p_g4mf_nodes; }

	TypedArray<Node4D> get_godot_nodes() const { return _godot_nodes; }
	void set_godot_nodes(const TypedArray<Node4D> &p_godot_nodes) { _godot_nodes = p_godot_nodes; }

	int append_g4mf_node(Ref<G4MFNode4D> p_node);
	int get_node_index(const Node4D *p_node);
	String reserve_unique_name(const String &p_requested_name);

	ExternalDataMode get_external_data_mode() const { return _external_data_mode; }
	void set_external_data_mode(ExternalDataMode p_external_data_mode) { _external_data_mode = p_external_data_mode; }
	bool is_text_file() const;
	bool should_separate_binary_blobs() const;
	bool should_separate_resource_files() const;
};

VARIANT_ENUM_CAST(G4MFState4D::ExternalDataMode);
