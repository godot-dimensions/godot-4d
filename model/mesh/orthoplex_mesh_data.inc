#pragma once

#include "../../godot_4d_defines.h"

constexpr real_t _1D6 = 1.0 / 6.0;
constexpr real_t _5D6 = 5.0 / 6.0;

// This file contains pre-computed data for 4D orthoplex meshes.
// We have to use #define below because static constants will crash GDExtension.
// This code is much harder to read when clang-format messes with the
// manual alignment, so disable clang-format just for this file.
/* clang-format off */
#define ORTHOPLEX_CELL_FACE_NORMALS PackedVector4Array { \
	Vector4(-0.5f, -0.5f, -0.5f, -0.5f), \
	Vector4(+0.5f, -0.5f, -0.5f, -0.5f), \
	Vector4(+0.5f, +0.5f, -0.5f, -0.5f), \
	Vector4(-0.5f, +0.5f, -0.5f, -0.5f), \
	Vector4(-0.5f, +0.5f, +0.5f, -0.5f), \
	Vector4(+0.5f, +0.5f, +0.5f, -0.5f), \
	Vector4(+0.5f, -0.5f, +0.5f, -0.5f), \
	Vector4(-0.5f, -0.5f, +0.5f, -0.5f), \
	Vector4(-0.5f, -0.5f, +0.5f, +0.5f), \
	Vector4(+0.5f, -0.5f, +0.5f, +0.5f), \
	Vector4(+0.5f, +0.5f, +0.5f, +0.5f), \
	Vector4(-0.5f, +0.5f, +0.5f, +0.5f), \
	Vector4(-0.5f, +0.5f, -0.5f, +0.5f), \
	Vector4(+0.5f, +0.5f, -0.5f, +0.5f), \
	Vector4(+0.5f, -0.5f, -0.5f, +0.5f), \
	Vector4(-0.5f, -0.5f, -0.5f, +0.5f), \
}

#define REPEAT_FOUR_TIMES(m) m, m, m, m
#define ORTHOPLEX_CELL_VERTEX_NORMALS PackedVector4Array { \
	REPEAT_FOUR_TIMES(Vector4(-0.5f, -0.5f, -0.5f, -0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(+0.5f, -0.5f, -0.5f, -0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(+0.5f, +0.5f, -0.5f, -0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(-0.5f, +0.5f, -0.5f, -0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(-0.5f, +0.5f, +0.5f, -0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(+0.5f, +0.5f, +0.5f, -0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(+0.5f, -0.5f, +0.5f, -0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(-0.5f, -0.5f, +0.5f, -0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(-0.5f, -0.5f, +0.5f, +0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(+0.5f, -0.5f, +0.5f, +0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(+0.5f, +0.5f, +0.5f, +0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(-0.5f, +0.5f, +0.5f, +0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(-0.5f, +0.5f, -0.5f, +0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(+0.5f, +0.5f, -0.5f, +0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(+0.5f, -0.5f, -0.5f, +0.5f)), \
	REPEAT_FOUR_TIMES(Vector4(-0.5f, -0.5f, -0.5f, +0.5f)), \
}

#define ORTHOPLEX_CELL_INDICES PackedInt32Array { \
	6, 0, 2, 4, 1, 2, 4, 6, 6, 1, 3, 4, 0, 3, 4, 6, /* -W and -Z */ \
	6, 0, 3, 5, 1, 3, 5, 6, 6, 1, 2, 5, 0, 2, 5, 6, /* -W and +Z */ \
	7, 0, 2, 5, 1, 2, 5, 7, 7, 1, 3, 5, 0, 3, 5, 7, /* +W and +Z */ \
	7, 0, 3, 4, 1, 3, 4, 7, 7, 1, 2, 4, 0, 2, 4, 7, /* +W and -Z */ \
}

#define ORTHOPLEX_CELL_UVW_MAP PackedVector3Array { \
	/* -W and -Z */ \
	Vector3(_1D6, _1D6, _1D6), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), Vector3(_5D6, _1D6, _1D6), \
	Vector3(_5D6, _5D6, _1D6), Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), Vector3(_1D6, _5D6, _1D6), \
	/* -W and +Z */ \
	Vector3(_1D6, _5D6, _5D6), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), Vector3(_5D6, _5D6, _5D6), \
	Vector3(_5D6, _1D6, _5D6), Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), \
	Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), Vector3(_1D6, _1D6, _5D6), \
	/* +W and +Z */ \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.5f), \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), \
	Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.5f), \
	/* +W and -Z */ \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), Vector3(0.5f, 0.5f, 0.5f), \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), Vector3(0.5f, 0.5f, 0.5f), \
};

#define ORTHOPLEX_EDGE_INDICES PackedInt32Array { \
	0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, /* -X connecting to YZW */ \
	1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 7, /* +X connecting to YZW */ \
	2, 4, 2, 5, 2, 6, 2, 7, /* -Y connecting to ZW */ \
	3, 4, 3, 5, 3, 6, 3, 7, /* +Y connecting to ZW */ \
	4, 6, 4, 7, /* -Z connecting to W */ \
	5, 6, 5, 7, /* +Z connecting to W */ \
}
/* clang-format on */
