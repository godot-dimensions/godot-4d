#pragma once

#include "../../math/rect4.h"

#include "tests/test_macros.h"

namespace TestRect4 {
TEST_CASE("[Rect4] Basic math functions") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	CHECK_MESSAGE(unit_rect.get_hypervolume() == doctest::Approx(1.0f), "Rect4 get_hypervolume of a unit rect should be 1.");
	CHECK_MESSAGE(unit_rect.get_surface_volume() == doctest::Approx(8.0f), "Rect4 get_surface_volume of a unit rect should be 8, the 8 boundary cells of a tesseract.");
	CHECK_MESSAGE(unit_rect.has_hypervolume(), "Rect4 has_hypervolume should be true when every size component is positive.");
	CHECK_MESSAGE(unit_rect.has_surface_volume(), "Rect4 has_surface_volume should be true when any size component is positive.");
	CHECK_MESSAGE(unit_rect.has_any_size(), "Rect4 has_any_size should be true when any size component is non-zero.");
	CHECK_MESSAGE(unit_rect.get_center() == Vector4(0.5f, 0.5f, 0.5f, 0.5f), "Rect4 get_center should be halfway between the position and the end.");
	CHECK_MESSAGE(unit_rect.get_end() == Vector4(1, 1, 1, 1), "Rect4 get_end should be the position plus the size.");

	const Rect4 box_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(2, 3, 4, 5));
	CHECK_MESSAGE(box_rect.get_hypervolume() == doctest::Approx(120.0f), "Rect4 get_hypervolume should multiply all four size components.");
	CHECK_MESSAGE(box_rect.get_surface_volume() == doctest::Approx(308.0f), "Rect4 get_surface_volume should be 2*(xyz + xyw + xzw + yzw).");

	const Rect4 flat_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 0));
	CHECK_MESSAGE(flat_rect.get_hypervolume() == doctest::Approx(0.0f), "Rect4 get_hypervolume of a rect that is flat in W should be 0.");
	CHECK_MESSAGE(!flat_rect.has_hypervolume(), "Rect4 has_hypervolume should be false when any size component is zero.");
	CHECK_MESSAGE(flat_rect.has_surface_volume(), "Rect4 has_surface_volume should be true when any size component is positive.");

	const Rect4 zero_rect = Rect4();
	CHECK_MESSAGE(!zero_rect.has_hypervolume(), "Rect4 has_hypervolume should be false for a zero-size rect.");
	CHECK_MESSAGE(!zero_rect.has_surface_volume(), "Rect4 has_surface_volume should be false for a zero-size rect.");
	CHECK_MESSAGE(!zero_rect.has_any_size(), "Rect4 has_any_size should be false for a zero-size rect.");

	const Rect4 negative_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(-1, -2, -3, -4));
	CHECK_MESSAGE(!negative_rect.has_hypervolume(), "Rect4 has_hypervolume should be false for a negative-size rect.");
	CHECK_MESSAGE(!negative_rect.has_surface_volume(), "Rect4 has_surface_volume should be false for a negative-size rect.");
	CHECK_MESSAGE(negative_rect.has_any_size(), "Rect4 has_any_size should be true for a negative-size rect.");
	const Rect4 absolute_rect = negative_rect.abs();
	CHECK_MESSAGE(absolute_rect.position == Vector4(-1, -2, -3, -4), "Rect4 abs should move the position to the negative end.");
	CHECK_MESSAGE(absolute_rect.size == Vector4(1, 2, 3, 4), "Rect4 abs should make the size positive.");

	Rect4 mutable_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(2, 2, 2, 2));
	mutable_rect.set_center(Vector4(5, 5, 5, 5));
	CHECK_MESSAGE(mutable_rect.position == Vector4(4, 4, 4, 4), "Rect4 set_center should move the position and keep the size.");
	CHECK_MESSAGE(mutable_rect.size == Vector4(2, 2, 2, 2), "Rect4 set_center should keep the size.");
	mutable_rect.set_end(Vector4(10, 10, 10, 10));
	CHECK_MESSAGE(mutable_rect.position == Vector4(4, 4, 4, 4), "Rect4 set_end should keep the position.");
	CHECK_MESSAGE(mutable_rect.size == Vector4(6, 6, 6, 6), "Rect4 set_end should adjust the size to reach the new end.");
}

