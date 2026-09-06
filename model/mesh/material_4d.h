#pragma once

#include "../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/shader_material.hpp>

#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "core/io/resource.h"
#include "scene/resources/material.h"
#endif

class Mesh4D;

class Material4D : public Resource {
	GDCLASS(Material4D, Resource);

public:
	enum ShadingMode : uint8_t {
		SHADING_MODE_UNSHADED,
	};

	// TODO: Switch to BitField in a future Godot version https://github.com/godotengine/godot/pull/95916
	// Most of these values are not used in the current implementation but are reserved for future use.
	enum ColorSourceFlags : uint16_t {
		COLOR_SOURCE_FLAG_NONE = 0, // Use for errors.
		COLOR_SOURCE_FLAG_SINGLE_COLOR = 1 << 0, // Uniform coloring.
		COLOR_SOURCE_FLAG_PER_VERT = 1 << 1, // 0D element coloring.
		COLOR_SOURCE_FLAG_PER_EDGE = 1 << 2, // 1D element coloring.
		COLOR_SOURCE_FLAG_PER_FACE = 1 << 3, // 2D element coloring.
		COLOR_SOURCE_FLAG_PER_CELL = 1 << 4, // 3D element coloring.
		COLOR_SOURCE_FLAG_TEXTURE3D_VERT_UVW = 1 << 5, // 0D texture mapping.
		COLOR_SOURCE_FLAG_TEXTURE3D_EDGE_UVW = 1 << 6, // 1D texture mapping.
		COLOR_SOURCE_FLAG_TEXTURE3D_FACE_UVW = 1 << 7, // 2D texture mapping.
		COLOR_SOURCE_FLAG_TEXTURE3D_CELL_UVW = 1 << 8, // 3D texture mapping.
		COLOR_SOURCE_FLAG_TEXTURE4D_DIRECT = 1 << 9, // 4D texture mapping.
		COLOR_SOURCE_FLAG_USES_COLOR_ARRAY = COLOR_SOURCE_FLAG_PER_VERT | COLOR_SOURCE_FLAG_PER_EDGE | COLOR_SOURCE_FLAG_PER_FACE | COLOR_SOURCE_FLAG_PER_CELL,
		COLOR_SOURCE_FLAG_USES_TEXTURE_3D = COLOR_SOURCE_FLAG_TEXTURE3D_VERT_UVW | COLOR_SOURCE_FLAG_TEXTURE3D_EDGE_UVW | COLOR_SOURCE_FLAG_TEXTURE3D_FACE_UVW | COLOR_SOURCE_FLAG_TEXTURE3D_CELL_UVW,
		COLOR_SOURCE_FLAG_USES_TEXTURE_MAP = COLOR_SOURCE_FLAG_USES_TEXTURE_3D | COLOR_SOURCE_FLAG_TEXTURE4D_DIRECT,
	};

	enum TextureTransformMode : uint8_t {
		TEXTURE_TRANSFORM_MODE_NONE,
		TEXTURE_TRANSFORM_MODE_ALL_CHANNELS,
		TEXTURE_TRANSFORM_MODE_PER_CHANNEL,
	};

protected:
	static void _bind_methods();

	// Shared properties.
	Vector3 _texture_map_offset = Vector3(0.0, 0.0, 0.0);
	Vector3 _texture_map_scale = Vector3(1.0, 1.0, 1.0);
	// The texture scale and offset properties are only used when the texture transform mode is not NONE.
	TextureTransformMode _texture_transform_mode = TEXTURE_TRANSFORM_MODE_NONE;

	// Albedo.
	PackedColorArray _edge_albedo_color_cache;
	PackedColorArray _albedo_color_array;
	Color _albedo_color = Color(1, 1, 1, 1);
	Vector3 _albedo_texture_map_offset = Vector3(0.0, 0.0, 0.0);
	Vector3 _albedo_texture_map_scale = Vector3(1.0, 1.0, 1.0);
	// This is usually derived from higher level properties.
	ColorSourceFlags _albedo_source_flags = COLOR_SOURCE_FLAG_SINGLE_COLOR;

	// Materials used for 4D rendering engines based on Godot3DRenderingEngine4D.
	Ref<ShaderMaterial> _cross_section_material_3d;
	Ref<ShaderMaterial> _projected_material_3d;

	// Update _cross_section_material_3d to match current settings on the material, skip if _cross_section_material_3d is null.
	virtual void update_cross_section_material_3d();
	virtual void update_projected_material_3d();

	GDVIRTUAL0(_update_cross_section_material_3d);
	GDVIRTUAL0(_update_projected_material_3d);

public:
	// Common functions.
	virtual Color get_albedo_color_of_edge(const int64_t p_edge_index, const Ref<Mesh4D> &p_for_mesh);
	virtual bool is_default_material() const;
	virtual void merge_with(const Ref<Material4D> &p_material, const int p_first_item_count, const int p_second_item_count);

	// Shared properties.
	TextureTransformMode get_texture_transform_mode() const { return _texture_transform_mode; }
	void set_texture_transform_mode(const TextureTransformMode p_texture_transform_mode);

	Vector3 get_texture_map_offset() const { return _texture_map_offset; }
	void set_texture_map_offset(const Vector3 &p_texture_map_offset);
	Vector3 get_texture_map_scale() const { return _texture_map_scale; }
	void set_texture_map_scale(const Vector3 &p_texture_map_scale);

	// Albedo.
	ColorSourceFlags get_albedo_source_flags() const { return _albedo_source_flags; }
	void set_albedo_source_flags(const ColorSourceFlags p_albedo_source_flags);

	Color get_albedo_color() const { return _albedo_color; }
	void set_albedo_color(const Color &p_albedo_color);

	PackedColorArray get_albedo_color_array() const { return _albedo_color_array; }
	void set_albedo_color_array(const PackedColorArray &p_albedo_color_array);
	void append_albedo_color(const Color &p_albedo_color);
	void resize_albedo_color_array(const int64_t p_size, const Color &p_fill_color = Color(1, 1, 1, 1));

	Vector3 get_albedo_texture_map_offset() const { return _albedo_texture_map_offset; }
	void set_albedo_texture_map_offset(const Vector3 &p_albedo_texture_map_offset);
	Vector3 get_albedo_texture_map_scale() const { return _albedo_texture_map_scale; }
	void set_albedo_texture_map_scale(const Vector3 &p_albedo_texture_map_scale);

	// Resolves the texture transform mode against the albedo channel's own offset/scale,
	// falling back to the shared "all channels" values or the identity as appropriate.
	Vector3 get_effective_albedo_texture_map_offset() const;
	Vector3 get_effective_albedo_texture_map_scale() const;

	// Materials used for 4D rendering engines based on Godot3DRenderingEngine4D.
	Ref<ShaderMaterial> get_cross_section_material_3d();
	Ref<ShaderMaterial> get_projected_material_3d();
};

VARIANT_ENUM_CAST(Material4D::ColorSourceFlags);
VARIANT_ENUM_CAST(Material4D::TextureTransformMode);
