#pragma once

#include "../nodes/node_4d.h"

class VoxelWorld4D;

// Marks a position around which voxel chunks should be kept loaded, usually
// by being attached to the player. Chunks within load_distance of the trigger
// are loaded; loaded chunks are only unloaded beyond
// load_distance * unload_distance_ratio, so that chunks at the boundary are
// not repeatedly loaded and unloaded as the trigger moves around.
class VoxelLoadTrigger4D : public Node4D {
	GDCLASS(VoxelLoadTrigger4D, Node4D);

	real_t _load_distance = 16.0f;
	real_t _unload_distance_ratio = 1.5f;
	// When set, only the VoxelWorld4D at this path loads chunks around this
	// trigger; when empty, every world does.
	NodePath _voxel_world_path;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// The scene tree group that every VoxelLoadTrigger4D in the tree belongs
	// to, used by voxel worlds to find the active triggers.
	static constexpr const char *GROUP_NAME = "_voxel_load_triggers_4d";

	real_t get_load_distance() const { return _load_distance; }
	void set_load_distance(const real_t p_load_distance);

	real_t get_unload_distance_ratio() const { return _unload_distance_ratio; }
	void set_unload_distance_ratio(const real_t p_unload_distance_ratio);

	real_t get_unload_distance() const { return _load_distance * _unload_distance_ratio; }

	NodePath get_voxel_world_path() const { return _voxel_world_path; }
	void set_voxel_world_path(const NodePath &p_voxel_world_path) { _voxel_world_path = p_voxel_world_path; }

	// Whether the given world loads chunks around this trigger.
	bool applies_to(const VoxelWorld4D *p_world) const;
};