TEST_CASE("[Rect4] Point functions") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	CHECK_MESSAGE(unit_rect.has_point(Vector4(0.5f, 0.5f, 0.5f, 0.5f)), "Rect4 has_point should contain an interior point.");
	CHECK_MESSAGE(unit_rect.has_point(Vector4(0, 0, 0, 0)), "Rect4 has_point should be inclusive of the position corner.");
	CHECK_MESSAGE(unit_rect.has_point(Vector4(1, 1, 1, 1)), "Rect4 has_point should be inclusive of the end corner.");
	CHECK_MESSAGE(!unit_rect.has_point(Vector4(1.5f, 0.5f, 0.5f, 0.5f)), "Rect4 has_point should not contain a point past the end.");
	CHECK_MESSAGE(!unit_rect.has_point(Vector4(-0.5f, 0.5f, 0.5f, 0.5f)), "Rect4 has_point should not contain a point before the position.");
	CHECK_MESSAGE(!unit_rect.has_point(Vector4(0.5f, 0.5f, 0.5f, 5)), "Rect4 has_point should check the W axis too.");

	CHECK_MESSAGE(unit_rect.get_nearest_point(Vector4(0.5f, 0.5f, 0.5f, 0.5f)) == Vector4(0.5f, 0.5f, 0.5f, 0.5f), "Rect4 get_nearest_point of an interior point should be the point itself.");
	CHECK_MESSAGE(unit_rect.get_nearest_point(Vector4(5, 0.5f, 0.5f, 0.5f)) == Vector4(1, 0.5f, 0.5f, 0.5f), "Rect4 get_nearest_point should clamp to the end.");
	CHECK_MESSAGE(unit_rect.get_nearest_point(Vector4(-5, -5, 5, 5)) == Vector4(0, 0, 1, 1), "Rect4 get_nearest_point should clamp each axis independently.");

	CHECK_MESSAGE(unit_rect.get_support_point(Vector4(1, 1, 1, 1)) == Vector4(1, 1, 1, 1), "Rect4 get_support_point in the all-positive direction should be the end corner.");
	CHECK_MESSAGE(unit_rect.get_support_point(Vector4(-1, -1, -1, -1)) == Vector4(0, 0, 0, 0), "Rect4 get_support_point in the all-negative direction should be the position corner.");
	CHECK_MESSAGE(unit_rect.get_support_point(Vector4(1, -1, 1, -1)) == Vector4(1, 0, 1, 0), "Rect4 get_support_point should pick each axis independently.");
	CHECK_MESSAGE(unit_rect.get_support_point(Vector4(0, 0, 0, 0)) == Vector4(0, 0, 0, 0), "Rect4 get_support_point with a zero direction should be the position corner.");

	Rect4 expanded = unit_rect.expand_to_point(Vector4(0.5f, 0.5f, 0.5f, 0.5f));
	CHECK_MESSAGE(expanded.position == Vector4(0, 0, 0, 0), "Rect4 expand_to_point with an interior point should not move the position.");
	CHECK_MESSAGE(expanded.size == Vector4(1, 1, 1, 1), "Rect4 expand_to_point with an interior point should not change the size.");
	expanded = unit_rect.expand_to_point(Vector4(2, 0.5f, 0.5f, 0.5f));
	CHECK_MESSAGE(expanded.position == Vector4(0, 0, 0, 0), "Rect4 expand_to_point past the end should not move the position.");
	CHECK_MESSAGE(expanded.size == Vector4(2, 1, 1, 1), "Rect4 expand_to_point past the end should grow the size.");
	expanded = unit_rect.expand_to_point(Vector4(-1, 0.5f, 0.5f, 0.5f));
	CHECK_MESSAGE(expanded.position == Vector4(-1, 0, 0, 0), "Rect4 expand_to_point before the position should move the position.");
	CHECK_MESSAGE(expanded.size == Vector4(2, 1, 1, 1), "Rect4 expand_to_point before the position should grow the size to keep the end.");

	Rect4 self_expanded = unit_rect;
	self_expanded.expand_self_to_point(Vector4(-1, 0.5f, 0.5f, 2));
	CHECK_MESSAGE(self_expanded.position == Vector4(-1, 0, 0, 0), "Rect4 expand_self_to_point should move the position when the point is before it.");
	CHECK_MESSAGE(self_expanded.size == Vector4(2, 1, 1, 2), "Rect4 expand_self_to_point should grow the size in both directions at once.");
}

