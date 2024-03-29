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

TEST(CXXIter, indexed) {
	{
		std::vector<std::string> input = {"1337", "42", "64"};
		std::vector<std::pair<size_t, std::string&>> output = CXXIter::from(input)
				.indexed()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre(Pair(0, "1337"), Pair(1, "42"), Pair(2, "64")));
	}
	{
		std::vector<std::string> input = {"1337", "42", "64"};
		std::vector<std::pair<size_t, std::string>> output = CXXIter::from(std::move(input))
				.indexed()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre(Pair(0, "1337"), Pair(1, "42"), Pair(2, "64")));
	}
}

TEST(CXXIter, flagLast) {
	{ // sizeHint
		std::vector<float> input = {1.35, 56.123};
		SizeHint sizeHint = CXXIter::from(input).flagLast().sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // reference
		std::vector<std::string> input = {"1337", "42", "64"};
		std::vector<std::pair<std::string&, bool>> output = CXXIter::from(input)
				.flagLast()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre(Pair("1337", false), Pair("42", false), Pair("64", true)));
	}
	{ //moved
		std::vector<std::string> input = {"1337", "42", "64"};
		std::vector<std::pair<std::string, bool>> output = CXXIter::from(std::move(input))
				.flagLast()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre(Pair("1337", false), Pair("42", false), Pair("64", true)));
	}
	{ // filterLast
		std::vector<std::string> input = {"1337", "42", "64"};
		std::vector<std::string> output = CXXIter::from(input)
				.flagLast()
				.filterMap([](const std::pair<std::string&, bool>& el) -> std::optional<std::string> {
					if(el.second) { return {}; } // remove last
					return el.first;
				})
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre("1337", "42"));
	}
	{ // flag after non-exact iterator
		std::vector<std::string> input = {"1337", "42", "420", "64"};
		std::vector<std::pair<std::string&, bool>> output = CXXIter::from(input)
				.filter([](const std::string& el) { return el.size() >= 3; })
				.flagLast()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre(Pair("1337", false), Pair("420", true)));
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

TEST(CXXIter, unique) {
	{ // sizeHint
		std::vector<size_t> input = {1, 1, 2, 3, 3, 4, 4, 5, 5, 5};
		SizeHint sizeHint = CXXIter::from(input)
				.unique()
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, 0);
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // basic data
		std::vector<size_t> input = {1, 1, 2, 3, 3, 4, 4, 5, 5, 5};
		std::vector<size_t> output = CXXIter::from(input)
				.unique()
				.copied()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 5);
		ASSERT_THAT(output, ElementsAre(1, 2, 3, 4, 5));
	}
	{ // basic data
		std::vector<double> input = {1.0, 1.5, 1.4, 2.0, 2.1, 2.99, 3.25, 4.5};
		std::vector<double> output = CXXIter::from(input)
				.unique()
				.copied()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(1.0, 1.5, 1.4, 2.0, 2.1, 2.99, 3.25, 4.5));
	}
	{ // with mapFn
		std::vector<double> input = {1.0, 1.5, 1.4, 2.0, 2.1, 2.99, 3.25, 4.5};
		std::vector<double> output = CXXIter::from(input)
				.unique([](double item) { return std::floor(item); })
				.copied()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 4);
		ASSERT_THAT(output, ElementsAre(1.0, 2.0, 3.25, 4.5));
	}
}

