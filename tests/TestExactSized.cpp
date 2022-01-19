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


TEST(CXXIter, ExactSizedEmptySource) {
	{
		auto iter = CXXIter::empty<std::string>();
		ASSERT_EQ(0, iter.size());
	}
	{
		auto iter = CXXIter::empty<std::string>()
				.map([](std::string blub) { return blub.size(); })
				.sort()
				.intersperse(CXXIter::empty<size_t>());
		ASSERT_EQ(0, iter.size());
	}
	{
		auto iter = CXXIter::empty<std::string>()
				.filter([](const std::string&) { return true; });
		static_assert(!CXXIterExactSizeIterator<decltype(iter)>);
	}
	{
		auto iter = CXXIter::empty<std::string>()
				.filterMap([](const std::string&) -> std::optional<bool> { return true; });
		static_assert(!CXXIterExactSizeIterator<decltype(iter)>);
	}
}

TEST(CXXIter, ExactSizedRepeatSource) {
	{
		auto iter = CXXIter::repeat<std::string>("asdf");
		ASSERT_EQ(iter.size(), SizeHint::INFINITE);
	}
	{
		auto iter = CXXIter::repeat<std::string>("test")
				.map([](std::string blub) { return blub.size(); })
				.sort()
				.intersperse(CXXIter::empty<size_t>());
		ASSERT_EQ(iter.size(), 1);
	}
	{
		auto iter = CXXIter::repeat<std::string>("test")
				.map([](std::string blub) { return blub.size(); })
				.sort()
				.intersperse(CXXIter::repeat<size_t>(1337));
		ASSERT_EQ(iter.size(), SizeHint::INFINITE);
	}
	{
		auto iter = CXXIter::repeat<std::string>("test")
				.map([](std::string blub) { return blub.size(); })
				.sort()
				.intersperse(CXXIter::repeat<size_t>(1337))
				.filterMap([](size_t item) -> std::optional<size_t> { return item; });
		static_assert(!CXXIterExactSizeIterator<decltype(iter)>);
	}
	{
		auto iter = CXXIter::repeat<std::string>("test")
				.map([](std::string blub) { return blub.size(); })
				.sort()
				.intersperse(CXXIter::repeat<size_t>(1337))
				.filter([](size_t) { return true; });
		static_assert(!CXXIterExactSizeIterator<decltype(iter)>);
	}
}

TEST(CXXIter, ExactSizedRangeSource) {
	{
		auto iter = CXXIter::range<float>(0.0f, 2.0f, 0.25f);
		ASSERT_EQ(iter.size(), 9);
	}
	{
		auto iter = CXXIter::range<float>(0.0f, 2.0f, 0.25f)
				.map([](float blub) { return std::to_string(blub); })
				.sort()
				.intersperse(CXXIter::empty<std::string>());
		ASSERT_EQ(iter.size(), 1);
	}
	{
		auto iter = CXXIter::range<float>(0.0f, 2.0f, 0.25f)
				.map([](float blub) { return std::to_string(blub); })
				.sort()
				.intersperse(CXXIter::empty<std::string>())
				.flatMap();
		static_assert(!CXXIterExactSizeIterator<decltype(iter)>);
	}
}

TEST(CXXIter, ExactSizedItemSource) {
	const std::vector<int> input = {1337, 42, 64};
	{
		auto iter = CXXIter::from(input);
		ASSERT_EQ(iter.size(), input.size());
	}
	{
		auto iter = CXXIter::from(input)
				.skip(1)
				.take(10)
				.map([](int blub) { return std::to_string(blub); })
				.sort()
				.intersperse(CXXIter::empty<std::string>());
		ASSERT_EQ(iter.size(), 1);
	}
	{
		auto iter = CXXIter::from(input)
				.map([](int blub) { return std::to_string(blub); })
				.sort()
				.intersperse(CXXIter::repeat<std::string>(","))
				.skip(1)
				.take(3);
		ASSERT_EQ(iter.size(), 3);
	}
	{
		auto iter = CXXIter::from(input)
				.map([](int blub) { return std::to_string(blub); })
				.sort()
				.intersperse(CXXIter::repeat<std::string>(","))
				.flatMap();
		static_assert(!CXXIterExactSizeIterator<decltype(iter)>);
	}
	{
		auto iter = CXXIter::from(input)
				.map([](int blub) { return std::to_string(blub); })
				.sort()
				.intersperse(CXXIter::repeat<std::string>(","))
				.filter([](const std::string&) { return true; });
		static_assert(!CXXIterExactSizeIterator<decltype(iter)>);
	}
}


//TODO: skip(cnt) and take(cnt)
