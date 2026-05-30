#pragma once

#include "../../../../model/mesh/poly/poly_mesh_builder_4d.h"
#include "../../../../model/mesh/tetra/array_tetra_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestPolyMeshBuilder4D {
TEST_CASE("[PolyMeshBuilder4D] Reconstruct From Tetra Mesh") {
	SUBCASE("Single tetra reconstructs to valid poly mesh") {
		Ref<ArrayTetraMesh4D> tetra_mesh;
		tetra_mesh.instantiate();
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);

		Ref<ArrayPolyMesh4D> poly_mesh = PolyMeshBuilder4D::reconstruct_from_tetra_mesh(tetra_mesh);
		CHECK_MESSAGE(poly_mesh.is_valid(), "Reconstruct should return a valid mesh reference.");
		CHECK_MESSAGE(poly_mesh->is_poly_mesh_data_valid(), "A single tetra should reconstruct into valid poly mesh data.");

		const Vector<Vector<PackedInt32Array>> indices = poly_mesh->get_poly_cell_indices();
		CHECK_MESSAGE(indices.size() >= 2, "Reconstructed mesh should contain faces and cells.");
		CHECK_MESSAGE(indices[0].size() >= 4, "A reconstructed tetra cell should have at least four boundary faces.");
		CHECK_MESSAGE(indices[1].size() >= 1, "A reconstructed tetra should produce at least one 3D cell.");
	}

	SUBCASE("Disconnected coplanar triangle islands no longer collapse to empty face output") {
		Ref<ArrayTetraMesh4D> tetra_mesh;
		tetra_mesh.instantiate();
		// Two tetrahedra sharing only the pivot vertex. Their opposite faces are coplanar (z = 0, w = 0)
		// but disconnected, which exercises the coplanar-island split path.
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(4, 0, 0, 0), Vector4(4, 1, 0, 0), Vector4(4, 0, 1, 0), true);

		Ref<ArrayPolyMesh4D> poly_mesh = PolyMeshBuilder4D::reconstruct_from_tetra_mesh(tetra_mesh);
		CHECK_MESSAGE(poly_mesh.is_valid(), "Reconstruct should return a mesh reference for disconnected coplanar islands.");

		const Vector<Vector<PackedInt32Array>> indices = poly_mesh->get_poly_cell_indices();
		CHECK_MESSAGE(indices.size() >= 2, "Reconstructed mesh should still contain faces and cells arrays.");
		CHECK_MESSAGE(indices[0].size() > 0, "Face list should not be empty after splitting disconnected coplanar islands.");
		CHECK_MESSAGE(indices[1].size() > 0, "Cell list should not be empty after splitting disconnected coplanar islands.");
		for (int64_t face_index = 0; face_index < indices[0].size(); face_index++) {
			CHECK_MESSAGE(indices[0][face_index].size() >= 3, "Each reconstructed face should have at least three edges.");
		}
	}

	SUBCASE("Duplicate tetrahedra produce invalid poly mesh data") {
		Ref<ArrayTetraMesh4D> tetra_mesh;
		tetra_mesh.instantiate();
		// Duplicate tetrahedra cancel all boundary triangles, which is malformed for poly reconstruction.
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);

		ERR_PRINT_OFF;
		Ref<ArrayPolyMesh4D> poly_mesh = PolyMeshBuilder4D::reconstruct_from_tetra_mesh(tetra_mesh);
		CHECK_MESSAGE(poly_mesh.is_valid(), "Reconstruct should still return a mesh reference for malformed input.");
		CHECK_MESSAGE(!poly_mesh->is_poly_mesh_data_valid(), "Duplicate tetra input should not reconstruct to valid poly mesh data.");
		ERR_PRINT_ON;
	}
}

} // namespace TestPolyMeshBuilder4D
