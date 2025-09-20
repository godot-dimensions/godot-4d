#pragma once

#include "../material_4d.h"

#if GODOT_MODULE
#include "scene/resources/texture.h"
#elif GDEXTENSION
#include <godot_cpp/classes/texture3d.hpp>
#endif

class TetraMaterial4D : public Material4D {
	GDCLASS(TetraMaterial4D, Material4D);

public:
	enum TetraColorSource {
		TETRA_COLOR_SOURCE_SINGLE_COLOR,
		TETRA_COLOR_SOURCE_PER_VERT_ONLY,
		TETRA_COLOR_SOURCE_PER_CELL_ONLY,
		TETRA_COLOR_SOURCE_TEXTURE3D_CELL_UVW_ONLY,
		TETRA_COLOR_SOURCE_TEXTURE4D_DIRECT_ONLY,
		TETRA_COLOR_SOURCE_PER_VERT_AND_SINGLE,
		TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE,
		TETRA_COLOR_SOURCE_TEXTURE3D_CELL_UVW_AND_SINGLE,
		TETRA_COLOR_SOURCE_TEXTURE4D_DIRECT_AND_SINGLE,
	};

private:
	TetraColorSource _albedo_source = TETRA_COLOR_SOURCE_SINGLE_COLOR;
	Ref<Texture3D> _albedo_texture_3d;
	static Ref<Shader> _cross_section_shader;

	static Material4D::ColorSourceFlags _tetra_source_to_flags(const TetraColorSource p_tetra_source);

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

	void update_cross_section_material() override;

public:
	virtual Color get_albedo_color_of_edge(const int64_t p_edge_index, const Ref<Mesh4D> &p_for_mesh) override;
	virtual void merge_with(const Ref<Material4D> &p_material, const int p_first_item_count, const int p_second_item_count) override;

	TetraColorSource get_albedo_source() const;
	void set_albedo_source(const TetraColorSource p_albedo_source);

	Ref<Texture3D> get_albedo_texture_3d() const;
	void set_albedo_texture_3d(const Ref<Texture3D> &p_albedo_texture_3d);

	static void init_shaders();
	static void cleanup_shaders();

	TetraMaterial4D();
};

VARIANT_ENUM_CAST(TetraMaterial4D::TetraColorSource);
