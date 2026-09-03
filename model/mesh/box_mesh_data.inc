#pragma once

#include "../../godot_4d_defines.h"

// This file contains pre-computed data for 4D box meshes.
// We have to use #define below because static constants will crash GDExtension.
// This code is much harder to read when clang-format messes with the
// manual alignment, so disable clang-format just for this file.
/* clang-format off */
#define BOX_NORMAL_VALUES PackedVector4Array { \
	Vector4(-1.0f, 0.0f, 0.0f, 0.0f), /* -X */ \
	Vector4(+1.0f, 0.0f, 0.0f, 0.0f), /* +X */ \
	Vector4(0.0f, -1.0f, 0.0f, 0.0f), /* -Y */ \
	Vector4(0.0f, +1.0f, 0.0f, 0.0f), /* +Y */ \
	Vector4(0.0f, 0.0f, -1.0f, 0.0f), /* -Z */ \
	Vector4(0.0f, 0.0f, +1.0f, 0.0f), /* +Z */ \
	Vector4(0.0f, 0.0f, 0.0f, -1.0f), /* -W */ \
	Vector4(0.0f, 0.0f, 0.0f, +1.0f), /* +W */ \
}

#define BOX_TEXTURE_MAP_FILL_EACH_SIDE_VALUES PackedVector3Array { \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), \
}

#define BOX_TEXTURE_MAP_COMPACT_2X2X2_GRID_VALUES PackedVector3Array { \
	Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 0.0), \
	Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 1.0), Vector3(0.0, 1.0, 1.0), Vector3(1.0, 1.0, 1.0), \
	Vector3(0.5, 0.0, 0.0), Vector3(0.0, 0.5, 0.0), Vector3(0.0, 0.0, 0.5), Vector3(0.5, 0.5, 0.5), \
	Vector3(0.5, 0.5, 0.0), Vector3(0.5, 0.0, 0.5), Vector3(0.0, 0.5, 0.5), Vector3(0.5, 1.0, 0.5), \
	Vector3(0.5, 0.5, 1.0), Vector3(0.0, 0.5, 1.0), Vector3(0.5, 1.0, 1.0), Vector3(0.0, 1.0, 0.5), \
	Vector3(1.0, 0.5, 0.5), Vector3(1.0, 0.5, 0.0), Vector3(1.0, 0.0, 0.5), Vector3(1.0, 0.5, 1.0), \
	Vector3(0.5, 0.0, 1.0), Vector3(0.5, 1.0, 0.0), Vector3(1.0, 1.0, 0.5), \
}

// C++ hex floating point literals suck, but this means that each side is 5/16ths of the texture in size.
#define BOX_TEXTURE_MAP_CROSS_ISLAND_VALUES PackedVector3Array { \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.5p0), \
	Vector3(0x0.5p0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.0p0, 0x0.5p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Ap0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	Vector3(0x0.Fp0, 0x0.Ap0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.5p0), Vector3(0x0.Fp0, 0x0.5p0, 0x0.Ap0), \
	Vector3(0x0.5p0, 0x0.0p0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.5p0), Vector3(0x0.5p0, 0x0.0p0, 0x0.5p0), Vector3(0x0.Ap0, 0x0.0p0, 0x0.Ap0), \
	Vector3(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.5p0), Vector3(0x0.5p0, 0x0.Fp0, 0x0.Ap0), Vector3(0x0.Ap0, 0x0.Fp0, 0x0.5p0), \
	Vector3(0x0.Ap0, 0x0.5p0, 0x0.0p0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.0p0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.0p0), Vector3(0x0.5p0, 0x0.5p0, 0x0.0p0), \
	Vector3(0x0.5p0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), Vector3(0x0.Ap0, 0x0.5p0, 0x0.Fp0), Vector3(0x0.5p0, 0x0.Ap0, 0x0.Fp0), \
	Vector3(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), Vector3(0x0.Bp0, 0x1.0p0, 0x1.0p0), Vector3(0x1.0p0, 0x0.Bp0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x0.Bp0), \
	Vector3(0x1.0p0, 0x0.Bp0, 0x0.Bp0), Vector3(0x0.Bp0, 0x1.0p0, 0x0.Bp0), Vector3(0x0.Bp0, 0x0.Bp0, 0x1.0p0), Vector3(0x1.0p0, 0x1.0p0, 0x1.0p0), \
}

