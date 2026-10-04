#pragma once

#include "../../voxel/data/voxel_data_4d.h"
#include "../../voxel/edit/box_voxel_edit_4d.h"
#include "../../voxel/edit/parallelogram_voxel_edit_4d.h"
#include "../../voxel/generators/clipped_voxel_generator_4d.h"
#include "../../voxel/generators/layered_voxel_generator_4d.h"
#include "../../voxel/generators/plane_voxel_generator_4d.h"

#include "tests/test_macros.h"

namespace TestVoxelGenerators4D {

static Ref<VoxelData4D> _generate_block(const Ref<VoxelGenerator4D> &p_generator, const Vector4i &p_corner, const int p_chunks) {
	Ref<VoxelData4D> data;
	data.instantiate();
	for (int32_t w = 0; w < p_chunks; w++) {
		for (int32_t z = 0; z < p_chunks; z++) {
			for (int32_t y = 0; y < p_chunks; y++) {
				for (int32_t x = 0; x < p_chunks; x++) {
					data->apply_generated_chunk(data->generate_chunk_content(p_corner + Vector4i(x, y, z, w) * VOXEL_4D_DATA_CHUNK_SIZE, p_generator));
				}
			}
		}
	}
	return data;
}

// Whether the two VoxelData4Ds hold the same materials at every voxel of the
// region and nearly the same surface data on every active edge interior to
// it: normals matching by direction and positions within the tolerance, for
// constructions that recompute the crossings along different arithmetic
// routes. Constructions whose crossings must agree bit-exactly compare their
// TestVoxelData4D::_region_state captures instead.
static bool _region_contents_close(const Ref<VoxelData4D> &p_a, const Ref<VoxelData4D> &p_b, const Rect4i &p_region, const real_t p_position_tolerance) {
	const Vector4i end = p_region.get_end();
	for (int32_t w = p_region.position.w; w < end.w; w++) {
		for (int32_t z = p_region.position.z; z < end.z; z++) {
			for (int32_t y = p_region.position.y; y < end.y; y++) {
				for (int32_t x = p_region.position.x; x < end.x; x++) {
					const Vector4i voxel = Vector4i(x, y, z, w);
					const VoxelMaterial4D material = p_a->get_material(voxel);
					if (material != p_b->get_material(voxel)) {
						return false;
					}
					for (int axis = 0; axis < 4; axis++) {
						Vector4i upper_voxel = voxel;
						upper_voxel[axis]++;
						if (!p_region.has_point(upper_voxel) || p_a->get_material(upper_voxel) == material) {
							continue;
						}
						const VoxelEdgeData4D data_a = p_a->get_edge_data(voxel, axis);
						const VoxelEdgeData4D data_b = p_b->get_edge_data(voxel, axis);
						if (Math::abs(data_a.normal.dot(data_b.normal)) < (real_t)0.999 || Math::abs(data_a.position - data_b.position) > p_position_tolerance) {
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}

TEST_CASE("[ParallelogramVoxelEdit4D] Equivalence to BoxVoxelEdit4D when axis-aligned") {
	// A uniform backdrop of air, generated as constants that the edits split.
	Ref<PlaneVoxelGenerator4D> air;
	air.instantiate();
	air->set_material_over(254);
	air->set_material_under(254);
	const Vector4i corner = VOXEL_4D_DATA_CHUNK_SIZE_VECTOR * -1;
	const Rect4i region = Rect4i(corner, VOXEL_4D_DATA_CHUNK_SIZE_VECTOR * 2);
	// Seeded, so any failure is reproducible.
	Ref<RandomNumberGenerator> rng;
	rng.instantiate();
	rng->set_seed(292524);
	bool all_equal = true;
	for (int trial = 0; trial < 4; trial++) {
		Rect4 rect;
		for (int axis = 0; axis < 4; axis++) {
			rect.position[axis] = rng->randf_range(-6.0f, -1.0f);
			rect.size[axis] = rng->randf_range(3.0f, 6.5f);
		}
		Ref<VoxelData4D> box_data = _generate_block(air, corner, 2);
		box_data->apply_edit(memnew(BoxVoxelEdit4D(rect, (VoxelMaterial4D)2)));
		Ref<VoxelData4D> parallelogram_data = _generate_block(air, corner, 2);
		parallelogram_data->apply_edit(memnew(ParallelogramVoxelEdit4D(Transform4D(Basis4D::from_scale(rect.size), rect.position), (VoxelMaterial4D)2)));
		// The crossings are computed along different routes, so they can land
		// on either side of a storage quantization step.
		all_equal = all_equal && _region_contents_close(box_data, parallelogram_data, region, (real_t)0.005);
	}
	CHECK_MESSAGE(all_equal, "A ParallelogramVoxelEdit4D with an axis-aligned scaling basis should match the BoxVoxelEdit4D of the same box.");
}

TEST_CASE("[VoxelGenerator4D] Composite generators cannot contain themselves") {
	// Generating with a generator that contains itself would recurse forever.
	Ref<LayeredVoxelGenerator4D> layered;
	layered.instantiate();
	Ref<ClippedVoxelGenerator4D> clipped;
	clipped.instantiate();
	Vector<Ref<VoxelGenerator4D>> layers;
	layers.push_back(layered);
	ERR_PRINT_OFF;
	layered->set_layers(layers);
	clipped->set_base(clipped);
	clipped->set_modifier(clipped);
	ERR_PRINT_ON;
	CHECK_MESSAGE(layered->get_layers().is_empty(), "LayeredVoxelGenerator4D should refuse itself as a layer.");
	CHECK_MESSAGE(clipped->get_base().is_null(), "ClippedVoxelGenerator4D should refuse itself as its base.");
	CHECK_MESSAGE(clipped->get_modifier().is_null(), "ClippedVoxelGenerator4D should refuse itself as its modifier.");

	// Indirectly, through each other.
	layers.clear();
	layers.push_back(clipped);
	layered->set_layers(layers);
	TypedArray<VoxelGenerator4D> bound_layers;
	bound_layers.push_back(clipped);
	bound_layers.push_back(layered);
	ERR_PRINT_OFF;
	clipped->set_base(layered);
	clipped->set_modifier(layered);
	layered->set_layers_bind(bound_layers);
	ERR_PRINT_ON;
	CHECK_MESSAGE(clipped->get_base().is_null(), "ClippedVoxelGenerator4D should refuse a base that contains it.");
	CHECK_MESSAGE(clipped->get_modifier().is_null(), "ClippedVoxelGenerator4D should refuse a modifier that contains it.");
	CHECK_MESSAGE(layered->get_layers().size() == 1, "LayeredVoxelGenerator4D should refuse layers containing itself when set through the bound setter, keeping its previous layers.");

	// Using one generator in several places is not a cycle.
	Ref<PlaneVoxelGenerator4D> plane;
	plane.instantiate();
	clipped->set_base(plane);
	clipped->set_modifier(plane);
	layers.push_back(plane);
	layered->set_layers(layers);
	CHECK_MESSAGE(clipped->get_base() == plane, "ClippedVoxelGenerator4D should accept a base also used elsewhere.");
	CHECK_MESSAGE(clipped->get_modifier() == plane, "ClippedVoxelGenerator4D should accept the same generator as its base and modifier.");
	CHECK_MESSAGE(layered->get_layers().size() == 2, "LayeredVoxelGenerator4D should accept layers that share generators.");
}
} // namespace TestVoxelGenerators4D
