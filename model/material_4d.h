#pragma once

#include "../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "core/io/resource.h"
#endif

class Material4D : public Resource {
	GDCLASS(Material4D, Resource);

public:
	enum ShadingMode {
		SHADING_MODE_UNSHADED,
	};

	// TODO: Switch to BitField in a future Godot version https://github.com/godotengine/godot/pull/89457
	// Most of these values are not used in the current implementation but are reserved for future use.
	enum ColorSourceFlags {
		COLOR_SOURCE_FLAG_NONE = 0, // Use for errors.
		COLOR_SOURCE_FLAG_SINGLE_COLOR = 1 << 0, // Uniform coloring.
		COLOR_SOURCE_FLAG_PER_VERT = 1 << 1, // 0D element coloring.
		COLOR_SOURCE_FLAG_PER_EDGE = 1 << 2, // 1D element coloring.
		COLOR_SOURCE_FLAG_PER_FACE = 1 << 3, // 2D element coloring.
		COLOR_SOURCE_FLAG_PER_CELL = 1 << 4, // 3D element coloring.
		COLOR_SOURCE_FLAG_VERT_UVW = 1 << 5, // 0D texture mapping.
		COLOR_SOURCE_FLAG_EDGE_UVW = 1 << 6, // 1D texture mapping.
		COLOR_SOURCE_FLAG_FACE_UVW = 1 << 7, // 2D texture mapping.
		COLOR_SOURCE_FLAG_CELL_UVW = 1 << 8, // 3D texture mapping.
		COLOR_SOURCE_FLAG_TEXTURE4D = 1 << 9, // 4D texture mapping.
		COLOR_SOURCE_FLAG_USES_COLOR_ARRAY = COLOR_SOURCE_FLAG_PER_VERT | COLOR_SOURCE_FLAG_PER_EDGE | COLOR_SOURCE_FLAG_PER_FACE | COLOR_SOURCE_FLAG_PER_CELL,
		COLOR_SOURCE_FLAG_USES_TEXTURE_MAP = COLOR_SOURCE_FLAG_VERT_UVW | COLOR_SOURCE_FLAG_EDGE_UVW | COLOR_SOURCE_FLAG_FACE_UVW | COLOR_SOURCE_FLAG_CELL_UVW | COLOR_SOURCE_FLAG_TEXTURE4D,
	};

protected:
	static void _bind_methods();

	ColorSourceFlags _albedo_source_flags = COLOR_SOURCE_FLAG_SINGLE_COLOR;
	Color _albedo_color = Color(1, 1, 1, 1);
	PackedColorArray _albedo_color_array;

public:
	virtual bool is_default_material() const;
	virtual void merge_with(const Ref<Material4D> &p_material, const int p_first_edge_count, const int p_second_edge_count);

	ColorSourceFlags get_albedo_source_flags() const;
	void set_albedo_source_flags(const ColorSourceFlags p_albedo_source_flags);

	Color get_albedo_color() const;
	void set_albedo_color(const Color &p_albedo_color);

	PackedColorArray get_albedo_color_array() const;
	void set_albedo_color_array(const PackedColorArray &p_albedo_color_array);
	void append_albedo_color(const Color &p_albedo_color);
	void resize_albedo_color_array(const int64_t p_size, const Color &p_fill_color = Color(1, 1, 1, 1));
};

VARIANT_ENUM_CAST(Material4D::ColorSourceFlags);
