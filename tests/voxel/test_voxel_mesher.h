#pragma once

#include "../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../voxel/voxel_mesher.h"

#include "tests/test_macros.h"

namespace TestVoxelMesher {
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

TEST_CASE("[VoxelMesher] Chunk meshes") {
	Ref<VoxelData> data;
	data.instantiate();

	// The chunk-sized region centered on the origin is entirely inside the
	// tiger's hole, where there are no solid voxels.
	Ref<ArrayTetraMesh4D> empty_chunk = VoxelMesher::generate_chunk_mesh(data, Vector4i(-VOXEL_MESH_CHUNK_SIZE / 2, -VOXEL_MESH_CHUNK_SIZE / 2, -VOXEL_MESH_CHUNK_SIZE / 2, -VOXEL_MESH_CHUNK_SIZE / 2));
	REQUIRE(empty_chunk.is_valid());
	CHECK_MESSAGE(empty_chunk->get_simplex_cell_indices().is_empty(), "VoxelMesher should generate no cells for a chunk of empty voxels.");

	Ref<ArrayTetraMesh4D> surface_chunk = VoxelMesher::generate_chunk_mesh(data, Vector4i(12, 0, 12, 0));
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

	// Across the whole world the surface is closed, so counting the meshes of
	// every chunk together, every triangle of every cell must be shared by an
	// even number of cells. An unpaired triangle means either two cells
	// triangulated a shared square along different diagonals, or two chunks
	// disagreed about the position of a shared border vertex. Vertices are
	// matched between chunks by their quantized world positions.
	HashMap<Vector4i, int32_t> global_vertex_ids;
	HashMap<int64_t, int32_t> triangle_counts;
	const Rect4i bounds = data->get_bounds();
	for (int32_t cw = bounds.position.w; cw < bounds.get_end().w; cw += VOXEL_MESH_CHUNK_SIZE) {
		for (int32_t cz = bounds.position.z; cz < bounds.get_end().z; cz += VOXEL_MESH_CHUNK_SIZE) {
			for (int32_t cy = bounds.position.y; cy < bounds.get_end().y; cy += VOXEL_MESH_CHUNK_SIZE) {
				for (int32_t cx = bounds.position.x; cx < bounds.get_end().x; cx += VOXEL_MESH_CHUNK_SIZE) {
					const Vector4i chunk = Vector4i(cx, cy, cz, cw);
					Ref<ArrayTetraMesh4D> chunk_mesh = VoxelMesher::generate_chunk_mesh(data, chunk);
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
	bool all_triangles_paired = !triangle_counts.is_empty();
	for (const KeyValue<int64_t, int32_t> &entry : triangle_counts) {
		all_triangles_paired = all_triangles_paired && (entry.value & 1) == 0;
	}
	CHECK_MESSAGE(all_triangles_paired, "VoxelMesher: every triangle of every cell of the whole world's meshes should be shared by an even number of cells.");
}
} // namespace TestVoxelMesher