#define _0_3RDS 0.0
#define _1_3RDS (1.0 / 3.0)
#define _2_3RDS (2.0 / 3.0)
#define _3_3RDS 1.0

#define BOX_TEXTURE_MAP_LONG_CROSS_VALUES PackedVector3Array { \
	Vector3(0x0.4p0, _1_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _2_3RDS), Vector3(0x0.4p0, _2_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _1_3RDS), \
	Vector3(0x0.4p0, _2_3RDS, _1_3RDS), Vector3(0x0.0p0, _1_3RDS, _1_3RDS), Vector3(0x0.4p0, _1_3RDS, _2_3RDS), Vector3(0x0.0p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _1_3RDS, _2_3RDS), Vector3(0x0.Cp0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.Cp0, _2_3RDS, _1_3RDS), Vector3(0x0.8p0, _2_3RDS, _2_3RDS), Vector3(0x0.8p0, _1_3RDS, _1_3RDS), Vector3(0x0.Cp0, _1_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, _0_3RDS, _2_3RDS), Vector3(0x0.8p0, _0_3RDS, _1_3RDS), Vector3(0x0.4p0, _0_3RDS, _1_3RDS), Vector3(0x0.8p0, _0_3RDS, _2_3RDS), \
	Vector3(0x0.8p0, _3_3RDS, _2_3RDS), Vector3(0x0.4p0, _3_3RDS, _1_3RDS), Vector3(0x0.4p0, _3_3RDS, _2_3RDS), Vector3(0x0.8p0, _3_3RDS, _1_3RDS), \
	Vector3(0x0.8p0, _1_3RDS, _0_3RDS), Vector3(0x0.4p0, _2_3RDS, _0_3RDS), Vector3(0x0.8p0, _2_3RDS, _0_3RDS), Vector3(0x0.4p0, _1_3RDS, _0_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, _3_3RDS), Vector3(0x0.8p0, _2_3RDS, _3_3RDS), Vector3(0x0.8p0, _1_3RDS, _3_3RDS), Vector3(0x0.4p0, _2_3RDS, _3_3RDS), \
	Vector3(0x1.0p0, _1_3RDS, _2_3RDS), Vector3(0x1.0p0, _2_3RDS, _1_3RDS), Vector3(0x1.0p0, _1_3RDS, _1_3RDS), Vector3(0x1.0p0, _2_3RDS, _2_3RDS), \
	Vector3(0x0.4p0, 0x0.0p0, _1_3RDS), Vector3(0x0.8p0, 0x0.0p0, _1_3RDS), Vector3(0x0.8p0, 0x0.0p0, _2_3RDS), Vector3(0x0.4p0, 0x0.0p0, _2_3RDS), \
	Vector3(0x0.4p0, _1_3RDS, 0x0.0p0), Vector3(0x0.8p0, _1_3RDS, 0x0.0p0), Vector3(0x0.4p0, _2_3RDS, 0x0.0p0), Vector3(0x0.8p0, _2_3RDS, 0x0.0p0), \
	Vector3(0x0.4p0, _1_3RDS, 0x1.0p0), Vector3(0x0.8p0, _1_3RDS, 0x1.0p0), Vector3(0x0.8p0, _2_3RDS, 0x1.0p0), Vector3(0x0.4p0, _2_3RDS, 0x1.0p0), \
	Vector3(0x0.4p0, 0x1.0p0, _2_3RDS), Vector3(0x0.8p0, 0x1.0p0, _2_3RDS), Vector3(0x0.4p0, 0x1.0p0, _1_3RDS), Vector3(0x0.8p0, 0x1.0p0, _1_3RDS), \
}

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

#define REPEAT_TWENTY_TIMES(m) m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m
#define BOX_40_CELL_NORMAL_INDICES PackedInt32Array { \
	REPEAT_TWENTY_TIMES(0), /* -X */ \
	REPEAT_TWENTY_TIMES(1), /* +X */ \
	REPEAT_TWENTY_TIMES(2), /* -Y */ \
	REPEAT_TWENTY_TIMES(3), /* +Y */ \
	REPEAT_TWENTY_TIMES(4), /* -Z */ \
	REPEAT_TWENTY_TIMES(5), /* +Z */ \
	REPEAT_TWENTY_TIMES(6), /* -W */ \
	REPEAT_TWENTY_TIMES(7), /* +W */ \
}

