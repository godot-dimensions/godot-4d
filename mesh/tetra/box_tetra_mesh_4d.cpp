#include "box_tetra_mesh_4d.h"

#include "../wire/box_wire_mesh_4d.h"

#define REPEAT_FIVE_TIMES(m) m, m, m, m, m
const PackedVector4Array BOX_40_CELL_NORMALS = {
	REPEAT_FIVE_TIMES(Vector4(-1.0f, 0.0f, 0.0f, 0.0f)), // -X
	REPEAT_FIVE_TIMES(Vector4(+1.0f, 0.0f, 0.0f, 0.0f)), // +X
	REPEAT_FIVE_TIMES(Vector4(0.0f, -1.0f, 0.0f, 0.0f)), // -Y
	REPEAT_FIVE_TIMES(Vector4(0.0f, +1.0f, 0.0f, 0.0f)), // +Y
	REPEAT_FIVE_TIMES(Vector4(0.0f, 0.0f, -1.0f, 0.0f)), // -Z
	REPEAT_FIVE_TIMES(Vector4(0.0f, 0.0f, +1.0f, 0.0f)), // +Z
	REPEAT_FIVE_TIMES(Vector4(0.0f, 0.0f, 0.0f, -1.0f)), // -W
	REPEAT_FIVE_TIMES(Vector4(0.0f, 0.0f, 0.0f, +1.0f)), // +W
};

#define REPEAT_SIX_TIMES(m) m, m, m, m, m, m
const PackedVector4Array BOX_48_CELL_NORMALS = {
	REPEAT_SIX_TIMES(Vector4(-1.0f, 0.0f, 0.0f, 0.0f)), // -X
	REPEAT_SIX_TIMES(Vector4(+1.0f, 0.0f, 0.0f, 0.0f)), // +X
	REPEAT_SIX_TIMES(Vector4(0.0f, -1.0f, 0.0f, 0.0f)), // -Y
	REPEAT_SIX_TIMES(Vector4(0.0f, +1.0f, 0.0f, 0.0f)), // +Y
	REPEAT_SIX_TIMES(Vector4(0.0f, 0.0f, -1.0f, 0.0f)), // -Z
	REPEAT_SIX_TIMES(Vector4(0.0f, 0.0f, +1.0f, 0.0f)), // +Z
	REPEAT_SIX_TIMES(Vector4(0.0f, 0.0f, 0.0f, -1.0f)), // -W
	REPEAT_SIX_TIMES(Vector4(0.0f, 0.0f, 0.0f, +1.0f)), // +W
};

// This code is much harder to read without alignment, so disabling clang-format just for the next hundred lines or so.
/* clang-format off */
static const PackedInt32Array BOX_40_CELL_INDICES = {
	 8,  4, 14,  2,  8, 10,  2, 14,  8,  0,  4,  2,  8, 12, 14,  4,  6,  4,  2, 14, // -X
	11, 13,  1,  7, 11,  3,  7,  1, 11, 15, 13,  7, 11,  9,  1, 13,  5, 13,  7,  1, // +X
	 4, 13,  1,  8,  4,  0,  8,  1,  4, 12, 13,  8,  4,  5,  1, 13,  9, 13,  8,  1, // -Y
	 14, 7, 11,  2, 14, 10,  2, 11, 14,  6,  7,  2, 14, 15, 11,  7,  3,  7,  2, 11, // +Y
	 1,  8, 11,  2,  1,  3,  2, 11,  1,  0,  8,  2,  1,  9, 11,  8, 10,  8,  2, 11, // -Z
	 13, 4,  7, 14, 13, 15, 14,  7, 13, 12,  4, 14, 13,  5,  7,  4,  6,  4, 14,  7, // +Z
	 1,  7,  4,  2,  1,  0,  2,  4,  1,  3,  7,  2,  1,  5,  4,  7,  6,  7,  2,  4, // -W
	 8, 14, 13, 11,  8,  9, 11, 13,  8, 10, 14, 11,  8, 12, 13, 14, 15, 14, 11, 13, // +W
};

