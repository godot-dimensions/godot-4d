#pragma once

#include "../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../voxel/data/voxel_data_4d.h"
#include "../../voxel/generators/box_voxel_generator_4d.h"
#include "../../voxel/generators/clipped_voxel_generator_4d.h"
#include "../../voxel/generators/cylinder_voxel_generator_4d.h"
#include "../../voxel/generators/layered_voxel_generator_4d.h"
#include "../../voxel/generators/plane_voxel_generator_4d.h"
#include "../../voxel/generators/tiger_test_generator_4d.h"
#include "../../voxel/voxel_mesher_4d.h"

#include "tests/test_macros.h"

namespace TestVoxelMesher4D {
constexpr VoxelMaterial4D SOLID_MATERIAL = (VoxelMaterial4D)0;

// Meshes one chunk the way the mesh handler does, through the neighborhood
// of the node covering the chunk's region.
static Ref<TetraMesh4D> _mesh_chunk(const Ref<VoxelData4D> &p_data, const Vector4i &p_chunk_position) {
	return VoxelMesher4D::generate_chunk_mesh(p_data->find_region_neighborhood(Rect4i(p_chunk_position, VOXEL_4D_MESH_CHUNK_SIZE_VECTOR)), p_chunk_position);
}

TEST_CASE("[VoxelMesher4D] Eigendecomposition") {
	const Basis4D test_matrices[3] = {
		// A generic symmetric matrix with distinct eigenvalues.
		Basis4D(Vector4(4, 1, 0, 2), Vector4(1, 3, 1, 0), Vector4(0, 1, 2, 1), Vector4(2, 0, 1, 5)),
		// A rank-deficient quadratic error function matrix: the sum of the
		// outer products of the unit normals (1, 0, 0, 0) and (0.6, 0.8, 0, 0).
		Basis4D(Vector4(1.36, 0.48, 0, 0), Vector4(0.48, 0.64, 0, 0), Vector4(), Vector4()),
		// Nearly repeated eigenvalues, with off-diagonals below the tolerance.
		Basis4D(Vector4(2, 1e-7, 0, 0), Vector4(1e-7, 2, 0, 0), Vector4(0, 0, 2, 0), Vector4(0, 0, 0, 7)),
	};
	bool vectors_orthonormal = true;
	bool decomposition_reconstructs = true;
	for (int matrix_index = 0; matrix_index < 3; matrix_index++) {
		Vector4 values;
		Basis4D vectors;
		VoxelMesher4D::eigen_decompose_symmetric_4(test_matrices[matrix_index], values, vectors);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				const real_t column_dot = vectors[i].dot(vectors[j]);
				real_t reconstructed = 0.0f;
				for (int k = 0; k < 4; k++) {
					reconstructed += values[k] * vectors[k][i] * vectors[k][j];
				}
				vectors_orthonormal = vectors_orthonormal && Math::abs(column_dot - (i == j ? 1.0f : 0.0f)) < (real_t)1e-6;
				decomposition_reconstructs = decomposition_reconstructs && Math::abs(reconstructed - test_matrices[matrix_index][i][j]) < (real_t)1e-2;
			}
		}
	}
	CHECK_MESSAGE(vectors_orthonormal, "VoxelMesher4D eigen_decompose_symmetric_4 should produce orthonormal eigenvectors.");
	CHECK_MESSAGE(decomposition_reconstructs, "VoxelMesher4D eigen_decompose_symmetric_4 vectors and values should multiply back to the input matrix.");
}

