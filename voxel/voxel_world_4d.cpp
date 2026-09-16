#include "voxel_world_4d.h"

void VoxelWorld4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_voxel_data"), &VoxelWorld4D::get_voxel_data);
}

void VoxelWorld4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Generally the world will be empty at this point, but just in case it isn't:
			_mesh_handler.mark_region_dirty(_voxel_data->get_bounds());
			set_process(true);
		} break;
		case NOTIFICATION_PROCESS: {
			_chunk_loader->update_loaded_chunks();
			_mesh_handler.update_dirty_meshes();
		} break;
	}
}

VoxelWorld4D::VoxelWorld4D() :
		_mesh_handler(this) {
	_voxel_data.instantiate();
	_chunk_loader = memnew(VoxelChunkLoader(this));
}

VoxelWorld4D::~VoxelWorld4D() {
	memdelete(_chunk_loader);
}
