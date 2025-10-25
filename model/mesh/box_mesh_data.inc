#pragma once

#include "../../godot_4d_defines.h"

// This file contains pre-computed data for 4D box meshes.
// We have to use #define below because static constants will crash GDExtension.
// This code is much harder to read when clang-format messes with the
// manual alignment, so disable clang-format just for this file.
/* clang-format off */
#define REPEAT_FIVE_TIMES(m) m, m, m, m, m
#define BOX_40_CELL_BOUNDARY_NORMALS PackedVector4Array { \
	REPEAT_FIVE_TIMES(Vector4(-1.0f, 0.0f, 0.0f, 0.0f)), /* -X */ \
	REPEAT_FIVE_TIMES(Vector4(+1.0f, 0.0f, 0.0f, 0.0f)), /* +X */ \
	REPEAT_FIVE_TIMES(Vector4(0.0f, -1.0f, 0.0f, 0.0f)), /* -Y */ \
	REPEAT_FIVE_TIMES(Vector4(0.0f, +1.0f, 0.0f, 0.0f)), /* +Y */ \
	REPEAT_FIVE_TIMES(Vector4(0.0f, 0.0f, -1.0f, 0.0f)), /* -Z */ \
	REPEAT_FIVE_TIMES(Vector4(0.0f, 0.0f, +1.0f, 0.0f)), /* +Z */ \
	REPEAT_FIVE_TIMES(Vector4(0.0f, 0.0f, 0.0f, -1.0f)), /* -W */ \
	REPEAT_FIVE_TIMES(Vector4(0.0f, 0.0f, 0.0f, +1.0f)), /* +W */ \
}

#define REPEAT_SIX_TIMES(m) m, m, m, m, m, m
#define BOX_48_CELL_BOUNDARY_NORMALS PackedVector4Array { \
	REPEAT_SIX_TIMES(Vector4(-1.0f, 0.0f, 0.0f, 0.0f)), /* -X */ \
	REPEAT_SIX_TIMES(Vector4(+1.0f, 0.0f, 0.0f, 0.0f)), /* +X */ \
	REPEAT_SIX_TIMES(Vector4(0.0f, -1.0f, 0.0f, 0.0f)), /* -Y */ \
	REPEAT_SIX_TIMES(Vector4(0.0f, +1.0f, 0.0f, 0.0f)), /* +Y */ \
	REPEAT_SIX_TIMES(Vector4(0.0f, 0.0f, -1.0f, 0.0f)), /* -Z */ \
	REPEAT_SIX_TIMES(Vector4(0.0f, 0.0f, +1.0f, 0.0f)), /* +Z */ \
	REPEAT_SIX_TIMES(Vector4(0.0f, 0.0f, 0.0f, -1.0f)), /* -W */ \
	REPEAT_SIX_TIMES(Vector4(0.0f, 0.0f, 0.0f, +1.0f)), /* +W */ \
}

#define REPEAT_TWENTY_TIMES(m) m, m, m, m, m
#define BOX_40_CELL_VERTEX_NORMALS PackedVector4Array { \
	REPEAT_TWENTY_TIMES(Vector4(-1.0f, 0.0f, 0.0f, 0.0f)), /* -X */ \
	REPEAT_TWENTY_TIMES(Vector4(+1.0f, 0.0f, 0.0f, 0.0f)), /* +X */ \
	REPEAT_TWENTY_TIMES(Vector4(0.0f, -1.0f, 0.0f, 0.0f)), /* -Y */ \
	REPEAT_TWENTY_TIMES(Vector4(0.0f, +1.0f, 0.0f, 0.0f)), /* +Y */ \
	REPEAT_TWENTY_TIMES(Vector4(0.0f, 0.0f, -1.0f, 0.0f)), /* -Z */ \
	REPEAT_TWENTY_TIMES(Vector4(0.0f, 0.0f, +1.0f, 0.0f)), /* +Z */ \
	REPEAT_TWENTY_TIMES(Vector4(0.0f, 0.0f, 0.0f, -1.0f)), /* -W */ \
	REPEAT_TWENTY_TIMES(Vector4(0.0f, 0.0f, 0.0f, +1.0f)), /* +W */ \
}

#define REPEAT_TWENTY_FOUR_TIMES(m) m, m, m, m, m, m
#define BOX_48_CELL_VERTEX_NORMALS PackedVector4Array { \
	REPEAT_TWENTY_FOUR_TIMES(Vector4(-1.0f, 0.0f, 0.0f, 0.0f)), /* -X */ \
	REPEAT_TWENTY_FOUR_TIMES(Vector4(+1.0f, 0.0f, 0.0f, 0.0f)), /* +X */ \
	REPEAT_TWENTY_FOUR_TIMES(Vector4(0.0f, -1.0f, 0.0f, 0.0f)), /* -Y */ \
	REPEAT_TWENTY_FOUR_TIMES(Vector4(0.0f, +1.0f, 0.0f, 0.0f)), /* +Y */ \
	REPEAT_TWENTY_FOUR_TIMES(Vector4(0.0f, 0.0f, -1.0f, 0.0f)), /* -Z */ \
	REPEAT_TWENTY_FOUR_TIMES(Vector4(0.0f, 0.0f, +1.0f, 0.0f)), /* +Z */ \
	REPEAT_TWENTY_FOUR_TIMES(Vector4(0.0f, 0.0f, 0.0f, -1.0f)), /* -W */ \
	REPEAT_TWENTY_FOUR_TIMES(Vector4(0.0f, 0.0f, 0.0f, +1.0f)), /* +W */ \
}

