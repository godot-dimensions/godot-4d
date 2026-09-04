#include "voxel_world_4d.h"

void VoxelWorld4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_voxel_data"), &VoxelWorld4D::get_voxel_data);
}

void VoxelWorld4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_mesh_handler.initialize();
		} break;
	}
}

VoxelWorld4D::VoxelWorld4D() :
		_mesh_handler(this) {
	_voxel_data.instantiate();
}
