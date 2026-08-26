#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, set = 0, binding = 0) uniform image2D projected_frame;

layout(push_constant, std430) uniform Params {
	ivec2 size;
	float opacity_base;
	int transparency; // logically a bool, 0 or 1, but pushing an int is more convenient.
}
params;

void main() {
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	if (uv.x >= params.size.x || uv.y >= params.size.y) {
		return;
	}
	vec4 color = imageLoad(projected_frame, uv);
	color /= color.a + params.opacity_base;
	color.a = mix(1., color.a, params.transparency);
	color.rgb = color.a > 0.0 ? color.rgb / color.a : vec3(0.0);
	imageStore(projected_frame, uv, color);
}
