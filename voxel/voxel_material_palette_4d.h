#pragma once

#include "data/voxel_material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture3d.hpp>
#elif GODOT_MODULE
#include "core/io/resource.h"
#include "scene/resources/texture.h"
#endif

// The properties of a world's voxel materials. The reserved materials (air
// and undefined) have no entries, so anything beyond CUSTOM_COUNT entries
// is ignored.
class VoxelMaterialPalette4D : public Resource {
	GDCLASS(VoxelMaterialPalette4D, Resource);

	PackedColorArray _colors;

protected:
	static void _bind_methods();

public:
	// The width of the 3D textures baked from palettes, which map each
	// material to its display properties. One of the channels that carries
	// mesh UVWs to the shader stores them with 8 bits per coordinate, so
	// each material's sample point must lie exactly on that 8-bit grid (a
	// multiple of 1/255). Those points are texel boundaries, so the
	// material's properties fill both texels flanking its sample point,
	// making the linear filter's blend return them exactly.
	static constexpr int TEXTURE_WIDTH = 510;

	// The texture coordinates at which meshes sample the given material.
	static Vector3 get_material_uvw(const VoxelMaterial4D p_material) {
		return Vector3((real_t)(int)p_material / (real_t)(TEXTURE_WIDTH / 2), 0.5f, 0.5f);
	}

	// Bakes the palette into the texture that colors meshes. Every texel is
	// written, so materials without entries, and every material of a null
	// palette, sample the fallback color.
	static Ref<Texture3D> bake_texture(const Ref<VoxelMaterialPalette4D> &p_palette);

	PackedColorArray get_colors() const { return _colors; }
	void set_colors(const PackedColorArray &p_colors);

	// The color of the given material; white where the palette has no entry.
	Color get_material_color(const VoxelMaterial4D p_material) const;
	Color get_material_color_bind(const int p_material) const { return get_material_color((VoxelMaterial4D)p_material); }
};
