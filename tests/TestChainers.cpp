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

using CXXIter::SizeHint;

// ################################################################################################
// CHAINERS
// ################################################################################################
TEST(CXXIter, cast) {
	{ // sizeHint
		std::vector<float> input = {1.35, 56.123};
		SizeHint sizeHint = CXXIter::from(input).cast<double>().sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{
		std::vector<float> input = {1.35, 56.123};
		std::vector<double> output = CXXIter::from(input)
				.cast<double>()
				.collect<std::vector>();
		ASSERT_EQ(input.size(), output.size());
		for(size_t i = 0; i < input.size(); ++i) { ASSERT_NEAR(input[i], output[i], 0.000005); }
	}
}

TEST(CXXIter, copied) {
	{ // sizeHint
		std::vector<float> input = {1.35, 56.123};
		SizeHint sizeHint = CXXIter::from(input).copied().sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // copied
		std::vector<std::string> input = {"inputString1", "inputString2"};
		std::vector<std::string> output = CXXIter::from(input)
				.copied() // clone values, now working with owned copies instead of references to input
				.modify([](std::string& item) { item[item.size() - 1] += 1; }) // modify copies, input untouched
				.collect<std::vector>();
		ASSERT_EQ(2, output.size());
		ASSERT_THAT(input, ElementsAre("inputString1", "inputString2"));
		ASSERT_THAT(output, ElementsAre("inputString2", "inputString3"));
	}
	{ // uncopied
		std::vector<std::string> input = {"inputString1", "inputString2"};
		std::vector<std::string> output = CXXIter::from(input)
				.modify([](std::string& item) { item[item.size() - 1] += 1; }) // modify
				.collect<std::vector>();
		ASSERT_EQ(2, output.size());
		ASSERT_THAT(input, ElementsAre("inputString2", "inputString3"));
		ASSERT_THAT(output, ElementsAre("inputString2", "inputString3"));
	}
}

TEST(CXXIter, filter) {
	{ // sizeHint
		std::vector<float> input = {1.35, 56.123};
		SizeHint sizeHint = CXXIter::from(input)
				.filter([](int item) { return (item % 2) == 0; })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, 0);
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{
		std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
		std::vector<int> output = CXXIter::from(input)
				.filter([](int item) { return (item % 2) == 0; })
				.collect<std::vector>();
		ASSERT_EQ(4, output.size());
		ASSERT_THAT(output, ElementsAre(2, 4, 6, 8));
	}
	{
		std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
		std::vector<int> output = CXXIter::SrcMov(std::move(input))
				.filter([](int item) { return (item % 2) == 0; })
				.collect<std::vector>();
		ASSERT_EQ(4, output.size());
		ASSERT_THAT(output, ElementsAre(2, 4, 6, 8));
	}
}

TEST(CXXIter, filterMap) {
	{ // sizeHint
		std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
		SizeHint sizeHint = CXXIter::from(input)
				.filterMap([](int item) -> std::optional<int> {
					if(item % 2 == 0) { return (item + 3); }
					return {};
				})
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, 0);
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{
		std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
		std::vector<int> output = CXXIter::from(input)
				.filterMap([](int item) -> std::optional<int> {
					if(item % 2 == 0) { return (item + 3); }
					return {};
				})
				.collect<std::vector>();
		ASSERT_EQ(4, output.size());
		ASSERT_THAT(output, ElementsAre(2+3, 4+3, 6+3, 8+3));
	}
}

TEST(CXXIter, map) {
	{ // sizeHint
		std::vector<int> input = {1337, 42};
		SizeHint sizeHint = CXXIter::from(input)
				.map([](int i) { return std::make_pair(i, std::to_string(i)); })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{
		std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
		std::unordered_set<int> output = CXXIter::from(input)
				.map([](const auto& pair) { return pair.first; })
				.collect<std::unordered_set>();
		for(const int& item : output) { ASSERT_TRUE(input.contains(item)); }
	}
	{
		std::vector<int> input = {1337, 42};
		std::unordered_map<int, std::string> output = CXXIter::from(input)
				.map([](int i) { return std::make_pair(i, std::to_string(i)); }) // construct pair
				.collect<std::unordered_map>(); // collect into map
		for(const int& item : input) {
			ASSERT_TRUE(output.contains(item));
			ASSERT_EQ(output[item], std::to_string(item));
		}
	}
}

TEST(CXXIter, modify) {
	{ // sizeHint
		std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
		SizeHint sizeHint = CXXIter::from(input)
				.modify([](auto& keyValue) { keyValue.second = "-" + keyValue.second; })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{
		std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
		std::unordered_map<int, std::string> output = CXXIter::from(input)
				.modify([](auto& keyValue) { keyValue.second = "-" + keyValue.second; }) // modify input
				.collect<std::unordered_map>(); // copy to output
		for(const auto& item : input) {
			ASSERT_TRUE(output.contains(item.first));
			ASSERT_EQ(item.second, input[item.first]);
			ASSERT_TRUE(item.second.starts_with("-"));
		}
	}
}

TEST(CXXIter, skip) {
	{ // sizeHint
		{
			std::vector<int> input = {42, 42, 42, 42, 1337, 42};
			SizeHint sizeHint = CXXIter::from(input)
					.skip(3)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 3);
			ASSERT_EQ(sizeHint.upperBound.value(), 3);
		}
		{
			std::vector<int> input = {42, 42, 42, 42, 1337, 42};
			SizeHint sizeHint = CXXIter::from(input)
					.skip(10)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), 0);
		}
	}
	{
		std::vector<int> input = {42, 42, 42, 42, 1337};
		std::vector<int> output = CXXIter::from(input)
				.skip(3) // skip first 3 values
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre(42, 1337));
	}
}

