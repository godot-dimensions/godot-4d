shader_type sky;

uniform vec4 color : source_color = vec4(0.0, 0.0, 0.0, 1.0);
uniform float energy_multiplier : hint_range(0, 128) = 1.0;

void sky() {
	COLOR = color.rgb * energy_multiplier;
}
