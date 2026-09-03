#pragma once

#include "../../godot_4d_defines.h"

constexpr real_t _1D6 = 1.0 / 6.0;
constexpr real_t _5D6 = 5.0 / 6.0;

// This file contains pre-computed data for 4D orthoplex meshes.
// We have to use #define below because static constants will crash GDExtension.
// This code is much harder to read when clang-format messes with the
// manual alignment, so disable clang-format just for this file.
/* clang-format off */
#define ORTHOPLEX_NORMAL_VALUES PackedVector4Array { \
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

#define ORTHOPLEX_CELL_TEXTURE_MAP_VALUES PackedVector3Array { \
	/* -W and -Z */ \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(_5D6, _1D6, _1D6), Vector3(_5D6, _5D6, _1D6), Vector3(0.5f, 1.0f, 0.5f), \
	Vector3(_1D6, _5D6, _1D6), Vector3(_1D6, _1D6, _1D6), Vector3(0.5f, 0.5f, 1.0f), Vector3(_5D6, _5D6, _5D6), \
	Vector3(_5D6, _1D6, _5D6), Vector3(_1D6, _1D6, _5D6), Vector3(_1D6, _5D6, _5D6), \
}

#define REPEAT_FOUR_TIMES(m) m, m, m, m
#define ORTHOPLEX_CELL_NORMAL_INDICES PackedInt32Array { \
	REPEAT_FOUR_TIMES(0), \
	REPEAT_FOUR_TIMES(1), \
	REPEAT_FOUR_TIMES(2), \
	REPEAT_FOUR_TIMES(3), \
	REPEAT_FOUR_TIMES(4), \
	REPEAT_FOUR_TIMES(5), \
	REPEAT_FOUR_TIMES(6), \
	REPEAT_FOUR_TIMES(7), \
	REPEAT_FOUR_TIMES(8), \
	REPEAT_FOUR_TIMES(9), \
	REPEAT_FOUR_TIMES(10), \
	REPEAT_FOUR_TIMES(11), \
	REPEAT_FOUR_TIMES(12), \
	REPEAT_FOUR_TIMES(13), \
	REPEAT_FOUR_TIMES(14), \
	REPEAT_FOUR_TIMES(15), \
}

#define ORTHOPLEX_CELL_VERTEX_INDICES PackedInt32Array { \
	6, 0, 2, 4, 1, 2, 4, 6, 6, 1, 3, 4, 0, 3, 4, 6, /* -W and -Z */ \
	6, 0, 3, 5, 1, 3, 5, 6, 6, 1, 2, 5, 0, 2, 5, 6, /* -W and +Z */ \
	7, 0, 2, 5, 1, 2, 5, 7, 7, 1, 3, 5, 0, 3, 5, 7, /* +W and +Z */ \
	7, 0, 3, 4, 1, 3, 4, 7, 7, 1, 2, 4, 0, 2, 4, 7, /* +W and -Z */ \
}

#define ORTHOPLEX_CELL_TEXTURE_MAP_INDICES PackedInt32Array { \
	/* -W and -Z */ \
	9, 1, 2, 3, \
	4, 2, 3, 5, \
	6, 4, 7, 3, \
	1, 7, 3, 8, \
	/* -W and +Z */ \
	14, 1, 7, 10, \
	4, 7, 10, 11, \
	12, 4, 2, 10, \
	1, 2, 10, 13, \
	/* +W and +Z */ \
	0, 1, 2, 10, \
	4, 2, 10, 0, \
	0, 4, 7, 10, \
	1, 7, 10, 0, \
	/* +W and -Z */ \
	0, 1, 7, 3, \
	4, 7, 3, 0, \
	0, 4, 2, 3, \
	1, 2, 3, 0, \
}