TEST(CXXIter, reverse) {
	{ // sizeHint
		std::vector<size_t> input = {1, 42, 2, 1337, 3, 4, 69, 5, 6, 5};
		SizeHint sizeHint = CXXIter::from(input).copied()
				.reverse()
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // Reverse using DoubleEndedIterator
		std::vector<size_t> input = {1, 42, 2, 1337, 3, 4, 69, 5, 6, 5};
		std::vector<size_t> output = CXXIter::from(input).copied()
				.reverse()
				.collect<std::vector>();
		ASSERT_EQ(input.size(), output.size());
		ASSERT_THAT(output, ElementsAre(5, 6, 5, 69, 4, 3, 1337, 2, 42, 1));
	}
	{ // Reverse using internal Cache
		std::vector<size_t> input1 = {1, 2, 3, 4, 5};
		std::vector<std::string> input2 = {"1", "2", "3", "4", "5", "6"};
		std::vector<std::pair<size_t, std::string>> output = CXXIter::from(input1).copied()
				.zip(CXXIter::from(input2).copied())
				.reverse()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size());
		ASSERT_THAT(output, ElementsAre(Pair(5, "5"), Pair(4, "4"), Pair(3, "3"), Pair(2, "2"), Pair(1, "1")));
	}

	{ // Double Reverse using DoubleEndedIterator
		std::vector<size_t> input = {1, 42, 2, 1337, 3, 4, 69, 5, 6, 5};
		std::vector<size_t> output = CXXIter::from(input).copied()
				.reverse()
				.reverse()
				.collect<std::vector>();
		ASSERT_EQ(input.size(), output.size());
		ASSERT_THAT(output, ElementsAre(1, 42, 2, 1337, 3, 4, 69, 5, 6, 5));
	}
	{ // Double Reverse using internal Cache
		std::vector<size_t> input1 = {1, 2, 3, 4, 5};
		std::vector<std::string> input2 = {"1", "2", "3", "4", "5", "6"};
		std::vector<std::pair<size_t, std::string>> output = CXXIter::from(input1).copied()
				.zip(CXXIter::from(input2).copied())
				.reverse()
				.reverse()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size());
		ASSERT_THAT(output, ElementsAre(Pair(1, "1"), Pair(2, "2"), Pair(3, "3"), Pair(4, "4"), Pair(5, "5")));
	}
}

TEST(CXXIter, chunkedExact_buffered) {
	{ // non-interleaved
		{ // sizeHint
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 3);
				ASSERT_EQ(sizeHint.upperBound.value(), 3);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.filter([](size_t) { return true; })
						.chunkedExact<3>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 0);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
		}

		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<3>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 3);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(31337, 69, 5));
			ASSERT_THAT(output[2], ElementsAre(1, 2, 3));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<3>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 2);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(31337, 69, 5));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<5>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 1);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512, 31337, 69));
		}
	}

	{ // overlapping, step width 2
		{ // sizeHint
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3, 2>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 4);
				ASSERT_EQ(sizeHint.upperBound.value(), 4);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3, 2>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 3);
				ASSERT_EQ(sizeHint.upperBound.value(), 3);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3, 2>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 3);
				ASSERT_EQ(sizeHint.upperBound.value(), 3);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.filter([](size_t) { return true; })
						.chunkedExact<3, 2>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 0);
				ASSERT_EQ(sizeHint.upperBound.value(), 3);
			}
		}

		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<3, 2>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 4);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(512, 31337, 69));
			ASSERT_THAT(output[2], ElementsAre(69, 5, 1));
			ASSERT_THAT(output[3], ElementsAre(1, 2, 3));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<3, 2>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 3);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(512, 31337, 69));
			ASSERT_THAT(output[2], ElementsAre(69, 5, 1));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<5, 3>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 2);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512, 31337, 69));
			ASSERT_THAT(output[1], ElementsAre(31337, 69, 5, 1, 2));
		}
	}


	{ // step width > context-size
		{ //sizeHint
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3, 4>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3, 4>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.chunkedExact<3, 4>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.copied()
						.filter([](size_t) { return true; })
						.chunkedExact<3, 4>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 0);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
		}

		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<3, 4>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 2);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(69, 5, 1));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<3, 4>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 2);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(69, 5, 1));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.copied()
					.chunkedExact<5, 6>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 1);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512, 31337, 69));
		}
	}
}

