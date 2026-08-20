void sky() {
	vec4 eye_direction_4d = vec4(EYEDIR, 0.0);
	vec3 cloud_factors = render_volumetric_cloud_factors(eye_direction_4d, FRAGCOORD.xy, AT_CUBEMAP_PASS);
	vec4 clouds = colorize_volumetric_cloud_factors(cloud_factors);
	vec3 base_sky = render_plain_sky(eye_direction_4d);
	COLOR = base_sky * (1.0 - clouds.a) + clouds.rgb;
}
