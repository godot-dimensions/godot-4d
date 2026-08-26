#pragma once

#include "../../nodes/node_4d.h"
#include "../../render/environment/sky/sky_material_4d.h"
#include "cloud/volumetric_cloud_material_4d.h"

class WorldEnvironment4D : public Node4D {
	GDCLASS(WorldEnvironment4D, Node4D);

public:
	enum ToneMapper : uint8_t {
		TONE_MAPPER_LINEAR,
		TONE_MAPPER_REINHARDT,
		TONE_MAPPER_FILMIC,
		TONE_MAPPER_ACES,
		TONE_MAPPER_AGX,
	};

private:
	Ref<SkyMaterial4D> _sky_material;
	Ref<VolumetricCloudMaterial4D> _cloud_material;
	Color _ambient_light_color = Color(0.0, 0.0, 0.0, 1.0);
	float _ambient_light_sky_contribution = 1.0f;
	float _tonemap_exposure = 1.0f;
	float _tonemap_white = 1.0f;
	ToneMapper _tone_mapper = TONE_MAPPER_LINEAR;

	bool _is_current = false;
	bool _is_registered_with_rendering_server = false;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;
	void _notification(int p_what);

public:
	bool is_current() const;
	void set_current(const bool p_enabled);
	void clear_current(const bool p_enable_next = true);
	void make_current();

	Ref<SkyMaterial4D> get_sky_material() const { return _sky_material; }
	void set_sky_material(const Ref<SkyMaterial4D> &p_sky_material) { _sky_material = p_sky_material; }
	Ref<VolumetricCloudMaterial4D> get_cloud_material() const { return _cloud_material; }
	void set_cloud_material(const Ref<VolumetricCloudMaterial4D> &p_cloud_material) { _cloud_material = p_cloud_material; }

	Color get_ambient_light_color() const { return _ambient_light_color; }
	void set_ambient_light_color(const Color &p_ambient_light_color) { _ambient_light_color = p_ambient_light_color; }

	float get_ambient_light_sky_contribution() const { return _ambient_light_sky_contribution; }
	void set_ambient_light_sky_contribution(const float p_ambient_light_sky_contribution);

	float get_tonemap_exposure() const { return _tonemap_exposure; }
	void set_tonemap_exposure(const float p_exposure) { _tonemap_exposure = p_exposure; }

	float get_tonemap_white() const { return _tonemap_white; }
	void set_tonemap_white(const float p_white) { _tonemap_white = p_white; }

	ToneMapper get_tonemapper() const { return _tone_mapper; }
	void set_tonemapper(const ToneMapper p_tone_mapper);
};

VARIANT_ENUM_CAST(WorldEnvironment4D::ToneMapper);
