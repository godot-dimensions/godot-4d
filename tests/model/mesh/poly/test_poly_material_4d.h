#pragma once

#include "../../../../model/mesh/poly/array_poly_mesh_4d.h"
#include "../../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../../model/mesh/poly/poly_material_4d.h"

#include "tests/test_macros.h"

namespace TestPolyMaterial4D {
inline Ref<PolyMaterial4D> make_single_color_poly_material(const Color &p_color) {
	Ref<PolyMaterial4D> material;
	material.instantiate();
	material->set_albedo_color(p_color);
	return material;
}

inline Ref<PolyMaterial4D> make_per_cell_poly_material(const PackedColorArray &p_cell_colors) {
	Ref<PolyMaterial4D> material;
	material.instantiate();
	material->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
	material->set_poly_albedo_color_array(p_cell_colors);
	return material;
}

// A box has 8 boundary cells (cubes).
inline Ref<ArrayPolyMesh4D> make_box_array_mesh() {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	return box->to_array_poly_mesh();
}

TEST_CASE("[PolyMaterial4D] Merge materials") {
	const Color red = Color(1, 0, 0);
	const Color green = Color(0, 1, 0);
	const Color blue = Color(0, 0, 1);
	const Color white = Color(1, 1, 1);

	SUBCASE("Merging two different single colors produces a per-cell color array") {
		Ref<PolyMaterial4D> first = make_single_color_poly_material(red);
		Ref<PolyMaterial4D> second = make_single_color_poly_material(blue);
		first->merge_with(second, 3, 2);
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		CHECK_MESSAGE((first->get_poly_albedo_color_array() == PackedColorArray{ red, red, red, blue, blue }), "The cell counts should decide how many of each single color end up in the per-cell array.");
		CHECK_MESSAGE(first->get_albedo_color_array().is_empty(), "The per-tetrahedron array is a cache that should be left empty until populated for a mesh.");
		CHECK_MESSAGE(second->get_poly_albedo_color_array().is_empty(), "Merging must not modify the other material.");
		CHECK(second->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_SINGLE_COLOR);
	}

	SUBCASE("Merging the same single color keeps a single color") {
		Ref<PolyMaterial4D> first = make_single_color_poly_material(red);
		Ref<PolyMaterial4D> second = make_single_color_poly_material(red);
		first->merge_with(second, 3, 2);
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_SINGLE_COLOR);
		CHECK(first->get_albedo_color() == red);
		CHECK(first->get_poly_albedo_color_array().is_empty());
	}

	SUBCASE("Merging a per-cell array with a single color appends the single color") {
		Ref<PolyMaterial4D> first = make_per_cell_poly_material(PackedColorArray{ red, green });
		Ref<PolyMaterial4D> second = make_single_color_poly_material(blue);
		first->merge_with(second, 2, 1);
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		CHECK((first->get_poly_albedo_color_array() == PackedColorArray{ red, green, blue }));
	}

	SUBCASE("Merging a single color with a per-cell array prepends the single color") {
		Ref<PolyMaterial4D> first = make_single_color_poly_material(blue);
		Ref<PolyMaterial4D> second = make_per_cell_poly_material(PackedColorArray{ red, green });
		first->merge_with(second, 1, 2);
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		CHECK((first->get_poly_albedo_color_array() == PackedColorArray{ blue, red, green }));
	}

	SUBCASE("Merging two per-cell arrays concatenates them") {
		Ref<PolyMaterial4D> first = make_per_cell_poly_material(PackedColorArray{ red, green });
		Ref<PolyMaterial4D> second = make_per_cell_poly_material(PackedColorArray{ blue, white, red });
		first->merge_with(second, 2, 3);
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		CHECK((first->get_poly_albedo_color_array() == PackedColorArray{ red, green, blue, white, red }));
	}

	SUBCASE("Merging a per-cell and single color material bakes the single color when it cannot be kept") {
		const Color gray = Color(0.5, 0.5, 0.5);
		Ref<PolyMaterial4D> first = make_per_cell_poly_material(PackedColorArray{ red, green });
		first->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE);
		first->set_albedo_color(gray);
		Ref<PolyMaterial4D> second = make_single_color_poly_material(blue);
		first->merge_with(second, 2, 1);
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		CHECK((first->get_poly_albedo_color_array() == PackedColorArray{ red * gray, green * gray, blue }));
	}

	SUBCASE("Merging a per-cell and single color material keeps the single color when it matches") {
		const Color gray = Color(0.5, 0.5, 0.5);
		Ref<PolyMaterial4D> first = make_per_cell_poly_material(PackedColorArray{ red, green });
		first->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE);
		first->set_albedo_color(gray);
		Ref<PolyMaterial4D> second = make_single_color_poly_material(gray);
		first->merge_with(second, 2, 1);
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE);
		CHECK(first->get_albedo_color() == gray);
		CHECK((first->get_poly_albedo_color_array() == PackedColorArray{ red, green, white }));
	}

	SUBCASE("Merging with a non-poly material's color array keeps only its single color") {
		Ref<PolyMaterial4D> first = make_single_color_poly_material(red);
		Ref<TetraMaterial4D> second;
		second.instantiate();
		second->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_VERT_AND_SINGLE);
		second->set_albedo_color(blue);
		second->set_albedo_color_array(PackedColorArray{ green, green, green, green });
		ERR_PRINT_OFF;
		first->merge_with(second, 2, 1);
		ERR_PRINT_ON;
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		CHECK_MESSAGE((first->get_poly_albedo_color_array() == PackedColorArray{ red, red, blue }), "Per-vertex colors cannot be mapped to cells, so only the other material's single color is used.");
	}

	SUBCASE("Merging with a non-poly material's color array without a single color inserts white") {
		Ref<PolyMaterial4D> first = make_single_color_poly_material(red);
		Ref<TetraMaterial4D> second;
		second.instantiate();
		second->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_VERT_ONLY);
		second->set_albedo_color_array(PackedColorArray{ green, green, green, green });
		ERR_PRINT_OFF;
		first->merge_with(second, 2, 1);
		ERR_PRINT_ON;
		CHECK(first->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		CHECK((first->get_poly_albedo_color_array() == PackedColorArray{ red, red, white }));
	}
}