TEST(CXXIter, chunkedExact_directAccess) {
	{ // non-interleaved
		{ // sizeHint
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 3);
				ASSERT_EQ(sizeHint.upperBound.value(), 3);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.filter([](size_t) { return true; })
						.chunkedExact<3>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 0);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
		}

		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.chunkedExact<3>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 3);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(31337, 69, 5));
			ASSERT_THAT(output[2], ElementsAre(1, 2, 3));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
			auto output = CXXIter::from(input)
					.chunkedExact<3>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 2);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(31337, 69, 5));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.chunkedExact<5>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 1);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512, 31337, 69));
		}
	}

	{ // overlapping, step width 2
		{ // sizeHint
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3, 2>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 4);
				ASSERT_EQ(sizeHint.upperBound.value(), 4);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3, 2>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 3);
				ASSERT_EQ(sizeHint.upperBound.value(), 3);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3, 2>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 3);
				ASSERT_EQ(sizeHint.upperBound.value(), 3);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.filter([](size_t) { return true; })
						.chunkedExact<3, 2>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 0);
				ASSERT_EQ(sizeHint.upperBound.value(), 3);
			}
		}

		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.chunkedExact<3, 2>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 4);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(512, 31337, 69));
			ASSERT_THAT(output[2], ElementsAre(69, 5, 1));
			ASSERT_THAT(output[3], ElementsAre(1, 2, 3));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
			auto output = CXXIter::from(input)
					.chunkedExact<3, 2>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 3);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(512, 31337, 69));
			ASSERT_THAT(output[2], ElementsAre(69, 5, 1));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.chunkedExact<5, 3>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 2);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512, 31337, 69));
			ASSERT_THAT(output[1], ElementsAre(31337, 69, 5, 1, 2));
		}
	}


	{ // step width > context-size
		{ //sizeHint
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3, 4>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3, 4>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.chunkedExact<3, 4>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 2);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
			{
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
				SizeHint sizeHint = CXXIter::from(input)
						.filter([](size_t) { return true; })
						.chunkedExact<3, 4>()
						.sizeHint();
				ASSERT_EQ(sizeHint.lowerBound, 0);
				ASSERT_EQ(sizeHint.upperBound.value(), 2);
			}
		}

		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.chunkedExact<3, 4>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 2);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(69, 5, 1));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
			auto output = CXXIter::from(input)
					.chunkedExact<3, 4>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 2);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
			ASSERT_THAT(output[1], ElementsAre(69, 5, 1));
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto output = CXXIter::from(input)
					.chunkedExact<5, 6>()
					.collect<std::vector>();
			ASSERT_EQ(output.size(), 1);
			ASSERT_THAT(output[0], ElementsAre(1337, 42, 512, 31337, 69));
		}
	}
}

TEST(CXXIter, chunkedExact_elementTypeSelection) {
	{ // contiguous
		{ // references
			{ // mutable
				std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				auto iter = CXXIter::from(input).chunkedExact<3, 2>();
				CXXIter::ExactChunk<size_t, 3>& output0 = iter.next().value();
				CXXIter::ExactChunk<size_t, 3>& output1 = iter.next().value();
				ASSERT_EQ(output0.data(), &input[0]);
				ASSERT_EQ(output1.data(), &input[2]);
				output1[1] = 9999;
				ASSERT_EQ(input[3], 9999);
			}
			{ // const
				const std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				auto iter = CXXIter::from(input).chunkedExact<3, 2>();
				const CXXIter::ExactChunk<size_t, 3>& output0 = iter.next().value();
				const CXXIter::ExactChunk<size_t, 3>& output1 = iter.next().value();
				ASSERT_EQ(output0.data(), &input[0]);
				ASSERT_EQ(output1.data(), &input[2]);
			}
		}
		{ // owned
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto iter = CXXIter::from(input).copied().chunkedExact<3, 2>();
			const CXXIter::ExactChunk<size_t, 3>& output0 = iter.next().value();
			const CXXIter::ExactChunk<size_t, 3>& output1 = iter.next().value();
			ASSERT_NE(output0.data(), &input[0]);
			ASSERT_NE(output1.data(), &input[2]);
		}
	}
	{ // non-contiguous
		{ // references
			{ // mutable
				std::deque<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				auto iter = CXXIter::from(input).chunkedExact<3, 2>();
				const CXXIter::ExactChunk<size_t&, 3>& output0 = iter.next().value();
				ASSERT_EQ(&output0[0].get(), &input[0]);
				const CXXIter::ExactChunk<size_t&, 3>& output1 = iter.next().value();
				ASSERT_EQ(&output1[0].get(), &input[2]);
				output1[1].get() = 9999;
				ASSERT_EQ(input[3], 9999);
			}
			{ // const
				const std::deque<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
				auto iter = CXXIter::from(input).chunkedExact<3, 2>();
				const CXXIter::ExactChunk<const size_t&, 3>& output0 = iter.next().value();
				ASSERT_EQ(&output0[0].get(), &input[0]);
				const CXXIter::ExactChunk<const size_t&, 3>& output1 = iter.next().value();
				ASSERT_EQ(&output1[0].get(), &input[2]);
			}
		}
		{ // owned
			std::deque<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			auto iter = CXXIter::from(input).copied().chunkedExact<3, 2>();
			const CXXIter::ExactChunk<size_t, 3>& output0 = iter.next().value();
			const CXXIter::ExactChunk<size_t, 3>& output1 = iter.next().value();
			ASSERT_NE(output0.data(), &input[0]);
			ASSERT_NE(output1.data(), &input[2]);
		}
	}
}

