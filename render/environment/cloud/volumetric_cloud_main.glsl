void sky() {
	vec4 eye_direction_4d = vec4(EYEDIR, 0.0);
	if (AT_QUARTER_RES_PASS) {
		COLOR = render_volumetric_cloud_factors(eye_direction_4d, FRAGCOORD.xy, AT_CUBEMAP_PASS);
	} else {
		COLOR = colorize_volumetric_cloud_factors(QUARTER_RES_COLOR.rgb).rgb;
	}
}