#define REPEAT_TWENTY_FOUR_TIMES(m) m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m
#define BOX_48_CELL_NORMAL_INDICES PackedInt32Array { \
	REPEAT_TWENTY_FOUR_TIMES(0), /* -X */ \
	REPEAT_TWENTY_FOUR_TIMES(1), /* +X */ \
	REPEAT_TWENTY_FOUR_TIMES(2), /* -Y */ \
	REPEAT_TWENTY_FOUR_TIMES(3), /* +Y */ \
	REPEAT_TWENTY_FOUR_TIMES(4), /* -Z */ \
	REPEAT_TWENTY_FOUR_TIMES(5), /* +Z */ \
	REPEAT_TWENTY_FOUR_TIMES(6), /* -W */ \
	REPEAT_TWENTY_FOUR_TIMES(7), /* +W */ \
}

#define BOX_40_CELL_VERTEX_INDICES PackedInt32Array { \
	 8,  4, 14,  2,  8, 10,  2, 14,  8,  0,  4,  2,  8, 12, 14,  4,  6,  4,  2, 14, /* -X */ \
	11, 13,  1,  7, 11,  3,  7,  1, 11, 15, 13,  7, 11,  9,  1, 13,  5, 13,  7,  1, /* +X */ \
	 4, 13,  1,  8,  4,  0,  8,  1,  4, 12, 13,  8,  4,  5,  1, 13,  9, 13,  8,  1, /* -Y */ \
	 14, 7, 11,  2, 14, 10,  2, 11, 14,  6,  7,  2, 14, 15, 11,  7,  3,  7,  2, 11, /* +Y */ \
	 1,  8, 11,  2,  1,  3,  2, 11,  1,  0,  8,  2,  1,  9, 11,  8, 10,  8,  2, 11, /* -Z */ \
	 13, 4,  7, 14, 13, 15, 14,  7, 13, 12,  4, 14, 13,  5,  7,  4,  6,  4, 14,  7, /* +Z */ \
	 1,  7,  4,  2,  1,  0,  2,  4,  1,  3,  7,  2,  1,  5,  4,  7,  6,  7,  2,  4, /* -W */ \
	 8, 14, 13, 11,  8,  9, 11, 13,  8, 10, 14, 11,  8, 12, 13, 14, 15, 14, 11, 13, /* +W */ \
}

#define BOX_48_CELL_VERTEX_INDICES PackedInt32Array { \
	 8,  0,  4,  6,  8,  4, 12,  6,  8, 12, 14,  6,  8, 14, 10,  6,  8, 10,  2,  6,  8,  2,  0,  6, /* -X */ \
	 9, 11, 15,  7,  9, 15, 13,  7,  9, 13,  5,  7,  9,  5,  1,  7,  9,  1,  3,  7,  9,  3, 11,  7, /* +X */ \
	 5, 13,  9,  8,  5,  9,  1,  8,  5,  1,  0,  8,  5,  0,  4,  8,  5,  4, 12,  8,  5, 12, 13,  8, /* -Y */ \
	10,  2,  6,  7, 10,  6, 14,  7, 10, 14, 15,  7, 10, 15, 11,  7, 10, 11,  3,  7, 10,  3,  2,  7, /* +Y */ \
	 8,  0,  2,  3,  8,  2, 10,  3,  8, 10, 11,  3,  8, 11,  9,  3,  8,  9,  1,  3,  8,  1,  0,  3, /* -Z */ \
	12, 14,  6,  7, 12,  6,  4,  7, 12,  4,  5,  7, 12,  5, 13,  7, 12, 13, 15,  7, 12, 15, 14,  7, /* +Z */ \
	 0,  1,  5,  7,  0,  3,  1,  7,  0,  2,  3,  7,  0,  6,  2,  7,  0,  4,  6,  7,  0,  5,  4,  7, /* -W */ \
	 8, 10, 14, 15,  8, 14, 12, 15,  8, 12, 13, 15,  8, 13,  9, 15,  8,  9, 11, 15,  8, 11, 10, 15, /* +W */ \
}