#define BOX_40_CELL_INDICES PackedInt32Array { \
	 8,  4, 14,  2,  8, 10,  2, 14,  8,  0,  4,  2,  8, 12, 14,  4,  6,  4,  2, 14, /* -X */ \
	11, 13,  1,  7, 11,  3,  7,  1, 11, 15, 13,  7, 11,  9,  1, 13,  5, 13,  7,  1, /* +X */ \
	 4, 13,  1,  8,  4,  0,  8,  1,  4, 12, 13,  8,  4,  5,  1, 13,  9, 13,  8,  1, /* -Y */ \
	 14, 7, 11,  2, 14, 10,  2, 11, 14,  6,  7,  2, 14, 15, 11,  7,  3,  7,  2, 11, /* +Y */ \
	 1,  8, 11,  2,  1,  3,  2, 11,  1,  0,  8,  2,  1,  9, 11,  8, 10,  8,  2, 11, /* -Z */ \
	 13, 4,  7, 14, 13, 15, 14,  7, 13, 12,  4, 14, 13,  5,  7,  4,  6,  4, 14,  7, /* +Z */ \
	 1,  7,  4,  2,  1,  0,  2,  4,  1,  3,  7,  2,  1,  5,  4,  7,  6,  7,  2,  4, /* -W */ \
	 8, 14, 13, 11,  8,  9, 11, 13,  8, 10, 14, 11,  8, 12, 13, 14, 15, 14, 11, 13, /* +W */ \
}

#define BOX_48_CELL_INDICES PackedInt32Array { \
	 8,  0,  4,  6,  8,  4, 12,  6,  8, 12, 14,  6,  8, 14, 10,  6,  8, 10,  2,  6,  8,  2,  0,  6, /* -X */ \
	 9, 11, 15,  7,  9, 15, 13,  7,  9, 13,  5,  7,  9,  5,  1,  7,  9,  1,  3,  7,  9,  3, 11,  7, /* +X */ \
	 5, 13,  9,  8,  5,  9,  1,  8,  5,  1,  0,  8,  5,  0,  4,  8,  5,  4, 12,  8,  5, 12, 13,  8, /* -Y */ \
	10,  2,  6,  7, 10,  6, 14,  7, 10, 14, 15,  7, 10, 15, 11,  7, 10, 11,  3,  7, 10,  3,  2,  7, /* +Y */ \
	 8,  0,  2,  3,  8,  2, 10,  3,  8, 10, 11,  3,  8, 11,  9,  3,  8,  9,  1,  3,  8,  1,  0,  3, /* -Z */ \
	12, 14,  6,  7, 12,  6,  4,  7, 12,  4,  5,  7, 12,  5, 13,  7, 12, 13, 15,  7, 12, 15, 14,  7, /* +Z */ \
	 0,  1,  5,  7,  0,  3,  1,  7,  0,  2,  3,  7,  0,  6,  2,  7,  0,  4,  6,  7,  0,  5,  4,  7, /* -W */ \
	 8, 10, 14, 15,  8, 14, 12, 15,  8, 12, 13, 15,  8, 13,  9, 15,  8,  9, 11, 15,  8, 11, 10, 15, /* +W */ \
}

// C++ hex floating point literals suck, but this means that each side is 5/16ths of the texture in size.
#define BOX_40_CELL_TEXTURE_MAP_CROSS_ISLAND PackedVector3Array { \
	/* -X */ \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.5p0, 0x0.Ap0), \
	Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), \
	/* +X */ \
	Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), \
	Vector3(0x0.Fp0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.5p0), \
	/* -Y */ \
	Vector3(0x0.5p0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.0p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.5p0), \
	/* +Y */ \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	/* -Z */ \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.0p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.0p0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.0p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.0p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.0p0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	/* +Z */ \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Fp0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.Fp0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), \
	/* -W */ \
	Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), Vector3(0x0.Bp0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.Bp0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.Bp0), \
	Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), Vector3(0x1.0p0, 0x0.Bp0, 0x0.Bp0), Vector3(0x1.0p0, 0x1.0p0, 0x0.Bp0), Vector3(0x1.0p0, 0x0.Bp0, 0x1.0p0), \
	Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), Vector3(0x0.Bp0, 0x1.0p0, 0x0.Bp0), Vector3(0x0.Bp0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.Bp0), \
	Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), Vector3(0x0.Bp0, 0x0.Bp0, 0x1.0p0), Vector3(0x1.0p0, 0x0.Bp0, 0x1.0p0), Vector3(0x0.Bp0, 0x1.0p0, 0x1.0p0), \
	Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.Bp0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.Bp0), Vector3(0x1.0p0, 0x0.Bp0, 0x1.0p0), \
	/* +W */ \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), \
}

