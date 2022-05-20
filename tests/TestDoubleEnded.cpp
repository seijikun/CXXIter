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
		ASSERT_EQ(src.next().value(), 1);
		ASSERT_EQ(src.nextBack().value(), 2);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
	{
		std::vector<float> input = {1.337f, 2.338f, 3.339f};
		auto src = CXXIter::from(std::move(input)).cast<size_t>();
		ASSERT_EQ(src.nextBack().value(), 3);
		ASSERT_EQ(src.nextBack().value(), 2);
		ASSERT_EQ(src.nextBack().value(), 1);
		ASSERT_FALSE(src.nextBack().has_value());
		ASSERT_FALSE(src.next().has_value());
	}
}