TEST(CXXIter, skipWhile) {
	{ // sizeHint
		std::vector<int> input = {42, 42, 42, 42, 1337, 42};
		SizeHint sizeHint = CXXIter::from(input)
				.skipWhile([](const int value) { return (value == 42); })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, 0);
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{
		std::vector<int> input = {42, 42, 42, 42, 1337, 42};
		std::vector<int> output = CXXIter::from(input)
				.skipWhile([](const int value) { return (value == 42); }) // skip leading 42s
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre(1337, 42));
		ASSERT_THAT(output, ElementsAre(1337, 42));
	}
}

TEST(CXXIter, take) {
	{ // sizeHint
		{
			std::vector<int> input = {42, 57, 64, 128, 1337, 10};
			SizeHint sizeHint = CXXIter::from(input)
					.take(3)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 3);
			ASSERT_EQ(sizeHint.upperBound.value(), 3);
		}
		{
			std::vector<int> input = {42, 57, 64, 128, 1337, 10};
			SizeHint sizeHint = CXXIter::from(input)
					.take(32)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input.size());
			ASSERT_EQ(sizeHint.upperBound.value(), input.size());
		}
	}
	{
		std::vector<int> input = {42, 57, 64, 128, 1337, 10};
		std::vector<int> output = CXXIter::from(input)
				.take(3) // take first 3 values
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre(42, 57, 64));
	}
	{
		std::string input = "test";
		std::string output = CXXIter::from(input)
				.take(3)
				.collect<std::basic_string>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_EQ(output, "tes");
	}
}

TEST(CXXIter, takeWhile) {
	{ // sizeHint
		std::vector<int> input = {42, 42, 42, 42, 1337, 42};
		SizeHint sizeHint = CXXIter::from(input)
				.takeWhile([](const int value) { return (value < 1000); })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, 0);
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{
		std::vector<int> input = {42, 57, 64, 128, 1337, 10};
		std::vector<int> output = CXXIter::from(input)
				.takeWhile([](const int value) { return (value < 1000); }) // take until first item > 1000
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 4);
		ASSERT_THAT(output, ElementsAre(42, 57, 64, 128));
	}
}

TEST(CXXIter, flatMap) {
	{ // sizeHint
		std::vector<std::pair<std::string, std::vector<int>>> input = {{"first pair", {1337, 42}}, {"second pair", {6, 123, 7888}}};
		SizeHint sizeHint = CXXIter::from(std::move(input))
				.flatMap([](auto&& item) { return std::get<1>(item); })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, 0);
		ASSERT_FALSE(sizeHint.upperBound.has_value());
	}
	{
		std::vector<std::pair<std::string, std::vector<int>>> input = {{"first pair", {1337, 42}}, {"second pair", {6, 123, 7888}}};
		std::vector<int> output = CXXIter::from(std::move(input))
				.flatMap([](auto&& item) { return std::get<1>(item); }) // flatten the std::vector<int> from the pair
				.collect<std::vector>(); // collect into vector containing {1337, 42, 6, 123, 7888}
		ASSERT_EQ(output.size(), 5);
		ASSERT_THAT(output, ElementsAre(1337, 42, 6, 123, 7888));
	}
	{
		std::vector<std::vector<int>> input = {{1337, 42}, {6, 123, 7888}};
		std::vector<int> output = CXXIter::from(std::move(input))
				.flatMap()
				.collect<std::vector>(); // collect into vector containing {1337, 42, 6, 123, 7888}
		ASSERT_EQ(output.size(), 5);
		ASSERT_THAT(output, ElementsAre(1337, 42, 6, 123, 7888));
	}
}

