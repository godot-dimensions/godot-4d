uniform vec4 color : source_color = vec4(0.0, 0.0, 0.0, 1.0);
uniform float energy_multiplier : hint_range(0, 128) = 1.0;

vec3 render_plain_sky(vec4 eye_direction_4d) {
	return color.rgb * energy_multiplier;
}
