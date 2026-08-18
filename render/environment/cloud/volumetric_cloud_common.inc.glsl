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
		base_shape_noise = simplex_noise_4d(shape_coordinates) * 0.5 + 0.5;
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
			octave_2_noise = simplex_noise_4d(octave_2_coordinates) * 0.5 + 0.5;
		}
		shape_noise_sum += (0.5 + (octave_2_noise - 0.5) * octave_weights.y) * fractal_strength;
		shape_noise_weight += fractal_strength;

		const float octave_3_lacunarity = octave_2_lacunarity * 2.01;
		float octave_3_strength = fractal_strength * fractal_strength;
		highp float octave_3_noise = 0.5;
		if (octave_weights.z > 0.0) {
			highp vec4 octave_3_coordinates = get_cloud_noise_coordinates(evolved_position_4d, cloud_shape_scale / octave_3_lacunarity, cloud_shape_octave_3_wind_offset_4d) + vec4(4.6, 23.8, 2.4, 31.7);
			octave_3_noise = simplex_noise_4d(octave_3_coordinates) * 0.5 + 0.5;
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
			highp float sampled_detail_noise = simplex_noise_4d(detail_coordinates) * 0.5 + 0.5;
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

void get_primary_cloud_light(out bool enabled, out highp vec4 direction_4d, out vec3 radiance) {
	enabled = false;
	direction_4d = vec4(0.0, 1.0, 0.0, 0.0);
	radiance = vec3(0.0);
	if (light0_enabled) {
		enabled = true;
		direction_4d = normalize(cloud_camera_basis_4d * light0_direction_4d);
		radiance = light0_color.rgb * light0_energy;
	} else if (light1_enabled) {
		enabled = true;
		direction_4d = normalize(cloud_camera_basis_4d * light1_direction_4d);
		radiance = light1_color.rgb * light1_energy;
	} else if (light2_enabled) {
		enabled = true;
		direction_4d = normalize(cloud_camera_basis_4d * light2_direction_4d);
		radiance = light2_color.rgb * light2_energy;
	} else if (light3_enabled) {
		enabled = true;
		direction_4d = normalize(cloud_camera_basis_4d * light3_direction_4d);
		radiance = light3_color.rgb * light3_energy;
	}
}

vec4 get_cloud_world_grid_octave_weights(int step_count, highp float sample_interval) {
	const float octave_2_lacunarity = 2.03;
	const float octave_3_lacunarity = octave_2_lacunarity * 2.01;
	// Spend additional quality on shape detail as the user raises the sampling budget.
	// The transitions depend only on that budget, so camera motion and view angle cannot
	// move level-of-detail boundaries through the sky.
	vec4 quality_weights = vec4(
			1.0,
			smoothstep(4.0, 12.0, float(step_count)),
			smoothstep(8.0, 24.0, float(step_count)),
			smoothstep(16.0, 40.0, float(step_count)));
	vec4 resolution_weights = vec4(
			1.0 - smoothstep(cloud_shape_scale, cloud_shape_scale * 2.0, sample_interval),
			1.0 - smoothstep(cloud_shape_scale / octave_2_lacunarity, cloud_shape_scale * 2.0 / octave_2_lacunarity, sample_interval),
			1.0 - smoothstep(cloud_shape_scale / octave_3_lacunarity, cloud_shape_scale * 2.0 / octave_3_lacunarity, sample_interval),
			1.0 - smoothstep(cloud_detail_scale, cloud_detail_scale * 2.0, sample_interval));
	return quality_weights * resolution_weights;
}

highp float next_cloud_grid_crossing(highp float position, highp float direction, highp float ray_start, highp float grid_spacing) {
	if (abs(direction) <= 0.000001) {
		return 1e30;
	}
	highp float grid_coordinate = position / grid_spacing;
	highp float next_plane_index = direction > 0.0 ? floor(grid_coordinate) + 1.0 : ceil(grid_coordinate) - 1.0;
	return ray_start + (next_plane_index * grid_spacing - position) / direction;
}

vec3 render_cloud_world_grid(highp vec4 ray_direction_4d, highp float ray_start, highp float ray_end, int step_count, bool primary_light_enabled, highp vec4 cloud_shadow_offset_4d, float cloud_shadow_extinction, float primary_light_phase) {
	// Samples are bounded by crossings of four world-aligned plane families, so camera
	// motion translates through one stationary sampling lattice instead of dragging a
	// ray-local pattern.
	highp float grid_spacing = cloud_maximum_ray_distance / float(step_count);
	vec4 octave_weights = get_cloud_world_grid_octave_weights(step_count, grid_spacing);
	highp vec4 start_position_4d = cloud_camera_position_4d + ray_direction_4d * ray_start;
	highp vec4 next_crossings = vec4(
			next_cloud_grid_crossing(start_position_4d.x, ray_direction_4d.x, ray_start, grid_spacing),
			next_cloud_grid_crossing(start_position_4d.y, ray_direction_4d.y, ray_start, grid_spacing),
			next_cloud_grid_crossing(start_position_4d.z, ray_direction_4d.z, ray_start, grid_spacing),
			next_cloud_grid_crossing(start_position_4d.w, ray_direction_4d.w, ray_start, grid_spacing));
	highp vec4 crossing_intervals = grid_spacing / max(abs(ray_direction_4d), vec4(0.000001));
	highp float segment_start = ray_start;
	highp float segment_start_density = sample_cloud_density(start_position_4d, octave_weights) * cloud_distance_fade(segment_start);
	float transmittance = 1.0;
	float direct_scattering = 0.0;
	// A normalized 4D direction has an L1 norm of at most 2, so it can cross about
	// twice the nominal number of grid planes. The additional 4 iterations cover
	// boundary rounding between the four plane families and the final segment:
	// 2 * maximum sampling_steps + 4 = 2 * 128 + 4 = 260.
	for (int step_index = 0; step_index < 260; step_index++) {
		if (segment_start >= ray_end || transmittance <= 0.01) {
			break;
		}
		highp float next_crossing = min(min(next_crossings.x, next_crossings.y), min(next_crossings.z, next_crossings.w));
		highp float segment_end = min(next_crossing, ray_end);
		highp vec4 segment_end_position_4d = cloud_camera_position_4d + ray_direction_4d * segment_end;
		highp float segment_end_density = sample_cloud_density(segment_end_position_4d, octave_weights) * cloud_distance_fade(segment_end);
		// Reuse every crossing density as the endpoint of both adjacent segments. This
		// reconstructs density linearly through each grid cell instead of assigning the
		// entire cell one midpoint value, softening discontinuities when the nearest plane
		// family changes.
		highp float segment_density = 0.5 * (segment_start_density + segment_end_density);
		highp float segment_length = segment_end - segment_start;
		if (segment_density > 0.0001 && segment_length > 0.0001) {
			float optical_depth = segment_density * cloud_extinction_coefficient * segment_length;
			float sample_transmittance = exp(-optical_depth);
			float sample_opacity = 1.0 - sample_transmittance;
			float scattering_weight = transmittance * sample_opacity;
			if (primary_light_enabled) {
				float light_transmittance = 1.0;
				if (cloud_shadow_extinction > 0.0) {
					highp float ray_distance = 0.5 * (segment_start + segment_end);
					highp vec4 shadow_position_4d = cloud_camera_position_4d + ray_direction_4d * ray_distance + cloud_shadow_offset_4d;
					highp float shadow_density = sample_cloud_density(shadow_position_4d, vec4(octave_weights.x, 0.0, 0.0, 0.0));
					light_transmittance = exp(-shadow_density * cloud_shadow_extinction);
				}
				direct_scattering += scattering_weight * light_transmittance * primary_light_phase;
			}
			transmittance *= sample_transmittance;
		}
		if (segment_end >= ray_end) {
			break;
		}
		// Advance every coincident plane crossing together to avoid zero-length cells at
		// grid edges and corners.
		highp float crossing_epsilon = max(grid_spacing * 0.00001, 0.0001);
		if (next_crossings.x <= next_crossing + crossing_epsilon) {
			next_crossings.x += crossing_intervals.x;
		}
		if (next_crossings.y <= next_crossing + crossing_epsilon) {
			next_crossings.y += crossing_intervals.y;
		}
		if (next_crossings.z <= next_crossing + crossing_epsilon) {
			next_crossings.z += crossing_intervals.z;
		}
		if (next_crossings.w <= next_crossing + crossing_epsilon) {
			next_crossings.w += crossing_intervals.w;
		}
		segment_start = segment_end;
		segment_start_density = segment_end_density;
	}
	float opacity = 1.0 - transmittance;
	// Ambient scattering and opacity accumulate the same transmittance weights.
	return vec3(opacity, direct_scattering, opacity);
}

// The return value packs ambient scattering, direct scattering, and opacity in RGB.
// Mobile sky subpasses may use an RGB-only format, so opacity cannot live in alpha.
vec3 render_volumetric_cloud_factors(highp vec4 camera_eye_direction_4d, highp vec2 fragment_coordinate, bool cubemap_pass) {
	if (cubemap_pass && !cloud_affect_radiance) {
		return vec3(0.0);
	}
	if (cloud_coverage <= 0.0 || cloud_density <= 0.0 || cloud_extinction_coefficient <= 0.0) {
		return vec3(0.0);
	}
	highp vec4 ray_direction_4d = normalize(cloud_camera_basis_4d * camera_eye_direction_4d);
	highp float ray_start;
	highp float ray_end;
	if (!intersect_cloud_layer(cloud_camera_position_4d, ray_direction_4d, ray_start, ray_end)) {
		return vec3(0.0);
	}

	bool primary_light_enabled;
	highp vec4 primary_light_direction_4d;
	vec3 primary_light_radiance;
	get_primary_cloud_light(primary_light_enabled, primary_light_direction_4d, primary_light_radiance);
	primary_light_enabled = primary_light_enabled && cloud_sun_intensity > 0.0 && any(notEqual(primary_light_radiance, vec3(0.0)));
	float primary_light_phase = 0.0;
	if (primary_light_enabled) {
		primary_light_phase = cloud_phase_4d(clamp(dot(ray_direction_4d, primary_light_direction_4d), -1.0, 1.0));
	}
	highp vec4 cloud_shadow_offset_4d = primary_light_direction_4d * cloud_shadow_distance;
	float cloud_shadow_extinction = cloud_extinction_coefficient * cloud_shadow_distance * cloud_shadow_strength;

	int step_count = clamp(cloud_sampling_steps, 4, 128);
	if (cloud_sampling_method == 0) {
		return render_cloud_world_grid(ray_direction_4d, ray_start, ray_end, step_count, primary_light_enabled, cloud_shadow_offset_4d, cloud_shadow_extinction, primary_light_phase);
	}
	highp float step_length = (ray_end - ray_start) / float(step_count);
	// Jitter converts coherent undersampling bands into fine noise. This method is most
	// useful with a large sampling budget; World Grid is stable at lower settings.
	highp float ray_distance = ray_start + step_length * cloud_ray_jitter(fragment_coordinate);
	float maximum_sampling_interval = cloud_maximum_ray_distance / float(step_count);
	const float octave_2_lacunarity = 2.03;
	const float octave_3_lacunarity = octave_2_lacunarity * 2.01;
	vec4 octave_weights = vec4(
			1.0,
			1.0 - smoothstep(cloud_shape_scale / octave_2_lacunarity, cloud_shape_scale * 2.0 / octave_2_lacunarity, maximum_sampling_interval),
			1.0 - smoothstep(cloud_shape_scale / octave_3_lacunarity, cloud_shape_scale * 2.0 / octave_3_lacunarity, maximum_sampling_interval),
			1.0 - smoothstep(cloud_detail_scale, cloud_detail_scale * 2.0, maximum_sampling_interval));
	float transmittance = 1.0;
	float direct_scattering = 0.0;
	for (int step_index = 0; step_index < 128; step_index++) {
		if (step_index >= step_count || ray_distance >= ray_end || transmittance <= 0.01) {
			break;
		}
		highp vec4 sample_position_4d = cloud_camera_position_4d + ray_direction_4d * ray_distance;
		highp float sample_density = sample_cloud_density(sample_position_4d, octave_weights);
		sample_density *= cloud_distance_fade(ray_distance);
		if (sample_density > 0.0001) {
			float optical_depth = sample_density * cloud_extinction_coefficient * step_length;
			float sample_transmittance = exp(-optical_depth);
			float sample_opacity = 1.0 - sample_transmittance;
			float scattering_weight = transmittance * sample_opacity;
			if (primary_light_enabled) {
				float light_transmittance = 1.0;
				if (cloud_shadow_extinction > 0.0) {
					highp vec4 shadow_position_4d = sample_position_4d + cloud_shadow_offset_4d;
					highp float shadow_density = sample_cloud_density(shadow_position_4d, vec4(octave_weights.x, 0.0, 0.0, 0.0));
					light_transmittance = exp(-shadow_density * cloud_shadow_extinction);
				}
				direct_scattering += scattering_weight * light_transmittance * primary_light_phase;
			}
			transmittance *= sample_transmittance;
		}
		ray_distance += step_length;
	}
	float opacity = 1.0 - transmittance;
	// Ambient scattering and opacity accumulate the same transmittance weights.
	return vec3(opacity, direct_scattering, opacity);
}

// Gradient and physical sky variants enable debanding, which also affects the
// quarter-resolution pass where these RGB channels hold cloud data rather than final color.
// Square-root companding makes the dither around zero negligible after decoding while
// preserving the full range. Plain and skyless variants do not need either operation.
vec3 encode_volumetric_cloud_factors(vec3 cloud_factors) {
	return sqrt(clamp(cloud_factors / vec3(1.0, 8.0, 1.0), vec3(0.0), vec3(1.0)));
}

vec3 decode_volumetric_cloud_factors(vec3 encoded_cloud_factors) {
	vec3 clamped_factors = clamp(encoded_cloud_factors, vec3(0.0), vec3(1.0));
	return clamped_factors * clamped_factors * vec3(1.0, 8.0, 1.0);
}

vec4 colorize_volumetric_cloud_factors(vec3 cloud_factors) {
	bool primary_light_enabled;
	highp vec4 primary_light_direction_4d;
	vec3 primary_light_radiance;
	get_primary_cloud_light(primary_light_enabled, primary_light_direction_4d, primary_light_radiance);
	vec3 scattered_light = cloud_ambient_color.rgb * cloud_ambient_intensity * cloud_factors.x;
	if (primary_light_enabled) {
		scattered_light += primary_light_radiance * cloud_sun_intensity * cloud_factors.y;
	}
	return vec4(scattered_light * cloud_albedo_color.rgb, cloud_factors.z);
}
