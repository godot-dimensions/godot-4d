shader_type sky;
render_mode use_debanding, use_quarter_res_pass;

// The quarter-resolution subpass keeps volumetric raymarching practical on RenderingDevice backends.

#include "../sky/gradient_sky_common.inc.glsl"