#define BOX_40_CELL_TEXTURE_MAP_FILL_EACH_SIDE_INDICES PackedInt32Array { \
	/* -X */ \
	1, 4, 7, 2, \
	1, 3, 2, 7, \
	1, 0, 4, 2, \
	1, 5, 7, 4, \
	6, 4, 2, 7, \
	/* +X */ \
	2, 4, 1, 7, \
	2, 3, 7, 1, \
	2, 6, 4, 7, \
	2, 0, 1, 4, \
	5, 4, 7, 1, \
	/* -Y */ \
	4, 7, 1, 2, \
	4, 0, 2, 1, \
	4, 6, 7, 2, \
	4, 5, 1, 7, \
	3, 7, 2, 1, \
	/* +Y */ \
	4, 7, 1, 2, \
	4, 0, 2, 1, \
	4, 6, 7, 2, \
	4, 5, 1, 7, \
	3, 7, 2, 1, \
	/* -Z */ \
	1, 4, 7, 2, \
	1, 3, 2, 7, \
	1, 0, 4, 2, \
	1, 5, 7, 4, \
	6, 4, 2, 7, \
	/* +Z */ \
	1, 4, 7, 2, \
	1, 3, 2, 7, \
	1, 0, 4, 2, \
	1, 5, 7, 4, \
	6, 4, 2, 7, \
	/* -W */ \
	0, 6, 5, 3, \
	0, 1, 3, 5, \
	0, 2, 6, 3, \
	0, 4, 5, 6, \
	7, 6, 3, 5, \
	/* +W */ \
	0, 6, 5, 3, \
	0, 1, 3, 5, \
	0, 2, 6, 3, \
	0, 4, 5, 6, \
	7, 6, 3, 5 \
}

#define BOX_40_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID_INDICES PackedInt32Array { \
	/* -X */ \
	6, 15, 14, 16, \
	6, 17, 16, 14, \
	6, 18, 15, 16, \
	6, 19, 14, 15, \
	11, 15, 16, 14, \
	/* +X */ \
	12, 13, 1, 20, \
	12, 21, 20, 1, \
	12, 11, 13, 20, \
	12, 8, 1, 13, \
	22, 13, 20, 1, \
	/* -Y */ \
	20, 13, 16, 5, \
	20, 23, 5, 16, \
	20, 22, 13, 5, \
	20, 11, 16, 13, \
	24, 13, 5, 16, \
	/* +Y */ \
	14, 15, 12, 2, \
	14, 9, 2, 12, \
	14, 19, 15, 2, \
	14, 11, 12, 15, \
	25, 15, 2, 12, \
	/* -Z */ \
	15, 3, 12, 20, \
	15, 11, 20, 12, \
	15, 26, 3, 20, \
	15, 25, 12, 3, \
	21, 3, 20, 12, \
	/* +Z */ \
	13, 4, 16, 14, \
	13, 11, 14, 16, \
	13, 10, 4, 14, \
	13, 24, 16, 4, \
	17, 4, 14, 16, \
	/* -W */ \
	7, 20, 15, 16, \
	7, 18, 16, 15, \
	7, 23, 20, 16, \
	7, 26, 15, 20, \
	11, 20, 16, 15, \
	/* +W */ \
	0, 14, 13, 12, \
	0, 8, 12, 13, \
	0, 9, 14, 12, \
	0, 10, 13, 14, \
	11, 14, 12, 13 \
}