#define BOX_40_CELL_TEXTURE_MAP_FILL_EACH_SIDE PackedVector3Array { \
	/* -X */ \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	/* +X */ \
	Vector3(0.0, 1.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.0, 0.0), \
	Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 1.0, 0.0), Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), \
	Vector3(1.0, 0.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.0, 0.0), \
	/* -Y */ \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 0.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(1.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 0.0, 0.0), \
	/* +Y */ \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 0.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(1.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 0.0, 0.0), \
	/* -Z */ \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	/* +Z */ \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	/* -W */ \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.0, 1.0), \
	/* +W */ \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.0, 1.0) \
}

#define BOX_40_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID PackedVector3Array { \
	/* -X */ \
	Vector3(0.0, 1.0, 1.0), Vector3(0.5, 1.0, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.5, 1.0), Vector3(0.5, 0.5, 1.0), Vector3(0.0, 0.5, 0.5), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.5, 1.0, 1.0), Vector3(0.5, 1.0, 0.5), Vector3(0.5, 0.5, 1.0), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 1.0, 0.5), \
	Vector3(0.5, 0.5, 0.5), Vector3(0.5, 1.0, 0.5), Vector3(0.5, 0.5, 1.0), Vector3(0.0, 0.5, 0.5), \
	/* +X */ \
	Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.0, 0.5), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 0.5, 0.0), Vector3(1.0, 0.5, 0.0), Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.0, 0.0), \
	Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.5, 0.0, 0.5), \
	Vector3(1.0, 0.0, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.0, 0.0), \
	/* -Y */ \
	Vector3(1.0, 0.5, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.5, 1.0), Vector3(1.0, 0.0, 1.0), \
	Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.5, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(0.5, 0.5, 1.0), \
	Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.0, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(1.0, 0.0, 1.0), \
	Vector3(1.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), Vector3(0.5, 0.0, 0.5), \
	Vector3(0.5, 0.0, 1.0), Vector3(0.5, 0.0, 0.5), Vector3(1.0, 0.0, 1.0), Vector3(0.5, 0.5, 1.0), \
	/* +Y */ \
	Vector3(0.0, 0.5, 0.5), Vector3(0.5, 1.0, 0.5), Vector3(0.5, 0.5, 0.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(0.0, 0.5, 0.5), Vector3(0.0, 0.5, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.5, 0.5, 0.0), \
	Vector3(0.0, 0.5, 0.5), Vector3(0.0, 1.0, 0.5), Vector3(0.5, 1.0, 0.5), Vector3(0.0, 1.0, 0.0), \
	Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 1.0, 0.5), \
	Vector3(0.5, 1.0, 0.0), Vector3(0.5, 1.0, 0.5), Vector3(0.0, 1.0, 0.0), Vector3(0.5, 0.5, 0.0), \
	/* -Z */ \
	Vector3(0.5, 1.0, 0.5), Vector3(1.0, 1.0, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 1.0, 0.5), Vector3(0.5, 0.5, 0.5), Vector3(1.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.0), \
	Vector3(0.5, 1.0, 0.5), Vector3(1.0, 1.0, 0.5), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 1.0, 0.5), Vector3(0.5, 1.0, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(1.0, 0.5, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.0), \
	/* +Z */ \
	Vector3(0.5, 0.0, 0.5), Vector3(0.0, 0.0, 1.0), Vector3(0.5, 0.5, 1.0), Vector3(0.0, 0.5, 0.5), \
	Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.5, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), \
	Vector3(0.5, 0.0, 0.5), Vector3(0.0, 0.0, 0.5), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.5, 0.5), \
	Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.0, 1.0), Vector3(0.5, 0.5, 1.0), Vector3(0.0, 0.0, 1.0), \
	Vector3(0.0, 0.5, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), \
	/* -W */ \
	Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.5, 0.5), Vector3(0.5, 1.0, 0.5), Vector3(0.5, 0.5, 1.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(0.5, 1.0, 1.0), Vector3(0.5, 0.5, 1.0), Vector3(0.5, 1.0, 0.5), \
	Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.5, 1.0), Vector3(1.0, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.5), Vector3(0.5, 1.0, 0.5), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 0.5, 0.5), Vector3(1.0, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), Vector3(0.5, 1.0, 0.5), \
	/* +W */ \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.5, 0.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.5, 0.0, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.0, 0.5), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.5, 0.0), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(0.0, 0.5, 0.5), \
	Vector3(0.5, 0.5, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.0, 0.5) \
}

#define _0_3RDS 0.0
#define _1_3RDS (1.0 / 3.0)
#define _2_3RDS (2.0 / 3.0)
#define _3_3RDS 1.0

