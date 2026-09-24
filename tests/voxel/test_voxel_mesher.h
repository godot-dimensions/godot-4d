#pragma once

#include "../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../voxel/data/voxel_data.h"
#include "../../voxel/voxel_mesher.h"

#include "tests/test_macros.h"

namespace TestVoxelMesher {
constexpr VoxelMaterial SOLID_MATERIAL = VoxelMaterial::RESERVED_COUNT;

// Meshes one chunk the way the mesh handler does, through the neighbourhood
// of the node covering the chunk's region.
static Ref<Mesh4D> _mesh_chunk(const Ref<VoxelData> &p_data, const Vector4i &p_chunk_position) {
	return VoxelMesher::generate_chunk_mesh(p_data->find_region_neighbourhood(Rect4i(p_chunk_position, VOXEL_MESH_CHUNK_SIZE_VECTOR)), p_chunk_position);
}

TEST_CASE("[VoxelMesher] Eigendecomposition") {
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
		VoxelMesher::eigen_decompose_symmetric_4(test_matrices[matrix_index], values, vectors);
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
	CHECK_MESSAGE(vectors_orthonormal, "VoxelMesher eigen_decompose_symmetric_4 should produce orthonormal eigenvectors.");
	CHECK_MESSAGE(decomposition_reconstructs, "VoxelMesher eigen_decompose_symmetric_4 vectors and values should multiply back to the input matrix.");
}

// Meshes every chunk of the data and checks that every triangle of every
// tetrahedral cell is shared by multiple cells, matching vertices between
// chunks by their quantized world positions. A world whose surface stays
// inside the defined region must mesh closed: an unpaired triangle means two
// cells triangulated a shared square along different diagonals, or two
// chunks disagreed about the position of a shared border vertex.
static bool _whole_world_triangles_paired(const Ref<VoxelData> &p_data) {
	HashMap<Vector4i, int32_t> global_vertex_ids;
	HashMap<int64_t, int32_t> triangle_counts;
	const Rect4i bounds = p_data->get_bounds();
	for (int32_t cw = bounds.position.w; cw < bounds.get_end().w; cw += VOXEL_MESH_CHUNK_SIZE) {
		for (int32_t cz = bounds.position.z; cz < bounds.get_end().z; cz += VOXEL_MESH_CHUNK_SIZE) {
			for (int32_t cy = bounds.position.y; cy < bounds.get_end().y; cy += VOXEL_MESH_CHUNK_SIZE) {
				for (int32_t cx = bounds.position.x; cx < bounds.get_end().x; cx += VOXEL_MESH_CHUNK_SIZE) {
					const Vector4i chunk = Vector4i(cx, cy, cz, cw);
					Ref<ArrayTetraMesh4D> chunk_mesh = _mesh_chunk(p_data, chunk);
					const PackedVector4Array chunk_vertices = chunk_mesh->get_vertices();
					const PackedInt32Array chunk_cells = chunk_mesh->get_simplex_cell_indices();
					LocalVector<int32_t> vertex_ids;
					vertex_ids.resize(chunk_vertices.size());
					for (int64_t i = 0; i < chunk_vertices.size(); i++) {
						const Vector4 world = chunk_vertices[i] + Vector4(chunk);
						const Vector4i quantized = Vector4i(
								(int32_t)Math::round(world.x * 1024.0),
								(int32_t)Math::round(world.y * 1024.0),
								(int32_t)Math::round(world.z * 1024.0),
								(int32_t)Math::round(world.w * 1024.0));
						int32_t *existing_id = global_vertex_ids.getptr(quantized);
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

TEST_CASE("[VoxelMesher] Chunk meshes") {
	Ref<VoxelData> data;
	data.instantiate();
	data->load_all_chunks();

	// The chunk-sized region centered on the origin is entirely inside the
	// tiger's hole, where there are no solid voxels.
	Ref<ArrayTetraMesh4D> empty_chunk = _mesh_chunk(data, Vector4i(-VOXEL_MESH_CHUNK_SIZE / 2, -VOXEL_MESH_CHUNK_SIZE / 2, -VOXEL_MESH_CHUNK_SIZE / 2, -VOXEL_MESH_CHUNK_SIZE / 2));
	REQUIRE(empty_chunk.is_valid());
	CHECK_MESSAGE(empty_chunk->get_simplex_cell_indices().is_empty(), "VoxelMesher should generate no cells for a chunk of empty voxels.");

	Ref<ArrayTetraMesh4D> surface_chunk = _mesh_chunk(data, Vector4i(12, 0, 12, 0));
	const PackedInt32Array cell_indices = surface_chunk->get_simplex_cell_indices();
	CHECK_MESSAGE(cell_indices.size() > 0, "VoxelMesher should generate cells for a chunk on the test shape's surface.");
	CHECK_MESSAGE(cell_indices.size() % 20 == 0, "VoxelMesher should generate 5 tetrahedra (20 indices) per face of the blocky topology.");

	const PackedVector4Array vertices = surface_chunk->get_vertices();
	bool vertices_in_range = vertices.size() > 0;
	for (const Vector4 &vertex : vertices) {
		for (int axis = 0; axis < 4; axis++) {
			vertices_in_range = vertices_in_range && vertex[axis] >= -1.0f && vertex[axis] <= (real_t)VOXEL_MESH_CHUNK_SIZE + 1.0f;
		}
	}
	CHECK_MESSAGE(vertices_in_range, "VoxelMesher vertices should be chunk-local, near the range [0, VOXEL_MESH_CHUNK_SIZE].");

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
	CHECK_MESSAGE(vertices_on_surface, "VoxelMesher vertices should lie close to the generated surface.");

	const PackedVector4Array normals = surface_chunk->get_simplex_cell_boundary_normals();
	CHECK_MESSAGE(normals.size() * 4 == cell_indices.size(), "VoxelMesher should generate one boundary normal per cell.");

	// The tiger's surface is closed and interior to the loaded region, so the
	// combined meshes of every chunk must be closed too.
	CHECK_MESSAGE(_whole_world_triangles_paired(data), "VoxelMesher: every triangle of every cell of the whole world's meshes should be shared by an even number of cells.");
}

// Deterministic pseudo-random materials inside a fixed box, with plain air
// outside so that the surface never reaches the edge of the loaded region,
// and pseudo-random surface data on every active edge. Materials and edge
// data are pure hashes of the position, so repeated reads and neighboring
// chunks stay consistent.
class RandomSurfaceGenerator : public VoxelGenerator {
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

	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override {
		if (!random_bounds.has_point(p_voxel)) {
			return VoxelMaterial::AIR;
		}
		// Half air, keeping the more important opaque-transparent boundaries
		// common, then mostly the first opaque material, with two rarer
		// opaque materials mixed in for opaque-opaque boundaries.
		const uint32_t hash = _position_hash(p_voxel, 0);
		if ((hash & 1) != 0) {
			return VoxelMaterial::AIR;
		}
		if ((hash & 2) != 0) {
			return SOLID_MATERIAL;
		}
		return (hash & 4) != 0 ? (VoxelMaterial)3 : (VoxelMaterial)4;
	}

	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override {
		const uint32_t hash = _position_hash(p_voxel, (uint32_t)(1 + p_axis));
		Vector4 normal;
		for (int i = 0; i < 4; i++) {
			normal[i] = (real_t)((int32_t)((hash >> (i * 8)) & 255) - 128);
		}
		if (normal.length_squared() < (real_t)1.0) {
			normal = Vector4(1, 0, 0, 0);
		}
		const real_t position = (real_t)(_scramble(hash) & 255) / (real_t)255.0;
		return VoxelEdgeData(normal, position);
	}
};

TEST_CASE("[VoxelMesher] Random world mesh closedness") {
	// Pseudo-random materials are the worst case for the mesher: disconnected
	// surfaces cross nearly every cell in every direction. The random region
	// is two chunks wide, surrounded by a loaded chunk of air on every side.
	Ref<RandomSurfaceGenerator> generator;
	generator.instantiate();
	generator->random_bounds = Rect4i(VOXEL_DATA_CHUNK_SIZE_VECTOR, VOXEL_DATA_CHUNK_SIZE_VECTOR * 2);
	Ref<VoxelData> data;
	data.instantiate();
	data->set_generator(generator);
	for (int32_t w = 0; w < 4; w++) {
		for (int32_t z = 0; z < 4; z++) {
			for (int32_t y = 0; y < 4; y++) {
				for (int32_t x = 0; x < 4; x++) {
					data->apply_generated_chunk(data->generate_chunk_content(Vector4i(x, y, z, w) * VOXEL_DATA_CHUNK_SIZE));
				}
			}
		}
	}
	CHECK_MESSAGE(_whole_world_triangles_paired(data), "VoxelMesher meshes of a world of random surfaces should still all be closed.");
}
} // namespace TestVoxelMesher
