#pragma once

#include "../math/basis_4d.h"
#include "../model/mesh/mesh_4d.h"
#include "data/voxel_data.h"
#include "voxel_constants.h"

// The algorithm that generates the mesh of one mesh chunk of voxel data.
namespace VoxelMesher {

// Computes the eigendecomposition of a symmetric matrix: values and vectors
// such that p_matrix = r_vectors * diag(r_values) * r_vectors^T, with the
// eigenvectors as the columns of r_vectors. Only accurate enough for vertex
// placement, to about 1e-5, not to full precision.
void eigen_decompose_symmetric_4(const Basis4D &p_matrix, Vector4 &r_values, Basis4D &r_vectors);

// Generates the mesh for the VOXEL_MESH_CHUNK_SIZE hypercube of the given
// voxel data whose lowest voxel coordinate is the given position, by dual
// contouring: one cube-topology face per face of a solid voxel in the chunk
// whose neighbor on that side is transparent, with each vertex placed to minimize
// the quadratic error of the surface crossings on the grid edges around it.
// Faces with an undefined voxel on either side are omitted, and a face
// between two chunks belongs to the chunk that contains its solid voxel.
// Vertex coordinates are local to the chunk, near the range
// [0, VOXEL_MESH_CHUNK_SIZE].
Ref<Mesh4D> generate_chunk_mesh(const Ref<VoxelData> &p_voxel_data, const Vector4i &p_chunk_position);

} // namespace VoxelMesher
