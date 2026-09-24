#pragma once

#include "../math/basis_4d.h"
#include "../model/mesh/tetra/tetra_mesh_4d.h"
#include "data/voxel_data_tree.h"
#include "voxel_constants.h"

// The algorithm that generates the mesh of one mesh chunk of voxel data.
namespace VoxelMesher {

// Computes the eigendecomposition of a symmetric matrix: values and vectors
// such that p_matrix = r_vectors * diag(r_values) * r_vectors^T, with the
// eigenvectors as the columns of r_vectors. Only accurate enough for vertex
// placement, to about 1e-5, not to full precision.
void eigen_decompose_symmetric_4(const Basis4D &p_matrix, Vector4 &r_values, Basis4D &r_vectors);

// Generates the mesh for the VOXEL_MESH_CHUNK_SIZE hypercube of voxel data
// whose lowest voxel coordinate is the given position, reading materials and
// surface data through the given neighbourhood of a node covering the region,
// by dual contouring: one cube-topology face per grid edge of the chunk with
// a solid voxel on one end and air on the other. The surface crossings on the
// grid edges around each vertex are grouped into surfaces, with one vertex
// per surface placed to minimize the quadratic error of its crossings.
// Faces with an undefined voxel on either side are omitted, and a face
// between two chunks belongs to the chunk that contains the lower voxel of
// its edge.
// Vertex coordinates are local to the chunk, near the range
// [0, VOXEL_MESH_CHUNK_SIZE].
Ref<TetraMesh4D> generate_chunk_mesh(const VoxelDataNeighbourhood &p_neighbourhood, const Vector4i &p_chunk_position);

} // namespace VoxelMesher
