shader_type sky;
render_mode use_quarter_res_pass;

// The quarter-resolution subpass keeps volumetric raymarching practical on RenderingDevice backends.

#include "../sky/plain_sky_common.inc.glsl"
#include "../sky/sky_render_parameters_4d.inc.glsl"
