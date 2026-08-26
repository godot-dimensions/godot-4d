shader_type sky;
render_mode use_debanding;

// Compatibility fallback for Godot versions whose sky subpasses do not compile reliably.

#include "../sky/physical_sky_common.inc.glsl"