#define BOX_40_CELL_TEXTURE_MAP_LONG_CROSS PackedVector3Array { \
	/* -X */ \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.0p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.0p0, _1_3RDS, _2_3RDS), \
	Vector3(0x0.0p0, _2_3RDS, _2_3RDS), Vector3(0x0.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), \
	/* +X */ \
	Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), \
	Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
	/* -Y */ \
	Vector3(0x0.4p0, _0_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _0_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _0_3RDS, _2_3RDS), Vector3(0x0.4p0, _0_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _0_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _0_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _0_3RDS, _2_3RDS), Vector3(0x0.8p0, _0_3RDS, _2_3RDS), Vector3(0x0.8p0, _0_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _0_3RDS, _1_3RDS), \
	/* +Y */ \
	Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _3_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _3_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _3_3RDS, _2_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), Vector3(0x0.4p0, _3_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _3_3RDS, _1_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), Vector3(0x0.4p0, _3_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
	/* -Z */ \
	Vector3(0x0.8p0, _1_3RDS, _0_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _0_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _0_3RDS), Vector3(0x0.8p0, _2_3RDS, _0_3RDS), Vector3(0x0.4p0, _2_3RDS, _0_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _0_3RDS), Vector3(0x0.4p0, _1_3RDS, _0_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _0_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _0_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _0_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
	/* +Z */ \
	Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _3_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _3_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _3_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), Vector3(0x0.4p0, _1_3RDS, _3_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _3_3RDS), Vector3(0x0.4p0, _1_3RDS, _3_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), \
	/* -W */ \
	Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), Vector3(0x1.0p0, _1_3RDS, _2_3RDS), Vector3(0x1.0p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x1.0p0, _1_3RDS, _1_3RDS), Vector3(0x1.0p0, _2_3RDS, _1_3RDS), Vector3(0x1.0p0, _1_3RDS, _2_3RDS), \
	Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), Vector3(0x1.0p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), Vector3(0x1.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x1.0p0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), Vector3(0x1.0p0, _2_3RDS, _1_3RDS), Vector3(0x1.0p0, _1_3RDS, _2_3RDS), \
	/* +W */ \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), \
}

#define BOX_48_CELL_TEXTURE_MAP_CROSS_ISLAND PackedVector3Array { \
	/* -X */ \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
	/* +X */ \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	/* -Y */ \
	Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.0p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.0p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	/* +Y */ \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
	/* -Z */ \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.0p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.0p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.0p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), \
	/* +Z */ \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Fp0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), \
	/* -W */ \
	Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.Bp0, 0x1.0p0, 0x1.0p0), Vector3(0x0.Bp0, 0x1.0p0, 0x0.Bp0), Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), \
	Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.Bp0, 0x0.Bp0, 0x1.0p0), Vector3(0x0.Bp0, 0x1.0p0, 0x1.0p0), Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), \
	Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.Bp0, 0x1.0p0), Vector3(0x0.Bp0, 0x0.Bp0, 0x1.0p0), Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), \
	Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.Bp0, 0x0.Bp0), Vector3(0x1.0p0, 0x0.Bp0, 0x1.0p0), Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), \
	Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.Bp0), Vector3(0x1.0p0, 0x0.Bp0, 0x0.Bp0), Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), \
	Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.Bp0, 0x1.0p0, 0x0.Bp0), Vector3(0x1.0p0, 0x1.0p0, 0x0.Bp0), Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), \
	/* +W */ \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), \
}

#define BOX_48_CELL_TEXTURE_MAP_FILL_EACH_SIDE PackedVector3Array { \
	/* -X */ \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 1.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), \
	Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 1.0), \
	/* +X */ \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	/* -Y */ \
	Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 1.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	Vector3(1.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), \
	/* +Y */ \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	/* -Z */ \
	Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), \
	/* +Z */ \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	/* -W */ \
	Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 0.0, 0.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 0.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, 0.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(0.0, 0.0, 0.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0), \
	Vector3(1.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 0.0, 0.0), \
	/* +W */ \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0) \
}