TEST(CXXIter, zip) {
	{ // sizeHint
		{
			std::vector<std::string> input1 = {"1337", "42"};
			std::vector<int> input2 = {1337, 42, 64, 31337};
			SizeHint sizeHint = CXXIter::from(input1).copied()
					.zip(CXXIter::from(input2).copied())
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input1.size());
			ASSERT_EQ(sizeHint.upperBound.value(), input1.size());
		}
		{
			std::vector<std::string> input1 = {"1337", "42"};
			std::vector<int> input2 = {1337, 42, 64, 31337};
			SizeHint sizeHint = CXXIter::from(input1).copied()
					.zip(CXXIter::from(input2).copied().filter([](const auto&) { return true; }))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), input1.size());
		}
		{
			std::vector<std::string> input1 = {"1337", "42"};
			std::vector<int> input2 = {1337, 42, 64, 31337};
			SizeHint sizeHint = CXXIter::from(input1).copied().filter([](const auto&) { return true; })
					.zip(CXXIter::from(input2).copied())
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), input1.size());
		}
	}
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<int> input2 = {1337, 42};
		std::vector<std::pair<std::string, int>> output = CXXIter::from(input1).copied()
				.zip(CXXIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size());
		ASSERT_THAT(output, ElementsAre(Pair("1337", 1337), Pair("42", 42)));
	}
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<int> input2 = {1337, 42, 80};
		std::vector<std::pair<std::string, int>> output = CXXIter::from(input1).copied()
				.zip(CXXIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre(Pair("1337", 1337), Pair("42", 42)));
	}
	{
		std::vector<std::string> input1 = {"1337", "42", "80"};
		std::vector<int> input2 = {1337, 42};
		std::vector<std::pair<std::string, int>> output = CXXIter::from(input1).copied()
				.zip(CXXIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre(Pair("1337", 1337), Pair("42", 42)));
	}
}

TEST(CXXIter, zipTuple) {
	{ // sizeHint
		{
			std::vector<std::string> input1 = {"1337", "42", "80"};
			std::vector<int> input2 = {1337, 42, 64, 31337};
			std::vector<float> input3 = {1337.0f, 42.0f};
			SizeHint sizeHint = CXXIter::from(input1).copied()
					.zipTuple(
						CXXIter::from(input2).copied(),
						CXXIter::from(input3).copied()
					)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input3.size());
			ASSERT_EQ(sizeHint.upperBound.value(), input3.size());
		}
		{
			std::vector<std::string> input1 = {"1337", "42", "80"};
			std::vector<int> input2 = {1337, 42, 64, 31337};
			std::vector<float> input3 = {1337.0f, 42.0f};
			SizeHint sizeHint = CXXIter::from(input1).copied()
					.zipTuple(
						CXXIter::from(input2).copied().filter([](const auto&) { return true; }),
						CXXIter::from(input3).copied()
					)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), input3.size());
		}
		{
			std::vector<std::string> input1 = {"1337", "42", "80"};
			std::vector<int> input2 = {1337, 42, 64, 31337};
			std::vector<float> input3 = {1337.0f, 42.0f};
			SizeHint sizeHint = CXXIter::from(input1).copied()
					.zipTuple(
						CXXIter::from(input2).copied(),
						CXXIter::from(input3).copied().filter([](const auto&) { return true; })
					)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), input3.size());
		}
		{
			std::vector<std::string> input1 = {"1337", "42", "80"};
			std::vector<int> input2 = {1337, 42, 64, 31337};
			std::vector<float> input3 = {1337.0f, 42.0f};
			SizeHint sizeHint = CXXIter::from(input1).copied().filter([](const auto&) { return true; })
					.zipTuple(CXXIter::from(input2).copied(), CXXIter::from(input3).copied())
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), input3.size());
		}
	}
	{
		std::vector<std::string> input1 = {"1337", "42", "31337"};
		std::vector<int> input2 = {1337, 42};
		std::vector<float> input3 = {1337.0f, 42.0f, 64.0f};
		std::vector<std::tuple<std::string, int, float>> output = CXXIter::from(input1).copied()
				.zipTuple(CXXIter::from(input2).copied(), CXXIter::from(input3).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input2.size());
		ASSERT_THAT(output, ElementsAre(std::make_tuple("1337", 1337, 1337.0f), std::make_tuple("42", 42, 42.0f)));
	}
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<int> input2 = {1337, 42, 80};
		std::vector<float> input3 = {1337.0f, 42.0f, 64.0f};
		std::vector<std::tuple<std::string, int, float>> output = CXXIter::from(input1).copied()
				.zipTuple(CXXIter::from(input2).copied(), CXXIter::from(input3).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size());
		ASSERT_THAT(output, ElementsAre(std::make_tuple("1337", 1337, 1337.0f), std::make_tuple("42", 42, 42.0f)));
	}
	{
		std::vector<std::string> input1 = {"1337", "42", "80"};
		std::vector<int> input2 = {1337, 42};
		std::vector<float> input3 = {1337.0f};
		std::vector<std::tuple<std::string, int, float>> output = CXXIter::from(input1).copied()
				.zipTuple(CXXIter::from(input2).copied(), CXXIter::from(input3).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input3.size());
		ASSERT_THAT(output, ElementsAre(std::make_tuple("1337", 1337, 1337.0f)));
	}
}

