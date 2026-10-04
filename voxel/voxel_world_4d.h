#pragma once

#include "../model/mesh/tetra/tetra_material_4d.h"
#include "../nodes/node_4d.h"
#include "data/voxel_data_4d.h"
#include "voxel_chunk_loader_4d.h"
#include "voxel_material_palette_4d.h"
#include "voxel_mesh_handler_4d.h"

class VoxelEdit4D;

// Places a volume of 4D voxel data (VoxelData4D) into the scene tree.
// May be used to represent smaller voxel-based objects, not just whole worlds.
class VoxelWorld4D : public Node4D {
	GDCLASS(VoxelWorld4D, Node4D);

	Ref<VoxelData4D> _voxel_data;
	Ref<VoxelMaterialPalette4D> _material_palette;
	// The material of every chunk mesh, colored by a texture with one texel
	// per voxel material, baked from the palette.
	Ref<TetraMaterial4D> _mesh_material;
	// While enabled, chunks entirely outside these bounds are never loaded,
	// and are unloaded if present; chunks partially inside may load. Not
	// necessarily aligned to the chunk grid.
	Rect4i _world_bounds;
	bool _world_bounds_enabled = false;
	// Applied to the MeshInstance4D of every chunk, see MeshInstance4D::get_allow_projection.
	bool _allow_projection = true;
	VoxelMeshHandler4D _mesh_handler;
	// Owned; an Object so it can receive messages, so it cannot be a value member.
	VoxelChunkLoader4D *_chunk_loader = nullptr;

	void _update_material_texture();

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void _validate_property(PropertyInfo &p_property) const;

public:
	Ref<VoxelData4D> get_voxel_data() const { return _voxel_data; }
	VoxelMeshHandler4D &get_mesh_handler() { return _mesh_handler; }

	Ref<VoxelGenerator4D> get_generator() const { return _voxel_data->get_generator(); }
	void set_generator(const Ref<VoxelGenerator4D> &p_generator);

	Ref<VoxelMaterialPalette4D> get_material_palette() const { return _material_palette; }
	void set_material_palette(const Ref<VoxelMaterialPalette4D> &p_material_palette);
	Ref<TetraMaterial4D> get_mesh_material() const { return _mesh_material; }

	bool get_allow_projection() const { return _allow_projection; }
	void set_allow_projection(const bool p_allow_projection);

	Rect4i get_world_bounds() const { return _world_bounds; }
	void set_world_bounds(const Rect4i &p_world_bounds);
	bool is_world_bounds_enabled() const { return _world_bounds_enabled; }
	void set_world_bounds_enabled(const bool p_enabled);
	Vector4i get_world_bounds_position() const { return _world_bounds.position; }
	void set_world_bounds_position(const Vector4i &p_position);
	Vector4i get_world_bounds_size() const { return _world_bounds.size; }
	void set_world_bounds_size(const Vector4i &p_size);

	void apply_edit(const Ref<VoxelEdit4D> &p_edit);

	VoxelWorld4D();
	~VoxelWorld4D();
};