TEST(CXXIter, chunkedExact_usage) {
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
		std::vector<CXXIter::ExactChunk<size_t, 3>> output = CXXIter::from(input)
			.chunkedExact<3>()
			.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre(ElementsAre(1337, 42, 512), ElementsAre(31337, 69, 5), ElementsAre(1, 2, 3)));
	}
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
		std::vector<CXXIter::ExactChunk<size_t, 3>> output = CXXIter::from(input)
			.chunkedExact<3>()
			.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre(ElementsAre(1337, 42, 512), ElementsAre(31337, 69, 5)));
	}
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5};
		std::vector<CXXIter::ExactChunk<size_t, 3>> output = CXXIter::from(input)
			.chunkedExact<3, 1>()
			.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre(ElementsAre(1337, 42, 512), ElementsAre(42, 512, 31337), ElementsAre(512, 31337, 69), ElementsAre(31337, 69, 5)));
	}
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
		std::vector<CXXIter::ExactChunk<size_t, 3>> output = CXXIter::from(input)
			.chunkedExact<3, 4>()
			.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre(ElementsAre(1337, 42, 512), ElementsAre(69, 5, 1)));
	}
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
		CXXIter::from(input)
			.chunkedExact<3, 4>()
			.forEach([](std::array<size_t, 3>& chunkRef) {
				chunkRef[0] += 1; chunkRef[1] += 2; chunkRef[2] += 3;
			});
		ASSERT_THAT(input, ElementsAre(1337+1, 42+2, 512+3, 31337, 69+1, 5+2, 1+3));
	}
	{
		std::deque<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
		std::vector<CXXIter::ExactChunk<size_t, 3>> output = CXXIter::from(input)
			.copied() // required to avoid std::reference_wrapper<...> in elements produced by chunkedExact()
			.chunkedExact<3, 4>()
			.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre(ElementsAre(1337, 42, 512), ElementsAre(69, 5, 1)));
	}
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
		// CXXIter::ExactChunk<size_t&, 3> resolves to std::array<std::reference_wrapper<size_t>, 3>
		std::vector<CXXIter::ExactChunk<size_t&, 3>> output = CXXIter::from(input)
			.filter([](const auto&) { return true; })
			.chunkedExact<3, 4>()
			.collect<std::vector>();
		// output == { {1337, 42, 512}, {69, 5, 1} }
	}
}