TEST(CXXIter, chain) {
	{ // sizeHint
		{
			std::vector<std::string> input1 = {"1337", "42"};
			std::vector<std::string> input2 = {"31337", "64"};
			SizeHint sizeHint = CXXIter::from(input1).copied()
					.chain(CXXIter::from(input2).copied())
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 4);
			ASSERT_EQ(sizeHint.upperBound.value(), 4);
		}
		{
			std::vector<std::string> input1 = {"1337", "42"};
			std::vector<std::string> input2 = {"31337", "64"};
			SizeHint sizeHint = CXXIter::from(input1).copied()
					.chain(CXXIter::from(input2).copied().filter([](const auto&) { return true; }))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 2);
			ASSERT_EQ(sizeHint.upperBound.value(), 4);
		}
		{
			std::vector<std::string> input1 = {"1337", "42"};
			std::vector<std::string> input2 = {"31337", "64"};
			SizeHint sizeHint = CXXIter::from(input1).copied().filter([](const auto&) { return true; })
					.chain(CXXIter::from(input2).copied())
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 2);
			ASSERT_EQ(sizeHint.upperBound.value(), 4);
		}
	}
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<std::string> input2 = {"31337", "64"};
		std::vector<std::string> output = CXXIter::from(input1).copied()
				.chain(CXXIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size() + input2.size());
		ASSERT_THAT(output, ElementsAre("1337", "42", "31337", "64"));
	}
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<std::string> input2 = {};
		std::vector<std::string> output = CXXIter::from(input1).copied()
				.chain(CXXIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size() + input2.size());
		ASSERT_THAT(output, ElementsAre("1337", "42"));
	}
	{
		std::vector<std::string> input1 = {};
		std::vector<std::string> input2 = {"31337", "64", "80"};
		std::vector<std::string> output = CXXIter::from(input1).copied()
				.chain(CXXIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size() + input2.size());
		ASSERT_THAT(output, ElementsAre("31337", "64", "80"));
	}
	{
		std::vector<std::string> input1 = {"asdf"};
		std::vector<std::string> input2 = {"31337", "64", "80"};
		std::vector<std::string> output = CXXIter::from(input1).copied()
				.chain(CXXIter::from(std::move(input2)))
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 4);
		ASSERT_THAT(output, ElementsAre("asdf", "31337", "64", "80"));
	}
}

