#pragma once

#include "data/voxel_data.h"

#if GDEXTENSION
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/templates/safe_refcount.hpp>
#elif GODOT_MODULE
#include "core/object/worker_thread_pool.h"
#include "core/templates/safe_refcount.h"
#endif

class VoxelLoadTrigger4D;
class VoxelWorld4D;

// Loads chunks of one VoxelWorld4D's voxel data: queued chunks are generated
// on worker threads, then stored into the data on the main thread as they
// finish, marking the affected meshes dirty. Loads can be queued at any time.
// This is conceptually part of VoxelWorld4D, split into its own class for
// readability; it is an Object so that the worker threads can send their
// finished loads back to it through the MessageQueue.
class VoxelChunkLoader : public Object {
	GDCLASS(VoxelChunkLoader, Object);

	// One requested load. The worker thread generating the chunk reads and
	// writes only its own task, so a task never outlives the load: the main
	// thread frees it when the load finishes or the loader is destroyed.
	// The revoked flag is the exception, written by the main thread while the
	// worker may read it, so it is atomic.
	struct ChunkLoadTask {
		Ref<VoxelData> data;
		Vector4i position;
		// The generated chunk, owned by the task until it is grafted.
		VoxelDataTree *chunk = nullptr;
		int64_t task_id = -1;
		Callable finished_callback;
		// Tells a task that has not started yet that its result will be
		// discarded, so it can skip the generation. Best effort: the task may
		// already be running or finished when this is set.
		SafeFlag revoked;
	};

	VoxelWorld4D *const _world;
	// Every requested load that has not yet been stored, keyed by chunk
	// position. Only touched from the main thread.
	HashMap<Vector4i, ChunkLoadTask *> _pending_loads;

	static void _generate_load_task(const uint64_t p_task_pointer);
	void _finish_load(const Vector4i &p_position);
	LocalVector<VoxelLoadTrigger4D *> _get_load_triggers() const;

protected:
	static void _bind_methods() {}

public:
	// Queues the data chunk containing the given voxel to be generated and
	// stored. Requests for chunks that are already pending are ignored.
	void queue_load(const Vector4i &p_voxel);

	// Immediately unloads the data chunk containing the given voxel, marking
	// the region dirty so that its mesh is removed, and cancels the chunk's
	// pending load if it has one.
	void unload(const Vector4i &p_voxel);

	// Loads and unloads chunks so that the loaded part of the world follows
	// the VoxelLoadTrigger4Ds in the scene tree.
	void update_loaded_chunks();

	explicit VoxelChunkLoader(VoxelWorld4D *p_world) :
			_world(p_world) {}
	~VoxelChunkLoader();
};
