#pragma once

#include "../../voxel/data/voxel_data.h"
#include "../../voxel/data/voxel_data_leaf.h"
#include "../../voxel/edit/sphere_voxel_edit.h"

#include "tests/test_macros.h"

namespace TestVoxelData {
TEST_CASE("[VoxelData] Hard-coded test data") {
	Ref<VoxelData> data;
	data.instantiate();
	CHECK_MESSAGE(data->get_bounds().size == Vector4i(), "VoxelData should start with no defined region.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels should start out undefined before their chunk is loaded.");

	data->load_all_chunks();
	const Rect4i test_region = Rect4i(VOXEL_DATA_CHUNK_SIZE_VECTOR * -4, VOXEL_DATA_CHUNK_SIZE_VECTOR * 8);
	const Vector4i last_voxel = test_region.get_end() - Vector4i(1, 1, 1, 1);
	CHECK_MESSAGE(data->get_bounds() == test_region, "VoxelData load_all_chunks should generate the hard-coded 8x8x8x8 chunk test region.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels inside the test region should be defined.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(test_region.position.x, last_voxel.y, test_region.position.z, last_voxel.w)), "VoxelData voxels at the test region's corners should be defined.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(test_region.get_end().x, 0, 0, 0)), "VoxelData is_voxel_defined should be false outside the bounds.");
	CHECK_MESSAGE(data->is_region_defined(test_region), "VoxelData the whole test region should be defined.");
	CHECK_MESSAGE(!data->is_region_defined(Rect4i(0, 0, 0, 0, test_region.get_end().x + 1, 1, 1, 1)), "VoxelData regions reaching outside the bounds should not be fully defined.");
	CHECK_MESSAGE(!data->is_region_defined(Rect4i(test_region.get_end().x + 1, 0, 0, 0, 1, 1, 1, 1)), "VoxelData regions entirely outside the bounds should not be defined.");
	// The remaining samples are fixed locations on the test tiger, inside the
	// test region for any chunk size of at least 4.
	CHECK_MESSAGE(data->get_material(Vector4i(0, 0, 0, 0)) == VoxelMaterial::AIR, "VoxelData test shape should be empty at the origin.");
	CHECK_MESSAGE(data->get_material(Vector4i(10, 0, 10, 0)) == VoxelMaterial::SOLID, "VoxelData test shape should be solid on the tiger's core circles.");
	CHECK_MESSAGE(data->get_material(Vector4i(100000, -5, 3, 12)) == VoxelMaterial::UNDEFINED, "VoxelData get_material outside the bounds should return UNDEFINED.");
}

static bool _bounds_alignment_holds(const Rect4i &p_bounds) {
	if (p_bounds.size.x < 2) {
		return true;
	}
	const int32_t half_mask = (p_bounds.size.x >> 1) - 1;
	return (p_bounds.position.x & half_mask) == 0 && (p_bounds.position.y & half_mask) == 0 && (p_bounds.position.z & half_mask) == 0 && (p_bounds.position.w & half_mask) == 0;
}

TEST_CASE("[VoxelData] Bounds expansion and contraction") {
	const Rect4i origin_chunk_bounds = Rect4i(Vector4i(), VOXEL_DATA_CHUNK_SIZE_VECTOR);
	Ref<VoxelData> data;
	data.instantiate();
	data->apply_generated_chunk(data->generate_chunk_content(Vector4i(1, 2, 3, 0)));
	CHECK_MESSAGE(data->is_region_defined(origin_chunk_bounds), "VoxelData apply_generated_chunk should define the whole chunk containing the given voxel.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(VOXEL_DATA_CHUNK_SIZE, 0, 0, 0)), "VoxelData apply_generated_chunk should not define neighboring chunks.");
	CHECK_MESSAGE(data->get_bounds() == origin_chunk_bounds, "VoxelData bounds should stay tight around the defined content.");

	// The content must survive every reshaping of the bounds below unchanged.
	const Vector4i samples[3] = { Vector4i(0, 0, 0, 0), Vector4i(3, 2, 1, 0), Vector4i(1, 3, 3, 3) };
	VoxelMaterial sample_materials[3];
	for (int i = 0; i < 3; i++) {
		sample_materials[i] = data->get_material(samples[i]);
	}

	const Vector4i far_voxel = Vector4i(65, 2, -7, 0);
	data->apply_generated_chunk(data->generate_chunk_content(far_voxel));
	CHECK_MESSAGE(data->is_voxel_defined(far_voxel), "VoxelData should expand its bounds to store chunks outside of them.");
	CHECK_MESSAGE(data->get_bounds().encloses_inclusive(origin_chunk_bounds), "VoxelData expanded bounds should still cover the old content.");
	CHECK_MESSAGE(_bounds_alignment_holds(data->get_bounds()), "VoxelData expanded bounds should satisfy the alignment invariant.");
	for (int i = 0; i < 3; i++) {
		CHECK_MESSAGE(data->get_material(samples[i]) == sample_materials[i], "VoxelData expansion should preserve the stored values.");
	}

	data->unload_chunk(far_voxel);
	CHECK_MESSAGE(data->get_bounds() == origin_chunk_bounds, "VoxelData bounds should contract back to the remaining content once the far chunk is unloaded.");
	for (int i = 0; i < 3; i++) {
		CHECK_MESSAGE(data->get_material(samples[i]) == sample_materials[i], "VoxelData contraction should preserve the stored values.");
	}

	// A chunk on the other side of the origin: the two chunks together only
	// fit in a root that is half-aligned on the X axis.
	data->apply_generated_chunk(data->generate_chunk_content(Vector4i(-1, 0, 0, 0)));
	CHECK_MESSAGE(data->get_bounds().size == VOXEL_DATA_CHUNK_SIZE_VECTOR * 2, "VoxelData chunks straddling the origin should fit in a half-aligned root instead of one twice the size.");
	CHECK_MESSAGE(data->get_bounds().position.x == -VOXEL_DATA_CHUNK_SIZE, "VoxelData bounds straddling the origin should be half-aligned on that axis.");
	CHECK_MESSAGE(_bounds_alignment_holds(data->get_bounds()), "VoxelData half-aligned bounds should satisfy the alignment invariant.");

	// Expanding from and contracting back to a half-aligned root exercises
	// the middle cases of both: the old root becomes the middle half of the
	// new root, and later contractions gather grandchildren back into it.
	data->apply_generated_chunk(data->generate_chunk_content(far_voxel));
	CHECK_MESSAGE(data->is_voxel_defined(far_voxel), "VoxelData should expand half-aligned bounds to store far away chunks.");
	CHECK_MESSAGE(_bounds_alignment_holds(data->get_bounds()), "VoxelData bounds expanded from a half-aligned root should satisfy the alignment invariant.");
	data->unload_chunk(far_voxel);
	CHECK_MESSAGE(data->get_bounds().size == VOXEL_DATA_CHUNK_SIZE_VECTOR * 2, "VoxelData bounds should contract back around the straddling chunks.");
	CHECK_MESSAGE(data->get_bounds().position.x == -VOXEL_DATA_CHUNK_SIZE, "VoxelData contraction should restore the half-aligned root around the straddling chunks.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(-1, 0, 0, 0)), "VoxelData contraction should keep both straddling chunks.");
	for (int i = 0; i < 3; i++) {
		CHECK_MESSAGE(data->get_material(samples[i]) == sample_materials[i], "VoxelData middle expansion and contraction should preserve the stored values.");
	}

	data->unload_chunk(Vector4i(-1, 0, 0, 0));
	data->unload_chunk(Vector4i(0, 0, 0, 0));
	CHECK_MESSAGE(data->get_bounds().size == Vector4i(), "VoxelData bounds should become empty once everything is unloaded.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels should be undefined once everything is unloaded.");
	CHECK_MESSAGE(data->get_material(Vector4i(0, 0, 0, 0)) == VoxelMaterial::UNDEFINED, "VoxelData get_material should return UNDEFINED once everything is unloaded.");

	data->apply_generated_chunk(data->generate_chunk_content(Vector4i(0, 0, 0, 0)));
	CHECK_MESSAGE(data->get_bounds() == origin_chunk_bounds, "VoxelData should accept chunks again after everything was unloaded.");
	for (int i = 0; i < 3; i++) {
		CHECK_MESSAGE(data->get_material(samples[i]) == sample_materials[i], "VoxelData chunks reloaded after a full unload should regenerate the same values.");
	}
}

TEST_CASE("[VoxelData] Edits and constant merging") {
	// A hypercube of 4x4x4x4 chunks, aligned so that a single root node fits
	// it exactly, and far enough from the origin that the hard-coded test
	// generator produces only air there, for any chunk size of at least 4.
	const Vector4i cube_position = VOXEL_DATA_CHUNK_SIZE_VECTOR * 8;
	const Rect4i cube_bounds = Rect4i(cube_position, VOXEL_DATA_CHUNK_SIZE_VECTOR * 4);
	Ref<VoxelData> data;
	data.instantiate();
	for (int32_t w = 0; w < 4; w++) {
		for (int32_t z = 0; z < 4; z++) {
			for (int32_t y = 0; y < 4; y++) {
				for (int32_t x = 0; x < 4; x++) {
					data->apply_generated_chunk(data->generate_chunk_content(cube_position + Vector4i(x, y, z, w) * VOXEL_DATA_CHUNK_SIZE));
				}
			}
		}
	}
	CHECK_MESSAGE(data->get_bounds() == cube_bounds, "VoxelData the loaded chunks should assemble into tight bounds around the cube.");
	const VoxelDataTree *root = data->find_region_neighbourhood(cube_bounds).node;
	REQUIRE_MESSAGE(root != nullptr, "VoxelData the cube should be covered by a single root node.");

	data->merge_edited_constants();
	root = data->find_region_neighbourhood(cube_bounds).node;
	REQUIRE_MESSAGE(root != nullptr, "VoxelData the merged cube should still be a node with the cube's bounds.");
	CHECK_MESSAGE(root->is_constant(), "VoxelData the merging pass should merge the all-air cube into one constant node.");
	CHECK_MESSAGE(root->get_constant_material() == VoxelMaterial::AIR, "VoxelData the merged cube's constant material should be air.");

	// A ball centered on the chunk one step in from the cube's corner, big
	// enough to cover all of that chunk's voxels and the voxels bordering
	// them, so that the chunk can revert to a constant, but far from the
	// cube's other corner.
	const Rect4i solid_chunk = Rect4i(cube_position + VOXEL_DATA_CHUNK_SIZE_VECTOR, VOXEL_DATA_CHUNK_SIZE_VECTOR);
	const Vector4i ball_center_voxel = solid_chunk.position + VOXEL_DATA_CHUNK_SIZE_VECTOR / 2;
	const Vector4 ball_center = Vector4(ball_center_voxel);
	const real_t ball_radius = VOXEL_DATA_CHUNK_SIZE + 1.0;
	Ref<SphereVoxelEdit> solid_edit = memnew(SphereVoxelEdit(ball_center, ball_radius, VoxelMaterial::SOLID));
	data->apply_edit(solid_edit);
	data->merge_edited_constants();
	CHECK_MESSAGE(data->get_bounds() == cube_bounds, "VoxelData edits should not change the bounds.");
	const VoxelDataTree *solid_node = data->find_region_neighbourhood(solid_chunk).node;
	REQUIRE_MESSAGE(solid_node != nullptr, "VoxelData the chunk the ball covers should be a node of its own.");
	CHECK_MESSAGE(solid_node->is_constant(), "VoxelData the merging pass should revert the chunk the ball covers to a constant.");
	CHECK_MESSAGE(solid_node->get_constant_material() == VoxelMaterial::SOLID, "VoxelData the chunk the ball covers should be constant solid.");
	// Probes on either side of the ball's surface, both in the same chunk one
	// step along X from the covered chunk.
	Vector4i inside_probe = ball_center_voxel;
	inside_probe.x += VOXEL_DATA_CHUNK_SIZE - 1;
	Vector4i outside_probe = ball_center_voxel;
	outside_probe.x += VOXEL_DATA_CHUNK_SIZE + 1;
	CHECK_MESSAGE(data->get_material(inside_probe) == VoxelMaterial::SOLID, "VoxelData voxels just inside the ball's surface should be solid.");
	CHECK_MESSAGE(data->get_material(outside_probe) == VoxelMaterial::AIR, "VoxelData voxels just outside the ball's surface should still be air.");
	CHECK_MESSAGE(data->get_material(cube_position) == VoxelMaterial::AIR, "VoxelData the cube's near corner, far from the ball, should still be air.");
	CHECK_MESSAGE(data->get_material(cube_bounds.get_end() - Vector4i(1, 1, 1, 1)) == VoxelMaterial::AIR, "VoxelData the cube's far corner should still be air.");

	// Carving the same ball back to air must leave no trace of the edits.
	Ref<SphereVoxelEdit> air_edit = memnew(SphereVoxelEdit(ball_center, ball_radius, VoxelMaterial::AIR));
	data->apply_edit(air_edit);
	data->merge_edited_constants();
	root = data->find_region_neighbourhood(cube_bounds).node;
	REQUIRE_MESSAGE(root != nullptr, "VoxelData the cube should merge back into a node with the cube's bounds.");
	CHECK_MESSAGE(root->is_constant(), "VoxelData undoing the edit should let the merging pass merge the whole cube again.");
	CHECK_MESSAGE(root->get_constant_material() == VoxelMaterial::AIR, "VoxelData the re-merged cube's constant material should be air.");
}

// Whether the central surface data invariant holds over the whole defined
// region: an edge between two defined voxels has stored data exactly when its
// ends' materials differ. An active edge owned by a constant node fails the
// check, since constants cannot store the data. Edges into undefined space
// are not judged: they may keep data for chunks that load later.
static bool _surface_data_invariant_holds(const Ref<VoxelData> &p_data) {
	const Rect4i bounds = p_data->get_bounds();
	const VoxelDataTree *root = p_data->find_region_neighbourhood(bounds).node;
	if (root == nullptr) {
		return true;
	}
	const Vector4i end = bounds.get_end();
	for (int32_t w = bounds.position.w; w < end.w; w++) {
		for (int32_t z = bounds.position.z; z < end.z; z++) {
			for (int32_t y = bounds.position.y; y < end.y; y++) {
				for (int32_t x = bounds.position.x; x < end.x; x++) {
					const Vector4i voxel = Vector4i(x, y, z, w);
					const VoxelMaterial material = p_data->get_material(voxel);
					if (material == VoxelMaterial::UNDEFINED) {
						continue;
					}
					const VoxelDataTree *node = root->find_deepest_node(voxel);
					const VoxelDataLeaf *leaf = node->is_leaf() ? node->get_leaf_data() : nullptr;
					for (int axis = 0; axis < 4; axis++) {
						Vector4i upper_voxel = voxel;
						upper_voxel[axis]++;
						const VoxelMaterial upper_material = p_data->get_material(upper_voxel);
						if (upper_material == VoxelMaterial::UNDEFINED) {
							continue;
						}
						const bool active = material != upper_material;
						const bool has_data = leaf != nullptr && leaf->has_edge_data(voxel - node->get_bounds().position, axis);
						if (active != has_data) {
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}

// Every material and stored edge datum in the region, for exact comparisons
// of the observable state regardless of how the tree represents it.
static Vector<int32_t> _region_state(const Ref<VoxelData> &p_data, const Rect4i &p_region) {
	Vector<int32_t> state;
	const VoxelDataTree *root = p_data->find_region_neighbourhood(p_data->get_bounds()).node;
	const Vector4i end = p_region.get_end();
	for (int32_t w = p_region.position.w; w < end.w; w++) {
		for (int32_t z = p_region.position.z; z < end.z; z++) {
			for (int32_t y = p_region.position.y; y < end.y; y++) {
				for (int32_t x = p_region.position.x; x < end.x; x++) {
					const Vector4i voxel = Vector4i(x, y, z, w);
					state.push_back((int32_t)p_data->get_material(voxel));
					const VoxelDataTree *node = root == nullptr ? nullptr : root->find_deepest_node(voxel);
					const VoxelDataLeaf *leaf = node != nullptr && node->is_leaf() ? node->get_leaf_data() : nullptr;
					for (int axis = 0; axis < 4; axis++) {
						int32_t entry = 0;
						if (leaf != nullptr && leaf->has_edge_data(voxel - node->get_bounds().position, axis)) {
							entry = 0x10000 | leaf->get_edge_data(voxel - node->get_bounds().position, axis).data;
						}
						state.push_back(entry);
					}
				}
			}
		}
	}
	return state;
}

TEST_CASE("[VoxelData] Edit surface data invariant and idempotence") {
	// A hypercube of 2x2x2x2 chunks of air (see the merging test for the
	// placement), so that the edits below cross the interior chunk borders.
	const Vector4i cube_position = VOXEL_DATA_CHUNK_SIZE_VECTOR * 8;
	Ref<VoxelData> data;
	data.instantiate();
	for (int32_t w = 0; w < 2; w++) {
		for (int32_t z = 0; z < 2; z++) {
			for (int32_t y = 0; y < 2; y++) {
				for (int32_t x = 0; x < 2; x++) {
					data->apply_generated_chunk(data->generate_chunk_content(cube_position + Vector4i(x, y, z, w) * VOXEL_DATA_CHUNK_SIZE));
				}
			}
		}
	}
	const Vector4 cube_center = Vector4(cube_position + VOXEL_DATA_CHUNK_SIZE_VECTOR);
	CHECK_MESSAGE(_surface_data_invariant_holds(data), "VoxelData generated chunks should satisfy the surface data invariant.");

	// Solid over air, then solid over the same solid at an offset (the
	// same-material extension rule), then carving air out of the middle.
	Ref<SphereVoxelEdit> first_solid = memnew(SphereVoxelEdit(cube_center, VOXEL_DATA_CHUNK_SIZE / 2.0 + 1.0, VoxelMaterial::SOLID));
	data->apply_edit(first_solid);
	CHECK_MESSAGE(_surface_data_invariant_holds(data), "VoxelData a solid edit over air should leave the surface data consistent.");
	Vector4 offset_center = cube_center;
	offset_center.x += VOXEL_DATA_CHUNK_SIZE / 4.0;
	Ref<SphereVoxelEdit> offset_solid = memnew(SphereVoxelEdit(offset_center, VOXEL_DATA_CHUNK_SIZE / 2.0 + 1.0, VoxelMaterial::SOLID));
	data->apply_edit(offset_solid);
	CHECK_MESSAGE(_surface_data_invariant_holds(data), "VoxelData an overlapping solid edit should leave the surface data consistent.");
	Ref<SphereVoxelEdit> carve = memnew(SphereVoxelEdit(cube_center, VOXEL_DATA_CHUNK_SIZE / 2.0, VoxelMaterial::AIR));
	data->apply_edit(carve);
	CHECK_MESSAGE(_surface_data_invariant_holds(data), "VoxelData carving air out of solid should leave the surface data consistent.");

	// Applying an edit twice must be indistinguishable from applying it once.
	data->apply_edit(first_solid);
	const Vector<int32_t> state_once = _region_state(data, data->get_bounds());
	data->apply_edit(first_solid);
	const Vector<int32_t> state_twice = _region_state(data, data->get_bounds());
	CHECK_MESSAGE(state_twice == state_once, "VoxelData edits should be idempotent in both materials and surface data.");

	data->merge_edited_constants();
	CHECK_MESSAGE(_surface_data_invariant_holds(data), "VoxelData the merging pass should preserve the surface data invariant.");
}

TEST_CASE("[VoxelData] Edits reconcile with chunks loaded later") {
	// One chunk of air (see the merging test for the placement); its +X
	// neighbor only loads after the edits.
	const Vector4i chunk_position = VOXEL_DATA_CHUNK_SIZE_VECTOR * 8;
	const Vector4i neighbor_position = chunk_position + Vector4i(VOXEL_DATA_CHUNK_SIZE, 0, 0, 0);
	Ref<VoxelData> data;
	data.instantiate();
	data->apply_generated_chunk(data->generate_chunk_content(chunk_position));

	// A solid ball centered on the border plane between the two chunks; only
	// the loaded half applies.
	const Vector4i ball_center_voxel = neighbor_position + Vector4i(0, VOXEL_DATA_CHUNK_SIZE / 2, VOXEL_DATA_CHUNK_SIZE / 2, VOXEL_DATA_CHUNK_SIZE / 2);
	const Vector4 ball_center = Vector4(ball_center_voxel);
	const real_t ball_radius = VOXEL_DATA_CHUNK_SIZE / 2.0;
	Ref<SphereVoxelEdit> solid_edit = memnew(SphereVoxelEdit(ball_center, ball_radius, VoxelMaterial::SOLID));
	data->apply_edit(solid_edit);
	const Vector4i inside_probe = ball_center_voxel - Vector4i(1, 0, 0, 0);
	CHECK_MESSAGE(data->get_material(inside_probe) == VoxelMaterial::SOLID, "VoxelData the loaded half of the ball should become solid.");
	CHECK_MESSAGE(_surface_data_invariant_holds(data), "VoxelData the surface data invariant should hold before the neighbor loads.");

	data->apply_generated_chunk(data->generate_chunk_content(neighbor_position));
	CHECK_MESSAGE(data->get_material(ball_center_voxel) == VoxelMaterial::AIR, "VoxelData the discarded half of the ball should not reappear when its chunk loads.");
	CHECK_MESSAGE(data->get_material(inside_probe) == VoxelMaterial::SOLID, "VoxelData the applied half of the ball should survive the neighbor loading.");
	CHECK_MESSAGE(_surface_data_invariant_holds(data), "VoxelData loading next to an edit should reconcile the border's surface data.");

	// Unloading the neighbor, carving the ball back out, and reloading leaves
	// the border data stored by the reconciliation above stale, since edits
	// skip edges into undefined chunks; the reload must remove it.
	data->unload_chunk(neighbor_position);
	Ref<SphereVoxelEdit> air_edit = memnew(SphereVoxelEdit(ball_center, ball_radius, VoxelMaterial::AIR));
	data->apply_edit(air_edit);
	data->apply_generated_chunk(data->generate_chunk_content(neighbor_position));
	CHECK_MESSAGE(data->get_material(inside_probe) == VoxelMaterial::AIR, "VoxelData carving the ball back out should restore air.");
	CHECK_MESSAGE(_surface_data_invariant_holds(data), "VoxelData loading next to an undone edit should remove the stale border surface data.");
}
} // namespace TestVoxelData
