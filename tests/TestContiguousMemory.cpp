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

using namespace CXXIter;


TEST(CXXIter, ContiguousMemoryCompile) {
	std::vector<uint32_t> src = {1, 3, 3, 7};
	auto rawIter = CXXIter::from(src);
	auto skipIter = CXXIter::from(src).skip(1);
	auto filterIter = CXXIter::from(src).filter([](uint32_t val) { return (val > 3); });

	static_assert(CXXIterContiguousMemoryIterator<decltype(rawIter)>);
	static_assert(CXXIterContiguousMemoryIterator<decltype(skipIter)>);
	static_assert(!CXXIterContiguousMemoryIterator<decltype(filterIter)>);
}

TEST(CXXIter, ContiguousSource) {
	{ // SrcCRef
		const std::vector<uint32_t> src = {1, 3, 3, 7};
		auto iter = CXXIter::from(src);
		using ContiguousTrait = trait::ContiguousMemoryIterator<decltype(iter)>;
		ContiguousTrait::ItemPtr itemPtr = ContiguousTrait::currentPtr(iter);
		ASSERT_EQ(itemPtr, &src[0]);
	}
	{ // SrcRef
		std::vector<uint32_t> src = {1, 3, 3, 7};
		auto iter = CXXIter::from(src);
		using ContiguousTrait = trait::ContiguousMemoryIterator<decltype(iter)>;
		ContiguousTrait::ItemPtr itemPtr = ContiguousTrait::currentPtr(iter);
		ASSERT_EQ(itemPtr, &src[0]);
	}
	{ // SrcMov
		std::vector<uint32_t> src = {1, 3, 3, 7};
		// The src will be moved into the iterator, but the heap memory region for the
		// actual vector elements will stay the same, so we take the pointer before
		// constructing an iterator here.
		uint32_t* firstElementPtr = &src[0];
		auto iter = CXXIter::from(std::move(src));
		using ContiguousTrait = trait::ContiguousMemoryIterator<decltype(iter)>;
		ContiguousTrait::ItemPtr itemPtr = ContiguousTrait::currentPtr(iter);
		ASSERT_EQ(itemPtr, firstElementPtr);
	}
}

TEST(CXXIter, ContiguousSkipN) {
	std::vector<uint32_t> src = {1, 3, 3, 7};
	auto iter = CXXIter::from(src).skip(1);
	using ContiguousTrait = trait::ContiguousMemoryIterator<decltype(iter)>;
	ContiguousTrait::ItemPtr itemPtr = ContiguousTrait::currentPtr(iter);
	ASSERT_EQ(itemPtr, &src[1]);
}