const PackedInt32Array BOX_48_CELL_INDICES = {
	 8,  0,  4,  6,  8,  4, 12,  6,  8, 12, 14,  6,  8, 14, 10,  6,  8, 10,  2,  6,  8,  2,  0,  6, // -X
	 9, 11, 15,  7,  9, 15, 13,  7,  9, 13,  5,  7,  9,  5,  1,  7,  9,  1,  3,  7,  9,  3, 11,  7, // +X
	 5, 13,  9,  8,  5,  9,  1,  8,  5,  1,  0,  8,  5,  0,  4,  8,  5,  4, 12,  8,  5, 12, 13,  8, // -Y
	10,  2,  6,  7, 10,  6, 14,  7, 10, 14, 15,  7, 10, 15, 11,  7, 10, 11,  3,  7, 10,  3,  2,  7, // +Y
	 8,  0,  2,  3,  8,  2, 10,  3,  8, 10, 11,  3,  8, 11,  9,  3,  8,  9,  1,  3,  8,  1,  0,  3, // -Z
	12, 14,  6,  7, 12,  6,  4,  7, 12,  4,  5,  7, 12,  5, 13,  7, 12, 13, 15,  7, 12, 15, 14,  7, // +Z
	 0,  1,  6,  7,  0,  6,  4,  7,  0,  4,  5,  7,  0,  5,  2,  7,  0,  2,  3,  7,  0,  3,  1,  7, // -W
	 8, 10, 14, 15,  8, 14, 12, 15,  8, 12, 13, 15,  8, 13,  9, 15,  8,  9, 11, 15,  8, 11, 10, 15, // +W
};

