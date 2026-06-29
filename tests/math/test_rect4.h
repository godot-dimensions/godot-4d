#pragma once

#include "../../math/rect4.h"

#include "tests/test_macros.h"

namespace TestRect4 {
TEST_CASE("[Rect4] Continuous Collision Depth") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	Vector4 normal;
	real_t depth = unit_rect.continuous_collision_depth(Vector4(0, 0, 0, 0), unit_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with no motion should not be stopped.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");
	depth = unit_rect.continuous_collision_depth(Vector4(1, 0, 0, 0), unit_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with perfectly overlapping rects should depenetrate (in any direction, -1.0 would also be acceptable).");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");
	depth = unit_rect.continuous_collision_depth(Vector4(-1, 0, 0, 0), unit_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with perfectly overlapping rects should depenetrate (in any direction, -1.0 would also be acceptable).");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");

	const Rect4 offset_3w_rect = Rect4(Vector4(0, 0, 0, 3), Vector4(1, 1, 1, 1));
	depth = unit_rect.continuous_collision_depth(Vector4(0, 0, 0, 0), offset_3w_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with no motion should not be stopped.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");
	depth = unit_rect.continuous_collision_depth(Vector4(0, 0, 0, 5), offset_3w_rect, &normal);
	CHECK_MESSAGE(depth == doctest::Approx(0.4f), "Rect4 continuous_collision_depth with high motion should be stopped by the obstacle.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, -1), "Rect4 continuous_collision_depth should give the correct normal.");
	depth = unit_rect.continuous_collision_depth(Vector4(0, 0, 0, -5), offset_3w_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with motion away from the obstacle should not be stopped.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");
	depth = unit_rect.continuous_collision_depth(Vector4(10, 0, 0, 0), offset_3w_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with motion perpendicular to the obstacle should not be stopped.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");

	const Rect4 offset_minus_z_rect = Rect4(Vector4(0, 0.5, -2, 0), Vector4(1, 1, 1, 1));
	depth = unit_rect.continuous_collision_depth(Vector4(0, 0, -2, 0), offset_minus_z_rect, &normal);
	CHECK_MESSAGE(depth == doctest::Approx(0.5f), "Rect4 continuous_collision_depth with motion towards the obstacle should be stopped.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 1, 0), "Rect4 continuous_collision_depth should give the correct normal.");
	depth = unit_rect.continuous_collision_depth(Vector4(10, 0, 0, 0), offset_minus_z_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with motion perpendicular to the obstacle should not be stopped.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");
	depth = unit_rect.continuous_collision_depth(Vector4(0, 10, 0, 0), offset_minus_z_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth with motion perpendicular to the obstacle should not be stopped.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");

	const Rect4 overlap_pos_y_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(2, 3, 4, 5));
	depth = unit_rect.continuous_collision_depth(Vector4(0, 2, 0, 0), overlap_pos_y_rect, &normal);
	CHECK_MESSAGE(depth == -0.5f, "Rect4 continuous_collision_depth overlapping should depenetrate the obstacle.");
	CHECK_MESSAGE(normal == Vector4(0, -1, 0, 0), "Rect4 continuous_collision_depth should give the correct normal.");
	depth = unit_rect.continuous_collision_depth(Vector4(0, -2, 0, 0), overlap_pos_y_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth overlapping should be allowed to move out of the obstacle.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");

	const Rect4 overlap_neg_x_rect = Rect4(Vector4(-1, 0, 0, 0), Vector4(2, 3, 4, 5));
	depth = unit_rect.continuous_collision_depth(Vector4(-2, 0, 0, 0), overlap_neg_x_rect, &normal);
	CHECK_MESSAGE(depth == -0.5f, "Rect4 continuous_collision_depth overlapping should depenetrate the obstacle.");
	CHECK_MESSAGE(normal == Vector4(1, 0, 0, 0), "Rect4 continuous_collision_depth should give the correct normal.");
	depth = unit_rect.continuous_collision_depth(Vector4(2, 0, 0, 0), overlap_neg_x_rect, &normal);
	CHECK_MESSAGE(depth == 1.0f, "Rect4 continuous_collision_depth overlapping should be allowed to move out of the obstacle.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, 0), "Rect4 continuous_collision_depth should give a zero normal when there is no collision.");
}

TEST_CASE("[Rect4] Raycast from outside") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	real_t distance;
	Vector4 normal;
	// Ray from outside, pointing at center.
	bool hit = unit_rect.raycast_intersects(Vector4(-2, 0.5, 0.5, 0.5), Vector4(1, 0, 0, 0), false, &distance, &normal);
	CHECK_MESSAGE(hit == true, "Raycast from outside should hit the box");
	CHECK_MESSAGE(distance == doctest::Approx(2.0f), "Raycast distance should be 2.0");
	CHECK_MESSAGE(normal == Vector4(-1, 0, 0, 0), "Normal should point backwards along X");
	// Ray from outside, pointing at corner.
	hit = unit_rect.raycast_intersects(Vector4(-1, -1, -1, -1), Vector4(1, 1, 1, 1).normalized(), false, &distance, &normal);
	CHECK_MESSAGE(hit == true, "Raycast at corner should hit");
	// Ray from outside, missing the box.
	hit = unit_rect.raycast_intersects(Vector4(-1, 2, 0.5, 0.5), Vector4(1, 0, 0, 0), false, &distance, &normal);
	CHECK_MESSAGE(hit == false, "Raycast missing the box should not hit");
	// Ray parallel to box, pointing away.
	hit = unit_rect.raycast_intersects(Vector4(2, 0.5, 0.5, 0.5), Vector4(1, 0, 0, 0), false, &distance, &normal);
	CHECK_MESSAGE(hit == false, "Raycast pointing away should not hit");
}

TEST_CASE("[Rect4] Raycast from inside") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	real_t distance;
	Vector4 normal;
	// When inside_is_zero=true, should return distance 0.
	bool hit = unit_rect.raycast_intersects(Vector4(0.5, 0.5, 0.5, 0.5), Vector4(1, 0, 0, 0), true, &distance, &normal);
	CHECK_MESSAGE(hit == true, "Raycast from inside should hit with inside_is_zero=true");
	CHECK_MESSAGE(distance == doctest::Approx(0.0f), "Raycast from inside with inside_is_zero should have distance 0");
}

TEST_CASE("[Rect4] Raycast parallel to axes") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	real_t distance;
	Vector4 normal;
	// Ray parallel to box, pointing along Y through center.
	bool hit = unit_rect.raycast_intersects(Vector4(0.5, -1, 0.5, 0.5), Vector4(0, 1, 0, 0), false, &distance, &normal);
	CHECK_MESSAGE(hit == true, "Ray through center should hit");
	CHECK_MESSAGE(distance == doctest::Approx(1.0f), "Distance should be 1.0");
	// Ray parallel to box, missing on one axis.
	hit = unit_rect.raycast_intersects(Vector4(0.5, -1, 2, 0.5), Vector4(0, 1, 0, 0), false, &distance, &normal);
	CHECK_MESSAGE(hit == false, "Ray missing on Z axis should not hit");
}
} // namespace TestRect4