TEST_CASE("[Rect4] Rect math functions") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	const Rect4 grown = unit_rect.grow(1.0f);
	CHECK_MESSAGE(grown.position == Vector4(-1, -1, -1, -1), "Rect4 grow should move the position outwards.");
	CHECK_MESSAGE(grown.size == Vector4(3, 3, 3, 3), "Rect4 grow should grow the size by twice the amount.");
	const Rect4 shrunk = unit_rect.grow(-0.25f);
	CHECK_MESSAGE(shrunk.position == Vector4(0.25f, 0.25f, 0.25f, 0.25f), "Rect4 grow with a negative amount should shrink the rect.");
	CHECK_MESSAGE(shrunk.size == Vector4(0.5f, 0.5f, 0.5f, 0.5f), "Rect4 grow with a negative amount should shrink the size.");

	// An intersection where only the position is clamped, so the size must shrink to match.
	const Rect4 big_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(10, 10, 10, 10));
	const Rect4 offset_rect = Rect4(Vector4(5, 5, 5, 5), Vector4(10, 10, 10, 10));
	Rect4 intersected = big_rect.intersection(offset_rect);
	CHECK_MESSAGE(intersected.position == Vector4(5, 5, 5, 5), "Rect4 intersection should clamp the position to the larger of the two.");
	CHECK_MESSAGE(intersected.get_end() == Vector4(10, 10, 10, 10), "Rect4 intersection should clamp the end to the smaller of the two.");
	CHECK_MESSAGE(intersected.size == Vector4(5, 5, 5, 5), "Rect4 intersection size should follow the clamped position and end.");
	// An intersection fully contained within this rect.
	const Rect4 inner_rect = Rect4(Vector4(2, 2, 2, 2), Vector4(3, 3, 3, 3));
	intersected = big_rect.intersection(inner_rect);
	CHECK_MESSAGE(intersected.position == Vector4(2, 2, 2, 2), "Rect4 intersection with an enclosed rect should give the enclosed position.");
	CHECK_MESSAGE(intersected.size == Vector4(3, 3, 3, 3), "Rect4 intersection with an enclosed rect should give the enclosed size.");
	// No intersection at all.
	const Rect4 far_rect = Rect4(Vector4(100, 100, 100, 100), Vector4(1, 1, 1, 1));
	intersected = big_rect.intersection(far_rect);
	CHECK_MESSAGE(!intersected.has_any_size(), "Rect4 intersection of non-overlapping rects should have no size.");

	Rect4 merged = unit_rect.merge(Rect4(Vector4(5, 5, 5, 5), Vector4(1, 1, 1, 1)));
	CHECK_MESSAGE(merged.position == Vector4(0, 0, 0, 0), "Rect4 merge should keep the smaller position.");
	CHECK_MESSAGE(merged.get_end() == Vector4(6, 6, 6, 6), "Rect4 merge should keep the larger end.");
	merged = unit_rect.merge(Rect4(Vector4(-5, -5, -5, -5), Vector4(1, 1, 1, 1)));
	CHECK_MESSAGE(merged.position == Vector4(-5, -5, -5, -5), "Rect4 merge should take the smaller position.");
	CHECK_MESSAGE(merged.size == Vector4(6, 6, 6, 6), "Rect4 merge should grow the size to enclose both rects.");
}