#define BOX_40_CELL_TEXTURE_MAP_CROSS_ISLAND_INDICES PackedInt32Array { \
	/* -X */ \
	0, 1, 2, 3, \
	0, 4, 3, 2, \
	0, 5, 1, 3, \
	0, 6, 2, 1, \
	7, 1, 3, 2, \
	/* +X */ \
	8, 9, 10, 11, \
	8, 12, 11, 10, \
	8, 13, 9, 11, \
	8, 14, 10, 9, \
	15, 9, 11, 10, \
	/* -Y */ \
	16, 9, 17, 0, \
	16, 18, 0, 17, \
	16, 6, 9, 0, \
	16, 19, 17, 9, \
	14, 9, 0, 17, \
	/* +Y */ \
	2, 20, 8, 21, \
	2, 4, 21, 8, \
	2, 22, 20, 21, \
	2, 13, 8, 20, \
	23, 20, 21, 8, \
	/* -Z */ \
	24, 0, 8, 25, \
	24, 26, 25, 8, \
	24, 27, 0, 25, \
	24, 14, 8, 0, \
	4, 0, 25, 8, \
	/* +Z */ \
	9, 28, 29, 2, \
	9, 13, 2, 29, \
	9, 6, 28, 2, \
	9, 30, 29, 28, \
	31, 28, 2, 29, \
	/* -W */ \
	32, 33, 34, 35, \
	32, 36, 35, 34, \
	32, 37, 33, 35, \
	32, 38, 34, 33, \
	39, 33, 35, 34, \
	/* +W */ \
	0, 2, 9, 8, \
	0, 14, 8, 9, \
	0, 4, 2, 8, \
	0, 6, 9, 2, \
	13, 2, 8, 9, \
}

#define BOX_40_CELL_TEXTURE_MAP_LONG_CROSS_INDICES PackedInt32Array { \
	/* -X */ \
	0, 1, 2, 3, \
	0, 4, 3, 2, \
	0, 5, 1, 3, \
	0, 6, 2, 1, \
	7, 1, 3, 2, \
	/* +X */ \
	8, 9, 10, 11, \
	8, 12, 11, 10, \
	8, 13, 9, 11, \
	8, 14, 10, 9, \
	15, 9, 11, 10, \
	/* -Y */ \
	16, 9, 17, 0, \
	16, 18, 0, 17, \
	16, 6, 9, 0, \
	16, 19, 17, 9, \
	14, 9, 0, 17, \
	/* +Y */ \
	2, 20, 8, 21, \
	2, 4, 21, 8, \
	2, 22, 20, 21, \
	2, 13, 8, 20, \
	23, 20, 21, 8, \
	/* -Z */ \
	24, 0, 8, 25, \
	24, 26, 25, 8, \
	24, 27, 0, 25, \
	24, 14, 8, 0, \
	4, 0, 25, 8, \
	/* +Z */ \
	9, 28, 29, 2, \
	9, 13, 2, 29, \
	9, 6, 28, 2, \
	9, 30, 29, 28, \
	31, 28, 2, 29, \
	/* -W */ \
	10, 11, 32, 33, \
	10, 34, 33, 32, \
	10, 12, 11, 33, \
	10, 15, 32, 11, \
	35, 11, 33, 32, \
	/* +W */ \
	0, 2, 9, 8, \
	0, 14, 8, 9, \
	0, 4, 2, 8, \
	0, 6, 9, 2, \
	13, 2, 8, 9, \
}

#define BOX_48_CELL_TEXTURE_MAP_FILL_EACH_SIDE_INDICES PackedInt32Array { \
	/* -X */ \
	1, 0, 4, 6, \
	1, 4, 5, 6, \
	1, 5, 7, 6, \
	1, 7, 3, 6, \
	1, 3, 2, 6, \
	1, 2, 0, 6, \
	/* +X */ \
	0, 2, 6, 7, \
	0, 6, 4, 7, \
	0, 4, 5, 7, \
	0, 5, 1, 7, \
	0, 1, 3, 7, \
	0, 3, 2, 7, \
	/* -Y */ \
	5, 7, 3, 2, \
	5, 3, 1, 2, \
	5, 1, 0, 2, \
	5, 0, 4, 2, \
	5, 4, 6, 2, \
	5, 6, 7, 2, \
	/* +Y */ \
	0, 2, 6, 7, \
	0, 6, 4, 7, \
	0, 4, 5, 7, \
	0, 5, 1, 7, \
	0, 1, 3, 7, \
	0, 3, 2, 7, \
	/* -Z */ \
	4, 0, 2, 3, \
	4, 2, 6, 3, \
	4, 6, 7, 3, \
	4, 7, 5, 3, \
	4, 5, 1, 3, \
	4, 1, 0, 3, \
	/* +Z */ \
	0, 2, 6, 7, \
	0, 6, 4, 7, \
	0, 4, 5, 7, \
	0, 5, 1, 7, \
	0, 1, 3, 7, \
	0, 3, 2, 7, \
	/* -W */ \
	7, 6, 2, 0, \
	7, 4, 6, 0, \
	7, 5, 4, 0, \
	7, 1, 5, 0, \
	7, 3, 1, 0, \
	7, 2, 3, 0, \
	/* +W */ \
	0, 2, 6, 7, \
	0, 6, 4, 7, \
	0, 4, 5, 7, \
	0, 5, 1, 7, \
	0, 1, 3, 7, \
	0, 3, 2, 7 \
}

