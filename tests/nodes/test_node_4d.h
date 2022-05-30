#pragma once

#include "../../nodes/node_4d.h"
#include "tests/test_macros.h"

namespace TestNode4D {
TEST_CASE("[Node4D] Global Transform") {
	Node4D parent = Node4D();
	Node4D child = Node4D();
	parent.add_child(&child);

	Transform4D parent_transform = Transform4D(Basis4D::from_scale_uniform(2), Vector4(1, 3, 5, 7));
	Transform4D child_transform = Transform4D(Basis4D::from_scale_uniform(3), Vector4(0.1, 0.2, 0.3, 0.4));
	parent.set_transform(parent_transform);
	child.set_transform(child_transform);
	CHECK_MESSAGE(child.get_global_position().is_equal_approx(Vector4(1.2, 3.4, 5.6, 7.8)), "Node4D global position should work as expected.");
	CHECK_MESSAGE(child.get_global_scale().is_equal_approx(Vector4(6, 6, 6, 6)), "Node4D global scale should work as expected.");
	CHECK_MESSAGE(child.get_global_transform().is_equal_approx(parent_transform * child_transform), "Node4D global transform should work as expected.");

	// ZX is the first rotation, so if we make this the parent rotation the test is intuitive (just add the ZX rotations).
	parent_transform.rotate_local(Euler4D(0.0, Math_TAU / 4.0, 0.0));
	child_transform.rotate_local(Euler4D(0.1, 0.2, 0.3));
	parent.set_transform(parent_transform);
	child.set_transform(child_transform);
	CHECK_MESSAGE(child.get_global_position().is_equal_approx(Vector4(1.6, 3.4, 4.8, 7.8)), "Node4D global position should work as expected.");
	CHECK_MESSAGE(child.get_global_rotation().is_equal_approx(Euler4D(0.1, Math_TAU / 4.0 + 0.2, 0.3)), "Node4D global rotation should work as expected.");
	CHECK_MESSAGE(child.get_global_scale().is_equal_approx(Vector4(6, 6, 6, 6)), "Node4D global scale should work as expected.");
	CHECK_MESSAGE(child.get_global_transform().is_equal_approx(parent_transform * child_transform), "Node4D global transform should work as expected.");
}
} // namespace TestNode4D