TEST(CXXIter, alternate) {
	{ // sizeHint
		{
			std::vector<int> input1 = {1, 3, 5, 7, 9};
			std::vector<int> input2 = {2, 4, 6, 8, 10};
			std::vector<int> input3 = {100, 200, 300, 400, 500};
			SizeHint sizeHint = CXXIter::from(input1)
					.alternate(CXXIter::from(input2), CXXIter::from(input3))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input1.size() + input2.size() + input3.size());
			ASSERT_EQ(sizeHint.upperBound.value(), input1.size() + input2.size() + input3.size());
		}
		{
			std::vector<int> input1 = {1, 3, 5, 7};
			std::vector<int> input2 = {2, 4, 6, 8, 10};
			std::vector<int> input3 = {100, 200, 300, 400, 500};
			SizeHint sizeHint = CXXIter::from(input1)
					.alternate(CXXIter::from(input2), CXXIter::from(input3))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input1.size() * 3);
			ASSERT_EQ(sizeHint.upperBound.value(), input1.size() * 3);
		}
		{
			std::vector<int> input1 = {1, 3, 5, 7, 9};
			std::vector<int> input2 = {2, 4, 6, 8};
			std::vector<int> input3 = {100, 200, 300, 400, 500};
			SizeHint sizeHint = CXXIter::from(input1)
					.alternate(CXXIter::from(input2), CXXIter::from(input3))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input2.size() * 3 + 1);
			ASSERT_EQ(sizeHint.upperBound.value(), input2.size() * 3 + 1);
		}
		{
			std::vector<int> input1 = {1, 3, 5, 7, 9};
			std::vector<int> input2 = {2, 4, 6, 8, 10};
			std::vector<int> input3 = {100, 200, 300, 400};
			SizeHint sizeHint = CXXIter::from(input1)
					.alternate(CXXIter::from(input2), CXXIter::from(input3))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input3.size() * 3 + 2);
			ASSERT_EQ(sizeHint.upperBound.value(), input3.size() * 3 + 2);
		}
	}
	{
		std::vector<int> input1 = {1, 3, 5, 7, 9};
		std::vector<int> input2 = {2, 4, 6, 8, 10};
		std::vector<int> input3 = {100, 200, 300, 400, 500};
		std::vector<int> output = CXXIter::from(input1)
				.alternate(CXXIter::from(input2), CXXIter::from(input3))
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size() + input2.size() + input3.size());
		ASSERT_THAT(output, ElementsAre(1, 2, 100, 3, 4, 200, 5, 6, 300, 7, 8, 400, 9, 10, 500));
	}
	{
		std::vector<int> input1 = {1, 3, 5, 7};
		std::vector<int> input2 = {2, 4, 6, 8, 10};
		std::vector<int> input3 = {100, 200, 300, 400, 500};
		std::vector<int> output = CXXIter::from(input1)
				.alternate(CXXIter::from(input2), CXXIter::from(input3))
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size() * 3);
		ASSERT_THAT(output, ElementsAre(1, 2, 100, 3, 4, 200, 5, 6, 300, 7, 8, 400));
	}
	{
		std::vector<int> input1 = {1, 3, 5, 7, 9};
		std::vector<int> input2 = {2, 4, 6, 8};
		std::vector<int> input3 = {100, 200, 300, 400, 500};
		std::vector<int> output = CXXIter::from(input1)
				.alternate(CXXIter::from(input2), CXXIter::from(input3))
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input2.size() * 3 + 1);
		ASSERT_THAT(output, ElementsAre(1, 2, 100, 3, 4, 200, 5, 6, 300, 7, 8, 400, 9));
	}
	{
		std::vector<int> input1 = {1, 3, 5, 7, 9};
		std::vector<int> input2 = {2, 4, 6, 8, 10};
		std::vector<int> input3 = {100, 200, 300, 400};
		std::vector<int> output = CXXIter::from(input1)
				.alternate(CXXIter::from(input2), CXXIter::from(input3))
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input3.size() * 3 + 2);
		ASSERT_THAT(output, ElementsAre(1, 2, 100, 3, 4, 200, 5, 6, 300, 7, 8, 400, 9, 10));
	}
}

