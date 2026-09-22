#pragma once

#include "../../math/euler_4d.h"
#include "../mesh/multi_surface_mesh_4d.h"
#include "../mesh/poly/array_poly_mesh_4d.h"
#include "../mesh/tetra/array_tetra_mesh_4d.h"
#include "../mesh/wire/array_wire_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/resource.hpp>
#elif GODOT_MODULE
#include "core/io/resource.h"
#include "scene/resources/mesh.h"
#endif

class Node4D;

// `.hox` files do not have a formal specification, so this is developed ad-hoc based on observed files.
class HoxDocument4D : public RefCounted {
	GDCLASS(HoxDocument4D, RefCounted);

public:
	// These values need to be kept stable as new ones are introduced.
	// Wireframe is intentionally not offered: hoxel data is volumetric, so a wireframe of it is not useful.
	enum HoxMeshFormat {
		HOX_MESH_FORMAT_POLYTOPE = 0,
		HOX_MESH_FORMAT_TETRAHEDRAL = 1,
	};

private:
	struct HoxelGrid4D {
		HashMap<Vector4i, int64_t> data; // Map from 4D coordinates to material index.
		Euler4D rotation = Euler4D(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
		Vector4i dimensions = Vector4i(0, 0, 0, 0); // Size.
		Vector4 translation = Vector4(0, 0, 0, 0);

		Basis4D get_grid_rotation() const {
			Basis4D result = Basis4D();
			// The grid rotation's pivot is the origin corner of the hoxel grid.
			// HoxelDraw's grid rotation values use radians, which matches Godot 4D.
			const Basis4D yz_basis = Basis4D::from_yz(rotation.yz);
			const Basis4D zx_basis = Basis4D::from_zx(rotation.zx);
			const Basis4D xy_basis = Basis4D::from_xy(rotation.xy);
			const Basis4D xw_basis = Basis4D::from_xw(rotation.xw);
			const Basis4D wy_basis = Basis4D::from_wy(rotation.wy);
			const Basis4D zw_basis = Basis4D::from_zw(rotation.zw);
			// HoxelDraw uses a different Euler order convention. One might assume the
			// difference is due to the Z-up coordinate system, but actually, the observed
			// order is not ideal for that (the Z ones would be last if that was the case).
			return zw_basis * wy_basis * yz_basis * zx_basis * xw_basis * xy_basis;
		}
	};

	struct HoxMaterial4D {
		Color albedo_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	};

	// Data in the file.
	Vector<HoxelGrid4D> _hoxel_grids;
	Vector<HoxMaterial4D> _materials;
	String _hoxel_draw_version;

	// Path data for the file.
	String _hox_filename = "";

	static Vector4i _import_parse_vector_int(const Variant &p_json_array_or_dict);
	static Vector4 _import_parse_vector_float(const Variant &p_json_array_or_dict);
	void _import_parse_hoxel_grids(const Array &p_json_hoxel_grids);
	void _import_parse_materials(const Array &p_json_materials);
	Error _import_parse_json_data(const Dictionary &p_hox_json);

	// Describes one hoxel's geometry in terms of its 16 corners, derived from BoxPolyMesh4D so the element
	// ordering matches the box exactly. Each corner is a 4-bit index where bit N set means the corner is on
	// the positive side of axis N. Note that the cell orientation does not survive into the output, because
	// faces and edges are shared between neighboring hoxels, so the outward normals are recorded explicitly.
	struct HoxelTemplate4D {
		PackedInt32Array edge_corners; // Two corner indices per edge.
		Vector<PackedInt32Array> face_edges; // Template edge indices per face.
		Vector<PackedInt32Array> cell_faces; // Template face indices per cell.
		Vector<PackedInt32Array> volume_cells; // Template cell indices per 4D volume (there is one, the hoxel itself).
		// Per element: the corner index of its minimum corner, and a mask of the axes it spans.
		PackedInt32Array edge_min_corner;
		PackedInt32Array edge_span_mask;
		PackedInt32Array face_min_corner;
		PackedInt32Array face_span_mask;
		PackedInt32Array cell_min_corner;
		// Per cell: the one axis it does not span, and whether it lies on the positive side of the hoxel.
		PackedInt32Array cell_fixed_axis;
		PackedInt32Array cell_positive_side;
	};

	// Accumulates the geometry of one output surface, sharing vertices, edges, and faces between hoxels.
	struct HoxSurfaceBuilder4D {
		PackedVector4Array vertices;
		PackedInt32Array edge_vertex_indices;
		Vector<PackedInt32Array> faces;
		Vector<PackedInt32Array> cells;
		PackedVector4Array cell_boundary_normals; // Per cell, pointing away from the hoxel that created it.
		Vector<PackedInt32Array> volumes; // Only filled when including interior geometry.
		PackedInt32Array cell_material_indices; // Only filled when merging materials into one surface.
		// Lookup maps from packed grid-local element keys to indices in the arrays above.
		// These are only meaningful within one hoxel grid, so they are cleared for each grid.
		HashMap<int64_t, int32_t> vertex_map;
		HashMap<int64_t, int32_t> edge_map;
		HashMap<int64_t, int32_t> face_map;
		HashMap<int64_t, int32_t> cell_map; // Only used when including interior geometry, since only then are cells shared.
	};

	static HoxelTemplate4D _make_hoxel_template();
	static int64_t _hoxel_element_key(const Vector4i &p_min_corner, const int p_span_mask);
	static Vector4i _hoxel_corner_offset(const int p_corner);
	void _import_generate_grid_hoxel_cells(HashMap<int64_t, HoxSurfaceBuilder4D> &r_builders, const HoxelTemplate4D &p_hoxel_template, const int64_t p_hoxel_grid_index, const bool p_apply_grid_transform, const bool p_merge_materials, const bool p_include_interior) const;
	static Ref<ArrayPolyMesh4D> _import_generate_poly_mesh_4d_from_builder(const HoxSurfaceBuilder4D &p_builder);
	Ref<MultiSurfaceMesh4D> _import_generate_multi_surface_mesh_4d_from_builders(const HashMap<int64_t, HoxSurfaceBuilder4D> &p_builders, const HoxMeshFormat p_mesh_format) const;

protected:
	static void _bind_methods();

public:
	// Static functions to start the import process and create a HoxDocument4D from the data.
	static Ref<HoxDocument4D> import_read_from_byte_array(const PackedByteArray &p_data, const String &p_filename = "");
	static Ref<HoxDocument4D> import_read_from_file(const String &p_path);

	// Generate meshes from the imported data.
	Ref<MultiSurfaceMesh4D> import_generate_multi_surface_mesh_4d(const bool p_include_interior = false, const HoxMeshFormat p_mesh_format = HOX_MESH_FORMAT_POLYTOPE) const;
	Ref<ArrayPolyMesh4D> import_generate_poly_mesh_4d(const bool p_include_interior = false) const;
	Node4D *import_generate_scene(const bool p_include_interior = false, const HoxMeshFormat p_mesh_format = HOX_MESH_FORMAT_POLYTOPE) const;
};

VARIANT_ENUM_CAST(HoxDocument4D::HoxMeshFormat);
