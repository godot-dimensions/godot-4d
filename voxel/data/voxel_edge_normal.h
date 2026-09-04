#pragma once

#include "../../godot_4d_defines.h"

// A surface normal compressed to four signed 8-bit components, accurate to
// well under a degree, which is plenty for meshing and shading.
struct VoxelEdgeNormal {
	int8_t x = 0;
	int8_t y = 0;
	int8_t z = 0;
	int8_t w = 0;

	static VoxelEdgeNormal encode(const Vector4 &p_normal) {
		const Vector4 normal = p_normal.normalized();
		VoxelEdgeNormal encoded;
		encoded.x = (int8_t)Math::round(normal.x * 127.0);
		encoded.y = (int8_t)Math::round(normal.y * 127.0);
		encoded.z = (int8_t)Math::round(normal.z * 127.0);
		encoded.w = (int8_t)Math::round(normal.w * 127.0);
		return encoded;
	}

	Vector4 decode() const {
		return Vector4(x, y, z, w).normalized();
	}
};

static_assert(sizeof(VoxelEdgeNormal) == 4);