#define BOX_48_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID PackedVector3Array { \
	/* -X */ \
	Vector3(0.0, 1.0, 1.0), Vector3(0.5, 1.0, 1.0), Vector3(0.5, 1.0, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.5, 1.0, 0.5), Vector3(0.0, 1.0, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.0, 1.0, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.5, 0.5), Vector3(0.0, 0.5, 1.0), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.0, 0.5, 1.0), Vector3(0.5, 0.5, 1.0), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 1.0, 1.0), Vector3(0.5, 0.5, 1.0), Vector3(0.5, 1.0, 1.0), Vector3(0.5, 0.5, 0.5), \
	/* +X */ \
	Vector3(0.5, 0.0, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.5, 0.5), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 0.0, 0.0), Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 0.0, 0.0), Vector3(0.5, 0.0, 0.5), Vector3(1.0, 0.0, 0.5), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 0.0, 0.0), Vector3(1.0, 0.0, 0.5), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 0.5, 0.0), Vector3(1.0, 0.5, 0.5), \
	Vector3(0.5, 0.0, 0.0), Vector3(1.0, 0.5, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(1.0, 0.5, 0.5), \
	/* -Y */ \
	Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.0, 1.0), Vector3(0.5, 0.5, 1.0), Vector3(1.0, 0.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), Vector3(1.0, 0.5, 1.0), Vector3(1.0, 0.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(1.0, 0.5, 1.0), Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.0, 0.5), Vector3(1.0, 0.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(1.0, 0.0, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(1.0, 0.0, 1.0), \
	/* +Y */ \
	Vector3(0.0, 0.5, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 1.0, 0.5), Vector3(0.5, 1.0, 0.5), \
	Vector3(0.0, 0.5, 0.0), Vector3(0.0, 1.0, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 1.0, 0.5), \
	Vector3(0.0, 0.5, 0.0), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.5), Vector3(0.5, 1.0, 0.5), \
	Vector3(0.0, 0.5, 0.0), Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 1.0, 0.5), \
	Vector3(0.0, 0.5, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 1.0, 0.0), Vector3(0.5, 1.0, 0.5), \
	Vector3(0.0, 0.5, 0.0), Vector3(0.5, 1.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.5, 1.0, 0.5), \
	/* -Z */ \
	Vector3(1.0, 1.0, 0.0), Vector3(1.0, 1.0, 0.5), Vector3(1.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.5, 0.0), Vector3(0.5, 0.5, 0.5), \
	Vector3(1.0, 1.0, 0.0), Vector3(1.0, 0.5, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.5, 0.5), \
	Vector3(1.0, 1.0, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 1.0, 0.0), Vector3(0.5, 0.5, 0.5), \
	Vector3(1.0, 1.0, 0.0), Vector3(0.5, 1.0, 0.0), Vector3(0.5, 1.0, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(1.0, 1.0, 0.0), Vector3(0.5, 1.0, 0.5), Vector3(1.0, 1.0, 0.5), Vector3(0.5, 0.5, 0.5), \
	/* +Z */ \
	Vector3(0.0, 0.0, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.0, 0.5, 1.0), Vector3(0.5, 0.5, 1.0), \
	Vector3(0.0, 0.0, 0.5), Vector3(0.0, 0.5, 1.0), Vector3(0.0, 0.0, 1.0), Vector3(0.5, 0.5, 1.0), \
	Vector3(0.0, 0.0, 0.5), Vector3(0.0, 0.0, 1.0), Vector3(0.5, 0.0, 1.0), Vector3(0.5, 0.5, 1.0), \
	Vector3(0.0, 0.0, 0.5), Vector3(0.5, 0.0, 1.0), Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.5, 1.0), \
	Vector3(0.0, 0.0, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), \
	Vector3(0.0, 0.0, 0.5), Vector3(0.5, 0.5, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), \
	/* -W */ \
	Vector3(0.5, 0.5, 0.5), Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.5, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(1.0, 1.0, 0.5), Vector3(1.0, 0.5, 0.5), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(0.5, 1.0, 0.5), Vector3(1.0, 1.0, 0.5), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(0.5, 1.0, 1.0), Vector3(0.5, 1.0, 0.5), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.5, 1.0), Vector3(0.5, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.5, 0.5, 0.5), Vector3(1.0, 0.5, 1.0), Vector3(0.5, 0.5, 1.0), Vector3(1.0, 1.0, 1.0), \
	/* +W */ \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.5, 0.0), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.5, 0.5), Vector3(0.0, 0.0, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.5), Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.5, 0.0, 0.5), Vector3(0.5, 0.0, 0.0), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.5, 0.0, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.0, 0.0, 0.0), Vector3(0.5, 0.5, 0.0), Vector3(0.0, 0.5, 0.0), Vector3(0.5, 0.5, 0.5) \
}