const PackedVector3Array BOX_40_CELL_UVW_MAP = {
	// -X
	Vector3(0.3, 0.3, 0.3), Vector3(0.0, 0.3, 0.6), Vector3(0.3, 0.6, 0.6), Vector3(0.0, 0.6, 0.3),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.3), Vector3(0.0, 0.6, 0.3), Vector3(0.3, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.0, 0.3, 0.3), Vector3(0.0, 0.3, 0.6), Vector3(0.0, 0.6, 0.3),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.3, 0.6), Vector3(0.3, 0.6, 0.6), Vector3(0.0, 0.3, 0.6),
	Vector3(0.0, 0.6, 0.6), Vector3(0.0, 0.3, 0.6), Vector3(0.0, 0.6, 0.3), Vector3(0.3, 0.6, 0.6),
	// +X
	Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.3, 0.6), Vector3(0.9, 0.3, 0.3), Vector3(0.9, 0.6, 0.6),
	Vector3(0.6, 0.6, 0.3), Vector3(0.9, 0.6, 0.3), Vector3(0.9, 0.6, 0.6), Vector3(0.9, 0.3, 0.3),
	Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.6, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.9, 0.6, 0.6),
	Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.3, 0.3), Vector3(0.9, 0.3, 0.3), Vector3(0.6, 0.3, 0.6),
	Vector3(0.9, 0.3, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.9, 0.6, 0.6), Vector3(0.9, 0.3, 0.3),
	// -Y
	Vector3(0.3, 0.0, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.0, 0.3), Vector3(0.3, 0.3, 0.3),
	Vector3(0.3, 0.0, 0.6), Vector3(0.3, 0.0, 0.3), Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.0, 0.3),
	Vector3(0.3, 0.0, 0.6), Vector3(0.3, 0.3, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.3, 0.3, 0.3),
	Vector3(0.3, 0.0, 0.6), Vector3(0.6, 0.0, 0.6), Vector3(0.6, 0.0, 0.3), Vector3(0.6, 0.3, 0.6),
	Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.3, 0.6), Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.0, 0.3),
	// +Y
	Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.9, 0.6), Vector3(0.6, 0.6, 0.3), Vector3(0.3, 0.9, 0.3),
	Vector3(0.3, 0.6, 0.6), Vector3(0.3, 0.6, 0.3), Vector3(0.3, 0.9, 0.3), Vector3(0.6, 0.6, 0.3),
	Vector3(0.3, 0.6, 0.6), Vector3(0.3, 0.9, 0.6), Vector3(0.6, 0.9, 0.6), Vector3(0.3, 0.9, 0.3),
	Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.6, 0.6), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.9, 0.6),
	Vector3(0.6, 0.9, 0.3), Vector3(0.6, 0.9, 0.6), Vector3(0.3, 0.9, 0.3), Vector3(0.6, 0.6, 0.3),
	// -Z
	Vector3(0.6, 0.3, 0.0), Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.3, 0.6, 0.0),
	Vector3(0.6, 0.3, 0.0), Vector3(0.6, 0.6, 0.0), Vector3(0.3, 0.6, 0.0), Vector3(0.6, 0.6, 0.3),
	Vector3(0.6, 0.3, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.0),
	Vector3(0.6, 0.3, 0.0), Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.3, 0.3, 0.3),
	Vector3(0.3, 0.6, 0.3), Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.0), Vector3(0.6, 0.6, 0.3),
	// +Z
	Vector3(0.6, 0.3, 0.6), Vector3(0.3, 0.3, 0.9), Vector3(0.6, 0.6, 0.9), Vector3(0.3, 0.6, 0.6),
	Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.6, 0.6), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.6, 0.9),
	Vector3(0.6, 0.3, 0.6), Vector3(0.3, 0.3, 0.6), Vector3(0.3, 0.3, 0.9), Vector3(0.3, 0.6, 0.6),
	Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.3, 0.9), Vector3(0.6, 0.6, 0.9), Vector3(0.3, 0.3, 0.9),
	Vector3(0.3, 0.6, 0.9), Vector3(0.3, 0.3, 0.9), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.6, 0.9),
	// -W
	Vector3(0.7, 0.7, 0.7), Vector3(0.7, 1.0, 1.0), Vector3(1.0, 0.7, 1.0), Vector3(1.0, 1.0, 0.7),
	Vector3(0.7, 0.7, 0.7), Vector3(1.0, 0.7, 0.7), Vector3(1.0, 1.0, 0.7), Vector3(1.0, 0.7, 1.0),
	Vector3(0.7, 0.7, 0.7), Vector3(0.7, 1.0, 0.7), Vector3(0.7, 1.0, 1.0), Vector3(1.0, 1.0, 0.7),
	Vector3(0.7, 0.7, 0.7), Vector3(0.7, 0.7, 1.0), Vector3(1.0, 0.7, 1.0), Vector3(0.7, 1.0, 1.0),
	Vector3(1.0, 1.0, 1.0), Vector3(0.7, 1.0, 1.0), Vector3(1.0, 1.0, 0.7), Vector3(1.0, 0.7, 1.0),
	// +W
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.6, 0.3),
	Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.3, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.3), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.6, 0.3),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.3, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.3, 0.6, 0.6),
	Vector3(0.6, 0.6, 0.6), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.3, 0.6),
};

