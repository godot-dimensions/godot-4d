shader_type sky;

// Compatibility fallback for Godot versions whose sky subpasses do not compile reliably.

#include "../sky/plain_sky_common.inc.glsl"
#include "../sky/sky_render_parameters_4d.inc.glsl"
