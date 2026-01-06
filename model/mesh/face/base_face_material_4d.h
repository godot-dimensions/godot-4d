#pragma once

#include "../material_4d.h"

#if GODOT_MODULE
#include "scene/resources/texture.h"
#elif GDEXTENSION
#include <godot_cpp/classes/texture3d.hpp>
#endif

class BaseFaceMaterial4D : public Material4D {
	GDCLASS(BaseFaceMaterial4D, Material4D); 

private:
	Color _w_negative_fade_rgbu;
	Color _w_positive_fade_rgbu;
	float _w_fade_distance;
	Color _base_color_rgba;
	float _base_color_u;
	float _tetrachromacy_amount;
	static Ref<Shader> _cross_section_shader;

protected:
	static void _bind_methods();

	virtual void update_cross_section_material() override;

public:
	//virtual void merge_with(const Ref<Material4D> &p_material, const int p_first_item_count, const int p_second_item_count) override;

	Color get_w_negative_fade_rgbu();
	void set_w_negative_fade_rgbu(Color value);
	Color get_w_positive_fade_rgbu();
	void set_w_positive_fade_rgbu(Color value);
	float get_w_fade_distance();
	void set_w_fade_distance(float value);
	Color get_base_color_rgba();
	void set_base_color_rgba(Color value);
	float get_base_color_u();
	void set_base_color_u(float value);
	float get_tetrachromacy_amount();
	void set_tetrachromacy_amount(float value);

	static void init_shaders();
	static void cleanup_shaders();

	//BaseFaceMaterial4D();
};
