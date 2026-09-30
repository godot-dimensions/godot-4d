#include "voxel_material_palette.h"

#if GDEXTENSION
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#include "scene/resources/image_texture.h"
#include "servers/rendering_server.h"
#endif

Ref<Texture3D> VoxelMaterialPalette::bake_texture(const Ref<VoxelMaterialPalette> &p_palette) {
	// Without a rendering server, as when running tests, there is nothing to
	// hold the texture.
	if (RenderingServer::get_singleton() == nullptr) {
		return Ref<Texture3D>();
	}
	Ref<Image> image = Image::create_empty(TEXTURE_WIDTH, 1, false, Image::FORMAT_RGBA8);
	for (int i = 0; i < TEXTURE_WIDTH; i++) {
		// Texels 2m - 1 and 2m flank material m's sample point, with the
		// last texel wrapping around below material 0's.
		const int material = ((i + 1) / 2) % (TEXTURE_WIDTH / 2);
		const Color color = p_palette.is_valid() ? p_palette->get_material_color((VoxelMaterial)material) : Color(1.0f, 1.0f, 1.0f);
		image->set_pixel(i, 0, color);
	}
	Ref<ImageTexture3D> texture;
	texture.instantiate();
#if GDEXTENSION
	TypedArray<Image> images;
	images.append(image);
#elif GODOT_MODULE
	Vector<Ref<Image>> images;
	images.push_back(image);
#endif
	texture->create(Image::FORMAT_RGBA8, TEXTURE_WIDTH, 1, 1, false, images);
	return texture;
}

void VoxelMaterialPalette::set_colors(const PackedColorArray &p_colors) {
	_colors = p_colors;
	emit_changed();
}

Color VoxelMaterialPalette::get_material_color(const VoxelMaterial p_material) const {
	const int index = (int)p_material;
	if (index >= _colors.size()) {
		return Color(1.0f, 1.0f, 1.0f);
	}
	return _colors[index];
}

void VoxelMaterialPalette::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_colors"), &VoxelMaterialPalette::get_colors);
	ClassDB::bind_method(D_METHOD("set_colors", "colors"), &VoxelMaterialPalette::set_colors);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "colors"), "set_colors", "get_colors");

	ClassDB::bind_method(D_METHOD("get_material_color", "material"), &VoxelMaterialPalette::get_material_color_bind);
}