#define BOX_48_CELL_TEXTURE_MAP_LONG_CROSS PackedVector3Array { \
	/* -X */ \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.0p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.0p0, _2_3RDS, _1_3RDS), Vector3(0x0.0p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _2_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _2_3RDS, _2_3RDS), \
	/* +X */ \
	Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	/* -Y */ \
	Vector3(0x0.8p0, _0_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.8p0, _0_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _0_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.8p0, _0_3RDS, _2_3RDS), Vector3(0x0.8p0, _0_3RDS, _1_3RDS), Vector3(0x0.4p0, _0_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.8p0, _0_3RDS, _2_3RDS), Vector3(0x0.4p0, _0_3RDS, _1_3RDS), Vector3(0x0.4p0, _0_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.8p0, _0_3RDS, _2_3RDS), Vector3(0x0.4p0, _0_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	Vector3(0x0.8p0, _0_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _1_3RDS), \
	/* +Y */ \
	Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _3_3RDS, _1_3RDS), Vector3(0x0.4p0, _3_3RDS, _2_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _3_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _3_3RDS, _1_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _3_3RDS, _1_3RDS), Vector3(0x0.4p0, _3_3RDS, _1_3RDS), Vector3(0x0.8p0, _3_3RDS, _2_3RDS), \
	/* -Z */ \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _0_3RDS), Vector3(0x0.4p0, _2_3RDS, _0_3RDS), Vector3(0x0.8p0, _2_3RDS, _0_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _0_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _0_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _0_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _0_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _0_3RDS), Vector3(0x0.8p0, _2_3RDS, _0_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _0_3RDS), Vector3(0x0.4p0, _1_3RDS, _0_3RDS), Vector3(0x0.8p0, _2_3RDS, _0_3RDS), \
	/* +Z */ \
	Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _3_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _3_3RDS), Vector3(0x0.4p0, _1_3RDS, _3_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _3_3RDS), Vector3(0x0.8p0, _1_3RDS, _3_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _3_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), \
	/* -W */ \
	Vector3(0x1.0p0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
	Vector3(0x1.0p0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
	Vector3(0x1.0p0, _2_3RDS, _2_3RDS), Vector3(0x1.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
	Vector3(0x1.0p0, _2_3RDS, _2_3RDS), Vector3(0x1.0p0, _1_3RDS, _1_3RDS), Vector3(0x1.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
	Vector3(0x1.0p0, _2_3RDS, _2_3RDS), Vector3(0x1.0p0, _2_3RDS, _1_3RDS), Vector3(0x1.0p0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
	Vector3(0x1.0p0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), Vector3(0x1.0p0, _2_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
	/* +W */ \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), \
}

#define BOX_POLY_CELL_INDICES Vector<Vector<PackedInt32Array>> { \
	Vector<PackedInt32Array> { \
		/* The order within MUST form a continuous closed loop of edges. */ \
		/* XY faces */ \
		PackedInt32Array { 0,  8,  1,  9}, PackedInt32Array { 2, 11,  3, 10}, \
		PackedInt32Array { 4, 12,  5, 13}, PackedInt32Array { 6, 15,  7, 14}, \
		/* XZ faces */ \
		PackedInt32Array { 0, 17,  2, 16}, PackedInt32Array { 1, 18,  3, 19}, \
		PackedInt32Array { 4, 21,  6, 20}, PackedInt32Array { 5, 22,  7, 23}, \
		/* YZ faces */ \
		PackedInt32Array { 8, 16, 10, 18}, PackedInt32Array { 9, 19, 11, 17}, \
		PackedInt32Array {12, 20, 14, 22}, PackedInt32Array {13, 23, 15, 21}, \
		/* XW faces */ \
		PackedInt32Array { 0, 24,  4, 25}, PackedInt32Array {28,  2, 29,  6}, \
		PackedInt32Array { 1, 26,  5, 27}, PackedInt32Array {30,  3, 31,  7}, \
		/* YW faces */ \
		PackedInt32Array {24,  8, 26, 12}, PackedInt32Array {10, 28, 14, 30}, \
		PackedInt32Array {25,  9, 27, 13}, PackedInt32Array {11, 29, 15, 31}, \
		/* ZW faces */ \
		PackedInt32Array {24, 16, 28, 20}, PackedInt32Array {17, 25, 21, 29}, \
		PackedInt32Array {26, 18, 30, 22}, PackedInt32Array {19, 27, 23, 31}, \
	}, \
	Vector<PackedInt32Array> { \
		/* The first two need to be connected, the order within determines */ \
		/* orientation, and the order without determines tetrahedralization. */ \
		PackedInt32Array {0,  5,  1,  4,  8,  9}, /* XYZ cell facing -W */ \
		PackedInt32Array {6, 12,  4, 13, 20, 21}, /* XZW cell facing -Y */ \
		PackedInt32Array {0, 12,  2, 14, 16, 18}, /* XYW cell facing -Z */ \
		PackedInt32Array {1, 13,  3, 15, 17, 19}, /* XYW cell facing +Z */ \
		PackedInt32Array {9, 19, 11, 18, 21, 23}, /* YZW cell facing +X */ \
		PackedInt32Array {8, 16, 10, 17, 20, 22}, /* YZW cell facing -X */ \
		PackedInt32Array {7, 15,  5, 14, 22, 23}, /* XZW cell facing +Y */ \
		PackedInt32Array {2,  6,  3,  7, 10, 11}, /* XYZ cell facing +W */ \
	}, \
	Vector<PackedInt32Array> { \
		/* The first two need to be connected, otherwise the order does not matter. */ \
		PackedInt32Array {0, 1, 2, 3, 4, 5, 6, 7} /* XYZW hyper-cell */ \
	} \
}

// The order of these needs to match the data in BOX_POLY_CELL_INDICES.
#define BOX_POLY_CELL_BOUNDARY_NORMALS PackedVector4Array { \
	Vector4(0.0f, 0.0f, 0.0f, -1.0f), /* -W */ \
	Vector4(0.0f, -1.0f, 0.0f, 0.0f), /* -Y */ \
	Vector4(0.0f, 0.0f, -1.0f, 0.0f), /* -Z */ \
	Vector4(0.0f, 0.0f, +1.0f, 0.0f), /* +Z */ \
	Vector4(+1.0f, 0.0f, 0.0f, 0.0f), /* +X */ \
	Vector4(-1.0f, 0.0f, 0.0f, 0.0f), /* -X */ \
	Vector4(0.0f, +1.0f, 0.0f, 0.0f), /* +Y */ \
	Vector4(0.0f, 0.0f, 0.0f, +1.0f), /* +W */ \
}

#define REPEAT_EIGHT_TIMES(m) m, m, m, m, m, m, m, m
#define BOX_POLY_CELL_VERTEX_NORMALS Vector<PackedVector4Array> { \
	PackedVector4Array { REPEAT_EIGHT_TIMES(Vector4(0.0f, 0.0f, 0.0f, -1.0f)) }, /* -W */ \
	PackedVector4Array { REPEAT_EIGHT_TIMES(Vector4(0.0f, -1.0f, 0.0f, 0.0f)) }, /* -Y */ \
	PackedVector4Array { REPEAT_EIGHT_TIMES(Vector4(0.0f, 0.0f, -1.0f, 0.0f)) }, /* -Z */ \
	PackedVector4Array { REPEAT_EIGHT_TIMES(Vector4(0.0f, 0.0f, +1.0f, 0.0f)) }, /* +Z */ \
	PackedVector4Array { REPEAT_EIGHT_TIMES(Vector4(+1.0f, 0.0f, 0.0f, 0.0f)) }, /* +X */ \
	PackedVector4Array { REPEAT_EIGHT_TIMES(Vector4(-1.0f, 0.0f, 0.0f, 0.0f)) }, /* -X */ \
	PackedVector4Array { REPEAT_EIGHT_TIMES(Vector4(0.0f, +1.0f, 0.0f, 0.0f)) }, /* +Y */ \
	PackedVector4Array { REPEAT_EIGHT_TIMES(Vector4(0.0f, 0.0f, 0.0f, +1.0f)) }, /* +W */ \
}

// The order of these needs to match the data in BOX_POLY_CELL_INDICES.
#define BOX_POLY_CELL_POLY_TEXTURE_MAP_CROSS_ISLAND Vector<PackedVector3Array> { \
	PackedVector3Array { /* -W */ \
		Vector3(0x1.0p0, 0x0.Bp0, 0x0.Bp0), Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), \
		Vector3(0x1.0p0, 0x1.0p0, 0x0.Bp0), Vector3(0x0.Bp0, 0x1.0p0, 0x0.Bp0), \
		Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.Bp0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x1.0p0, 0x0.Bp0, 0x1.0p0), Vector3(0x0.Bp0, 0x0.Bp0, 0x1.0p0), \
	}, \
	PackedVector3Array { /* -Y */ \
		Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), \
		Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), \
		Vector3(0x0.5p0, 0x0.0p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.5p0), \
		Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.0p0, 0x0.Ap0), \
	}, \
	PackedVector3Array { /* -Z */ \
		Vector3(0x0.5p0, 0x0.5p0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.0p0), \
		Vector3(0x0.5p0, 0x0.Ap0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), \
		Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), \
		Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	}, \
	PackedVector3Array { /* +Z */ \
		Vector3(0x0.5p0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Fp0), \
		Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Fp0), \
		Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), \
		Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), \
	}, \
	PackedVector3Array { /* +X */ \
		Vector3(0x0.Fp0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.5p0), \
		Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.Ap0), \
		Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), \
		Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	}, \
	PackedVector3Array { /* -X */ \
		Vector3(0x0.0p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.5p0), \
		Vector3(0x0.0p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
		Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), \
		Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), \
	}, \
	PackedVector3Array { /* +Y */ \
		Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
		Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), \
		Vector3(0x0.5p0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
		Vector3(0x0.5p0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.5p0), \
	}, \
	PackedVector3Array { /* +W */ \
		Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), \
		Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
		Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), \
		Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), \
	}, \
}