TEST(CXXIter, chunked) {
	{ // sizeHint
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
			SizeHint sizeHint = CXXIter::from(input)
					.copied()
					.chunked<3>()
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 3);
			ASSERT_EQ(sizeHint.upperBound.value(), 3);
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
			SizeHint sizeHint = CXXIter::from(input)
					.copied()
					.chunked<3>()
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 3);
			ASSERT_EQ(sizeHint.upperBound.value(), 3);
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
			SizeHint sizeHint = CXXIter::from(input)
					.copied()
					.chunked<3>()
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 3);
			ASSERT_EQ(sizeHint.upperBound.value(), 3);
		}
		{
			std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
			SizeHint sizeHint = CXXIter::from(input)
					.copied()
					.filter([](size_t) { return true; })
					.chunked<3>()
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), 3);
		}
	}
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
		auto output = CXXIter::from(input)
				.copied()
				.chunked<3>()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
		ASSERT_THAT(output[1], ElementsAre(31337, 69, 5));
		ASSERT_THAT(output[2], ElementsAre(1, 2, 3));
	}
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
		auto output = CXXIter::from(input)
				.copied()
				.chunked<3>()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output[0], ElementsAre(1337, 42, 512));
		ASSERT_THAT(output[1], ElementsAre(31337, 69, 5));
		ASSERT_THAT(output[2], ElementsAre(1));
	}
	{
		std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
		auto output = CXXIter::from(input)
				.copied()
				.chunked<5>()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output[0], ElementsAre(1337, 42, 512, 31337, 69));
		ASSERT_THAT(output[1], ElementsAre(5, 1, 2, 3));
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
	struct SimpleState {
		size_t state = 0;
		SimpleState(size_t state) : state(state) {}
		size_t getAddState() {
			state += 1;
			return state;
		}
	};

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
	{ // move source
		std::vector<SimpleState> input = { 0, 1336, 41, 68 };
		size_t output = CXXIter::from(std::move(input))
				.map([](SimpleState&& state) { return state.getAddState(); })
				.sum();
		ASSERT_EQ(output, 1 + 1337 + 42 + 69);
		ASSERT_EQ(input.size(), 0);
	}
	{ // ref source
		std::vector<SimpleState> input = { 1, 1337, 42, 69 };
		size_t output = CXXIter::from(input)
				.map([](SimpleState& state) { return state.getAddState(); })
				.sum();
		ASSERT_EQ(output, 2 + 1338 + 43 + 70);
		ASSERT_EQ(input[0].getAddState(), 3);
		ASSERT_EQ(input[1].getAddState(), 1339);
		ASSERT_EQ(input[2].getAddState(), 44);
		ASSERT_EQ(input[3].getAddState(), 71);
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
		{
			std::vector<int> input = {42, 42, 42, 42, 1337, 42};
			SizeHint sizeHint = CXXIter::from(input)
					.skip(0)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input.size());
			ASSERT_EQ(sizeHint.upperBound.value(), input.size());
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
	{
		std::vector<int> input = {42, 42, 42, 42, 1337};
		std::vector<int> output = CXXIter::from(input)
				.skip(0)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(42, 42, 42, 42, 1337));
	}
	{
		std::vector<int> input = {42, 42, 42, 42, 1337, 69, 69, 31337};
		std::vector<int> output = CXXIter::from(input)
				.skip(3)
				.skip(1)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 4);
		ASSERT_THAT(output, ElementsAre(1337, 69, 69, 31337));
	}
	{
		std::vector<int> input = {42, 42, 42, 42, 1337, 69, 69, 31337};
		std::vector<int> output = CXXIter::from(input)
				.skip(99)
				.skip(1)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 0);
	}
	{
		std::vector<int> input = {42, 42, 42, 42, 1337, 69, 69, 31337};
		std::vector<int> output = CXXIter::from(input)
				.skip(1)
				.skip(99)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 0);
	}
	{
		std::vector<int> input = {42, 42, 42, 42, 1337, 69, 69, 31337};
		std::vector<int> output = CXXIter::from(input)
				.skip(8)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 0);
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
		{
			std::vector<int> input = {42, 57, 64, 128, 1337, 10};
			SizeHint sizeHint = CXXIter::from(input)
					.take(0)
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), 0);
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
	{
		std::string input = "test";
		std::string output = CXXIter::from(input)
				.take(0)
				.collect<std::basic_string>();
		ASSERT_EQ(output.size(), 0);
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

#ifdef CXXITER_HAS_COROUTINE
TEST(CXXIter, generateFrom) {
	{
		std::vector<int> input = {1337, 42};
		std::vector<int> output = CXXIter::from(input)
				.generateFrom([](const int& item) -> CXXIter::Generator<int> {
					for(int i = -2; i <= 2; ++i) {
						co_yield (item + i);
					}
				})
				.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre(1335, 1336, 1337, 1338, 1339, 40, 41, 42, 43, 44));
	}
	{ // cloning item reference - pass on as copied reference
		std::vector<std::string> input = {"1337", "42"};
		std::vector<std::string> output = CXXIter::from(input)
				.generateFrom([](const std::string& item) -> CXXIter::Generator<const std::string&> {
					for(size_t i = 0; i < item.size(); ++i) {
						co_yield item;
					}
				})
				.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre("1337", "1337", "1337", "1337", "42", "42"));
	}
	{ // cloning item - pass on as owned copy
		std::vector<std::string> input = {"1337", "42"};
		std::vector<std::string> output = CXXIter::from(input)
				.generateFrom([](const std::string& item) -> CXXIter::Generator<std::string> {
					for(size_t i = 0; i < item.size(); ++i) {
						co_yield item;
					}
				})
				.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre("1337", "1337", "1337", "1337", "42", "42"));
	}
	{
		std::vector<std::string> input = {"1337", "42"};
		std::vector<std::string> output = CXXIter::from(std::move(input))
				.generateFrom([](std::string item) -> CXXIter::Generator<std::string> {
					for(size_t i = 0; i < item.size(); ++i) {
						co_yield item;
					}
				})
				.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre("1337", "1337", "1337", "1337", "42", "42"));
	}
	{ // generate different item type than input type
		std::vector<size_t> input = {1, 2, 3, 4};
		std::vector<std::string> output = CXXIter::from(std::move(input))
				.generateFrom([](size_t item) -> CXXIter::Generator<std::string> {
					for(size_t i = 0; i < item; ++i) {
						co_yield std::to_string(i);
					}
				})
				.collect<std::vector>();
		ASSERT_THAT(output, ElementsAre("0", "0", "1", "0", "1", "2", "0", "1", "2", "3"));
	}
	{ // correct exception propagation
		try {
			std::vector<int> input = {1337, 42};
			std::vector<int> output = CXXIter::from(input)
					.generateFrom([](int item) -> CXXIter::Generator<int> {
						co_yield (item + 1);
						throw std::runtime_error("Exception From GeneratorFn");
					})
					.collect<std::vector>();
			ASSERT_TRUE(false); // unreachable!
		} catch (std::runtime_error& ex) {
			ASSERT_EQ(ex.what(), std::string("Exception From GeneratorFn"));
		}
	}
	{ // empty return
		std::vector<std::string> input = {"1337", "42"};
		std::vector<std::string> output = CXXIter::from(std::move(input))
				.generateFrom([](std::string item) -> CXXIter::Generator<std::string> {
					if constexpr(false) {
						co_yield item;
					}
				})
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 0);
		ASSERT_THAT(output, ElementsAre());
	}
}
#endif

