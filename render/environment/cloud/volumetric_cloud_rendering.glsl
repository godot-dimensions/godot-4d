
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