#define BOX_POLY_CELL_POLY_TEXTURE_MAP_FILL_EACH_SIDE Vector<PackedVector3Array> { \
	PackedVector3Array { /* -W */ \
		Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), \
		Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), \
		Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), \
	}, \
	PackedVector3Array { /* -Y */ \
		Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), \
		Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), \
		Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), \
	}, \
	PackedVector3Array { /* -Z */ \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), \
		Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), \
		Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), \
	}, \
	PackedVector3Array { /* +Z */ \
		Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), \
		Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), \
		Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), \
	}, \
	PackedVector3Array { /* +X */ \
		Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), \
		Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), \
	}, \
	PackedVector3Array { /* -X */ \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), \
		Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), \
		Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), \
	}, \
	PackedVector3Array { /* +Y */ \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), \
		Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), \
	}, \
	PackedVector3Array { /* +W */ \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), \
		Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), \
		Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), \
		Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), \
	}, \
}


#define BOX_POLY_CELL_POLY_TEXTURE_MAP_COMPACT_2X2X2_GRID Vector<PackedVector3Array> { \
	PackedVector3Array { /* -W */ \
		Vector3(0x0.8p0, 0x0.8p0, 0x0.8p0), Vector3(0x1.0p0, 0x0.8p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x1.0p0, 0x0.8p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x0.8p0, 0x0.8p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.8p0, 0x1.0p0), \
	}, \
	PackedVector3Array { /* -Y */ \
		Vector3(0x0.8p0, 0x0.0p0, 0x0.8p0), Vector3(0x1.0p0, 0x0.0p0, 0x0.8p0), \
		Vector3(0x1.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x0.8p0, 0x0.0p0, 0x1.0p0), \
		Vector3(0x0.8p0, 0x0.8p0, 0x0.8p0), Vector3(0x1.0p0, 0x0.8p0, 0x0.8p0), \
		Vector3(0x1.0p0, 0x0.8p0, 0x1.0p0), Vector3(0x0.8p0, 0x0.8p0, 0x1.0p0), \
	}, \
	PackedVector3Array { /* -Z */ \
		Vector3(0x0.8p0, 0x0.8p0, 0x0.8p0), Vector3(0x1.0p0, 0x0.8p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x1.0p0, 0x0.8p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x0.8p0, 0x0.0p0), Vector3(0x1.0p0, 0x0.8p0, 0x0.0p0), \
		Vector3(0x0.8p0, 0x1.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.0p0), \
	}, \
	PackedVector3Array { /* +Z */ \
		Vector3(0x0.0p0, 0x0.0p0, 0x1.0p0), Vector3(0x0.8p0, 0x0.0p0, 0x1.0p0), \
		Vector3(0x0.8p0, 0x0.8p0, 0x1.0p0), Vector3(0x0.0p0, 0x0.8p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.8p0), Vector3(0x0.8p0, 0x0.0p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x0.8p0, 0x0.8p0), Vector3(0x0.0p0, 0x0.8p0, 0x0.8p0), \
	}, \
	PackedVector3Array { /* +X */ \
		Vector3(0x1.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x1.0p0, 0x0.8p0, 0x0.0p0), \
		Vector3(0x1.0p0, 0x0.8p0, 0x0.8p0), Vector3(0x1.0p0, 0x0.0p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x0.0p0, 0x0.8p0), Vector3(0x0.8p0, 0x0.8p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x0.0p0, 0x0.0p0), Vector3(0x0.8p0, 0x0.8p0, 0x0.0p0), \
	}, \
	PackedVector3Array { /* -X */ \
		Vector3(0x0.8p0, 0x0.8p0, 0x0.8p0), Vector3(0x0.8p0, 0x1.0p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x0.8p0, 0x1.0p0), Vector3(0x0.8p0, 0x1.0p0, 0x1.0p0), \
		Vector3(0x0.0p0, 0x0.8p0, 0x0.8p0), Vector3(0x0.0p0, 0x1.0p0, 0x0.8p0), \
		Vector3(0x0.0p0, 0x0.8p0, 0x1.0p0), Vector3(0x0.0p0, 0x1.0p0, 0x1.0p0), \
	}, \
	PackedVector3Array { /* +Y */ \
		Vector3(0x0.0p0, 0x0.8p0, 0x0.0p0), Vector3(0x0.8p0, 0x0.8p0, 0x0.0p0), \
		Vector3(0x0.0p0, 0x0.8p0, 0x0.8p0), Vector3(0x0.8p0, 0x0.8p0, 0x0.8p0), \
		Vector3(0x0.0p0, 0x1.0p0, 0x0.8p0), Vector3(0x0.8p0, 0x1.0p0, 0x0.8p0), \
		Vector3(0x0.0p0, 0x1.0p0, 0x0.0p0), Vector3(0x0.8p0, 0x1.0p0, 0x0.0p0), \
	}, \
	PackedVector3Array { /* +W */ \
		Vector3(0x0.0p0, 0x0.0p0, 0x0.0p0), Vector3(0x0.8p0, 0x0.0p0, 0x0.0p0), \
		Vector3(0x0.0p0, 0x0.8p0, 0x0.0p0), Vector3(0x0.8p0, 0x0.8p0, 0x0.0p0), \
		Vector3(0x0.8p0, 0x0.0p0, 0x0.8p0), Vector3(0x0.0p0, 0x0.0p0, 0x0.8p0), \
		Vector3(0x0.8p0, 0x0.8p0, 0x0.8p0), Vector3(0x0.0p0, 0x0.8p0, 0x0.8p0), \
	}, \
}

