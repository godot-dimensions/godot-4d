#include "simplex_noise_4d.inc.glsl"

// Supplied by EnvironmentRenderBridge4DTo3D. These transform camera-local 4D rays
// into the WorldEnvironment4D node's local coordinate system, where the cloud field lives.
uniform vec4 cloud_camera_position_4d = vec4(0.0);
uniform mat4 cloud_camera_basis_4d = mat4(1.0);
uniform vec2 cloud_evolution_cos_sin = vec2(1.0, 0.0);
uniform vec4 cloud_shape_wind_offset_4d = vec4(0.0);
uniform vec4 cloud_shape_octave_2_wind_offset_4d = vec4(0.0);
uniform vec4 cloud_shape_octave_3_wind_offset_4d = vec4(0.0);
uniform vec4 cloud_detail_wind_offset_4d = vec4(0.0);

uniform vec4 cloud_albedo_color : source_color = vec4(1.0);
uniform vec4 cloud_ambient_color : source_color = vec4(0.42, 0.48, 0.58, 1.0);
uniform float cloud_ambient_intensity : hint_range(0, 4) = 0.45;
uniform float cloud_sun_intensity : hint_range(0, 4) = 1.0;
uniform float cloud_phase_anisotropy : hint_range(-0.95, 0.95) = 0.55;

uniform float cloud_bottom_height : hint_range(-100000, 100000, 1) = 1000.0;
uniform float cloud_vertical_thickness : hint_range(1, 100000, 1) = 2500.0;
uniform float cloud_shape_scale : hint_range(1, 100000, 1) = 1800.0;
uniform float cloud_shape_fractal_strength : hint_range(0, 1) = 0.5;
uniform float cloud_detail_scale : hint_range(1, 100000, 1) = 350.0;
uniform float cloud_detail_strength : hint_range(0, 1) = 0.35;
uniform float cloud_coverage : hint_range(0, 1) = 0.5;
uniform float cloud_density : hint_range(0, 8) = 1.0;

uniform float cloud_extinction_coefficient : hint_range(0, 0.1, 0.00001) = 0.002;
uniform float cloud_shadow_distance : hint_range(0, 100000, 1) = 600.0;
uniform float cloud_shadow_strength : hint_range(0, 8) = 1.0;
uniform float cloud_maximum_ray_distance : hint_range(1, 1000000, 1) = 20000.0;
uniform float cloud_distance_fade_start : hint_range(0, 0.99) = 0.0;
uniform float cloud_inv_distance_fade_curve = 2.4;
uniform int cloud_sampling_method = 1;
uniform int cloud_sampling_steps : hint_range(4, 128) = 64;
uniform bool cloud_affect_radiance = true;

highp vec4 evolve_cloud_position(highp vec4 position_4d) {
	highp float evolution_cos = cloud_evolution_cos_sin.x;
	highp float evolution_sin = cloud_evolution_cos_sin.y;
	return vec4(
			position_4d.x * evolution_cos - position_4d.w * evolution_sin,
			position_4d.y * evolution_cos - position_4d.z * evolution_sin,
			position_4d.z * evolution_cos + position_4d.y * evolution_sin,
			position_4d.w * evolution_cos + position_4d.x * evolution_sin);
}

highp vec4 get_cloud_noise_coordinates(highp vec4 evolved_position_4d, highp float scale, highp vec4 wind_offset_4d) {
	highp float safe_scale = max(scale, 0.0001);
	return evolved_position_4d / safe_scale - wind_offset_4d;
}

float cloud_height_profile(highp float height_fraction) {
	float bottom_fade = smoothstep(0.0, 0.12, height_fraction);
	float top_fade = 1.0 - smoothstep(0.55, 1.0, height_fraction);
	return bottom_fade * top_fade;
}