// Meshes every chunk of the data and checks that every triangle of every
// tetrahedral cell is shared by multiple cells, matching vertices between
// chunks by their quantized world positions. A world whose surface stays
// inside the defined region must mesh closed: an unpaired triangle means two
// cells triangulated a shared square along different diagonals, or two
// chunks disagreed about the position of a shared border vertex.
static bool _whole_world_triangles_paired(const Ref<VoxelData4D> &p_data) {
	HashMap<Vector4i, int32_t> global_vertex_ids;
	HashMap<int64_t, int32_t> triangle_counts;
	const Rect4i bounds = p_data->get_bounds();
	for (int32_t cw = bounds.position.w; cw < bounds.get_end().w; cw += VOXEL_4D_MESH_CHUNK_SIZE) {
		for (int32_t cz = bounds.position.z; cz < bounds.get_end().z; cz += VOXEL_4D_MESH_CHUNK_SIZE) {
			for (int32_t cy = bounds.position.y; cy < bounds.get_end().y; cy += VOXEL_4D_MESH_CHUNK_SIZE) {
				for (int32_t cx = bounds.position.x; cx < bounds.get_end().x; cx += VOXEL_4D_MESH_CHUNK_SIZE) {
					const Vector4i chunk = Vector4i(cx, cy, cz, cw);
					Ref<ArrayTetraMesh4D> chunk_mesh = _mesh_chunk(p_data, chunk);
					const PackedVector4Array chunk_vertices = chunk_mesh->get_vertex_positions();
					const PackedInt32Array chunk_cells = chunk_mesh->get_simplex_cell_vertex_indices();
					LocalVector<int32_t> vertex_ids;
					vertex_ids.resize(chunk_vertices.size());
					for (int64_t i = 0; i < chunk_vertices.size(); i++) {
						// With single precision, two chunks' copies of a shared
						// border vertex can differ by an ulp or so after being
						// offset by different chunk-local lattice positions. A
						// copy close to a rounding boundary also checks the
						// neighboring quantized positions across that boundary.
						Vector4i quantized;
						Vector4i alternate;
						int alternate_axes = 0;
						for (int axis = 0; axis < 4; axis++) {
							const double scaled = ((double)chunk_vertices[i][axis] + (double)chunk[axis]) * 1024.0;
							quantized[axis] = (int32_t)Math::round(scaled);
							const double fraction = scaled - Math::floor(scaled);
							alternate[axis] = quantized[axis] + (fraction < 0.5 ? 1 : -1);
							if (Math::abs(fraction - 0.5) < 0.01) {
								alternate_axes |= 1 << axis;
							}
						}
						int32_t *existing_id = nullptr;
						for (int mask = 0; mask < 16 && existing_id == nullptr; mask++) {
							if ((mask & ~alternate_axes) != 0) {
								continue;
							}
							Vector4i candidate = quantized;
							for (int axis = 0; axis < 4; axis++) {
								if ((mask & (1 << axis)) != 0) {
									candidate[axis] = alternate[axis];
								}
							}
							existing_id = global_vertex_ids.getptr(candidate);
						}
						if (existing_id != nullptr) {
							vertex_ids[i] = *existing_id;
						} else {
							vertex_ids[i] = (int32_t)global_vertex_ids.size();
							global_vertex_ids.insert(quantized, vertex_ids[i]);
						}
					}
					for (int64_t cell = 0; cell < chunk_cells.size() / 4; cell++) {
						for (int skipped = 0; skipped < 4; skipped++) {
							int32_t triangle[3];
							int triangle_size = 0;
							for (int i = 0; i < 4; i++) {
								if (i != skipped) {
									triangle[triangle_size++] = vertex_ids[chunk_cells[cell * 4 + i]];
								}
							}
							if (triangle[0] > triangle[1]) {
								SWAP(triangle[0], triangle[1]);
							}
							if (triangle[1] > triangle[2]) {
								SWAP(triangle[1], triangle[2]);
							}
							if (triangle[0] > triangle[1]) {
								SWAP(triangle[0], triangle[1]);
							}
							const int64_t key = ((int64_t)triangle[0] << 42) | ((int64_t)triangle[1] << 21) | (int64_t)triangle[2];
							int32_t *count = triangle_counts.getptr(key);
							if (count != nullptr) {
								(*count)++;
							} else {
								triangle_counts.insert(key, 1);
							}
						}
					}
				}
			}
		}
	}
	if (triangle_counts.is_empty()) {
		return false;
	}
	for (const KeyValue<int64_t, int32_t> &entry : triangle_counts) {
		if (entry.value == 1) {
			return false;
		}
	}
	return true;
}

