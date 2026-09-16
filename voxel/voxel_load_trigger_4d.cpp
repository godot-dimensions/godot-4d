#include "voxel_load_trigger_4d.h"

#include "voxel_world_4d.h"

bool VoxelLoadTrigger4D::applies_to(const VoxelWorld4D *p_world) const {
	if (_voxel_world_path.is_empty()) {
		return true;
	}
	return get_node_or_null(_voxel_world_path) == static_cast<const Node *>(p_world);
}

void VoxelLoadTrigger4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			add_to_group(GROUP_NAME);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_from_group(GROUP_NAME);
		} break;
	}
}

void VoxelLoadTrigger4D::set_load_distance(const real_t p_load_distance) {
	ERR_FAIL_COND_MSG(p_load_distance < 0.0, "VoxelLoadTrigger4D load distance must not be negative. Refusing to set.");
	_load_distance = p_load_distance;
}

void VoxelLoadTrigger4D::set_unload_distance_ratio(const real_t p_unload_distance_ratio) {
	ERR_FAIL_COND_MSG(p_unload_distance_ratio < 1.0, "VoxelLoadTrigger4D unload distance ratio must be at least 1, so that chunks are not unloaded while still inside the load distance. Refusing to set.");
	_unload_distance_ratio = p_unload_distance_ratio;
}

void VoxelLoadTrigger4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_load_distance"), &VoxelLoadTrigger4D::get_load_distance);
	ClassDB::bind_method(D_METHOD("set_load_distance", "load_distance"), &VoxelLoadTrigger4D::set_load_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "load_distance", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater,suffix:m"), "set_load_distance", "get_load_distance");

	ClassDB::bind_method(D_METHOD("get_unload_distance_ratio"), &VoxelLoadTrigger4D::get_unload_distance_ratio);
	ClassDB::bind_method(D_METHOD("set_unload_distance_ratio", "unload_distance_ratio"), &VoxelLoadTrigger4D::set_unload_distance_ratio);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "unload_distance_ratio", PROPERTY_HINT_RANGE, "1,4,0.01,or_greater"), "set_unload_distance_ratio", "get_unload_distance_ratio");

	ClassDB::bind_method(D_METHOD("get_voxel_world_path"), &VoxelLoadTrigger4D::get_voxel_world_path);
	ClassDB::bind_method(D_METHOD("set_voxel_world_path", "voxel_world_path"), &VoxelLoadTrigger4D::set_voxel_world_path);
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "voxel_world_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "VoxelWorld4D"), "set_voxel_world_path", "get_voxel_world_path");
}
