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
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
	List<Node *> nodes;
	tree->get_nodes_in_group(VoxelLoadTrigger4D::GROUP_NAME, &nodes);
#else
	Vector<Node *> nodes = tree->get_nodes_in_group(VoxelLoadTrigger4D::GROUP_NAME);
#endif
	for (Node *node : nodes) {
		VoxelLoadTrigger4D *trigger = Object::cast_to<VoxelLoadTrigger4D>(node);
		if (trigger != nullptr && trigger->applies_to(_world)) {
			triggers.push_back(trigger);
		}
	}
#endif
	return triggers;
}

// One trigger's requirements, in voxel space.
struct TriggerRange {
	Vector4 center;
	real_t load_radius = 0.0f;
	real_t unload_radius = 0.0f;
};

// The squared distances from the point to the nearest and farthest chunk
// centers in the region, which must be chunk-aligned.
static void _chunk_center_distance_range(const Rect4i &p_region, const Vector4 &p_point, real_t &r_min_squared, real_t &r_max_squared) {
	r_min_squared = 0.0f;
	r_max_squared = 0.0f;
	for (int axis = 0; axis < 4; axis++) {
		const real_t low = p_region.position[axis] + (real_t)VOXEL_DATA_CHUNK_SIZE * 0.5f;
		const real_t high = p_region.get_end()[axis] - (real_t)VOXEL_DATA_CHUNK_SIZE * 0.5f;
		const real_t coordinate = p_point[axis];
		const real_t outside = MAX(MAX(low - coordinate, coordinate - high), (real_t)0.0f);
		r_min_squared += outside * outside;
		const real_t farthest = MAX(Math::abs(coordinate - low), Math::abs(coordinate - high));
		r_max_squared += farthest * farthest;
	}
}

// The nearest voxel inside the bounds, or the voxel itself if the bounds are
// null (unlimited) or it is already inside.
static Vector4i _clamp_voxel(const Vector4i &p_voxel, const Rect4i *p_bounds) {
	if (p_bounds == nullptr) {
		return p_voxel;
	}
	Vector4i clamped = p_voxel;
	const Vector4i end = p_bounds->get_end();
	for (int axis = 0; axis < 4; axis++) {
		// Degenerate empty bounds collapse to their position corner; the upper
		// limit must not drop below the lower, as the probe walk's termination
		// relies on this clamp being monotone.
		clamped[axis] = CLAMP(clamped[axis], p_bounds->position[axis], MAX(end[axis] - 1, p_bounds->position[axis]));
	}
	return clamped;
}

// Collects the chunks that the triggers require to be loaded or unloaded:
// chunks whose center is within a load radius must be defined or pending, and
// chunks whose center is within no unload radius must be neither. Chunks
// entirely outside the world bounds (given as null when unlimited) may
// neither load nor be kept. The recursion stops at nodes that already satisfy
// every applicable requirement. p_node is the deepest real node covering
// p_bounds: below childless nodes the recursion continues over the regions
// where children would be, without splitting the node. The scan does not
// modify the tree, so that its shape stays stable while it is being
// traversed; the collected chunks are queued and unloaded afterwards.
static void _scan_required_chunks(const VoxelDataTree *p_node, const Rect4i &p_bounds, const LocalVector<TriggerRange> &p_triggers, const Rect4i *p_world_bounds, LocalVector<Vector4i> &r_loads, LocalVector<Vector4i> &r_unloads) {
	const bool covered = p_node->is_defined_or_pending();
	const bool inside_world = p_world_bounds == nullptr || p_world_bounds->intersects_exclusive(p_bounds);
	if (p_bounds.size.x == VOXEL_DATA_CHUNK_SIZE) {
		bool wanted = false;
		bool keep = false;
		if (inside_world) {
			for (const TriggerRange &trigger : p_triggers) {
				real_t min_squared;
				real_t max_squared;
				_chunk_center_distance_range(p_bounds, trigger.center, min_squared, max_squared);
				wanted = wanted || min_squared <= trigger.load_radius * trigger.load_radius;
				keep = keep || min_squared <= trigger.unload_radius * trigger.unload_radius;
			}
		}
		if (wanted && !covered) {
			r_loads.push_back(p_bounds.position);
		} else if (!keep && covered) {
			r_unloads.push_back(p_bounds.position);
		}
		return;
	}
	bool any_load = false;
	bool fully_kept = false;
	if (inside_world) {
		for (const TriggerRange &trigger : p_triggers) {
			real_t min_squared;
			real_t max_squared;
			_chunk_center_distance_range(p_bounds, trigger.center, min_squared, max_squared);
			any_load = any_load || min_squared <= trigger.load_radius * trigger.load_radius;
			fully_kept = fully_kept || max_squared <= trigger.unload_radius * trigger.unload_radius;
		}
	}
	// Loads can only be needed where a load radius reaches a part that is not
	// covered, and unloads only where something occupies a part not entirely
	// within one trigger's unload radius. (Parts kept by a combination of
	// several triggers' radii recurse needlessly, but settle correctly at the
	// chunks.)
	const bool occupied = p_node->is_parent() || covered;
	if ((covered || !any_load) && (fully_kept || !occupied)) {
		return;
	}
	if (p_node->is_parent()) {
		for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
			const VoxelDataTree *child = p_node->get_child(i);
			_scan_required_chunks(child, child->get_bounds(), p_triggers, p_world_bounds, r_loads, r_unloads);
		}
		return;
	}
	const Vector4i half_size = p_bounds.size / 2;
	for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
		const Vector4i offset = Vector4i(
				(i & 1) ? half_size.x : 0,
				(i & 2) ? half_size.y : 0,
				(i & 4) ? half_size.z : 0,
				(i & 8) ? half_size.w : 0);
		_scan_required_chunks(p_node, Rect4i(p_bounds.position + offset, half_size), p_triggers, p_world_bounds, r_loads, r_unloads);
	}
}