TEST_CASE("[Rect4] Rect comparison functions") {
	const Rect4 outer_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(10, 10, 10, 10));
	const Rect4 inner_rect = Rect4(Vector4(2, 2, 2, 2), Vector4(3, 3, 3, 3));
	CHECK_MESSAGE(outer_rect.encloses_exclusive(inner_rect), "Rect4 encloses_exclusive should be true for a strictly enclosed rect.");
	CHECK_MESSAGE(outer_rect.encloses_inclusive(inner_rect), "Rect4 encloses_inclusive should be true for a strictly enclosed rect.");
	CHECK_MESSAGE(!outer_rect.encloses_exclusive(outer_rect), "Rect4 encloses_exclusive should be false for an identical rect.");
	CHECK_MESSAGE(outer_rect.encloses_inclusive(outer_rect), "Rect4 encloses_inclusive should be true for an identical rect.");
	CHECK_MESSAGE(!inner_rect.encloses_inclusive(outer_rect), "Rect4 encloses_inclusive should be false when the argument is the larger rect.");

	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	const Rect4 touching_rect = Rect4(Vector4(1, 0, 0, 0), Vector4(1, 1, 1, 1));
	CHECK_MESSAGE(!unit_rect.intersects_exclusive(touching_rect), "Rect4 intersects_exclusive should be false for rects that only share a face.");
	CHECK_MESSAGE(unit_rect.intersects_inclusive(touching_rect), "Rect4 intersects_inclusive should be true for rects that only share a face.");
	const Rect4 separated_rect = Rect4(Vector4(2, 0, 0, 0), Vector4(1, 1, 1, 1));
	CHECK_MESSAGE(!unit_rect.intersects_inclusive(separated_rect), "Rect4 intersects_inclusive should be false for separated rects.");
	const Rect4 overlapping_rect = Rect4(Vector4(0.5f, 0.5f, 0.5f, 0.5f), Vector4(1, 1, 1, 1));
	CHECK_MESSAGE(unit_rect.intersects_exclusive(overlapping_rect), "Rect4 intersects_exclusive should be true for genuinely overlapping rects.");

	CHECK_MESSAGE(unit_rect.is_equal_approx(Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1))), "Rect4 is_equal_approx should be true for identical rects.");
	CHECK_MESSAGE(!unit_rect.is_equal_approx(touching_rect), "Rect4 is_equal_approx should be false for different rects.");
	CHECK_MESSAGE(unit_rect == Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1)), "Rect4 operator== should be true for identical rects.");
	CHECK_MESSAGE(unit_rect != touching_rect, "Rect4 operator!= should be true for different rects.");
	CHECK_MESSAGE(unit_rect.is_finite(), "Rect4 is_finite should be true for a finite rect.");
	CHECK_MESSAGE(!Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, (real_t)Math_INF)).is_finite(), "Rect4 is_finite should be false when any component is infinite.");
}

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
	depth = unit_rect.continuous_collision_depth(Vector4(-0.0f, -0.0f, -0.0f, 5), offset_3w_rect, &normal);
	CHECK_MESSAGE(depth == doctest::Approx(0.4f), "Rect4 continuous_collision_depth with negative zero components should be stopped by the obstacle.");
	CHECK_MESSAGE(normal == Vector4(0, 0, 0, -1), "Rect4 continuous_collision_depth with negative zero components should give the correct normal.");
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

TEST_CASE("[Rect4] Continuous Collision Overlaps") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	const Rect4 offset_3w_rect = Rect4(Vector4(0, 0, 0, 3), Vector4(1, 1, 1, 1));
	bool overlaps = unit_rect.continuous_collision_overlaps(Vector4(-0.0f, -0.0f, -0.0f, 5), offset_3w_rect);
	CHECK_MESSAGE(overlaps, "Rect4 continuous_collision_overlaps with negative zero components should overlap the obstacle.");
	overlaps = unit_rect.continuous_collision_overlaps(Vector4(-0.0f, -0.0f, -0.0f, -5), offset_3w_rect);
	CHECK_MESSAGE(!overlaps, "Rect4 continuous_collision_overlaps with negative zero components and motion away should not overlap the obstacle.");
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
	// When inside_is_zero=false, the ray should hit the forward exit surface, not the entry surface behind the origin.
	hit = unit_rect.raycast_intersects(Vector4(0.5, 0.5, 0.5, 0.5), Vector4(1, 0, 0, 0), false, &distance, &normal);
	CHECK_MESSAGE(hit == true, "Raycast from inside should hit with inside_is_zero=false");
	CHECK_MESSAGE(distance == doctest::Approx(0.5f), "Raycast from inside should return the distance to the forward exit surface");
	CHECK_MESSAGE(normal == Vector4(1, 0, 0, 0), "Raycast from inside should return the outward normal of the exit surface");
}

TEST_CASE("[Rect4] Raycast max distance is exclusive") {
	const Rect4 unit_rect = Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	const Vector4 origin = Vector4(-2, 0.5, 0.5, 0.5);
	const Vector4 direction = Vector4(1, 0, 0, 0);
	Dictionary result = unit_rect.raycast_intersects_dict(origin, direction, 1.999, false);
	CHECK_FALSE((bool)result["hit"]);
	result = unit_rect.raycast_intersects_dict(origin, direction, 2.0, false);
	CHECK_FALSE((bool)result["hit"]);
	result = unit_rect.raycast_intersects_dict(origin, direction, 2.001, false);
	REQUIRE((bool)result["hit"]);
	CHECK((double)result["distance"] == doctest::Approx(2.0));
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
