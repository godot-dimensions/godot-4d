#include "voxel_mesh_handler.h"

#include "voxel_mesher.h"
#include "voxel_world_4d.h"

static int32_t _floor_to_mesh_chunk_grid(const int32_t p_coord) {
	return p_coord - (int32_t)Math::posmod(p_coord, VOXEL_MESH_CHUNK_SIZE);
}

void VoxelMeshHandler::initialize() {
	ERR_FAIL_NULL(_world);
	if (!_chunk_meshes.is_empty()) {
		return;
	}
	const Ref<VoxelData> voxel_data = _world->get_voxel_data();
	const Rect4i bounds = voxel_data->get_bounds();
	const Vector4i start = Vector4i(
			_floor_to_mesh_chunk_grid(bounds.position.x),
			_floor_to_mesh_chunk_grid(bounds.position.y),
			_floor_to_mesh_chunk_grid(bounds.position.z),
			_floor_to_mesh_chunk_grid(bounds.position.w));
	const Vector4i end = bounds.get_end();
	for (int32_t w = start.w; w < end.w; w += VOXEL_MESH_CHUNK_SIZE) {
		for (int32_t z = start.z; z < end.z; z += VOXEL_MESH_CHUNK_SIZE) {
			for (int32_t y = start.y; y < end.y; y += VOXEL_MESH_CHUNK_SIZE) {
				for (int32_t x = start.x; x < end.x; x += VOXEL_MESH_CHUNK_SIZE) {
					const Vector4i chunk_position = Vector4i(x, y, z, w);
					const Ref<Mesh4D> mesh = VoxelMesher::generate_chunk_mesh(voxel_data, chunk_position);
					if (mesh->get_vertices().is_empty()) {
						continue;
					}
					MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
					mesh_instance->set_mesh(mesh);
					mesh_instance->set_position(Vector4(chunk_position));
					_world->add_child(mesh_instance);
					_chunk_meshes.insert(chunk_position, mesh_instance);
				}
			}
		}
	}
}
