#include <vector>

#include "TestCommon.h"

// ################################################################################################
// HELPERS
// ################################################################################################
TEST(CXXIter, unzip) {
	std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
	std::vector<std::pair<size_t, float>> output = CXXIter::from(input).copied()
		.indexed()
		.sortBy<CXXIter::DESCENDING>(CXXIter::unzip<1>())
		.collect<std::vector>();
	ASSERT_EQ(input.size(), output.size());
	ASSERT_THAT(output, ElementsAre(Pair(3, 3.0f), Pair(1, 2.0f), Pair(0, 1.0f), Pair(2, 0.5f), Pair(4, -42.0f)));
}