TEST_CASE("[VoxelMesher4D] Chunk meshes") {
	Ref<VoxelData4D> data;
	data.instantiate();
	data->set_generator(memnew(TigerTestGenerator4D));
	data->load_all_chunks();

	// The chunk-sized region centered on the origin is entirely inside the
	// tiger's hole, where there are no solid voxels.
	Ref<ArrayTetraMesh4D> empty_chunk = _mesh_chunk(data, Vector4i(-VOXEL_4D_MESH_CHUNK_SIZE / 2, -VOXEL_4D_MESH_CHUNK_SIZE / 2, -VOXEL_4D_MESH_CHUNK_SIZE / 2, -VOXEL_4D_MESH_CHUNK_SIZE / 2));
	REQUIRE(empty_chunk.is_valid());
	CHECK_MESSAGE(empty_chunk->get_simplex_cell_vertex_indices().is_empty(), "VoxelMesher4D should generate no cells for a chunk of empty voxels.");

	Ref<ArrayTetraMesh4D> surface_chunk = _mesh_chunk(data, Vector4i(12, 0, 12, 0));
	const PackedInt32Array cell_indices = surface_chunk->get_simplex_cell_vertex_indices();
	CHECK_MESSAGE(cell_indices.size() > 0, "VoxelMesher4D should generate cells for a chunk on the test shape's surface.");
	CHECK_MESSAGE(cell_indices.size() % 20 == 0, "VoxelMesher4D should generate 5 tetrahedra (20 indices) per face of the blocky topology.");

	const PackedVector4Array vertices = surface_chunk->get_vertex_positions();
	bool vertices_in_range = vertices.size() > 0;
	for (const Vector4 &vertex : vertices) {
		for (int axis = 0; axis < 4; axis++) {
			vertices_in_range = vertices_in_range && vertex[axis] >= -1.0f && vertex[axis] <= (real_t)VOXEL_4D_MESH_CHUNK_SIZE + 1.0f;
		}
	}
	CHECK_MESSAGE(vertices_in_range, "VoxelMesher4D vertices should be chunk-local, near the range [0, VOXEL_4D_MESH_CHUNK_SIZE].");

	// Dual contouring should place every vertex close to the true surface of
	// the test shape, much closer than the blocky lattice points would be.
	bool vertices_on_surface = vertices.size() > 0;
	for (const Vector4 &vertex : vertices) {
		const Vector4 world = vertex + Vector4(12, 0, 12, 0);
		const double xy = Math::sqrt(world.x * world.x + world.y * world.y) - 10.0;
		const double zw = Math::sqrt(world.z * world.z + world.w * world.w) - 10.0;
		const double signed_distance = 4.5 - Math::sqrt(xy * xy + zw * zw);
		vertices_on_surface = vertices_on_surface && Math::abs(signed_distance) < 0.75;
	}
	CHECK_MESSAGE(vertices_on_surface, "VoxelMesher4D vertices should lie close to the generated surface.");

	const PackedVector4Array normals = surface_chunk->get_simplex_cell_boundary_normals();
	CHECK_MESSAGE(normals.size() * 4 == cell_indices.size(), "VoxelMesher4D should generate one boundary normal per cell.");

	// The tiger's surface is closed and interior to the loaded region, so the
	// combined meshes of every chunk must be closed too.
	CHECK_MESSAGE(_whole_world_triangles_paired(data), "VoxelMesher4D: every triangle of every cell of the whole world's meshes should be shared by an even number of cells.");
}

// Deterministic pseudo-random materials inside a fixed box, with plain air
// outside so that the surface never reaches the edge of the loaded region,
// and pseudo-random surface data on every active edge. Materials and edge
// data are pure hashes of the position, so repeated reads and neighboring
// chunks stay consistent.
class RandomSurfaceGenerator : public VoxelGenerator4D {
	static uint32_t _scramble(uint32_t p_value) {
		p_value ^= p_value >> 16;
		p_value *= 0x7feb352d;
		p_value ^= p_value >> 15;
		p_value *= 0x846ca68b;
		p_value ^= p_value >> 16;
		return p_value;
	}

	static uint32_t _position_hash(const Vector4i &p_voxel, const uint32_t p_salt) {
		uint32_t hash = _scramble(p_salt ^ 0x9e3779b9);
		hash = _scramble(hash ^ (uint32_t)p_voxel.x);
		hash = _scramble(hash ^ (uint32_t)p_voxel.y);
		hash = _scramble(hash ^ (uint32_t)p_voxel.z);
		hash = _scramble(hash ^ (uint32_t)p_voxel.w);
		return hash;
	}

public:
	Rect4i random_bounds;

	virtual VoxelMaterial4D get_material(const Vector4i &p_voxel) const override {
		if (!random_bounds.has_point(p_voxel)) {
			return VoxelMaterial4D::AIR;
		}
		// Half air, keeping the more important opaque-transparent boundaries
		// common, then mostly the first opaque material, with two rarer
		// opaque materials mixed in for opaque-opaque boundaries.
		const uint32_t hash = _position_hash(p_voxel, 0);
		if ((hash & 1) != 0) {
			return VoxelMaterial4D::AIR;
		}
		if ((hash & 2) != 0) {
			return SOLID_MATERIAL;
		}
		return (hash & 4) != 0 ? (VoxelMaterial4D)3 : (VoxelMaterial4D)4;
	}