void VoxelChunkLoader::update_loaded_chunks() {
	ERR_FAIL_NULL(_world);
	const LocalVector<VoxelLoadTrigger4D *> load_triggers = _get_load_triggers();
	if (load_triggers.is_empty()) {
		return;
	}
	const Ref<VoxelData> voxel_data = _world->get_voxel_data();
	if (voxel_data->get_generator().is_null()) {
		return;
	}
	const Transform4D to_voxel_space = _world->get_global_transform().inverse();
	LocalVector<TriggerRange> triggers;
	triggers.reserve(load_triggers.size());
	for (VoxelLoadTrigger4D *load_trigger : load_triggers) {
		// The distances are in voxel units, so only the trigger's position is
		// mapped into voxel space, not the distances.
		TriggerRange trigger;
		trigger.center = to_voxel_space.xform(load_trigger->get_global_position());
		trigger.load_radius = load_trigger->get_load_distance();
		trigger.unload_radius = load_trigger->get_unload_distance();
		triggers.push_back(trigger);
	}
	const Rect4i world_bounds_rect = _world->get_world_bounds();
	const Rect4i *world_bounds = _world->is_world_bounds_enabled() ? &world_bounds_rect : nullptr;
	// Load the extremes of each trigger's range first, so that the tree's
	// bounds grow to cover every chunk the scan below could need to load;
	// everything the scan finds then lies inside the tree. Clamping the probes
	// into the world bounds keeps the whole walk within chunks that intersect
	// them, and still reaches the extreme loadable chunks in each direction.
	for (const TriggerRange &trigger : triggers) {
		const Vector4i center_chunk = voxel_data->get_chunk_position(_clamp_voxel(Vector4i(trigger.center.floor()), world_bounds));
		for (int axis = 0; axis < 4; axis++) {
			for (int sign = -1; sign <= 1; sign += 2) {
				Vector4 probe = trigger.center;
				probe[axis] += sign * trigger.load_radius;
				Vector4i chunk = voxel_data->get_chunk_position(_clamp_voxel(Vector4i(probe.floor()), world_bounds));
				// Walk inward to the farthest chunk actually in load range.
				real_t min_squared;
				real_t max_squared;
				_chunk_center_distance_range(Rect4i(chunk, VOXEL_DATA_CHUNK_SIZE_VECTOR), trigger.center, min_squared, max_squared);
				while (chunk != center_chunk && min_squared > trigger.load_radius * trigger.load_radius) {
					chunk[axis] -= sign * VOXEL_DATA_CHUNK_SIZE;
					_chunk_center_distance_range(Rect4i(chunk, VOXEL_DATA_CHUNK_SIZE_VECTOR), trigger.center, min_squared, max_squared);
				}
				// The intersection check only fails with degenerate world
				// bounds that are empty on some axis.
				if (min_squared <= trigger.load_radius * trigger.load_radius &&
						(world_bounds == nullptr || world_bounds->intersects_exclusive(Rect4i(chunk, VOXEL_DATA_CHUNK_SIZE_VECTOR))) &&
						!voxel_data->is_voxel_defined_or_pending(chunk)) {
					queue_load(chunk);
				}
			}
		}
	}
	const Rect4i bounds = voxel_data->get_bounds();
	if (bounds.size == Vector4i()) {
		return;
	}
	const VoxelDataTree *root = voxel_data->find_region_neighbourhood(bounds).node;
	ERR_FAIL_NULL(root);
	LocalVector<Vector4i> chunks_to_load;
	LocalVector<Vector4i> chunks_to_unload;
	_scan_required_chunks(root, bounds, triggers, world_bounds, chunks_to_load, chunks_to_unload);
	for (const Vector4i &chunk_position : chunks_to_load) {
		queue_load(chunk_position);
	}
	for (const Vector4i &chunk_position : chunks_to_unload) {
		unload(chunk_position);
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
	voxel_data->mark_region_pending(Rect4i(chunk_position, VOXEL_DATA_CHUNK_SIZE_VECTOR));
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