TEST_CASE("[ArrayPolyMesh4D] Merge meshes with poly materials") {
	const Color red = Color(1, 0, 0);
	const Color blue = Color(0, 0, 1);
	const Color white = Color(1, 1, 1);

	SUBCASE("Merging two boxes with different single colors gives one color per boundary cell") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		Ref<PolyMaterial4D> red_material = make_single_color_poly_material(red);
		mesh->set_material(red_material);
		Ref<ArrayPolyMesh4D> other = make_box_array_mesh();
		Ref<PolyMaterial4D> blue_material = make_single_color_poly_material(blue);
		other->set_material(blue_material);
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		const Ref<PolyMaterial4D> merged_material = mesh->get_material();
		REQUIRE_MESSAGE(merged_material.is_valid(), "The merged material should still be a PolyMaterial4D.");
		CHECK_MESSAGE(merged_material.ptr() != red_material.ptr(), "Merging should not mutate the original material, which may be shared with other meshes.");
		CHECK(red_material->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_SINGLE_COLOR);
		CHECK(red_material->get_poly_albedo_color_array().is_empty());
		CHECK(blue_material->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_SINGLE_COLOR);
		CHECK(merged_material->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
		const PackedColorArray cell_colors = merged_material->get_poly_albedo_color_array();
		REQUIRE_MESSAGE(cell_colors.size() == 16, "Each box has 8 boundary cells, so the merged material should have 16 cell colors, not one per vertex.");
		for (int64_t i = 0; i < 8; i++) {
			CHECK(cell_colors[i] == red);
			CHECK(cell_colors[8 + i] == blue);
		}
		// The per-cell colors can be expanded to one color per tetrahedron of the merged mesh.
		merged_material->populate_albedo_color_array_for_poly_mesh(mesh);
		const PackedColorArray tet_colors = merged_material->get_albedo_color_array();
		const int64_t tet_count = mesh->get_simplex_cell_vertex_indices().size() / 4;
		REQUIRE(tet_count > 0);
		REQUIRE(tet_colors.size() == tet_count);
		CHECK(tet_colors[0] == red);
		CHECK(tet_colors[tet_count - 1] == blue);
	}

	SUBCASE("Merging into a mesh without a material creates a poly material with white for the existing cells") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		Ref<ArrayPolyMesh4D> other = make_box_array_mesh();
		PackedColorArray other_cell_colors;
		for (int i = 0; i < 8; i++) {
			other_cell_colors.append(Color(i / 8.0f, 0, 1));
		}
		Ref<PolyMaterial4D> other_material = make_per_cell_poly_material(other_cell_colors);
		other->set_material(other_material);
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		const Ref<PolyMaterial4D> merged_material = mesh->get_material();
		REQUIRE_MESSAGE(merged_material.is_valid(), "The new material should be a PolyMaterial4D to hold the per-cell colors.");
		CHECK(merged_material.ptr() != other_material.ptr());
		CHECK_MESSAGE(merged_material->get_albedo_source() == TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE, "Both materials have a white single color, so it can be kept alongside the per-cell array.");
		CHECK(merged_material->get_albedo_color() == white);
		const PackedColorArray cell_colors = merged_material->get_poly_albedo_color_array();
		REQUIRE(cell_colors.size() == 16);
		for (int64_t i = 0; i < 8; i++) {
			CHECK(cell_colors[i] == white);
			CHECK(cell_colors[8 + i] == other_cell_colors[i]);
		}
		CHECK_MESSAGE((other_material->get_poly_albedo_color_array() == other_cell_colors), "Merging must not modify the other mesh's material.");
	}

	SUBCASE("Merging into a mesh without a material shares a single color material") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		Ref<ArrayPolyMesh4D> other = make_box_array_mesh();
		Ref<PolyMaterial4D> other_material = make_single_color_poly_material(blue);
		other->set_material(other_material);
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		CHECK_MESSAGE(mesh->get_material().ptr() == other_material.ptr(), "A single color needs no per-cell data, so the material can be shared as-is.");
	}
}
} // namespace TestPolyMaterial4D
