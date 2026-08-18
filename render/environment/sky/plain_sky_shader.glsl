shader_type sky;

#include "plain_sky_common.inc.glsl"

void sky() {
	COLOR = render_plain_sky(vec4(EYEDIR, 0.0));
}