// Generated from island order: [7, 4, 5, 6, 1, 3, 2, 0]
#define BOX_POLY_CELL_POLY_TEXTURE_MAP_LONG_CROSS Vector<PackedVector3Array> { \
	PackedVector3Array { /* -W */ \
		Vector3(0x1.0p0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), \
		Vector3(0x1.0p0, _2_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), \
		Vector3(0x1.0p0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
		Vector3(0x1.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), \
	}, \
	PackedVector3Array { /* -Y */ \
		Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), \
		Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), \
		Vector3(0x0.4p0, 0x0.0p0, _1_3RDS), Vector3(0x0.8p0, 0x0.0p0, _1_3RDS), \
		Vector3(0x0.8p0, 0x0.0p0, _2_3RDS), Vector3(0x0.4p0, 0x0.0p0, _2_3RDS), \
	}, \
	PackedVector3Array { /* -Z */ \
		Vector3(0x0.4p0, _1_3RDS, 0x0.0p0), Vector3(0x0.8p0, _1_3RDS, 0x0.0p0), \
		Vector3(0x0.4p0, _2_3RDS, 0x0.0p0), Vector3(0x0.8p0, _2_3RDS, 0x0.0p0), \
		Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), \
		Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
	}, \
	PackedVector3Array { /* +Z */ \
		Vector3(0x0.4p0, _1_3RDS, 0x1.0p0), Vector3(0x0.8p0, _1_3RDS, 0x1.0p0), \
		Vector3(0x0.8p0, _2_3RDS, 0x1.0p0), Vector3(0x0.4p0, _2_3RDS, 0x1.0p0), \
		Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), \
		Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), \
	}, \
	PackedVector3Array { /* +X */ \
		Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), \
		Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), \
		Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), \
		Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
	}, \
	PackedVector3Array { /* -X */ \
		Vector3(0x0.0p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _2_3RDS, _1_3RDS), \
		Vector3(0x0.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _2_3RDS), \
		Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _2_3RDS, _1_3RDS), \
		Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), \
	}, \
	PackedVector3Array { /* +Y */ \
		Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
		Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), \
		Vector3(0x0.4p0, 0x1.0p0, _2_3RDS), Vector3(0x0.8p0, 0x1.0p0, _2_3RDS), \
		Vector3(0x0.4p0, 0x1.0p0, _1_3RDS), Vector3(0x0.8p0, 0x1.0p0, _1_3RDS), \
	}, \
	PackedVector3Array { /* +W */ \
		Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), \
		Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _1_3RDS), \
		Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), \
		Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), \
	}, \
}

// These are the same for all box types: poly, tetra, wire.
#define BOX_EDGE_INDICES PackedInt32Array { \
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, /* X */ \
	0, 2, 1, 3, 4, 6, 5, 7, 8, 10, 9, 11, 12, 14, 13, 15, /* Y */ \
	0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15, /* Z */ \
	0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15, /* W */ \
}
/* clang-format on */
