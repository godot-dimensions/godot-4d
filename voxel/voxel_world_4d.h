#pragma once

#include "../nodes/node_4d.h"
#include "data/voxel_data.h"
#include "voxel_chunk_loader.h"
#include "voxel_mesh_handler.h"

class VoxelEdit;

// Places a volume of 4D voxel data (VoxelData) into the scene tree.
// May be used to represent smaller voxel-based objects, not just whole worlds.
class VoxelWorld4D : public Node4D {
	GDCLASS(VoxelWorld4D, Node4D);

	Ref<VoxelData> _voxel_data;
	// While enabled, chunks entirely outside these bounds are never loaded,
	// and are unloaded if present; chunks partially inside may load. Not
	// necessarily aligned to the chunk grid.
	Rect4i _world_bounds;
	bool _world_bounds_enabled = false;
	VoxelMeshHandler _mesh_handler;
	// Owned; an Object so it can receive messages, so it cannot be a value member.
	VoxelChunkLoader *_chunk_loader = nullptr;

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void _validate_property(PropertyInfo &p_property) const;

public:
	Ref<VoxelData> get_voxel_data() const { return _voxel_data; }
	VoxelMeshHandler &get_mesh_handler() { return _mesh_handler; }

	Rect4i get_world_bounds() const { return _world_bounds; }
	void set_world_bounds(const Rect4i &p_world_bounds) { _world_bounds = p_world_bounds; }
	bool is_world_bounds_enabled() const { return _world_bounds_enabled; }
	void set_world_bounds_enabled(const bool p_enabled) {
		_world_bounds_enabled = p_enabled;
		notify_property_list_changed();
	}
	Vector4i get_world_bounds_position() const { return _world_bounds.position; }
	void set_world_bounds_position(const Vector4i &p_position) { _world_bounds.position = p_position; }
	Vector4i get_world_bounds_size() const { return _world_bounds.size; }
	void set_world_bounds_size(const Vector4i &p_size) { _world_bounds.size = p_size; }

	void apply_edit(const Ref<VoxelEdit> &p_edit);

	VoxelWorld4D();
	~VoxelWorld4D();
};
