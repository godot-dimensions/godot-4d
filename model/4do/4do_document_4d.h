#pragma once

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

class MultiSurfaceMesh4D;

// C++ identifiers cannot start with a number, so we use "FourDO" instead of "4DO" in the class name.
class FourDODocument4D : public RefCounted {
	GDCLASS(FourDODocument4D, RefCounted);

	enum VertexIndexType : uint8_t {
		VERTEX_INDEX_TYPE_UNKNOWN = (uint8_t)-1,
		VERTEX_INDEX_TYPE_POSITION = 0, // v
		VERTEX_INDEX_TYPE_NORMAL = 1, // vn
		VERTEX_INDEX_TYPE_TEXTURE_COORDINATE = 2, // vt
		VERTEX_INDEX_TYPE_COLOR = 3, // vc
	};

	struct FourDOMaterial4D {
		Color base_color_factor = Color(1.0f, 1.0f, 1.0f, 1.0f);
		float metallic_factor = 0.0f;
		float roughness_factor = 0.0f;
	};

	struct FourDOVertexInstance4D {
		int32_t v = -1;
		int32_t vn = -1;
		int32_t vt = -1;
		int32_t vc = -1;
	};

	struct FourDOCubicCell4D {
		FourDOVertexInstance4D verts[8];
		int32_t color_index = -1;
	};

	struct FourDOTetrahedralCell4D {
		FourDOVertexInstance4D verts[4];
		int32_t color_index = -1;
	};

	struct FourDOSurface4D {
		Vector<FourDOCubicCell4D> cubic_cells;
		Vector<FourDOTetrahedralCell4D> tetrahedral_cells;
		Vector<PackedInt32Array> polyhedron_face_indices;
		bool is_empty() const {
			return cubic_cells.is_empty() && tetrahedral_cells.is_empty() && polyhedron_face_indices.is_empty();
		}
		bool has_any_normal_indices = false;
		bool has_any_texture_map_indices = false;
		bool has_any_color_indices = false;
	};

	// Store polylines separately to avoid wireframes preventing the generation of solid meshes.
	// This is purposefully different from G4MF. A mesh surface in G4MF semantically says "here is some mesh data", with
	// the interpretation of how it should be rendered left to the engine or user. However, 4DO's semantics are more
	// along the lines of "please render this thing", so if the file says it wants both, we should preserve both.
	struct FourDOPolylineSurface4D {
		Vector<Vector<FourDOVertexInstance4D>> polylines;
	};

	// Data in the file.
	Vector<VertexIndexType> _vertex_format;
	HashMap<String, FourDOMaterial4D> _materials;
	PackedVector4Array _vertex_positions;
	PackedVector4Array _vertex_normals;
	PackedVector3Array _vertex_texture_coordinates;
	PackedColorArray _vertex_colors;
	Vector<Vector<FourDOVertexInstance4D>> _face_vertex_indices;
	HashMap<String, FourDOSurface4D> _surfaces; // By material.
	HashMap<String, FourDOPolylineSurface4D> _polyline_surfaces;
	PackedStringArray _material_filenames;
	int64_t _4do_version = -1;
	// Set by the version 1 `cellformat`/`tformat`/`plformat` commands when cells start with a color index.
	// HoxelDraw still writes these in version 2 files, even though the specification dropped cell-level data.
	bool _cell_format_has_color_index = false;

	// Path data for the file.
	String _4do_base_path = "";
	String _4do_filename = "";

	FourDOVertexInstance4D _import_parse_vertex_instance(const String &p_vertex_instance_string, FourDOSurface4D &r_for_surface) const;
	Error _import_parse_material_raw_text(const String &p_material_raw_text);
	Error _import_parse_4do_raw_text(const String &p_4do_raw_text);
	void _import_gather_cell_color(const int32_t p_color_index, const int64_t p_cell_index, const int64_t p_cell_count, PackedColorArray &r_per_cell_colors) const;
	void _import_gather_vertex_colors(const FourDOVertexInstance4D *p_first_vert_inst, const int64_t p_vert_count, PackedColorArray &r_per_vertex_colors) const;
	Ref<Material4D> _import_generate_material(const String &p_material_name, const PackedColorArray &p_per_cell_colors, const PackedColorArray &p_per_vertex_colors, const bool p_for_poly_mesh) const;
	static bool _has_repeated_vertex(const FourDOVertexInstance4D *p_first_vert_inst, const int64_t p_vert_count);
	static void _remap_vert_indices_for_poly(const FourDODocument4D::FourDOVertexInstance4D *p_first_vert_inst, const int64_t p_vert_count, const Vector<PackedInt32Array> &p_boundary_cell_vertex_indices, const int64_t p_poly_cell_index, Vector<PackedInt32Array> &r_cell_normal_indices, Vector<PackedInt32Array> &r_cell_texture_map_indices, const bool p_has_any_normal_indices, bool p_has_any_texture_map_indices);

protected:
	static void _bind_methods();

public:
	// Static functions to start the import process and create a FourDODocument4D from the data.
	static Ref<FourDODocument4D> import_read_from_byte_array(const PackedByteArray &p_data, const String &p_base_path = "", const String &p_filename = "");
	static Ref<FourDODocument4D> import_read_from_file(const String &p_path);

	// Supplemental read functions.
	Error import_read_materials_from_byte_array(const PackedByteArray &p_data);
	Error import_read_materials_from_file(const String &p_path);

	// Generate meshes from the imported data.
	Ref<MultiSurfaceMesh4D> import_generate_multi_surface_mesh_4d() const;
	// Just MultiSurfaceMesh4D. It's not clear how to best make an ArrayPolyMesh4D, the similar Hox class
	// provides albedo colors via a texture, but 4DO already supplies its own texture coordinates.
};
