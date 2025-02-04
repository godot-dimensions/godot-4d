#pragma once

#include "../../math/rect4.h"
#include "tests/test_macros.h"

namespace TestRect4 {
TEST_CASE("[Rect4] Continuous Collision Depth") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	real_t depth = unit_rect.continuous_collision_depth(Vector4(0, 0, 0, 0), unit_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with no motion should not be stopped.");
	depth = unit_rect.continuous_collision_depth(Vector4(1, 0, 0, 0), unit_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with perfectly overlapping rects should depenetrate (in any direction, -1.0 would also be acceptable).");
	depth = unit_rect.continuous_collision_depth(Vector4(-1, 0, 0, 0), unit_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with perfectly overlapping rects should depenetrate (in any direction, -1.0 would also be acceptable).");

	const Rect4 offset_3w_rect = Rect4(Vector4(0, 0, 0, 3), Vector4(1, 1, 1, 1));
	depth = unit_rect.continuous_collision_depth(Vector4(0, 0, 0, 0), offset_3w_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with no motion should not be stopped.");
	depth = unit_rect.continuous_collision_depth(Vector4(0, 0, 0, 5), offset_3w_rect);
	CHECK_MESSAGE(depth == doctest::Approx(0.4f), "Rect4 continuous_collision_depth with high motion should be stopped by the obstacle.");
	depth = unit_rect.continuous_collision_depth(Vector4(0, 0, 0, -5), offset_3w_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with motion away from the obstacle should not be stopped.");
	depth = unit_rect.continuous_collision_depth(Vector4(10, 0, 0, 0), offset_3w_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with motion perpendicular to the obstacle should not be stopped.");

	const Rect4 offset_minus_z_rect = Rect4(Vector4(0, 0.5, -2, 0), Vector4(1, 1, 1, 1));
	depth = unit_rect.continuous_collision_depth(Vector4(0, 0, -2, 0), offset_minus_z_rect);
	CHECK_MESSAGE(depth == doctest::Approx(0.5f), "Rect4 continuous_collision_depth with motion towards the obstacle should be stopped.");
	depth = unit_rect.continuous_collision_depth(Vector4(10, 0, 0, 0), offset_minus_z_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with motion perpendicular to the obstacle should not be stopped.");
	depth = unit_rect.continuous_collision_depth(Vector4(0, 10, 0, 0), offset_minus_z_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with motion perpendicular to the obstacle should not be stopped.");

	const Rect4 overlap_pos_y_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(2, 3, 4, 5));
	depth = unit_rect.continuous_collision_depth(Vector4(0, 2, 0, 0), overlap_pos_y_rect);
	CHECK_MESSAGE(depth == -0.5f, "Rect4 continuous_collision_depth overlapping should depenetrate the obstacle.");
	depth = unit_rect.continuous_collision_depth(Vector4(0, -2, 0, 0), overlap_pos_y_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth overlapping should be allowed to move out of the obstacle.");

	const Rect4 overlap_neg_x_rect = Rect4(Vector4(-1, 0, 0, 0), Vector4(2, 3, 4, 5));
	depth = unit_rect.continuous_collision_depth(Vector4(-2, 0, 0, 0), overlap_neg_x_rect);
	CHECK_MESSAGE(depth == -0.5f, "Rect4 continuous_collision_depth overlapping should depenetrate the obstacle.");
	depth = unit_rect.continuous_collision_depth(Vector4(2, 0, 0, 0), overlap_neg_x_rect);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth overlapping should be allowed to move out of the obstacle.");
}
} // namespace TestRect4
