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
// CONSUMERS
// ################################################################################################
TEST(CXXIter, forEach) {
	// additional container type parameters
	std::vector<std::string> input = {"1337", "42", "64"};
	std::vector<std::string> output;
	CXXIter::from(input)
			.forEach([&output](std::string& item) {
				output.push_back(std::forward<std::string>(item));
			});
	ASSERT_EQ(output.size(), 3);
	ASSERT_THAT(output, ElementsAre("1337", "42", "64"));
}

TEST(CXXIter, fold) {
	std::vector<double> input = {1.331335363800390, 1.331335363800390, 1.331335363800390, 1.331335363800390};
	double output = CXXIter::from(input)
			.fold(1.0, [](double& workingValue, double item) {
				workingValue *= item;
			});
	ASSERT_NEAR(output, 3.141592653589793, 0.0000000005);
}

TEST(CXXIter, findIdx) {
	{ // item
		std::vector<int> input = {42, 1337, 52};
		std::optional<size_t> output = CXXIter::from(input).findIdx(1337);
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 1);
	}
	{
		std::vector<std::string> input = {"42", "1337", "52"};
		std::optional<size_t> output = CXXIter::from(input).findIdx("42");
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 0);
	}
	{
		std::vector<std::string> input = {"42", "1337", "52"};
		std::optional<size_t> output = CXXIter::from(input).findIdx("not found");
		ASSERT_FALSE(output.has_value());
	}
	{ // searchFn
		std::vector<int> input = {1337, 31337, 41, 43, 42, 64};
		std::optional<size_t> output = CXXIter::from(input)
				.findIdx([](int item) { return (item % 2 == 0); });
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 4);
	}
	{
		std::vector<int> input = {1337, 31337, 41, 43};
		std::optional<size_t> output = CXXIter::from(input)
				.findIdx([](int item) { return (item % 2 == 0); });
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, find) {
	{
		std::vector<std::string> input = {"42", "1337", "52"};
		CXXIter::IterValue<std::string&> output = CXXIter::from(input)
				.find([](const std::string& item) {
					return item.size() == 4;
				});
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), "1337");
	}
	{
		std::vector<std::string> input = {"42", "1337", "52"};
		CXXIter::IterValue<std::string&> output = CXXIter::from(input)
				.find([](const std::string& item) {
					return item.size() == 3;
				});
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, count) {
	{
		std::vector<int> input = {42, 1337, 52};
		size_t output = CXXIter::from(input).count();
		ASSERT_EQ(output, 3);
	}
	{
		std::vector<int> input = {};
		size_t output = CXXIter::from(input).count();
		ASSERT_EQ(output, 0);
	}
}

TEST(CXXIter, sum) {
	{ // default startValue
		std::vector<int> input = {42, 1337, 52};
		int output = CXXIter::from(input).sum();
		ASSERT_EQ(output, 1431);
	}
	{ // custom startValue
		std::vector<int> input = {42, 1337, 52};
		int output = CXXIter::from(input).sum(29906);
		ASSERT_EQ(output, 31337);
	}
	{ // default startValue, empty iterator
		std::vector<int> input = {};
		int output = CXXIter::from(input).sum();
		ASSERT_EQ(output, 0);
	}
	{ // custom startValue, empty iterator
		std::vector<int> input = {};
		int output = CXXIter::from(input).sum(31337);
		ASSERT_EQ(output, 31337);
	}
}

TEST(CXXIter, stringJoin) {
	{ // non-empty input
		std::vector<int> input = {42, 1337, 64};
		std::string output = CXXIter::from(input)
				.map([](const auto& item) { return std::to_string(item); })
				.stringJoin(", ");
		ASSERT_EQ(output, "42, 1337, 64");
	}
	{ // empty input
		std::vector<int> input = {};
		std::string output = CXXIter::from(input)
				.map([](const auto& item) { return std::to_string(item); })
				.stringJoin(", ");
		ASSERT_EQ(output, "");
	}
}

