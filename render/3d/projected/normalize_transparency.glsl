#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, set = 0, binding = 0) uniform image2D projected_frame;

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
	vec4 color = imageLoad(projected_frame, uv);
	color.xyz /= color.a + 1.0;
	color.a = 1.;
	imageStore(projected_frame, uv, color);
}