	virtual VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const override {
		const uint32_t hash = _position_hash(p_voxel, (uint32_t)(1 + p_axis));
		Vector4 normal;
		for (int i = 0; i < 4; i++) {
			normal[i] = (real_t)((int32_t)((hash >> (i * 8)) & 255) - 128);
		}
		if (normal.length_squared() < (real_t)1.0) {
			normal = Vector4(1, 0, 0, 0);
		}
		const real_t position = (real_t)(_scramble(hash) & 255) / (real_t)255.0;
		return VoxelEdgeData4D(normal, position);
	}
};

TEST_CASE("[VoxelMesher4D] Random world mesh closedness") {
	// Pseudo-random materials are the worst case for the mesher: disconnected
	// surfaces cross nearly every cell in every direction. The random region
	// is two chunks wide, surrounded by a loaded chunk of air on every side.
	Ref<RandomSurfaceGenerator> generator;
	generator.instantiate();
	generator->random_bounds = Rect4i(VOXEL_4D_DATA_CHUNK_SIZE_VECTOR, VOXEL_4D_DATA_CHUNK_SIZE_VECTOR * 2);
	Ref<VoxelData4D> data;
	data.instantiate();
	data->set_generator(generator);
	for (int32_t w = 0; w < 4; w++) {
		for (int32_t z = 0; z < 4; z++) {
			for (int32_t y = 0; y < 4; y++) {
				for (int32_t x = 0; x < 4; x++) {
					data->apply_generated_chunk(data->generate_chunk_content(Vector4i(x, y, z, w) * VOXEL_4D_DATA_CHUNK_SIZE));
				}
			}
		}
	}
	CHECK_MESSAGE(_whole_world_triangles_paired(data), "VoxelMesher4D meshes of a world of random surfaces should still all be closed.");
}

TEST_CASE("[VoxelMesher4D] Composed generator world closedness") {
	// A composition exercising clipping, one-sided layers, and generation of
	// partially UNDEFINED content (replaced with air): a sloped ground plane
	// defined only below its surface, a solid box clipped to it, and a
	// cylinder window above it all whose outside floods everything else with
	// air. Every surface stays inside the loaded region, so the meshes must
	// come out closed.
	Ref<PlaneVoxelGenerator4D> ground;
	ground.instantiate();
	ground->set_normal(Vector4(0.3f, 1.0f, 0.2f, -0.1f));
	ground->set_distance(0.8f);
	ground->set_material_over(255);
	ground->set_material_under(1);
	Ref<BoxVoxelGenerator4D> block;
	block.instantiate();
	block->set_center(Vector4(-2.3f, 1.4f, 0.6f, -1.2f));
	block->set_size(Vector4(5.3f, 5.8f, 4.6f, 5.1f));
	block->set_material_inner(2);
	block->set_material_outer(255);
	Ref<ClippedVoxelGenerator4D> clipped;
	clipped.instantiate();
	clipped->set_base(ground);
	clipped->set_modifier(block);
	Ref<CylinderVoxelGenerator4D> window;
	window.instantiate();
	window->set_center(Vector4(0.7f, -0.4f, 0.3f, 0.9f));
	window->set_height(9.3f);
	window->set_radius(6.2f);
	window->set_material_inner(255);
	window->set_material_outer(254);
	Ref<LayeredVoxelGenerator4D> composed;
	composed.instantiate();
	Vector<Ref<VoxelGenerator4D>> layers;
	layers.push_back(clipped);
	layers.push_back(window);
	composed->set_layers(layers);

	Ref<VoxelData4D> data;
	data.instantiate();
	data->set_generator(composed);
	for (int32_t w = -1; w < 1; w++) {
		for (int32_t z = -1; z < 1; z++) {
			for (int32_t y = -1; y < 1; y++) {
				for (int32_t x = -1; x < 1; x++) {
					data->apply_generated_chunk(data->generate_chunk_content(Vector4i(x, y, z, w) * VOXEL_4D_DATA_CHUNK_SIZE));
				}
			}
		}
	}
	CHECK_MESSAGE(_whole_world_triangles_paired(data), "VoxelMesher4D meshes of a composed generator world should all be closed.");
}
} // namespace TestVoxelMesher4D