const PackedVector3Array BOX_48_CELL_UVW_MAP = {
	// -X
	Vector3(0.3, 0.3, 0.3), Vector3(0.0, 0.3, 0.3), Vector3(0.0, 0.3, 0.6), Vector3(0.0, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.0, 0.3, 0.6), Vector3(0.3, 0.3, 0.6), Vector3(0.0, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.3, 0.6), Vector3(0.3, 0.6, 0.6), Vector3(0.0, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.6), Vector3(0.3, 0.6, 0.3), Vector3(0.0, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.3), Vector3(0.0, 0.6, 0.3), Vector3(0.0, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.0, 0.6, 0.3), Vector3(0.0, 0.3, 0.3), Vector3(0.0, 0.6, 0.6),
	// +X
	Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.6, 0.6), Vector3(0.9, 0.6, 0.6),
	Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.6, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.9, 0.6, 0.6),
	Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.3, 0.6), Vector3(0.9, 0.3, 0.6), Vector3(0.9, 0.6, 0.6),
	Vector3(0.6, 0.3, 0.3), Vector3(0.9, 0.3, 0.6), Vector3(0.9, 0.3, 0.3), Vector3(0.9, 0.6, 0.6),
	Vector3(0.6, 0.3, 0.3), Vector3(0.9, 0.3, 0.3), Vector3(0.9, 0.6, 0.3), Vector3(0.9, 0.6, 0.6),
	Vector3(0.6, 0.3, 0.3), Vector3(0.9, 0.6, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.9, 0.6, 0.6),
	// -Y
	Vector3(0.6, 0.0, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.3, 0.3), Vector3(0.3, 0.3, 0.3),
	Vector3(0.6, 0.0, 0.6), Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.0, 0.3), Vector3(0.3, 0.3, 0.3),
	Vector3(0.6, 0.0, 0.6), Vector3(0.6, 0.0, 0.3), Vector3(0.3, 0.0, 0.3), Vector3(0.3, 0.3, 0.3),
	Vector3(0.6, 0.0, 0.6), Vector3(0.3, 0.0, 0.3), Vector3(0.3, 0.0, 0.6), Vector3(0.3, 0.3, 0.3),
	Vector3(0.6, 0.0, 0.6), Vector3(0.3, 0.0, 0.6), Vector3(0.3, 0.3, 0.6), Vector3(0.3, 0.3, 0.3),
	Vector3(0.6, 0.0, 0.6), Vector3(0.3, 0.3, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.3, 0.3, 0.3),
	// +Y
	Vector3(0.3, 0.6, 0.3), Vector3(0.3, 0.9, 0.3), Vector3(0.3, 0.9, 0.6), Vector3(0.6, 0.9, 0.6),
	Vector3(0.3, 0.6, 0.3), Vector3(0.3, 0.9, 0.6), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.9, 0.6),
	Vector3(0.3, 0.6, 0.3), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.6, 0.6), Vector3(0.6, 0.9, 0.6),
	Vector3(0.3, 0.6, 0.3), Vector3(0.6, 0.6, 0.6), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.9, 0.6),
	Vector3(0.3, 0.6, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.9, 0.3), Vector3(0.6, 0.9, 0.6),
	Vector3(0.3, 0.6, 0.3), Vector3(0.6, 0.9, 0.3), Vector3(0.3, 0.9, 0.3), Vector3(0.6, 0.9, 0.6),
	// -Z
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.3, 0.0), Vector3(0.3, 0.6, 0.0), Vector3(0.6, 0.6, 0.0),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.0), Vector3(0.3, 0.6, 0.3), Vector3(0.6, 0.6, 0.0),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.6, 0.0),
	Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.6, 0.0),
	Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.3, 0.0), Vector3(0.6, 0.6, 0.0),
	Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.3, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(0.6, 0.6, 0.0),
	// +Z
	Vector3(0.3, 0.3, 0.6), Vector3(0.3, 0.6, 0.6), Vector3(0.3, 0.6, 0.9), Vector3(0.6, 0.6, 0.9),
	Vector3(0.3, 0.3, 0.6), Vector3(0.3, 0.6, 0.9), Vector3(0.3, 0.3, 0.9), Vector3(0.6, 0.6, 0.9),
	Vector3(0.3, 0.3, 0.6), Vector3(0.3, 0.3, 0.9), Vector3(0.6, 0.3, 0.9), Vector3(0.6, 0.6, 0.9),
	Vector3(0.3, 0.3, 0.6), Vector3(0.6, 0.3, 0.9), Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.6, 0.9),
	Vector3(0.3, 0.3, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.6, 0.6), Vector3(0.6, 0.6, 0.9),
	Vector3(0.3, 0.3, 0.6), Vector3(0.6, 0.6, 0.6), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.6, 0.9),
	// -W
	Vector3(1.0, 0.7, 0.7), Vector3(0.7, 0.7, 0.7), Vector3(0.7, 0.7, 1.0), Vector3(0.7, 1.0, 1.0),
	Vector3(1.0, 0.7, 0.7), Vector3(0.7, 0.7, 1.0), Vector3(1.0, 0.7, 1.0), Vector3(0.7, 1.0, 1.0),
	Vector3(1.0, 0.7, 0.7), Vector3(1.0, 0.7, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.7, 1.0, 1.0),
	Vector3(1.0, 0.7, 0.7), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.7), Vector3(0.7, 1.0, 1.0),
	Vector3(1.0, 0.7, 0.7), Vector3(1.0, 1.0, 0.7), Vector3(0.7, 1.0, 0.7), Vector3(0.7, 1.0, 1.0),
	Vector3(1.0, 0.7, 0.7), Vector3(0.7, 1.0, 0.7), Vector3(0.7, 0.7, 0.7), Vector3(0.7, 1.0, 1.0),
	// +W
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.3), Vector3(0.3, 0.6, 0.6), Vector3(0.6, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.6, 0.6), Vector3(0.3, 0.3, 0.6), Vector3(0.6, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.3, 0.3, 0.6), Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.3, 0.6), Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.3, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.6, 0.6, 0.6),
	Vector3(0.3, 0.3, 0.3), Vector3(0.6, 0.6, 0.3), Vector3(0.3, 0.6, 0.3), Vector3(0.6, 0.6, 0.6),
};
/* clang-format on */