#define BOX_48_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID_INDICES PackedInt32Array { \
	/* -X */ \
	6, 18, 15, 11, \
	6, 15, 19, 11, \
	6, 19, 14, 11, \
	6, 14, 17, 11, \
	6, 17, 16, 11, \
	6, 16, 18, 11, \
	/* +X */ \
	8, 12, 11, 20, \
	8, 11, 13, 20, \
	8, 13, 22, 20, \
	8, 22, 1, 20, \
	8, 1, 21, 20, \
	8, 21, 12, 20, \
	/* -Y */ \
	11, 13, 24, 5, \
	11, 24, 16, 5, \
	11, 16, 23, 5, \
	11, 23, 20, 5, \
	11, 20, 22, 5, \
	11, 22, 13, 5, \
	/* +Y */ \
	9, 2, 19, 15, \
	9, 19, 14, 15, \
	9, 14, 11, 15, \
	9, 11, 12, 15, \
	9, 12, 25, 15, \
	9, 25, 2, 15, \
	/* -Z */ \
	3, 26, 20, 11, \
	3, 20, 21, 11, \
	3, 21, 12, 11, \
	3, 12, 25, 11, \
	3, 25, 15, 11, \
	3, 15, 26, 11, \
	/* +Z */ \
	10, 14, 17, 16, \
	10, 17, 4, 16, \
	10, 4, 24, 16, \
	10, 24, 13, 16, \
	10, 13, 11, 16, \
	10, 11, 14, 16, \
	/* -W */ \
	11, 20, 23, 7, \
	11, 26, 20, 7, \
	11, 15, 26, 7, \
	11, 18, 15, 7, \
	11, 16, 18, 7, \
	11, 23, 16, 7, \
	/* +W */ \
	0, 9, 14, 11, \
	0, 14, 10, 11, \
	0, 10, 13, 11, \
	0, 13, 8, 11, \
	0, 8, 12, 11, \
	0, 12, 9, 11 \
}

#define BOX_48_CELL_TEXTURE_MAP_CROSS_ISLAND_INDICES PackedInt32Array { \
	/* -X */ \
	0, 5, 1, 7, \
	0, 1, 6, 7, \
	0, 6, 2, 7, \
	0, 2, 4, 7, \
	0, 4, 3, 7, \
	0, 3, 5, 7, \
	/* +X */ \
	14, 8, 13, 11, \
	14, 13, 9, 11, \
	14, 9, 15, 11, \
	14, 15, 10, 11, \
	14, 10, 12, 11, \
	14, 12, 8, 11, \
	/* -Y */ \
	19, 9, 14, 0, \
	19, 14, 17, 0, \
	19, 17, 18, 0, \
	19, 18, 16, 0, \
	19, 16, 6, 0, \
	19, 6, 9, 0, \
	/* +Y */ \
	4, 21, 22, 20, \
	4, 22, 2, 20, \
	4, 2, 13, 20, \
	4, 13, 8, 20, \
	4, 8, 23, 20, \
	4, 23, 21, 20, \
	/* -Z */ \
	0, 27, 25, 26, \
	0, 25, 4, 26, \
	0, 4, 8, 26, \
	0, 8, 14, 26, \
	0, 14, 24, 26, \
	0, 24, 27, 26, \
	/* +Z */ \
	6, 2, 31, 29, \
	6, 31, 28, 29, \
	6, 28, 30, 29, \
	6, 30, 9, 29, \
	6, 9, 13, 29, \
	6, 13, 2, 29, \
	/* -W */ \
	39, 33, 37, 32, \
	39, 38, 33, 32, \
	39, 34, 38, 32, \
	39, 36, 34, 32, \
	39, 35, 36, 32, \
	39, 37, 35, 32, \
	/* +W */ \
	0, 4, 2, 13, \
	0, 2, 6, 13, \
	0, 6, 9, 13, \
	0, 9, 14, 13, \
	0, 14, 8, 13, \
	0, 8, 4, 13, \
}

