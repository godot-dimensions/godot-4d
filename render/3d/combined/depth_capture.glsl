#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D depth_input;
layout(r32f, set = 0, binding = 1) uniform image2D depth_output;

layout(push_constant, std430) uniform Params {
	ivec2 size;
	ivec2 reserved;
}
params;

void main() {
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	if (uv.x >= params.size.x || uv.y >= params.size.y) {
		return;
	}
	float depth = texelFetch(depth_input, uv, 0).x;
	imageStore(depth_output, uv, vec4(depth, 0.0, 0.0, 0.0));
}
