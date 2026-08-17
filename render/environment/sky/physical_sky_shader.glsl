// AI-generated. Adapted from Godot's PhysicalSkyMaterial shader to evaluate directions in 4D.
shader_type sky;
render_mode use_debanding;

uniform vec4 world_up_direction_4d = vec4(0.0, 1.0, 0.0, 0.0);

uniform vec4 ground_color : source_color = vec4(0.1, 0.07, 0.034, 1.0);
uniform vec4 mie_color : source_color = vec4(0.69, 0.729, 0.812, 1.0);
uniform vec4 rayleigh_color : source_color = vec4(0.3, 0.405, 0.6, 1.0);

uniform float energy_multiplier : hint_range(0, 128) = 1.0;
uniform float sun_glow_intensity : hint_range(0, 16) = 0.1;
uniform float sun_glow_half_width_radians : hint_range(0, 3.14159265359) = 0.05;
uniform float mie_anisotropy : hint_range(-1, 1) = 0.8;
uniform float mie_coefficient : hint_range(0, 0.001, 0.000001) = 0.000005;
uniform float rayleigh_coefficient : hint_range(0, 0.00064, 0.0000001) = 0.00002;

uniform vec4 light0_direction_4d = vec4(0.0, 0.0, 1.0, 0.0);
uniform vec4 light0_color : source_color = vec4(1.0);
uniform float light0_energy = 1.0;
uniform float light0_angular_radius_radians = 0.0;
uniform bool light0_enabled = false;

uniform vec4 light1_direction_4d = vec4(0.0, 0.0, 1.0, 0.0);
uniform vec4 light1_color : source_color = vec4(1.0);
uniform float light1_energy = 1.0;
uniform float light1_angular_radius_radians = 0.0;
uniform bool light1_enabled = false;

uniform vec4 light2_direction_4d = vec4(0.0, 0.0, 1.0, 0.0);
uniform vec4 light2_color : source_color = vec4(1.0);
uniform float light2_energy = 1.0;
uniform float light2_angular_radius_radians = 0.0;
uniform bool light2_enabled = false;

uniform vec4 light3_direction_4d = vec4(0.0, 0.0, 1.0, 0.0);
uniform vec4 light3_color : source_color = vec4(1.0);
uniform float light3_energy = 1.0;
uniform float light3_angular_radius_radians = 0.0;
uniform bool light3_enabled = false;

// Optical length at zenith for molecules.
const float RAYLEIGH_ZENITH_SIZE = 8.4e3;
const float MIE_ZENITH_SIZE = 1.25e3;

float henyey_greenstein(float cos_theta, float g) {
	const float k = 0.0795774715459;
	return k * (1.0 - g * g) / pow(1.0 + g * g - 2.0 * g * cos_theta, 1.5);
}