#define BOX_48_CELL_TEXTURE_MAP_LONG_CROSS_INDICES PackedInt32Array { \
	/* -X */ \
	0, 5, 1, 7, \
	0, 1, 6, 7, \
	0, 6, 2, 7, \
	0, 2, 4, 7, \
	0, 4, 3, 7, \
	0, 3, 5, 7, \
	/* +X */ \
	14, 8, 13, 11, \
	14, 13, 9, 11, \
	14, 9, 15, 11, \
	14, 15, 10, 11, \
	14, 10, 12, 11, \
	14, 12, 8, 11, \
	/* -Y */ \
	19, 9, 14, 0, \
	19, 14, 17, 0, \
	19, 17, 18, 0, \
	19, 18, 16, 0, \
	19, 16, 6, 0, \
	19, 6, 9, 0, \
	/* +Y */ \
	4, 21, 22, 20, \
	4, 22, 2, 20, \
	4, 2, 13, 20, \
	4, 13, 8, 20, \
	4, 8, 23, 20, \
	4, 23, 21, 20, \
	/* -Z */ \
	0, 27, 25, 26, \
	0, 25, 4, 26, \
	0, 4, 8, 26, \
	0, 8, 14, 26, \
	0, 14, 24, 26, \
	0, 24, 27, 26, \
	/* +Z */ \
	6, 2, 31, 29, \
	6, 31, 28, 29, \
	6, 28, 30, 29, \
	6, 30, 9, 29, \
	6, 9, 13, 29, \
	6, 13, 2, 29, \
	/* -W */ \
	35, 11, 12, 10, \
	35, 15, 11, 10, \
	35, 32, 15, 10, \
	35, 34, 32, 10, \
	35, 33, 34, 10, \
	35, 12, 33, 10, \
	/* +W */ \
	0, 4, 2, 13, \
	0, 2, 6, 13, \
	0, 6, 9, 13, \
	0, 9, 14, 13, \
	0, 14, 8, 13, \
	0, 8, 4, 13, \
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
		PackedInt32Array {13, 1,  3, 15, 17, 19}, /* XYW cell facing +Z */ \
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
#define BOX_POLY_CELL_NORMAL_INDICES Vector<PackedInt32Array> { \
	PackedInt32Array { REPEAT_EIGHT_TIMES(6) }, /* -W */ \
	PackedInt32Array { REPEAT_EIGHT_TIMES(2) }, /* -Y */ \
	PackedInt32Array { REPEAT_EIGHT_TIMES(4) }, /* -Z */ \
	PackedInt32Array { REPEAT_EIGHT_TIMES(5) }, /* +Z */ \
	PackedInt32Array { REPEAT_EIGHT_TIMES(1) }, /* +X */ \
	PackedInt32Array { REPEAT_EIGHT_TIMES(0) }, /* -X */ \
	PackedInt32Array { REPEAT_EIGHT_TIMES(3) }, /* +Y */ \
	PackedInt32Array { REPEAT_EIGHT_TIMES(7) }, /* +W */ \
}

// The order of these needs to match the data in BOX_POLY_CELL_INDICES.
#define BOX_POLY_CELL_POLY_TEXTURE_MAP_FILL_EACH_SIDE_INDICES Vector<PackedInt32Array> { \
	PackedInt32Array { /* -W */ \
		7, 6, \
		5, 4, \
		1, 0, \
		3, 2, \
	}, \
	PackedInt32Array { /* -Y */ \
		2, 3, \
		7, 6, \
		0, 1, \
		5, 4, \
	}, \
	PackedInt32Array { /* -Z */ \
		0, 1, \
		2, 3, \
		4, 5, \
		6, 7, \
	}, \
	PackedInt32Array { /* +Z */ \
		4, 0, \
		5, 1, \
		7, 6, \
		3, 2, \
	}, \
	PackedInt32Array { /* +X */ \
		1, 3, \
		7, 5, \
		4, 6, \
		0, 2, \
	}, \
	PackedInt32Array { /* -X */ \
		0, 2, \
		4, 6, \
		1, 3, \
		5, 7, \
	}, \
	PackedInt32Array { /* +Y */ \
		0, 1, \
		4, 5, \
		6, 7, \
		2, 3, \
	}, \
	PackedInt32Array { /* +W */ \
		0, 1, \
		2, 3, \
		5, 4, \
		7, 6, \
	}, \
}

