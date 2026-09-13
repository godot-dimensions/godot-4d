#include "voxel_world_4d.h"

void VoxelWorld4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_voxel_data"), &VoxelWorld4D::get_voxel_data);
}

void VoxelWorld4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Temporary: queue every not yet defined chunk in the fixed
			// bounds for loading. A real streaming system would decide what
			// to load dynamically.
			const Rect4i bounds = _voxel_data->get_bounds();
			const Vector4i end = bounds.get_end();
			for (int32_t w = bounds.position.w; w < end.w; w += VOXEL_DATA_CHUNK_SIZE) {
				for (int32_t z = bounds.position.z; z < end.z; z += VOXEL_DATA_CHUNK_SIZE) {
					for (int32_t y = bounds.position.y; y < end.y; y += VOXEL_DATA_CHUNK_SIZE) {
						for (int32_t x = bounds.position.x; x < end.x; x += VOXEL_DATA_CHUNK_SIZE) {
							const Vector4i chunk_position = Vector4i(x, y, z, w);
							if (!_voxel_data->is_region_defined(Rect4i(chunk_position, VOXEL_DATA_CHUNK_SIZE_VECTOR))) {
								_chunk_loader->queue_load(chunk_position);
							}
						}
					}
				}
			}
			_mesh_handler.mark_region_dirty(bounds);
			set_process(true);
		} break;
		case NOTIFICATION_PROCESS: {
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