void BoxTetraMesh4D::_clear_caches() {
	_cell_positions_cache.clear();
	_vertices_cache.clear();
}

Vector4 BoxTetraMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void BoxTetraMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
	_clear_caches();
}

Vector4 BoxTetraMesh4D::get_size() const {
	return _size;
}

void BoxTetraMesh4D::set_size(const Vector4 &p_size) {
	_size = p_size;
	_clear_caches();
}

BoxTetraMesh4D::BoxTetraDecomp BoxTetraMesh4D::get_tetra_decomp() const {
	return _tetra_decomp;
}

void BoxTetraMesh4D::set_tetra_decomp(const BoxTetraDecomp p_decomp) {
	_tetra_decomp = p_decomp;
	_clear_caches();
}

PackedInt32Array BoxTetraMesh4D::get_cell_indices() {
	return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_INDICES : BOX_48_CELL_INDICES;
}

PackedVector4Array BoxTetraMesh4D::get_cell_positions() {
	if (_cell_positions_cache.is_empty()) {
		const PackedVector4Array vertices = get_vertices();
		for (const int i : (_tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_INDICES : BOX_48_CELL_INDICES)) {
			_cell_positions_cache.append(vertices[i]);
		}
	}
	return _cell_positions_cache;
}

PackedVector4Array BoxTetraMesh4D::get_cell_normals() {
	return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_NORMALS : BOX_48_CELL_NORMALS;
}

PackedVector3Array BoxTetraMesh4D::get_cell_uvw_map() {
	return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_UVW_MAP : BOX_48_CELL_UVW_MAP;
}

PackedVector4Array BoxTetraMesh4D::get_vertices() {
	if (_vertices_cache.is_empty()) {
		const Vector4 he = get_half_extents();
		_vertices_cache.append(Vector4(-he.x, -he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, +he.z, +he.w));
	}
	return _vertices_cache;
}

Ref<BoxTetraMesh4D> BoxTetraMesh4D::from_box_wire_mesh(const Ref<BoxWireMesh4D> &p_wire_mesh) {
	Ref<BoxTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(p_wire_mesh->get_size());
	tetra_mesh->set_material(p_wire_mesh->get_material());
	return tetra_mesh;
}

Ref<BoxWireMesh4D> BoxTetraMesh4D::to_box_wire_mesh() const {
	Ref<BoxWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<WireMesh4D> BoxTetraMesh4D::to_wire_mesh() {
	return to_box_wire_mesh();
}

void BoxTetraMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &BoxTetraMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &BoxTetraMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxTetraMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxTetraMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_method(D_METHOD("get_tetra_decomp"), &BoxTetraMesh4D::get_tetra_decomp);
	ClassDB::bind_method(D_METHOD("set_tetra_decomp", "decomp"), &BoxTetraMesh4D::set_tetra_decomp);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tetra_decomp", PROPERTY_HINT_ENUM, "40 Cell,48 Cell"), "set_tetra_decomp", "get_tetra_decomp");

	ClassDB::bind_static_method("BoxTetraMesh4D", D_METHOD("from_box_wire_mesh", "wire_mesh"), &BoxTetraMesh4D::from_box_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_box_wire_mesh"), &BoxTetraMesh4D::to_box_wire_mesh);

	BIND_ENUM_CONSTANT(BOX_TETRA_DECOMP_40_CELL);
	BIND_ENUM_CONSTANT(BOX_TETRA_DECOMP_48_CELL);
}
