#include "voxel_chunk_loader.h"

#include "voxel_mesh_handler.h"
#include "voxel_world_4d.h"

void VoxelChunkLoader::_generate_load_task(const uint64_t p_task_pointer) {
	ChunkLoadTask *task = (ChunkLoadTask *)(uintptr_t)p_task_pointer;
	task->chunk = task->data->generate_chunk_content(task->position);
	// The message carries only the position; the task itself stays owned by
	// the loader, so nothing is lost if the message outlives the loader.
	task->finished_callback.call_deferred(task->position);
}

void VoxelChunkLoader::queue_load(const Vector4i &p_voxel) {
	ERR_FAIL_NULL(_world);
	const Ref<VoxelData> voxel_data = _world->get_voxel_data();
	const Vector4i chunk_position = voxel_data->get_chunk_position(p_voxel);
	if (_pending_loads.has(chunk_position)) {
		return;
	}
	ChunkLoadTask *task = memnew(ChunkLoadTask);
	task->data = voxel_data;
	task->position = chunk_position;
	task->finished_callback = callable_mp(this, &VoxelChunkLoader::_finish_load);
	_pending_loads.insert(chunk_position, task);
	task->task_id = WorkerThreadPool::get_singleton()->add_task(callable_mp_static(&VoxelChunkLoader::_generate_load_task).bind((uint64_t)(uintptr_t)task), false, "Voxel chunk load");
}

void VoxelChunkLoader::_finish_load(const Vector4i &p_position) {
	HashMap<Vector4i, ChunkLoadTask *>::Iterator entry = _pending_loads.find(p_position);
	if (!entry) {
		return;
	}
	ChunkLoadTask *task = entry->value;
	// The task has already finished; this just releases the pool's record.
	WorkerThreadPool::get_singleton()->wait_for_task_completion(task->task_id);
	task->data->apply_generated_chunk(task->chunk);
	_world->get_mesh_handler().mark_region_dirty(Rect4i(task->position, VOXEL_DATA_CHUNK_SIZE_VECTOR));
	_pending_loads.remove(entry);
	memdelete(task);
}

VoxelChunkLoader::~VoxelChunkLoader() {
	// The worker tasks write into their task structs, so each must finish
	// before its task can be freed. Chunks that were generated but never
	// grafted are still owned by their tasks.
	for (const KeyValue<Vector4i, ChunkLoadTask *> &entry : _pending_loads) {
		ChunkLoadTask *task = entry.value;
		WorkerThreadPool::get_singleton()->wait_for_task_completion(task->task_id);
		if (task->chunk != nullptr) {
			memdelete(task->chunk);
		}
		memdelete(task);
	}
}
