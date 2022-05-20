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


TEST(CXXIter, DoubleEndedSources) {
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
	}
}