highp float sample_cloud_shape_density(highp vec4 position_4d, vec4 octave_weights) {
	if (cloud_coverage <= 0.0 || cloud_density <= 0.0) {
		return 0.0;
	}
	highp vec4 evolved_position_4d = evolve_cloud_position(position_4d);
	highp vec4 shape_coordinates = get_cloud_noise_coordinates(evolved_position_4d, cloud_shape_scale, cloud_shape_wind_offset_4d);
	highp float base_shape_noise = 0.5;
	if (octave_weights.x > 0.0) {
		base_shape_noise = simplex_4d_noise(shape_coordinates) * 0.5 + 0.5;
	}
	// Unresolved procedural noise is filtered toward its mean rather than sampled
	// stochastically. This makes low raymarch budgets lose contrast gracefully instead
	// of turning into grain or coherent rings.
	highp float shape_noise_sum = 0.5 + (base_shape_noise - 0.5) * octave_weights.x;
	float shape_noise_weight = 1.0;
	float fractal_strength = cloud_shape_fractal_strength;
	if (fractal_strength > 0.0) {
		const float octave_2_lacunarity = 2.03;
		highp float octave_2_noise = 0.5;
		if (octave_weights.y > 0.0) {
			highp vec4 octave_2_coordinates = get_cloud_noise_coordinates(evolved_position_4d, cloud_shape_scale / octave_2_lacunarity, cloud_shape_octave_2_wind_offset_4d) + vec4(13.7, 5.2, 17.1, 9.3);
			octave_2_noise = simplex_4d_noise(octave_2_coordinates) * 0.5 + 0.5;
		}
		shape_noise_sum += (0.5 + (octave_2_noise - 0.5) * octave_weights.y) * fractal_strength;
		shape_noise_weight += fractal_strength;

		const float octave_3_lacunarity = octave_2_lacunarity * 2.01;
		float octave_3_strength = fractal_strength * fractal_strength;
		highp float octave_3_noise = 0.5;
		if (octave_weights.z > 0.0) {
			highp vec4 octave_3_coordinates = get_cloud_noise_coordinates(evolved_position_4d, cloud_shape_scale / octave_3_lacunarity, cloud_shape_octave_3_wind_offset_4d) + vec4(4.6, 23.8, 2.4, 31.7);
			octave_3_noise = simplex_4d_noise(octave_3_coordinates) * 0.5 + 0.5;
		}
		shape_noise_sum += (0.5 + (octave_3_noise - 0.5) * octave_weights.z) * octave_3_strength;
		shape_noise_weight += octave_3_strength;
	}
	highp float shape_noise = shape_noise_sum / shape_noise_weight;
	float coverage_threshold = 1.0 - cloud_coverage;
	float shaped_density = smoothstep(coverage_threshold, coverage_threshold + 0.15, shape_noise);
	if (shaped_density > 0.0 && cloud_detail_strength > 0.0) {
		highp float detail_noise = 0.5;
		if (octave_weights.w > 0.0) {
			highp vec4 detail_coordinates = get_cloud_noise_coordinates(evolved_position_4d, cloud_detail_scale, cloud_detail_wind_offset_4d) + vec4(19.1, 7.7, 3.4, 11.8);
			highp float sampled_detail_noise = simplex_4d_noise(detail_coordinates) * 0.5 + 0.5;
			detail_noise += (sampled_detail_noise - 0.5) * octave_weights.w;
		}
		// When detail is too fine for the sampling interval, retain its mean erosion.
		// Omitting it entirely makes low-quality clouds systematically denser and turns
		// the interior into an opaque, uniform wall.
		shaped_density = max(0.0, shaped_density - (1.0 - detail_noise) * cloud_detail_strength);
	}
	return shaped_density * cloud_density;
}

highp float sample_cloud_density(highp vec4 position_4d, vec4 octave_weights) {
	highp float height_fraction = (position_4d.y - cloud_bottom_height) / max(cloud_vertical_thickness, 0.0001);
	if (height_fraction <= 0.0 || height_fraction >= 1.0) {
		return 0.0;
	}
	return sample_cloud_shape_density(position_4d, octave_weights) * cloud_height_profile(height_fraction);
}

bool intersect_cloud_layer(highp vec4 ray_origin_4d, highp vec4 ray_direction_4d, out highp float ray_start, out highp float ray_end) {
	highp float layer_top = cloud_bottom_height + cloud_vertical_thickness;
	if (abs(ray_direction_4d.y) < 0.000001) {
		if (ray_origin_4d.y <= cloud_bottom_height || ray_origin_4d.y >= layer_top) {
			return false;
		}
		ray_start = 0.0;
		ray_end = cloud_maximum_ray_distance;
		return true;
	}
	highp float bottom_distance = (cloud_bottom_height - ray_origin_4d.y) / ray_direction_4d.y;
	highp float top_distance = (layer_top - ray_origin_4d.y) / ray_direction_4d.y;
	ray_start = max(0.0, min(bottom_distance, top_distance));
	ray_end = min(cloud_maximum_ray_distance, max(bottom_distance, top_distance));
	return ray_end > ray_start;
}

float cloud_phase_4d(float cos_theta) {
	float g = clamp(cloud_phase_anisotropy, -0.95, 0.95);
	float denominator = max(1.0 + g * g - 2.0 * g * cos_theta, 0.01);
	// In four spatial dimensions, directions lie on S3. The squared denominator is the
	// 4D counterpart to the 3D Henyey-Greenstein phase function's 3/2 exponent.
	return min((1.0 - g * g) / (denominator * denominator), 8.0);
}

highp float cloud_ray_jitter(highp vec2 fragment_coordinate) {
	return fract(sin(dot(fragment_coordinate, vec2(12.9898, 78.233))) * 43758.5453);
}

float cloud_distance_fade(highp float ray_distance) {
	float fade_start_distance = cloud_maximum_ray_distance * cloud_distance_fade_start;
	if (ray_distance <= fade_start_distance) {
		return 1.0;
	}
	float fade_progress = clamp((ray_distance - fade_start_distance) / max(cloud_maximum_ray_distance - fade_start_distance, 0.0001), 0.0, 1.0);
	// This is an exponential extinction envelope whose progress is shaped like the
	// easing curves in GradientSkyMaterial4D. It is effectively transparent at the cutoff.
	return exp(-8.0 * pow(fade_progress, max(cloud_inv_distance_fade_curve, 0.0001)));
}
