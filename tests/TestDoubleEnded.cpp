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


TEST(CXXIter, doubleEndedSources) {
	{ // SrcCRef
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f};
			auto src = SrcCRef(input);
			ASSERT_EQ(src.nextBack().value(), 1.339f);
			ASSERT_EQ(src.nextBack().value(), 1.338f);
			ASSERT_EQ(src.nextBack().value(), 1.337f);
			ASSERT_FALSE(src.nextBack().has_value());
			ASSERT_FALSE(src.next().has_value());
		}
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f};
			auto src = SrcCRef(input);
			ASSERT_EQ(src.nextBack().value(), 1.339f);
			ASSERT_EQ(src.next().value(), 1.337f);
			ASSERT_EQ(src.nextBack().value(), 1.338f);
			ASSERT_FALSE(src.nextBack().has_value());
			ASSERT_FALSE(src.next().has_value());
		}
		{
			std::vector<float> input = {};
			ASSERT_FALSE(SrcCRef(input).nextBack().has_value());
		}
	}
	{ // SrcRef
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f};
			auto src = SrcRef(input);
			ASSERT_EQ(src.nextBack().value(), 1.339f);
			ASSERT_EQ(src.nextBack().value(), 1.338f);
			ASSERT_EQ(src.nextBack().value(), 1.337f);
			ASSERT_FALSE(src.nextBack().has_value());
			ASSERT_FALSE(src.next().has_value());
		}
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f};
			auto src = SrcRef(input);
			ASSERT_EQ(src.nextBack().value(), 1.339f);
			ASSERT_EQ(src.next().value(), 1.337f);
			ASSERT_EQ(src.nextBack().value(), 1.338f);
			ASSERT_FALSE(src.nextBack().has_value());
			ASSERT_FALSE(src.next().has_value());
		}
		{
			std::vector<float> input = {};
			ASSERT_FALSE(SrcRef(input).nextBack().has_value());
		}
	}
	{ // SrcMov
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f};
			auto src = SrcMov(std::move(input));
			ASSERT_EQ(src.nextBack().value(), 1.339f);
			ASSERT_EQ(src.nextBack().value(), 1.338f);
			ASSERT_EQ(src.nextBack().value(), 1.337f);
			ASSERT_FALSE(src.nextBack().has_value());
			ASSERT_FALSE(src.next().has_value());
		}
		{
			std::vector<float> input = {1.337f, 1.338f, 1.339f};
			auto src = SrcMov(std::move(input));
			ASSERT_EQ(src.nextBack().value(), 1.339f);
			ASSERT_EQ(src.next().value(), 1.337f);
			ASSERT_EQ(src.nextBack().value(), 1.338f);
			ASSERT_FALSE(src.nextBack().has_value());
			ASSERT_FALSE(src.next().has_value());
		}
		{
			std::vector<float> input = {};
			ASSERT_FALSE(SrcMov(std::move(input)).nextBack().has_value());
		}
	}
}

