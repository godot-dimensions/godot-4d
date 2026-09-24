#include "voxel_chunk_loader.h"

#include "voxel_load_trigger_4d.h"
#include "voxel_mesh_handler.h"
#include "voxel_world_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/scene_tree.hpp>
#elif GODOT_MODULE
#include "scene/main/scene_tree.h"
#endif

LocalVector<VoxelLoadTrigger4D *> VoxelChunkLoader::_get_load_triggers() const {
	LocalVector<VoxelLoadTrigger4D *> triggers;
	SceneTree *tree = _world->get_tree();
	if (tree == nullptr) {
		return triggers;
	}
#if GDEXTENSION
	const TypedArray<Node> nodes = tree->get_nodes_in_group(VoxelLoadTrigger4D::GROUP_NAME);
	for (int64_t i = 0; i < nodes.size(); i++) {
		VoxelLoadTrigger4D *trigger = Object::cast_to<VoxelLoadTrigger4D>(nodes[i]);
		if (trigger != nullptr && trigger->applies_to(_world)) {
			triggers.push_back(trigger);
		}
	}
#elif GODOT_MODULE
	List<Node *> nodes;
	tree->get_nodes_in_group(VoxelLoadTrigger4D::GROUP_NAME, &nodes);
	for (Node *node : nodes) {
		VoxelLoadTrigger4D *trigger = Object::cast_to<VoxelLoadTrigger4D>(node);
		if (trigger != nullptr && trigger->applies_to(_world)) {
			triggers.push_back(trigger);
		}
	}
#endif
	return triggers;
}

void VoxelChunkLoader::update_loaded_chunks() {
	ERR_FAIL_NULL(_world);
	const LocalVector<VoxelLoadTrigger4D *> load_triggers = _get_load_triggers();
	if (load_triggers.is_empty()) {
		return;
	}
	const Ref<VoxelData> voxel_data = _world->get_voxel_data();
	const Transform4D to_voxel_space = _world->get_global_transform().inverse();
	for (VoxelLoadTrigger4D *load_trigger : load_triggers) {
		// The load distance is in voxel units, so only the trigger's position
		// is mapped into voxel space, not the distance.
		const Vector4 center = to_voxel_space.xform(load_trigger->get_global_position());
		const real_t radius = load_trigger->get_load_distance();
		const Vector4 extents = Vector4(radius, radius, radius, radius);
		const Vector4i min_chunk = voxel_data->get_chunk_position(Vector4i((center - extents).floor()));
		const Vector4i max_chunk = voxel_data->get_chunk_position(Vector4i((center + extents).floor()));
		constexpr real_t HALF_CHUNK = VOXEL_DATA_CHUNK_SIZE * 0.5f;
		for (int32_t w = min_chunk.w; w <= max_chunk.w; w += VOXEL_DATA_CHUNK_SIZE) {
			for (int32_t z = min_chunk.z; z <= max_chunk.z; z += VOXEL_DATA_CHUNK_SIZE) {
				for (int32_t y = min_chunk.y; y <= max_chunk.y; y += VOXEL_DATA_CHUNK_SIZE) {
					for (int32_t x = min_chunk.x; x <= max_chunk.x; x += VOXEL_DATA_CHUNK_SIZE) {
						const Vector4i chunk_position = Vector4i(x, y, z, w);
						const Vector4 chunk_center = Vector4(chunk_position) + Vector4(HALF_CHUNK, HALF_CHUNK, HALF_CHUNK, HALF_CHUNK);
						if (chunk_center.distance_squared_to(center) > radius * radius) {
							continue;
						}
						if (!_pending_loads.has(chunk_position) && !voxel_data->is_region_defined(Rect4i(chunk_position, VOXEL_DATA_CHUNK_SIZE_VECTOR))) {
							queue_load(chunk_position);
						}
					}
				}
			}
		}
	}
}

void VoxelChunkLoader::_generate_load_task(const uint64_t p_task_pointer) {
	ChunkLoadTask *task = (ChunkLoadTask *)(uintptr_t)p_task_pointer;
	if (task->revoked.is_set()) {
		// The result would be discarded, and whoever revoked the task is
		// already waiting for it, not for a message.
		return;
	}
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

void VoxelChunkLoader::unload(const Vector4i &p_voxel) {
	ERR_FAIL_NULL(_world);
	const Ref<VoxelData> voxel_data = _world->get_voxel_data();
	const Vector4i chunk_position = voxel_data->get_chunk_position(p_voxel);
	HashMap<Vector4i, ChunkLoadTask *>::Iterator entry = _pending_loads.find(chunk_position);
	if (entry) {
		ChunkLoadTask *task = entry->value;
		task->revoked.set();
		// If it isn't already started, this makes the thread pool run it immediately,
		// and thus end it immediately since it's marked revoked.
		WorkerThreadPool::get_singleton()->wait_for_task_completion(task->task_id);
		if (task->chunk != nullptr) {
			// It could have finished earlier this frame though.
			memdelete(task->chunk);
		}
		_pending_loads.remove(entry);
		memdelete(task);
	}
	if (voxel_data->unload_chunk(chunk_position)) {
		// Neighboring meshes built while this chunk existed are still valid,
		// so only the mesh of the chunk itself needs removing: mark a single
		// voxel in the middle, so that the marked region's growth does not
		// reach into the neighboring mesh chunks.
		_world->get_mesh_handler().mark_region_dirty(Rect4i(chunk_position + VOXEL_DATA_CHUNK_SIZE_VECTOR / 2, Vector4i(1, 1, 1, 1)));
	}
}

void VoxelChunkLoader::_finish_load(const Vector4i &p_position) {
	HashMap<Vector4i, ChunkLoadTask *>::Iterator entry = _pending_loads.find(p_position);
	if (!entry) {
		return;
	}
	ChunkLoadTask *task = entry->value;
	// The task has already finished; waiting for it just releases the pool's record.
	// There's a slight chance, if a chunk is quickly loaded, unloaded, then loaded
	// again, that the first load went through and the second isn't finished. It
	// generates the same data both times though, so this is inconsequential.
	WorkerThreadPool::get_singleton()->wait_for_task_completion(task->task_id);
	task->data->apply_generated_chunk(task->chunk);
	_world->get_mesh_handler().mark_region_dirty(Rect4i(task->position, VOXEL_DATA_CHUNK_SIZE_VECTOR));
	_pending_loads.remove(entry);
	memdelete(task);
}

VoxelChunkLoader::~VoxelChunkLoader() {
	// Revoke every task before waiting for any, so that as many as possible
	// skip their generation.
	for (const KeyValue<Vector4i, ChunkLoadTask *> &entry : _pending_loads) {
		entry.value->revoked.set();
	}
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