TEST(CXXIter, mean) {
	{ // non-empty input
		std::vector<float> input = {1.0f, 2.0f, 3.0f};
		std::optional<float> output = CXXIter::from(input).mean();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 2.0f, 0.0000000005);
	}
	{ // empty input
		std::vector<float> input = {};
		std::optional<float> output = CXXIter::from(input).mean();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, last) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CXXIter::from(input).last().toStdOptional();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 52);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CXXIter::from(input).last().toStdOptional();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, nth) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CXXIter::from(input).nth(1).toStdOptional();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 1337);
	}
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CXXIter::from(input).nth(10).toStdOptional();
		ASSERT_FALSE(output.has_value());
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CXXIter::from(input).nth(0).toStdOptional();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, min) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CXXIter::from(input).min();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 42);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CXXIter::from(input).min();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, minBy) {
	{
		std::vector<std::string> input = {"smol", "middle", "largeString"};
		std::optional<std::string> output = CXXIter::from(input)
				.minBy([](const std::string& str) { return str.size(); })
				.toStdOptional();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), "smol");
	}
	{
		std::vector<std::string> input = {};
		std::optional<std::string> output = CXXIter::from(input)
				.minBy([](const std::string& str) { return str.size(); })
				.toStdOptional();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, max) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CXXIter::from(input).max();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 1337);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CXXIter::from(input).max();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, maxBy) {
	{
		std::vector<std::string> input = {"smol", "middle", "largeString"};
		std::optional<std::string> output = CXXIter::from(input)
				.maxBy([](const std::string& str) { return str.size(); })
				.toStdOptional();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), "largeString");
	}
	{
		std::vector<std::string> input = {};
		std::optional<std::string> output = CXXIter::from(input)
				.maxBy([](const std::string& str) { return str.size(); })
				.toStdOptional();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, collect) {
	{
		// additional container type parameters
		std::vector<std::string> input = {"1337", "42", "64"};
		std::vector<std::string, std::allocator<std::string>> output = CXXIter::from(input)
				.collect<std::vector, std::allocator<std::string>>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre("1337", "42", "64"));
	}

	// test as many permutations of items to target collections

	#define COLLECTOR_TEST_FOR_CONTAINER(TARGET_CONTAINER) \
	{ \
		std::vector<std::string> input = {"1337", "42", "64"}; \
		auto output = CXXIter::from(input).collect<TARGET_CONTAINER>(); \
		ASSERT_EQ(output.size(), 3); \
	} \

	#define PAIR_COLLECTOR_TEST_FOR_CONTAINER(TARGET_CONTAINER) \
	{ \
		std::vector<TestPair> input = {{"1337", 1337}, {"42", 42}, {"64", 64}}; \
		auto output = CXXIter::from(input).collect<TARGET_CONTAINER>(); \
		ASSERT_EQ(output.size(), 3); \
	}

	// CustomContainer
	COLLECTOR_TEST_FOR_CONTAINER(CustomContainer); PAIR_COLLECTOR_TEST_FOR_CONTAINER(CustomContainer);

	// back-inserter containers
	COLLECTOR_TEST_FOR_CONTAINER(std::vector); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::vector);
	COLLECTOR_TEST_FOR_CONTAINER(std::list); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::list);
	COLLECTOR_TEST_FOR_CONTAINER(std::deque); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::deque);

	// insert containers
	COLLECTOR_TEST_FOR_CONTAINER(std::set); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::set);
	COLLECTOR_TEST_FOR_CONTAINER(std::multiset); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::multiset);
	COLLECTOR_TEST_FOR_CONTAINER(std::unordered_set); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_set);
	COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multiset); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multiset);

	// associative containers
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::map);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::multimap);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_map);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multimap);

	#undef COLLECTOR_TEST_FOR_CONTAINER
	#undef PAIR_COLLECTOR_TEST_FOR_CONTAINER
}