vec4 evaluate_sun(vec4 eye_direction_4d, bool enabled, vec4 light_direction_4d, vec3 light_color, float light_energy, float light_angular_radius_radians, float rayleigh_scatter, float mie_scatter, bool draw_sun_glow) {
	if (!enabled) {
		return vec4(0.0);
	}

	float zenith_angle = clamp(dot(world_up_direction_4d, light_direction_4d), -1.0, 1.0);
	float sun_energy = max(0.0, 0.757 * zenith_angle) * light_energy;
	float sun_below_horizon_factor = clamp(1.0 - exp(zenith_angle), 0.0, 1.0);

	// Rayleigh coefficients.
	// Convert from inverse meters to the scale used by Godot's physical sky calculation.
	float scaled_rayleigh_coefficient = rayleigh_coefficient * 10.0 - sun_below_horizon_factor * 0.0001;
	vec3 rayleigh_beta = scaled_rayleigh_coefficient * rayleigh_color.rgb;
	// Mie coefficients from Preetham.
	vec3 mie_beta = mie_coefficient * 0.434 * mie_color.rgb;

	// Light extinction based on thickness of atmosphere.
	vec3 extinction = exp(-(rayleigh_beta * rayleigh_scatter + mie_beta * mie_scatter));

	// In-scattering.
	float cos_theta = clamp(dot(eye_direction_4d, light_direction_4d), -1.0, 1.0);
	float rayleigh_phase = (3.0 / (16.0 * PI)) * (1.0 + pow(cos_theta * 0.5 + 0.5, 2.0));
	vec3 beta_r_theta = rayleigh_beta * rayleigh_phase;
	float mie_phase = henyey_greenstein(cos_theta, mie_anisotropy);
	vec3 beta_m_theta = mie_beta * mie_phase;

	vec3 scattering = sun_energy * ((beta_r_theta + beta_m_theta) / max(rayleigh_beta + mie_beta, vec3(0.000000001)));
	vec3 lin = pow(max(scattering * (1.0 - extinction), vec3(0.0)), vec3(1.5));
	// Hack from https://github.com/mrdoob/three.js/blob/master/examples/jsm/objects/Sky.js
	lin *= mix(vec3(1.0), pow(max(scattering * extinction, vec3(0.0)), vec3(0.5)), clamp(pow(1.0 - zenith_angle, 5.0), 0.0, 1.0));

	// Hack in the ground color.
	lin *= mix(ground_color.rgb, vec3(1.0), smoothstep(-0.1, 0.1, dot(world_up_direction_4d, eye_direction_4d)));

	// Solar disk and out-scattering.
	float sun_disk_radius = max(light_angular_radius_radians, 0.000001);
	float sun_disk = smoothstep(cos(sun_disk_radius), cos(sun_disk_radius * 0.5), cos_theta);
	float sun_shape = sun_disk;
	if (draw_sun_glow && sun_glow_intensity > 0.0 && sun_glow_half_width_radians > 0.0) {
		// Keep this optical glow out of radiance cubemaps so it does not affect ambient lighting or reflections.
		float angular_distance_outside_disk = max(acos(cos_theta) - sun_disk_radius, 0.0);
		float normalized_glow_distance = angular_distance_outside_disk / sun_glow_half_width_radians;
		float sun_glow = sun_glow_intensity * exp2(-normalized_glow_distance * normalized_glow_distance);
		sun_shape = max(sun_shape, sun_glow);
	}
	vec3 sun_disk_and_glow = (sun_energy * extinction) * sun_shape * light_color;

	float sun_fade = 1.0 - sun_below_horizon_factor;
	return vec4(lin + sun_disk_and_glow, sun_fade);
}

void sky() {
	vec4 eye_direction_4d = vec4(EYEDIR, 0.0);

	// Optical length.
	float zenith = max(0.0, dot(world_up_direction_4d, eye_direction_4d));
	float optical_mass = 1.0 / (zenith + 0.15 * pow(3.885 + 54.5 * zenith, -1.253));
	float rayleigh_scatter = RAYLEIGH_ZENITH_SIZE * optical_mass;
	float mie_scatter = MIE_ZENITH_SIZE * optical_mass;

	bool draw_sun_glow = !AT_CUBEMAP_PASS;
	vec4 sun0 = evaluate_sun(eye_direction_4d, light0_enabled, light0_direction_4d, light0_color.rgb, light0_energy, light0_angular_radius_radians, rayleigh_scatter, mie_scatter, draw_sun_glow);
	vec4 sun1 = evaluate_sun(eye_direction_4d, light1_enabled, light1_direction_4d, light1_color.rgb, light1_energy, light1_angular_radius_radians, rayleigh_scatter, mie_scatter, draw_sun_glow);
	vec4 sun2 = evaluate_sun(eye_direction_4d, light2_enabled, light2_direction_4d, light2_color.rgb, light2_energy, light2_angular_radius_radians, rayleigh_scatter, mie_scatter, draw_sun_glow);
	vec4 sun3 = evaluate_sun(eye_direction_4d, light3_enabled, light3_direction_4d, light3_color.rgb, light3_energy, light3_angular_radius_radians, rayleigh_scatter, mie_scatter, draw_sun_glow);

	vec3 color = sun0.rgb + sun1.rgb + sun2.rgb + sun3.rgb;
	float sun_fade = max(max(sun0.a, sun1.a), max(sun2.a, sun3.a));
	COLOR = pow(max(color, vec3(0.0)), vec3(1.0 / (1.2 + 1.2 * sun_fade))) * energy_multiplier;
}