TEST(CXXIter, stepBy) {
	{
		std::vector<int> input = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		std::vector<int> output = CXXIter::from(input)
				.stepBy(1)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 11);
		ASSERT_THAT(output, ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
	}
	{
		std::vector<int> input = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		std::vector<int> output = CXXIter::from(input)
				.stepBy(2)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 6);
		ASSERT_THAT(output, ElementsAre(0, 2, 4, 6, 8, 10));
	}
	{
		std::vector<int> input = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		std::vector<int> output = CXXIter::from(input)
				.stepBy(3)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 4);
		ASSERT_THAT(output, ElementsAre(0, 3, 6, 9));
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
	{
		std::vector<std::string> input1 = {"1337", "42", "80"};
		std::vector<int> input2 = {1337, 42};
		std::vector<std::pair<std::string&, int&>> output = CXXIter::from(input1)
				.zip(CXXIter::from(input2))
				.modify([](std::pair<std::string&, int&> pair) {
					std::get<1>(pair) += 1;
				})
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre(Pair("1337", 1338), Pair("42", 43)));
		ASSERT_THAT(input2, ElementsAre(1338, 43));
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
		{
			std::vector<std::string> input1 = {"1337", "42"};
			auto iter2 = CXXIter::repeat<std::string>("endless");
			SizeHint sizeHint = CXXIter::from(input1).copied().filter([](const auto&) { return true; })
					.chain(iter2.copied())
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, SizeHint::INFINITE);
			ASSERT_FALSE(sizeHint.upperBound.has_value());
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