TEST(CXXIter, doubleEndedCast) {
	{
		std::vector<float> input = {1.337f, 2.338f, 3.339f};
		auto src = CXXIter::from(std::move(input)).cast<size_t>();
		ASSERT_EQ(src.nextBack().value(), 3);
		ASSERT_EQ(src.nextBack().value(), 2);
		ASSERT_EQ(src.nextBack().value(), 1);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{
		std::vector<float> input = {1.337f, 2.338f, 3.339f};
		auto src = CXXIter::from(std::move(input)).cast<size_t>();
		ASSERT_EQ(src.nextBack().value(), 3);
		ASSERT_EQ(src.next().value(), 1);
		ASSERT_EQ(src.nextBack().value(), 2);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
}

TEST(CXXIter, doubleEndedChainer) {
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<std::string> input2 = {"31337", "64"};
		auto src = CXXIter::from(std::move(input1))
				.chain(CXXIter::from(std::move(input2)));
		ASSERT_EQ(src.nextBack().value(), "64");
		ASSERT_EQ(src.nextBack().value(), "31337");
		ASSERT_EQ(src.nextBack().value(), "42");
		ASSERT_EQ(src.nextBack().value(), "1337");
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<std::string> input2 = {"31337", "64"};
		auto src = CXXIter::from(std::move(input1))
				.chain(CXXIter::from(std::move(input2)));
		ASSERT_EQ(src.nextBack().value(), "64");
		ASSERT_EQ(src.next().value(), "1337");
		ASSERT_EQ(src.next().value(), "42");
		ASSERT_EQ(src.nextBack().value(), "31337");
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
}

TEST(CXXIter, doubleEndedFilter) {
	{
		std::vector<float> input = {1.337f, 2.338f, 3.339f, 4.340f, 0.1f};
		auto src = CXXIter::from(std::move(input)).filter([](float val) { return val > 2.0f; });
		ASSERT_EQ(src.nextBack().value(), 4.340f);
		ASSERT_EQ(src.nextBack().value(), 3.339f);
		ASSERT_EQ(src.nextBack().value(), 2.338f);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{
		std::vector<float> input = {1.337f, 2.338f, 3.339f, 4.340f, 0.1f};
		auto src = CXXIter::from(std::move(input)).filter([](float val) { return val > 2.0f; });
		ASSERT_EQ(src.nextBack().value(), 4.340f);
		ASSERT_EQ(src.next().value(), 2.338f);
		ASSERT_EQ(src.nextBack().value(), 3.339f);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
}

TEST(CXXIter, doubleEndedFilterMap) {
	{
		std::vector<float> input = {1.337f, 2.338f, 3.339f, 4.340f, 0.1f};
		auto src = CXXIter::from(std::move(input)).filterMap([](float val) -> std::optional<size_t> {
			if(val <= 2.0f) { return {}; }
			return static_cast<size_t>(val);
		});
		ASSERT_EQ(src.nextBack().value(), 4);
		ASSERT_EQ(src.nextBack().value(), 3);
		ASSERT_EQ(src.nextBack().value(), 2);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{
		std::vector<float> input = {1.337f, 2.338f, 3.339f, 4.340f, 0.1f};
		auto src = CXXIter::from(std::move(input)).filterMap([](float val) -> std::optional<size_t> {
			if(val <= 2.0f) { return {}; }
			return static_cast<size_t>(val);
		});
		ASSERT_EQ(src.nextBack().value(), 4);
		ASSERT_EQ(src.next().value(), 2);
		ASSERT_EQ(src.nextBack().value(), 3);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
}

TEST(CXXIter, doubleEndedModify) {
	{
		size_t iterState = 0;
		std::vector<std::pair<int, std::string>> input = { {1337, "1337"}, {42, "42"}, {69, "69"}, {31337, "31337"} };
		auto src = CXXIter::from(input)
				.modify([iterState](auto& keyValue) mutable {
					keyValue.second = std::to_string(iterState++) + "-" + keyValue.second;
				});
		for(size_t i = 0; i < input.size(); ++i) {
			ASSERT_TRUE(src.nextBack().has_value());
		}
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());

		ASSERT_THAT(input, ElementsAre( Pair(1337, "3-1337"), Pair(42, "2-42"), Pair(69, "1-69"), Pair(31337, "0-31337") ));
	}
}

TEST(CXXIter, doubleEndedMap) {
	{
		size_t iterState = 0;
		std::vector<float> input = {1.337f, 2.338f, 3.339f, 4.340f, 0.1f};
		auto src = CXXIter::from(input).map([iterState](float val) mutable -> size_t {
			iterState += 100;
			return static_cast<size_t>(val) + iterState;
		});
		ASSERT_EQ(src.nextBack().value(), 100);
		ASSERT_EQ(src.nextBack().value(), 204);
		ASSERT_EQ(src.nextBack().value(), 303);
		ASSERT_EQ(src.nextBack().value(), 402);
		ASSERT_EQ(src.nextBack().value(), 501);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{
		size_t iterState = 0;
		std::vector<float> input = {1.337f, 2.338f, 3.339f, 4.340f, 0.1f};
		auto src = CXXIter::from(input).map([iterState](float val) mutable -> size_t {
			iterState += 100;
			return static_cast<size_t>(val) + iterState;
		});
		ASSERT_EQ(src.nextBack().value(), 100);
		ASSERT_EQ(src.nextBack().value(), 204);
		ASSERT_EQ(src.next().value(), 301);
		ASSERT_EQ(src.next().value(), 402);
		ASSERT_EQ(src.nextBack().value(), 503);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
}

TEST(CXXIter, doubleEndedSort) {
	{ // ASCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		auto src = CXXIter::from(input)
			.sort<false>([](const float& a, const float& b) { return (a < b); });
		ASSERT_EQ(src.nextBack().value(), 3.0f);
		ASSERT_EQ(src.nextBack().value(), 2.0f);
		ASSERT_EQ(src.nextBack().value(), 1.0f);
		ASSERT_EQ(src.nextBack().value(), 0.5f);
		ASSERT_EQ(src.nextBack().value(), -42.0f);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{ // ASCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		auto src = CXXIter::from(input)
			.sort<false>([](const float& a, const float& b) { return (a < b); });
		ASSERT_EQ(src.nextBack().value(), 3.0f);
		ASSERT_EQ(src.next().value(), -42.0f);
		ASSERT_EQ(src.next().value(), 0.5f);
		ASSERT_EQ(src.nextBack().value(), 2.0f);
		ASSERT_EQ(src.nextBack().value(), 1.0f);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{ // DESCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		auto src = CXXIter::from(input)
			.sort<CXXIter::DESCENDING, false>();
		ASSERT_EQ(src.nextBack().value(), -42.0f);
		ASSERT_EQ(src.nextBack().value(), 0.5f);
		ASSERT_EQ(src.nextBack().value(), 1.0f);
		ASSERT_EQ(src.nextBack().value(), 2.0f);
		ASSERT_EQ(src.nextBack().value(), 3.0f);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{ // DESCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		auto src = CXXIter::from(input)
				.sort<CXXIter::DESCENDING, false>();
		ASSERT_EQ(src.nextBack().value(), -42.0f);
		ASSERT_EQ(src.next().value(), 3.0f);
		ASSERT_EQ(src.next().value(), 2.0f);
		ASSERT_EQ(src.nextBack().value(), 0.5f);
		ASSERT_EQ(src.nextBack().value(), 1.0f);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
}
