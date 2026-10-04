#pragma once

#include "../math/basis_4d.h"
#include "../model/mesh/tetra/tetra_mesh_4d.h"
#include "data/voxel_data_tree_4d.h"
#include "voxel_constants_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_map.hpp>
#elif GODOT_MODULE
#include "core/templates/hash_map.h"
#endif

// The algorithm that generates the mesh of one mesh chunk of voxel data.
class VoxelMesher4D {
	struct CellSurfaces4D;

	static int _edge_slot(const int p_axis, const Vector4i &p_cell_local_lower);
	static Vector4 _solve_vertex(const Basis4D &p_ata, const Vector4 &p_atb, const Vector4 &p_point_sum, const int p_crossing_count);
	static real_t _couple_mismatch(const Vector4 &p_point_1, const Vector4 &p_normal_1, const Vector4 &p_point_2, const Vector4 &p_normal_2);
	static int _find_surface_root(int *p_parent, int p_slot);
	static void _union_surfaces(int *p_parent, const int p_slot_a, const int p_slot_b);
	static void _pair_square_crossings(int *p_union_set, const bool *p_active, const int p_slots[4], const VoxelMaterial4D p_corner_materials[4], const Vector4 *p_points, const Vector4 *p_normals);
	static const CellSurfaces4D &_get_cell_surfaces(HashMap<Vector4i, CellSurfaces4D> &r_cells, PackedVector4Array &r_vertices, const VoxelDataNeighborhood4D &p_neighborhood, const Vector4i &p_chunk_position, const Vector4i &p_lattice_local);

public:
	// Computes the eigen-decomposition of a symmetric matrix: values and vectors
	// such that p_matrix = r_vectors * diag(r_values) * r_vectors^T, with the
	// eigenvectors as the columns of r_vectors. Only accurate enough for vertex
	// placement, to about 1e-5, not to full precision.
	static void eigen_decompose_symmetric_4(const Basis4D &p_matrix, Vector4 &r_values, Basis4D &r_vectors);

	// Generates the mesh for the VOXEL_4D_MESH_CHUNK_SIZE hypercube of voxel data
	// whose lowest voxel coordinate is the given position, reading materials and
	// surface data through the given neighborhood of a node covering the region,
	// by dual contouring: one cube-topology face per grid edge of the chunk with
	// a solid voxel on one end and air on the other. The surface crossings on the
	// grid edges around each vertex are grouped into surfaces, with one vertex
	// per surface placed to minimize the quadratic error of its crossings.
	// Faces with an undefined voxel on either side are omitted, and a face
	// between two chunks belongs to the chunk that contains the lower voxel of
	// its edge.
	// Vertex coordinates are local to the chunk, near the range
	// [0, VOXEL_4D_MESH_CHUNK_SIZE].
	static Ref<TetraMesh4D> generate_chunk_mesh(const VoxelDataNeighborhood4D &p_neighborhood, const Vector4i &p_chunk_position);
};