TEST(CXXIter, groupBy) {
	struct CakeMeasurement {
		std::string cakeType;
		float cakeWeight;
		bool operator==(const CakeMeasurement& o) const {
			return cakeType == o.cakeType && cakeWeight == o.cakeWeight;
		}
	};
	std::vector<CakeMeasurement> input = { {"ApplePie", 1.3f}, {"Sacher", 0.5f}, {"ApplePie", 1.8f} };

	{ // sizeHint
		SizeHint sizeHint = CXXIter::from(input)
				.groupBy([](const CakeMeasurement& item) { return item.cakeType; })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, 1);
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{
		auto output = CXXIter::from(input)
				.groupBy([](const CakeMeasurement& item) { return item.cakeType; })
				.collect<std::unordered_map>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_EQ(output["ApplePie"].size(), 2);
		ASSERT_EQ(output["ApplePie"][0], input[0]);
		ASSERT_EQ(output["ApplePie"][1], input[2]);
		ASSERT_EQ(output["Sacher"].size(), 1);
		ASSERT_EQ(output["Sacher"][0], input[1]);
	}
	{ // mapping of groupIdentifier to first value of each group
		auto output = CXXIter::from(input)
				.groupBy([](const CakeMeasurement& item) { return item.cakeType; })
				.map([](std::pair<std::string, std::vector<CakeMeasurement>>&& itemGroup) {
					return std::make_pair(itemGroup.first, itemGroup.second[0]);
				})
				.collect<std::unordered_map>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_EQ(output["ApplePie"].cakeType, "ApplePie");
		ASSERT_EQ(output["ApplePie"].cakeWeight, 1.3f);
		ASSERT_EQ(output["Sacher"].cakeType, "Sacher");
		ASSERT_EQ(output["Sacher"].cakeWeight, 0.5f);
	}
	{
		auto output = CXXIter::from(std::move(input))
				.groupBy([](const CakeMeasurement& item) { return item.cakeType; })
				.collect<std::unordered_map>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_EQ(output["ApplePie"].size(), 2);
		ASSERT_EQ(output["ApplePie"][0].cakeType, "ApplePie");
		ASSERT_EQ(output["ApplePie"][0].cakeWeight, 1.3f);
		ASSERT_EQ(output["ApplePie"][1].cakeType, "ApplePie");
		ASSERT_EQ(output["ApplePie"][1].cakeWeight, 1.8f);
		ASSERT_EQ(output["Sacher"].size(), 1);
		ASSERT_EQ(output["Sacher"][0].cakeType, "Sacher");
		ASSERT_EQ(output["Sacher"][0].cakeWeight, 0.5f);
	}
	{
		std::vector<CakeMeasurement> emptyInput;
		auto output = CXXIter::from(std::move(emptyInput))
				.groupBy([](const CakeMeasurement& item) { return item.cakeType; })
				.collect<std::unordered_map>();
		ASSERT_EQ(output.size(), 0);
	}
}

TEST(CXXIter, sorted) {
	{ // sizeHint
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		SizeHint sizeHint = CXXIter::from(input)
				.sorted<false>([](const float& a, const float& b) {
					return (a < b);
				})
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // ASCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		std::vector<float> output = CXXIter::from(input)
			.sorted<false>([](const float& a, const float& b) {
				return (a < b);
			})
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(-42.0f, 0.5f, 1.0f, 2.0f, 3.0f));
	}
	{ // DESCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		std::vector<float> output = CXXIter::from(input)
			.sorted<false>([](const float& a, const float& b) {
				return (a > b);
			})
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(3.0f, 2.0f, 1.0f, 0.5f, -42.0f));
	}
	{ // ASCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		std::vector<float> output = CXXIter::from(input)
			.sorted<CXXIter::ASCENDING, false>()
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(-42.0f, 0.5f, 1.0f, 2.0f, 3.0f));
	}
	{ // DESCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		std::vector<float> output = CXXIter::from(input)
			.sorted<CXXIter::DESCENDING, false>()
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(3.0f, 2.0f, 1.0f, 0.5f, -42.0f));
	}
}

TEST(CXXIter, sortedBy) {
	{ // sizeHint
		std::vector<std::string> input = {"test1", "test2", "test23", "test", "tes"};
		SizeHint sizeHint = CXXIter::from(input)
				.sortedBy<CXXIter::ASCENDING, true>([](const std::string& item) { return item.size(); })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // ASCENDING
		std::vector<std::string> input = {"test1", "test2", "test23", "test", "tes"};
		std::vector<std::string> output = CXXIter::from(input)
			.sortedBy<CXXIter::ASCENDING, true>([](const std::string& item) { return item.size(); })
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre("tes", "test", "test1", "test2", "test23"));
	}
	{ // DESCENDING
		std::vector<std::string> input = {"test1", "test2", "test23", "test", "tes"};
		std::vector<std::string> output = CXXIter::from(input)
			.sortedBy<CXXIter::DESCENDING, true>([](const std::string& item) { return item.size(); })
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre("test23", "test1", "test2", "test", "tes"));
	}
}
