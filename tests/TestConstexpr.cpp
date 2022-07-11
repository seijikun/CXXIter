#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <optional>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <unordered_set>
#include <unordered_map>

#include "TestCommon.h"

// ################################################################################################
// CONSTEXPR
// ################################################################################################
TEST(CXXIter, constexprCorrectness) {
	{
		constexpr size_t output = CXXIter::range(0, 5).size();
		static_assert(output == 6);
	}
	{
		constexpr size_t output = CXXIter::range(0, 5).sum();
		static_assert(output == 15);
	}
	{
		constexpr std::array<size_t, 4> input = {42, 69, 77, 1337};
		constexpr size_t output = CXXIter::from(input).mean().value();
		static_assert(output == 381);
	}
}