#define BOX_POLY_CELL_POLY_TEXTURE_MAP_COMPACT_2X2X2_GRID_INDICES Vector<PackedInt32Array> { \
	PackedInt32Array { /* -W */ \
		11, 20, \
		15, 26, \
		18, 7, \
		16, 23, \
	}, \
	PackedInt32Array { /* -Y */ \
		13, 22, \
		5, 24, \
		11, 20, \
		23, 16, \
	}, \
	PackedInt32Array { /* -Z */ \
		11, 20, \
		15, 26, \
		12, 21, \
		25, 3, \
	}, \
	PackedInt32Array { /* +Z */ \
		4, 10, \
		24, 13, \
		16, 17, \
		11, 14, \
	}, \
	PackedInt32Array { /* +X */ \
		1, 21, \
		20, 22, \
		13, 11, \
		8, 12, \
	}, \
	PackedInt32Array { /* -X */ \
		11, 15, \
		16, 18, \
		14, 19, \
		17, 6, \
	}, \
	PackedInt32Array { /* +Y */ \
		9, 12, \
		14, 11, \
		19, 15, \
		2, 25, \
	}, \
	PackedInt32Array { /* +W */ \
		0, 8, \
		9, 12, \
		13, 10, \
		11, 14, \
	}, \
}

#define BOX_POLY_CELL_POLY_TEXTURE_MAP_CROSS_ISLAND_INDICES Vector<PackedInt32Array> { \
	PackedInt32Array { /* -W */ \
		36, 32, \
		35, 37, \
		39, 33, \
		34, 38, \
	}, \
	PackedInt32Array { /* -Y */ \
		0, 14, \
		9, 6, \
		18, 17, \
		19, 16, \
	}, \
	PackedInt32Array { /* -Z */ \
		27, 24, \
		25, 26, \
		0, 14, \
		4, 8, \
	}, \
	PackedInt32Array { /* +Z */ \
		28, 6, \
		30, 9, \
		29, 31, \
		13, 2, \
	}, \
	PackedInt32Array { /* +X */ \
		10, 12, \
		11, 15, \
		9, 13, \
		14, 8, \
	}, \
	PackedInt32Array { /* -X */ \
		5, 3, \
		1, 7, \
		0, 4, \
		6, 2, \
	}, \
	PackedInt32Array { /* +Y */ \
		4, 8, \
		2, 13, \
		22, 20, \
		21, 23, \
	}, \
	PackedInt32Array { /* +W */ \
		0, 14, \
		4, 8, \
		9, 6, \
		13, 2, \
	}, \
}

// Generated from island order: [7, 4, 5, 6, 1, 3, 2, 0]
#define BOX_POLY_CELL_POLY_TEXTURE_MAP_LONG_CROSS_INDICES Vector<PackedInt32Array> { \
	PackedInt32Array { /* -W */ \
		34, 10, \
		33, 12, \
		35, 11, \
		32, 15, \
	}, \
	PackedInt32Array { /* -Y */ \
		0, 14, \
		9, 6, \
		36, 37, \
		38, 39, \
	}, \
	PackedInt32Array { /* -Z */ \
		40, 41, \
		42, 43, \
		0, 14, \
		4, 8, \
	}, \
	PackedInt32Array { /* +Z */ \
		44, 6, \
		45, 9, \
		46, 47, \
		13, 2, \
	}, \
	PackedInt32Array { /* +X */ \
		10, 12, \
		11, 15, \
		9, 13, \
		14, 8, \
	}, \
	PackedInt32Array { /* -X */ \
		5, 3, \
		1, 7, \
		0, 4, \
		6, 2, \
	}, \
	PackedInt32Array { /* +Y */ \
		4, 8, \
		2, 13, \
		48, 49, \
		50, 51, \
	}, \
	PackedInt32Array { /* +W */ \
		0, 14, \
		4, 8, \
		9, 6, \
		13, 2, \
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
