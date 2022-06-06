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


TEST(CXXIter, randomAccessSources) {
	{ // SrcCRef
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f, 1.340f, 1.350f};
			auto src = SrcCRef(input);
			ASSERT_EQ(src.next().value(), 1.337f);
			src.advanceBy(2);
			ASSERT_EQ(src.next().value(), 1.340f);
			ASSERT_EQ(src.next().value(), 1.350f);
			ASSERT_FALSE(src.next().has_value());
		}
	}
	{ // SrcRef
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f, 1.340f, 1.350f};
			auto src = SrcRef(input);
			ASSERT_EQ(src.next().value(), 1.337f);
			src.advanceBy(2);
			ASSERT_EQ(src.next().value(), 1.340f);
			ASSERT_EQ(src.next().value(), 1.350f);
			ASSERT_FALSE(src.next().has_value());
		}
	}
	{ // SrcMov
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f, 1.340f, 1.350f};
			auto src = SrcMov(std::move(input));
			ASSERT_EQ(src.next().value(), 1.337f);
			src.advanceBy(2);
			ASSERT_EQ(src.next().value(), 1.340f);
			ASSERT_EQ(src.next().value(), 1.350f);
			ASSERT_FALSE(src.next().has_value());
		}
	}
}

TEST(CXXIter, randomAccessCast) {
	std::vector<float> input = {1.337f, 2.338f, 3.339f, 4.340f, 5.350f};
	auto src = CXXIter::from(std::move(input)).cast<size_t>();
	ASSERT_EQ(src.next().value(), 1);
	src.advanceBy(2);
	ASSERT_EQ(src.next().value(), 4);
	ASSERT_EQ(src.next().value(), 5);
	ASSERT_FALSE(src.next().has_value());
}

TEST(CXXIter, randomAccessChainer) {
	std::vector<std::string> input1 = {"1337", "42"};
	std::vector<std::string> input2 = {"31337", "64", "69"};
	auto src = CXXIter::from(std::move(input1))
			.chain(CXXIter::from(std::move(input2)));
	ASSERT_EQ(src.next().value(), "1337");
	src.advanceBy(2);
	ASSERT_EQ(src.next().value(), "64");
	ASSERT_EQ(src.next().value(), "69");
	ASSERT_FALSE(src.next().has_value());
}

TEST(CXXIter, randomAccessFilter) {
	std::vector<float> input = {1.337f, 2.338f, 0.01f, 3.339f, 4.340f, 5.350f};
	auto src = CXXIter::from(std::move(input)).filter([](float val) { return val > 2.0f; });
	ASSERT_EQ(src.next().value(), 2.338f);
	src.advanceBy(2);
	ASSERT_EQ(src.next().value(), 5.350f);
	ASSERT_FALSE(src.next().has_value());
}

TEST(CXXIter, randomAccessFilterMap) {
	std::vector<float> input = {1.337f, 2.338f, 0.01f, 3.339f, 4.340f, 5.1f};
	auto src = CXXIter::from(std::move(input)).filterMap([](float val) -> std::optional<size_t> {
		if(val <= 2.0f) { return {}; }
		return static_cast<size_t>(val);
	});
	ASSERT_EQ(src.next().value(), 2);
	src.advanceBy(2);
	ASSERT_EQ(src.next().value(), 5);
	ASSERT_FALSE(src.next().has_value());
}

TEST(CXXIter, randomAccessModify) {
	std::vector<std::pair<int, std::string>> input = { {1337, "1337"}, {42, "42"}, {69, "69"}, {31337, "31337"} };
	auto src = CXXIter::from(input)
			.modify([](auto& keyValue) {
				keyValue.second += "-" + std::to_string(keyValue.first + 1);
			});
	ASSERT_TRUE(src.next().has_value());
	src.advanceBy(2);
	ASSERT_TRUE(src.next().has_value());
	ASSERT_FALSE(src.next().has_value());

	ASSERT_THAT(input, ElementsAre( Pair(1337, "1337-1338"), Pair(42, "42"), Pair(69, "69"), Pair(31337, "31337-31338") ));
}

TEST(CXXIter, randomAccessMap) {
	size_t iterState = 0;
	std::vector<float> input = {1.337f, 2.338f, 3.339f, 4.340f, 0.1f};
	auto src = CXXIter::from(input).map([iterState](float val) mutable -> size_t {
		iterState += 100;
		return static_cast<size_t>(val) + iterState;
	});
	ASSERT_EQ(src.next().value(), 101);
	src.advanceBy(2);
	ASSERT_EQ(src.next().value(), 404);
	ASSERT_EQ(src.next().value(), 500);
	ASSERT_FALSE(src.next().has_value());
}

//TEST(CXXIter, doubleEndedSort) {
//	{ // ASCENDING
//		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
//		auto src = CXXIter::from(input)
//			.sort<false>([](const float& a, const float& b) { return (a < b); });
//		ASSERT_EQ(src.nextBack().value(), 3.0f);
//		ASSERT_EQ(src.nextBack().value(), 2.0f);
//		ASSERT_EQ(src.nextBack().value(), 1.0f);
//		ASSERT_EQ(src.nextBack().value(), 0.5f);
//		ASSERT_EQ(src.nextBack().value(), -42.0f);
//		ASSERT_FALSE(src.nextBack().has_value());
//		ASSERT_FALSE(src.next().has_value());
//	}
//	{ // ASCENDING
//		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
//		auto src = CXXIter::from(input)
//			.sort<false>([](const float& a, const float& b) { return (a < b); });
//		ASSERT_EQ(src.nextBack().value(), 3.0f);
//		ASSERT_EQ(src.next().value(), -42.0f);
//		ASSERT_EQ(src.next().value(), 0.5f);
//		ASSERT_EQ(src.nextBack().value(), 2.0f);
//		ASSERT_EQ(src.nextBack().value(), 1.0f);
//		ASSERT_FALSE(src.nextBack().has_value());
//		ASSERT_FALSE(src.next().has_value());
//	}
//	{ // DESCENDING
//		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
//		auto src = CXXIter::from(input)
//			.sort<CXXIter::DESCENDING, false>();
//		ASSERT_EQ(src.nextBack().value(), -42.0f);
//		ASSERT_EQ(src.nextBack().value(), 0.5f);
//		ASSERT_EQ(src.nextBack().value(), 1.0f);
//		ASSERT_EQ(src.nextBack().value(), 2.0f);
//		ASSERT_EQ(src.nextBack().value(), 3.0f);
//		ASSERT_FALSE(src.nextBack().has_value());
//		ASSERT_FALSE(src.next().has_value());
//	}
//	{ // DESCENDING
//		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
//		auto src = CXXIter::from(input)
//				.sort<CXXIter::DESCENDING, false>();
//		ASSERT_EQ(src.nextBack().value(), -42.0f);
//		ASSERT_EQ(src.next().value(), 3.0f);
//		ASSERT_EQ(src.next().value(), 2.0f);
//		ASSERT_EQ(src.nextBack().value(), 0.5f);
//		ASSERT_EQ(src.nextBack().value(), 1.0f);
//		ASSERT_FALSE(src.nextBack().has_value());
//		ASSERT_FALSE(src.next().has_value());
//	}
//}