TEST(CXXIter, intersperse) {
	{ // sizeHint
		{
			std::vector<int> input = { 1, 2, 3, 4, 5, 6 };
			SizeHint sizeHint = CXXIter::from(input).copied()
					.intersperse(CXXIter::repeat(0))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input.size() * 2 - 1);
			ASSERT_EQ(sizeHint.upperBound.value(), input.size() * 2 - 1);
		}
		{
			std::vector<std::string> input = { "Apple", "Orange", "Cake" };
			SizeHint sizeHint = CXXIter::from(input).copied()
					.intersperse(CXXIter::repeat<std::string>(", "))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, input.size() * 2 - 1);
			ASSERT_EQ(sizeHint.upperBound.value(), input.size() * 2 - 1);
		}
		{ // separator iterator ends too early
			std::vector<int> input = { 1, 2, 3, 4, 5, 6 };
			SizeHint sizeHint = CXXIter::from(input).copied()
					.intersperse(CXXIter::range(100, 102, 1))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 4 + 3);
			ASSERT_EQ(sizeHint.upperBound.value(), 4 + 3);
		}
		{ // separator iterator empty
			std::vector<int> input = { 1, 2, 3, 4, 5, 6 };
			SizeHint sizeHint = CXXIter::from(input).copied()
					.intersperse(CXXIter::empty<int>())
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 1);
			ASSERT_EQ(sizeHint.upperBound.value(), 1);
		}
		{ // input empty
			SizeHint sizeHint = CXXIter::empty<int>()
					.intersperse(CXXIter::repeat<int>(0))
					.sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 0);
			ASSERT_EQ(sizeHint.upperBound.value(), 0);
		}
	}
	{
		std::vector<int> input = { 1, 2, 3, 4, 5, 6 };
		std::vector<int> output = CXXIter::from(input).copied()
				.intersperse(CXXIter::repeat(0))
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size() * 2 - 1);
		ASSERT_THAT(output, ElementsAre(1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6));
	}
	{
		std::vector<std::string> input = { "Apple", "Orange", "Cake" };
		std::vector<std::string> output = CXXIter::from(input).copied()
				.intersperse(CXXIter::repeat<std::string>(", "))
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size() * 2 - 1);
		ASSERT_THAT(output, ElementsAre("Apple", ", ", "Orange", ", ", "Cake"));
	}
	{ // separator iterator ends too early
		std::vector<int> input = { 1, 2, 3, 4, 5, 6 };
		std::vector<int> output = CXXIter::from(input).copied()
				.intersperse(CXXIter::range(100, 102, 1))
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 7);
		ASSERT_THAT(output, ElementsAre(1, 100, 2, 101, 3, 102, 4));
	}
	{ // separator iterator empty
		std::vector<int> input = { 1, 2, 3, 4, 5, 6 };
		std::vector<int> output = CXXIter::from(input).copied()
				.intersperse(CXXIter::empty<int>())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 1);
		ASSERT_THAT(output, ElementsAre(1));
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

TEST(CXXIter, sort) {
	{ // sizeHint
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		SizeHint sizeHint = CXXIter::from(input)
				.sort<false>([](const float& a, const float& b) {
					return (a < b);
				})
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // ASCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		std::vector<float> output = CXXIter::from(input)
			.sort<false>([](const float& a, const float& b) {
				return (a < b);
			})
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(-42.0f, 0.5f, 1.0f, 2.0f, 3.0f));
	}
	{ // DESCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		std::vector<float> output = CXXIter::from(input)
			.sort<false>([](const float& a, const float& b) {
				return (a > b);
			})
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(3.0f, 2.0f, 1.0f, 0.5f, -42.0f));
	}
	{ // ASCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		std::vector<float> output = CXXIter::from(input)
			.sort<CXXIter::ASCENDING, false>()
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(-42.0f, 0.5f, 1.0f, 2.0f, 3.0f));
	}
	{ // DESCENDING
		std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
		std::vector<float> output = CXXIter::from(input)
			.sort<CXXIter::DESCENDING, false>()
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre(3.0f, 2.0f, 1.0f, 0.5f, -42.0f));
	}
}

TEST(CXXIter, sortBy) {
	{ // sizeHint
		std::vector<std::string> input = {"test1", "test2", "test23", "test", "tes"};
		SizeHint sizeHint = CXXIter::from(input)
				.sortBy<CXXIter::ASCENDING, true>([](const std::string& item) { return item.size(); })
				.sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // ASCENDING
		std::vector<std::string> input = {"test1", "test2", "test23", "test", "tes"};
		std::vector<std::string> output = CXXIter::from(input)
			.sortBy<CXXIter::ASCENDING, true>([](const std::string& item) { return item.size(); })
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre("tes", "test", "test1", "test2", "test23"));
	}
	{ // DESCENDING
		std::vector<std::string> input = {"test1", "test2", "test23", "test", "tes"};
		std::vector<std::string> output = CXXIter::from(input)
			.sortBy<CXXIter::DESCENDING, true>([](const std::string& item) { return item.size(); })
			.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre("test23", "test1", "test2", "test", "tes"));
	}

	{ // Lambda capture
		float ref = 600.0;
		std::vector<std::string> input = {"1337", "55", "500", "10000"};
		auto output = CXXIter::from(input)
				.sortBy([&](const std::string& val) { return std::abs(std::stof(val) - ref); })
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input.size());
		ASSERT_THAT(output, ElementsAre("500", "55", "1337", "10000"));
	}
}
