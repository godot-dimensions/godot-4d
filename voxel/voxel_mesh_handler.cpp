#include "voxel_mesh_handler.h"

#include "voxel_mesher.h"
#include "voxel_world_4d.h"

static int32_t _floor_to_mesh_chunk_grid(const int32_t p_coord) {
	return p_coord - (int32_t)Math::posmod(p_coord, VOXEL_MESH_CHUNK_SIZE);
}

// How many dirty chunk meshes to regenerate in one update_dirty_meshes call,
// bounding the meshing work done in a single frame.
static constexpr int MESHES_UPDATED_PER_TICK = 16;

void VoxelMeshHandler::mark_region_dirty(const Rect4i &p_region) {
	ERR_FAIL_NULL(_world);
	// A chunk's mesh reads the voxel values and edge normals up to one step
	// outside of the chunk, so the meshes to regenerate are those of the
	// chunks overlapping the changed region grown by 1. The region is not
	// clamped to the data bounds: those may already have contracted away from
	// an unloaded chunk whose mesh still needs removing, and out-of-bounds
	// chunks cost only a definedness check.
	const Rect4i affected = p_region.grow(1);
	const Vector4i start = Vector4i(
			_floor_to_mesh_chunk_grid(affected.position.x),
			_floor_to_mesh_chunk_grid(affected.position.y),
			_floor_to_mesh_chunk_grid(affected.position.z),
			_floor_to_mesh_chunk_grid(affected.position.w));
	const Vector4i end = affected.get_end();
	for (int32_t w = start.w; w < end.w; w += VOXEL_MESH_CHUNK_SIZE) {
		for (int32_t z = start.z; z < end.z; z += VOXEL_MESH_CHUNK_SIZE) {
			for (int32_t y = start.y; y < end.y; y += VOXEL_MESH_CHUNK_SIZE) {
				for (int32_t x = start.x; x < end.x; x += VOXEL_MESH_CHUNK_SIZE) {
					_dirty_chunks.insert(Vector4i(x, y, z, w));
				}
			}
		}
	}
}

void VoxelMeshHandler::update_dirty_meshes() {
	ERR_FAIL_NULL(_world);
	const Ref<VoxelData> voxel_data = _world->get_voxel_data();
	// Chunks are updated oldest mark first, since the set is insertion ordered.
	for (int updated_count = 0; !_dirty_chunks.is_empty() && updated_count < MESHES_UPDATED_PER_TICK;) {
		const Vector4i chunk_position = *_dirty_chunks.begin();
		_dirty_chunks.erase(chunk_position);
		// A chunk whose region is only partially defined gets no mesh, so
		// that chunks aren't repeatedly re-meshed as more parts load. A
		// constant region needs no mesh either: constants never border a
		// different material, so a constant region has no faces.
		const Rect4i mesh_region = Rect4i(chunk_position, VOXEL_MESH_CHUNK_SIZE_VECTOR);
		const VoxelDataNeighbourhood neighbourhood = voxel_data->find_region_neighbourhood(mesh_region);
		Ref<TetraMesh4D> mesh;
		if (neighbourhood.node != nullptr && !neighbourhood.node->is_constant() && neighbourhood.node->is_region_defined(mesh_region)) {
			updated_count++;
			mesh = VoxelMesher::generate_chunk_mesh(neighbourhood, chunk_position);
		}
		HashMap<Vector4i, MeshInstance4D *>::Iterator existing = _chunk_meshes.find(chunk_position);
		if (mesh.is_null() || mesh->get_vertex_positions().is_empty()) {
			if (existing) {
				memdelete(existing->value);
				_chunk_meshes.remove(existing);
			}
			continue;
		}
		if (existing) {
			existing->value->set_mesh(mesh);
		} else {
			MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
			mesh_instance->set_mesh(mesh);
			mesh_instance->set_position(Vector4(chunk_position));
			_world->add_child(mesh_instance);
			_chunk_meshes.insert(chunk_position, mesh_instance);
		}
	}
}
