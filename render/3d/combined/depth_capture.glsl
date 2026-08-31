#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D depth_input;
layout(r32f, set = 0, binding = 1) uniform image2D depth_output;

layout(push_constant, std430) uniform Params {
	ivec2 input_size;
	ivec2 output_size;
}
params;

void main() {
	ivec2 output_uv = ivec2(gl_GlobalInvocationID.xy);
	if (output_uv.x >= params.output_size.x || output_uv.y >= params.output_size.y) {
		return;
	}
	vec2 normalized_uv = (vec2(output_uv) + vec2(0.5)) / vec2(params.output_size);
	ivec2 input_uv = min(ivec2(normalized_uv * vec2(params.input_size)), params.input_size - ivec2(1));
	float depth = texelFetch(depth_input, input_uv, 0).x;
	imageStore(depth_output, output_uv, vec4(depth, 0.0, 0.0, 0.0));
}
