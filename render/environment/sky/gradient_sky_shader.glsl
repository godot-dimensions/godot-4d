// Adapted from Godot's ProceduralSkyMaterial shader to evaluate directions in 4D.
shader_type sky;
render_mode use_debanding;

uniform vec4 world_up_direction_4d = vec4(0.0, 1.0, 0.0, 0.0);

// Keep these values in sync with `GradientSkyMaterial4D`,
// `EditorPreviewEnvironment4D`, and Godot's 3D `ProceduralSkyMaterial`.
uniform vec4 horizon_color : source_color = vec4(0.646, 0.656, 0.67, 1.0);
uniform vec4 top_color : source_color = vec4(0.385, 0.454, 0.55, 1.0);
uniform vec4 bottom_color : source_color = vec4(0.2, 0.169, 0.133, 1.0);

uniform float inv_top_curve : hint_range(1, 100) = 4.0;
uniform float inv_bottom_curve : hint_range(1, 100) = 30.0;
uniform float sun_angle_max = 0.877;
uniform float inv_sun_curve : hint_range(1, 100) = 22.78;
uniform float energy_multiplier : hint_range(0, 128) = 1.0;

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

vec3 apply_sun(vec3 sky, vec4 eye_direction_4d, bool enabled, vec4 light_direction_4d, vec3 light_color, float light_energy, float light_angular_radius_radians) {
	if (!enabled) {
		return sky;
	}
	// Note: "light_direction_4d" is not the direction the light rays travel in, it is the
	// direction towards the light source, and the direction to the visible sun in the sky.
	float sun_angle = dot(light_direction_4d, eye_direction_4d);
	float sun_angular_radius_radians = cos(light_angular_radius_radians);
	if (sun_angle > sun_angular_radius_radians) {
		return light_color * light_energy;
	}
	if (sun_angle > sun_angle_max) {
		float c2 = (sun_angular_radius_radians - sun_angle) / (sun_angular_radius_radians - sun_angle_max);
		return mix(sky, light_color * light_energy, clamp(pow(1.0 - c2, inv_sun_curve), 0.0, 1.0));
	}
	return sky;
}

void sky() {
	vec4 eye_direction_4d = vec4(EYEDIR, 0.0);
	float v_angle = clamp(dot(world_up_direction_4d, eye_direction_4d), -1.0, 1.0);
	vec3 sky_color = mix(top_color.rgb, horizon_color.rgb, clamp(pow(1.0 - v_angle, inv_top_curve), 0.0, 1.0));

	sky_color = apply_sun(sky_color, eye_direction_4d, light0_enabled, light0_direction_4d, light0_color.rgb, light0_energy, light0_angular_radius_radians);
	sky_color = apply_sun(sky_color, eye_direction_4d, light1_enabled, light1_direction_4d, light1_color.rgb, light1_energy, light1_angular_radius_radians);
	sky_color = apply_sun(sky_color, eye_direction_4d, light2_enabled, light2_direction_4d, light2_color.rgb, light2_energy, light2_angular_radius_radians);
	sky_color = apply_sun(sky_color, eye_direction_4d, light3_enabled, light3_direction_4d, light3_color.rgb, light3_energy, light3_angular_radius_radians);

	vec3 ground = mix(bottom_color.rgb, horizon_color.rgb, clamp(pow(1.0 + v_angle, inv_bottom_curve), 0.0, 1.0));
	COLOR = mix(ground, sky_color, step(0.0, v_angle)) * energy_multiplier;
}
