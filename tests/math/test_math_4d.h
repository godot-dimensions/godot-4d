#pragma once

#include "../../math/math_4d.h"

#include "tests/test_macros.h"

namespace TestMath4D {

TEST_CASE("[Math4D] Remap int32 indices through a table") {
	// Old elements 0..5 where 1 and 4 were removed, 2 and 3 were merged into new element 1, and 5 became 2.
	const PackedInt32Array old_to_new = { 0, -1, 1, 1, -1, 2 };
	const PackedInt32Array indices = { 0, 2, 3, 5, 1, 9 }; // 9 is out of the table's range.
	CHECK(Math4D::remap_int32_array(indices, old_to_new) == PackedInt32Array{ 0, 1, 1, 2, -1, -1 });
	HashSet<int32_t> index_set;
	index_set.insert(0);
	index_set.insert(1);
	index_set.insert(3);
	index_set.insert(5);
	index_set.insert(9);
	const HashSet<int32_t> remapped_set = Math4D::remap_int32_set(index_set, old_to_new);
	CHECK(remapped_set.size() == 3);
	CHECK(remapped_set.has(0));
	CHECK(remapped_set.has(1));
	CHECK(remapped_set.has(2));
	Vector<PackedInt32Array> arrays = { { 0, 2, 3, 5 }, { 1, 4 }, { 3, 2, 9 } };
	Vector<PackedInt32Array> deduplicated = arrays;
	Math4D::remap_int32_arrays(deduplicated, old_to_new, true);
	REQUIRE(deduplicated.size() == 3);
	CHECK(deduplicated[0] == PackedInt32Array{ 0, 1, 2 });
	CHECK(deduplicated[1] == PackedInt32Array{ -1 });
	CHECK(deduplicated[2] == PackedInt32Array{ 1, -1 });
	Vector<PackedInt32Array> positioned = arrays;
	Math4D::remap_int32_arrays(positioned, old_to_new, false);
	CHECK(positioned[0] == PackedInt32Array{ 0, 1, 1, 2 });
	CHECK(positioned[1] == PackedInt32Array{ -1, -1 });
	CHECK(positioned[2] == PackedInt32Array{ 1, 1, -1 });
}

} // namespace TestMath4D
