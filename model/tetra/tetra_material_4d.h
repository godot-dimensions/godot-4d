#pragma once

#include "../material_4d.h"

class TetraMaterial4D : public Material4D {
	GDCLASS(TetraMaterial4D, Material4D);

public:
	enum TetraColorSource {
		TETRA_COLOR_SOURCE_SINGLE_COLOR,
		TETRA_COLOR_SOURCE_PER_VERT_ONLY,
		TETRA_COLOR_SOURCE_PER_CELL_ONLY,
		TETRA_COLOR_SOURCE_CELL_UVW_ONLY,
		TETRA_COLOR_SOURCE_TEXTURE4D_ONLY,
		TETRA_COLOR_SOURCE_PER_VERT_AND_SINGLE,
		TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE,
		TETRA_COLOR_SOURCE_CELL_UVW_AND_SINGLE,
		TETRA_COLOR_SOURCE_TEXTURE4D_AND_SINGLE,
	};

private:
	TetraColorSource _albedo_source = TETRA_COLOR_SOURCE_SINGLE_COLOR;

protected:
	static void _bind_methods();
	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	virtual void merge_with(const Ref<Material4D> &p_material, const int p_first_item_count, const int p_second_item_count) override;

	TetraColorSource get_albedo_source() const;
	void set_albedo_source(const TetraColorSource p_albedo_source);

	TetraMaterial4D();
};

VARIANT_ENUM_CAST(TetraMaterial4D::TetraColorSource);
