#pragma once

#include "data/voxel_data_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/templates/safe_refcount.hpp>
#elif GODOT_MODULE
#include "core/object/worker_thread_pool.h"
#include "core/templates/hash_map.h"
#include "core/templates/local_vector.h"
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
class VoxelChunkLoader4D : public Object {
	GDCLASS(VoxelChunkLoader4D, Object);

	// One requested load. The worker thread generating the chunk reads and
	// writes only its own task, so a task never outlives the load: the main
	// thread frees it when the load finishes or the loader is destroyed.
	// The revoked flag is the exception, written by the main thread while the
	// worker may read it, so it is atomic.
	struct ChunkLoadTask4D {
		Ref<VoxelData4D> data;
		Vector4i position;
		// The generated chunk, owned by the task until it is grafted.
		VoxelDataTree4D *chunk = nullptr;
		int64_t task_id = -1;
		Callable finished_callback;
		// Tells a task that has not started yet that its result will be
		// discarded, so it can skip the generation. Best effort: the task may
		// already be running or finished when this is set.
		SafeFlag revoked;
	};

	struct TriggerRange4D;

	VoxelWorld4D *const _world;
	// Every requested load that has not yet been stored, keyed by chunk
	// position. Only touched from the main thread.
	HashMap<Vector4i, ChunkLoadTask4D *> _pending_loads;

	static void _generate_load_task(const uint64_t p_task_pointer);
	void _finish_load(const Vector4i &p_position);
	LocalVector<VoxelLoadTrigger4D *> _get_load_triggers() const;
	static void _chunk_center_distance_range(const Rect4i &p_region, const Vector4 &p_point, real_t &r_min_squared, real_t &r_max_squared);
	static Vector4i _clamp_voxel(const Vector4i &p_voxel, const Rect4i *p_bounds);
	static void _scan_required_chunks(const VoxelDataTree4D *p_node, const Rect4i &p_bounds, const LocalVector<TriggerRange4D> &p_triggers, const Rect4i *p_world_bounds, LocalVector<Vector4i> &r_loads, LocalVector<Vector4i> &r_unloads);

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

	// Only for GDExtension class registration, which requires a default
	// constructor. A loader without a world does nothing.
	VoxelChunkLoader4D() :
			_world(nullptr) {}
	explicit VoxelChunkLoader4D(VoxelWorld4D *p_world) :
			_world(p_world) {}
	~VoxelChunkLoader4D();
};