#define ORTHOPLEX_POLY_CELL_INDICES Vector<Vector<PackedInt32Array>> { \
	Vector<PackedInt32Array> { \
		/* XYZ faces */ \
		PackedInt32Array { 0, 12,  2}, PackedInt32Array { 0, 13,  3}, PackedInt32Array { 1, 16,  2}, PackedInt32Array { 1, 17,  3}, \
		PackedInt32Array { 6, 12,  8}, PackedInt32Array { 6, 13,  9}, PackedInt32Array { 7, 16,  8}, PackedInt32Array { 7, 17,  9}, \
		/* XYW faces */ \
		PackedInt32Array { 0, 14,  4}, PackedInt32Array { 0, 15,  5}, PackedInt32Array { 1, 18,  4}, PackedInt32Array { 1, 19,  5}, \
		PackedInt32Array { 6, 14, 10}, PackedInt32Array { 6, 15, 11}, PackedInt32Array { 7, 18, 10}, PackedInt32Array { 7, 19, 11}, \
		/* XZW faces */ \
		PackedInt32Array { 2, 20,  4}, PackedInt32Array { 2, 21,  5}, PackedInt32Array { 3, 22,  4}, PackedInt32Array { 3, 23,  5}, \
		PackedInt32Array { 8, 20, 10}, PackedInt32Array { 8, 21, 11}, PackedInt32Array { 9, 22, 10}, PackedInt32Array { 9, 23, 11}, \
		/* YZW faces */ \
		PackedInt32Array {12, 20, 14}, PackedInt32Array {12, 21, 15}, PackedInt32Array {13, 22, 14}, PackedInt32Array {13, 23, 15}, \
		PackedInt32Array {16, 20, 18}, PackedInt32Array {16, 21, 19}, PackedInt32Array {17, 22, 18}, PackedInt32Array {17, 23, 19}, \
	}, \
	Vector<PackedInt32Array> { \
		/* The order within determines orientation, the order without matches OrthoplexTetraMesh4D. */ \
		PackedInt32Array { 8, 0, 16, 24}, PackedInt32Array {4, 12, 20, 24}, PackedInt32Array {14, 6, 20, 28}, PackedInt32Array {2, 10, 16, 28}, \
		PackedInt32Array {10, 3, 18, 30}, PackedInt32Array {7, 14, 22, 30}, PackedInt32Array {12, 5, 22, 26}, PackedInt32Array {1,  8, 18, 26}, \
		PackedInt32Array { 9, 1, 19, 27}, PackedInt32Array {5, 13, 23, 27}, PackedInt32Array {15, 7, 23, 31}, PackedInt32Array {3, 11, 19, 31}, \
		PackedInt32Array {11, 2, 17, 29}, PackedInt32Array {6, 15, 21, 29}, PackedInt32Array {13, 4, 21, 25}, PackedInt32Array {0,  9, 17, 25}, \
	}, \
	Vector<PackedInt32Array> { \
		PackedInt32Array {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15} /* XYZW hyper-cell */ \
	} \
}

// The order of these needs to match the data in ORTHOPLEX_POLY_CELL_INDICES.
#define ORTHOPLEX_POLY_CELL_VERTEX_NORMAL_INDICES Vector<PackedInt32Array> { \
	PackedInt32Array { REPEAT_FOUR_TIMES(0), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(1), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(2), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(3), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(4), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(5), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(6), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(7), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(8), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(9), }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(10) }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(11) }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(12) }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(13) }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(14) }, \
	PackedInt32Array { REPEAT_FOUR_TIMES(15) }, \
}

#define ORTHOPLEX_POLY_CELL_POLY_TEXTURE_MAP_INDICES Vector<PackedInt32Array> { \
	/* -W and -Z */ \
	PackedInt32Array { 1, 2, 9, 3 }, \
	PackedInt32Array { 4, 2, 3, 5 }, \
	PackedInt32Array { 4, 7, 6, 3 }, \
	PackedInt32Array { 1, 7, 3, 8 }, \
	/* -W and +Z */ \
	PackedInt32Array { 1, 7, 14, 10 }, \
	PackedInt32Array { 4, 7, 10, 11 }, \
	PackedInt32Array { 4, 2, 12, 10 }, \
	PackedInt32Array { 1, 2, 10, 13 }, \
	/* +W and +Z */ \
	PackedInt32Array { 1, 2, 0, 10 }, \
	PackedInt32Array { 4, 2, 10, 0 }, \
	PackedInt32Array { 4, 7, 0, 10 }, \
	PackedInt32Array { 1, 7, 10, 0 }, \
	/* +W and -Z */ \
	PackedInt32Array { 1, 7, 0, 3 }, \
	PackedInt32Array { 4, 7, 3, 0 }, \
	PackedInt32Array { 4, 2, 0, 3 }, \
	PackedInt32Array { 1, 2, 3, 0 }, \
}

// These are the same for all orthoplex types: poly, tetra, wire.
#define ORTHOPLEX_EDGE_INDICES PackedInt32Array { \
	0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, /* -X connecting to YZW */ \
	1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 7, /* +X connecting to YZW */ \
	2, 4, 2, 5, 2, 6, 2, 7, /* -Y connecting to ZW */ \
	3, 4, 3, 5, 3, 6, 3, 7, /* +Y connecting to ZW */ \
	4, 6, 4, 7, /* -Z connecting to W */ \
	5, 6, 5, 7, /* +Z connecting to W */ \
}
/* clang-format on */
