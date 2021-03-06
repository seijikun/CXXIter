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
TEST(CXXIter, iterator) {
	std::vector<size_t> input = {1, 3, 3, 7};
	auto iter = CXXIter::from(input)
			.map([](size_t item) { return (item + 1); })
			.filter([](size_t item) { return (item >= 4); })
			.skip(1);

	std::vector<size_t> output;
	for(size_t item : iter) {
		output.push_back(item);
	}
	ASSERT_EQ(output.size(), 2);
	ASSERT_THAT(output, ElementsAre(4, 8));
}

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

TEST(CXXIter, all) {
	auto boolTester = [](const std::vector<bool>& input) -> bool {
		return CXXIter::from(input).copied().all();
	};

	ASSERT_FALSE(boolTester({false, false, false, false}));
	ASSERT_FALSE(boolTester({true, true, true, false}));
	ASSERT_FALSE(boolTester({false, true, true, true}));
	ASSERT_FALSE(boolTester({false, false, true, true}));
	ASSERT_FALSE(boolTester({true, true, false, true}));
	ASSERT_TRUE(boolTester({true, true, true, true}));

	auto intAsBoolFn = [](uint32_t item) -> bool { return (item != 0); };
	auto intTester = [&intAsBoolFn](const std::vector<uint32_t>& input) -> bool {
		return CXXIter::from(input).copied().all(intAsBoolFn);
	};
	ASSERT_FALSE(intTester({0, 0, 0, 0}));
	ASSERT_FALSE(intTester({1, 1, 1, 0}));
	ASSERT_FALSE(intTester({0, 1, 1, 1}));
	ASSERT_FALSE(intTester({0, 0, 1, 1}));
	ASSERT_FALSE(intTester({1, 1, 0, 1}));
	ASSERT_TRUE(boolTester({1, 1, 1, 1}));
}

TEST(CXXIter, any) {
	auto boolTester = [](const std::vector<bool>& input) -> bool {
		return CXXIter::from(input).copied().any();
	};

	ASSERT_FALSE(boolTester({false, false, false, false}));
	ASSERT_TRUE(boolTester({true, true, true, false}));
	ASSERT_TRUE(boolTester({false, true, true, true}));
	ASSERT_TRUE(boolTester({false, false, true, true}));
	ASSERT_TRUE(boolTester({true, true, false, true}));
	ASSERT_TRUE(boolTester({true, true, true, true}));

	auto intAsBoolFn = [](uint32_t item) -> bool { return (item != 0); };
	auto intTester = [&intAsBoolFn](const std::vector<uint32_t>& input) -> bool {
		return CXXIter::from(input).copied().any(intAsBoolFn);
	};
	ASSERT_FALSE(intTester({0, 0, 0, 0}));
	ASSERT_TRUE(intTester({1, 1, 1, 0}));
	ASSERT_TRUE(intTester({0, 1, 1, 1}));
	ASSERT_TRUE(intTester({0, 0, 1, 1}));
	ASSERT_TRUE(intTester({1, 1, 0, 1}));
	ASSERT_TRUE(boolTester({1, 1, 1, 1}));
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
	{
		std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
		size_t output = CXXIter::from(input)
				.count([](int item){ return (item % 2 == 0); });
		ASSERT_EQ(output, 5);
	}
	{
		std::vector<int> input = {1, 3, 5, 7, 9, 11};
		size_t output = CXXIter::from(input)
				.count([](int item){ return (item % 2 == 0); });
		ASSERT_EQ(output, 0);
	}
	{
		std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
		size_t output = CXXIter::from(input)
				.map([](int item) { return (item % 2 == 0); })
				.count(true);
		ASSERT_EQ(output, 5);
	}
	{
		std::vector<int> input = {1, 3, 5, 7, 9, 11};
		size_t output = CXXIter::from(input)
				.map([](int item) { return (item % 2 == 0); })
				.count(true);
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
	{ // non-empty input, Norm::N_MINUS_ONE
		std::vector<float> input = {1.0f, 2.0f, 3.0f};
		std::optional<float> output = CXXIter::from(input)
				.mean<CXXIter::StatisticNormalization::N_MINUS_ONE>();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 3.0f, 0.0000000005);
	}
	{ // empty input, Norm::N
		std::vector<float> input = {};
		std::optional<float> output = CXXIter::from(input).mean();
		ASSERT_FALSE(output.has_value());
	}

	// structure with retarded default ctor, that initializes internal value with weird value
	// Have to provide sumStart to mean() to get meaningful results
	// - This unit-test was definitely not added because a certain c++ math vector template
	//   library has a retarded default ctor that doesn't initialize values...
	struct VecWithDumbDefaultCtor {
		double val;
		VecWithDumbDefaultCtor(double val = 1.0) : val(val) {}
		VecWithDumbDefaultCtor& operator +=(const VecWithDumbDefaultCtor& o) { val += o.val; return *this; }
		VecWithDumbDefaultCtor& operator /=(double div) { val /= div; return *this; }
		VecWithDumbDefaultCtor operator/ (double div) { VecWithDumbDefaultCtor tmp = *this; tmp /= div; return tmp; }
	};
	{ // using sumStart from default ctor
		std::vector<VecWithDumbDefaultCtor> input = {1.0, 2.0, 3.0};
		std::optional<VecWithDumbDefaultCtor> output = CXXIter::from(input)
				.mean<CXXIter::StatisticNormalization::N_MINUS_ONE, VecWithDumbDefaultCtor, double>();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value().val, 3.5f, 0.0000000005);
	}
	{ // using sumStart from passed initial value
		std::vector<VecWithDumbDefaultCtor> input = {1.0, 2.0, 3.0};
		std::optional<VecWithDumbDefaultCtor> output = CXXIter::from(input)
				.mean<CXXIter::StatisticNormalization::N_MINUS_ONE, VecWithDumbDefaultCtor, double>({0.0});
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value().val, 3.0f, 0.0000000005);
	}
}

TEST(CXXIter, variance) {
	{ // empty input
		std::vector<float> input = {};
		std::optional<float> output = CXXIter::from(input).variance();
		ASSERT_FALSE(output.has_value());
	}
	{ // input (too few elements)
		std::vector<float> input = { 1.0f };
		std::optional<float> output = CXXIter::from(input).variance();
		ASSERT_FALSE(output.has_value());
	}
	{ // non-empty input, Norm::N
		std::vector<float> input = {1.0f, 2.0f, 3.0f};
		std::optional<float> output = CXXIter::from(input).variance();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 0.6666666f, 0.00001f);
	}
	{ // non-empty input, Norm::N_MINUS_ONE
		std::vector<float> input = {1.0f, 2.0f, 3.0f};
		std::optional<float> output = CXXIter::from(input)
				.variance<CXXIter::StatisticNormalization::N_MINUS_ONE>();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 1.0f, 0.00001f);
	}
	{ // non-empty input, Norm::N
		std::vector<float> input = {2.0f, 4.0f, 4.0f, 4.0f, 5.0f, 5.0f, 7.0f, 9.0f};
		std::optional<float> output = CXXIter::from(input).variance();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 4.0f, 0.0001f);
	}
	{ // non-empty input, Norm::N_MINUS_ONE
		std::vector<float> input = {2.0f, 4.0f, 4.0f, 4.0f, 5.0f, 5.0f, 7.0f, 9.0f};
		std::optional<float> output = CXXIter::from(input)
				.variance<CXXIter::StatisticNormalization::N_MINUS_ONE>();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 4.5714f, 0.0001f);
	}
}

TEST(CXXIter, stddev) {
	{ // empty input
		std::vector<float> input = {};
		std::optional<float> output = CXXIter::from(input).stddev();
		ASSERT_FALSE(output.has_value());
	}
	{ // input (too few elements)
		std::vector<float> input = { 1.0f };
		std::optional<float> output = CXXIter::from(input).stddev();
		ASSERT_FALSE(output.has_value());
	}
	{ // non-empty input, Norm::N
		std::vector<float> input = {1.0f, 2.0f, 3.0f};
		std::optional<float> output = CXXIter::from(input).stddev();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 0.816496, 0.00001f);
	}
	{ // non-empty input, Norm::N_MINUS_ONE
		std::vector<float> input = {1.0f, 2.0f, 3.0f};
		std::optional<float> output = CXXIter::from(input)
				.stddev<CXXIter::StatisticNormalization::N_MINUS_ONE>();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 1.0f, 0.00001f);
	}
	{ // non-empty input, Norm::N
		std::vector<float> input = {2.0f, 4.0f, 4.0f, 4.0f, 5.0f, 5.0f, 7.0f, 9.0f};
		std::optional<float> output = CXXIter::from(input).stddev();
		ASSERT_TRUE(output.has_value());
		ASSERT_NEAR(output.value(), 2.0f, 0.00000005f);
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
		std::optional<int> output = CXXIter::from(input).copied().min().toStdOptional();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 42);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CXXIter::from(input).copied().min().toStdOptional();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, minIdx) {
	{
		std::vector<int> input = {1337, 42, 52};
		std::optional<size_t> output = CXXIter::from(input).minIdx();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 1);
	}
	{
		std::vector<int> input = {};
		std::optional<size_t> output = CXXIter::from(input).minIdx();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, minBy) {
	{
		const std::vector<std::string> input = {"middle", "smol", "largeString"};
		std::optional<std::string> output = CXXIter::SrcCRef(input)
				.minBy([](const std::string& str) { return str.size(); })
				.toStdOptional();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), "smol");
		ASSERT_THAT(input, ElementsAre("middle", "smol", "largeString"));
	}
	{
		std::vector<std::string> input = {"middle", "smol", "largeString"};
		std::optional<std::string> output = CXXIter::SrcRef(input)
				.minBy([](const std::string& str) { return str.size(); })
				.toStdOptional();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), "smol");
		ASSERT_THAT(input, ElementsAre("middle", "smol", "largeString"));
	}
	{
		std::vector<std::string> input = {"middle", "smol", "largeString"};
		std::optional<std::string> output = CXXIter::SrcMov(std::move(input))
				.minBy([](std::string&& str) { return str.size(); })
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

TEST(CXXIter, minIdxBy) {
	{
		const std::vector<std::string> input = {"middle", "smol", "largeString"};
		std::optional<size_t> output = CXXIter::SrcCRef(input)
				.minIdxBy([](const std::string& str) { return str.size(); });
		ASSERT_EQ(output.value(), 1);
		ASSERT_THAT(input, ElementsAre("middle", "smol", "largeString"));
	}
	{
		std::vector<std::string> input = {"middle", "smol", "largeString"};
		std::optional<size_t> output = CXXIter::SrcRef(input)
				.minIdxBy([](const std::string& str) { return str.size(); });
		ASSERT_EQ(output.value(), 1);
		ASSERT_THAT(input, ElementsAre("middle", "smol", "largeString"));
	}
	{
		std::vector<std::string> input = {"middle", "smol", "largeString"};
		std::optional<size_t> output = CXXIter::SrcMov(std::move(input))
				.minIdxBy([](std::string&& str) { return str.size(); });
		ASSERT_EQ(output.value(), 1);
	}
	{
		std::vector<std::string> input = {};
		std::optional<size_t> output = CXXIter::from(input)
				.minIdxBy([](const std::string& str) { return str.size(); });
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, max) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CXXIter::from(input).copied().max().toStdOptional();
		ASSERT_EQ(output.value(), 1337);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CXXIter::from(input).copied().max().toStdOptional();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, maxIdx) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<size_t> output = CXXIter::from(input).maxIdx();
		ASSERT_EQ(output.value(), 1);
	}
	{
		std::vector<int> input = {};
		std::optional<size_t> output = CXXIter::from(input).maxIdx();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, maxBy) {
	{
		const std::vector<std::string> input = {"smol", "middle", "largeString"};
		std::optional<std::string> output = CXXIter::SrcCRef(input)
				.maxBy([](const std::string& str) { return str.size(); })
				.toStdOptional();
		ASSERT_EQ(output.value(), "largeString");
		ASSERT_THAT(input, ElementsAre("smol", "middle", "largeString"));
	}
	{
		std::vector<std::string> input = {"smol", "middle", "largeString"};
		std::optional<std::string> output = CXXIter::SrcRef(input)
				.maxBy([](std::string& str) { return str.size(); })
				.toStdOptional();
		ASSERT_EQ(output.value(), "largeString");
		ASSERT_THAT(input, ElementsAre("smol", "middle", "largeString"));
	}
	{
		std::vector<std::string> input = {"smol", "middle", "largeString"};
		std::optional<std::string> output = CXXIter::SrcMov(std::move(input))
				.maxBy([](std::string&& str) { return str.size(); })
				.toStdOptional();
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

TEST(CXXIter, maxIdxBy) {
	{
		const std::vector<std::string> input = {"smol", "middle", "largeString"};
		std::optional<size_t> output = CXXIter::SrcCRef(input)
				.maxIdxBy([](const std::string& str) { return str.size(); });
		ASSERT_EQ(output.value(), 2);
		ASSERT_THAT(input, ElementsAre("smol", "middle", "largeString"));
	}
	{
		std::vector<std::string> input = {"smol", "middle", "largeString"};
		std::optional<size_t> output = CXXIter::SrcRef(input)
				.maxIdxBy([](std::string& str) { return str.size(); });
		ASSERT_EQ(output.value(), 2);
		ASSERT_THAT(input, ElementsAre("smol", "middle", "largeString"));
	}
	{
		std::vector<std::string> input = {"smol", "middle", "largeString"};
		std::optional<size_t> output = CXXIter::SrcMov(std::move(input))
				.maxIdxBy([](std::string&& str) { return str.size(); });
		ASSERT_EQ(output.value(), 2);
	}
	{
		std::vector<std::string> input = {};
		std::optional<size_t> output = CXXIter::from(input)
				.maxIdxBy([](const std::string& str) { return str.size(); });
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CXXIter, collect) {
	{ // additional container type parameters
		std::vector<std::string> input = {"1337", "42", "64"};
		std::vector<std::string, std::allocator<std::string>> output = CXXIter::from(input)
				.collect<std::vector, std::allocator<std::string>>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre("1337", "42", "64"));
	}
	{ // collect to string
		std::string input = "ceasarencrypt";
		std::string output = CXXIter::from(input)
				.map([](char c) -> char { return (c + 1); })
				.collect<std::basic_string>();
		ASSERT_EQ(output, "dfbtbsfodszqu");
	}

	// test as many permutations of items to target collections

	#define COLLECTOR_TEST_FOR_CONTAINER(TARGET_CONTAINER) { \
		std::vector<std::string> input = {"1337", "42", "64"}; \
		auto output = CXXIter::from(input).collect<TARGET_CONTAINER>(); \
		ASSERT_EQ(output.size(), 3); \
	} \

	#define PAIR_COLLECTOR_TEST_FOR_CONTAINER(TARGET_CONTAINER) { \
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

TEST(CXXIter, collect2) {
	{ // collect to string
		std::string input = "ceasarencrypt";
		std::string output = CXXIter::from(input)
				.map([](char c) -> char { return (c + 1); })
				.collect<std::string>();
		ASSERT_EQ(output, "dfbtbsfodszqu");
	}

	// test as many permutations of items to target collections

	#define COLLECTOR_TEST_FOR_CONTAINER(...) { \
		std::vector<std::string> input = {"1337", "42", "64"}; \
		auto output = CXXIter::from(input).collect<__VA_ARGS__>(); \
		ASSERT_EQ(output.size(), 3); \
	}

	#define PAIR_COLLECTOR_TEST_FOR_CONTAINER(...) { \
		std::vector<TestPair> input = {{"1337", 1337}, {"42", 42}, {"64", 64}}; \
		auto output = CXXIter::from(input).collect<__VA_ARGS__>(); \
		ASSERT_EQ(output.size(), 3); \
	}

	// CustomContainer
	COLLECTOR_TEST_FOR_CONTAINER(CustomContainer<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(CustomContainer<TestPair>);

	// back-inserter containers
	COLLECTOR_TEST_FOR_CONTAINER(std::vector<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::vector<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::list<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::list<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::deque<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::deque<TestPair>);

	// insert containers
	COLLECTOR_TEST_FOR_CONTAINER(std::set<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::set<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::multiset<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::multiset<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::unordered_set<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_set<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multiset<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multiset<TestPair>);

	// associative containers
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::map<std::string, int>);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::multimap<std::string, int>);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_map<std::string, int>);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multimap<std::string, int>);

	// std::array
	COLLECTOR_TEST_FOR_CONTAINER(std::array<std::string, 3>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::array<TestPair, 3>);

	#undef COLLECTOR_TEST_FOR_CONTAINER
	#undef PAIR_COLLECTOR_TEST_FOR_CONTAINER
}

TEST(CXXIter, collectInto) {
	{ // additional container type parameters
		std::vector<std::string> input = {"1337", "42", "64"};
		std::vector<std::string, std::allocator<std::string>> output;
		CXXIter::from(input).collectInto(output);
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre("1337", "42", "64"));
	}
	{ // collect to string
		std::string input = "ceasarencrypt";
		std::string output;
		CXXIter::from(input)
				.map([](char c) -> char { return (c + 1); })
				.collectInto(output);
		ASSERT_EQ(output, "dfbtbsfodszqu");
	}
	{ // collect to std::array
		std::vector<float> input = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
		std::array<float, 3> output;
		CXXIter::from(input).copied()
				.filter([](float item) { return item < 3.5; })
				.collectInto(output);
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre(1.0, 2.0, 3.0));
	}

	// test as many permutations of items to target collections

	#define COLLECTOR_TEST_FOR_CONTAINER(...) { \
		std::vector<std::string> input = {"1337", "42", "64"}; \
		__VA_ARGS__ output = {"pre-existing item in output"}; \
		CXXIter::from(input).collectInto(output); \
		ASSERT_EQ(output.size(), 3 + 1); \
	} \

	#define PAIR_COLLECTOR_TEST_FOR_CONTAINER(...) { \
		std::vector<TestPair> input = {{"1337", 1337}, {"42", 42}, {"64", 64}}; \
		__VA_ARGS__ output = { std::make_pair(std::string("pre-existing"), 5)}; \
		CXXIter::from(input).collectInto(output); \
		ASSERT_EQ(output.size(), 3 + 1); \
	}

	// CustomContainer
	COLLECTOR_TEST_FOR_CONTAINER(CustomContainer<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(CustomContainer<TestPair>);

	// back-inserter containers
	COLLECTOR_TEST_FOR_CONTAINER(std::vector<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::vector<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::list<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::list<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::deque<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::deque<TestPair>);

	// insert containers
	COLLECTOR_TEST_FOR_CONTAINER(std::set<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::set<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::multiset<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::multiset<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::unordered_set<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_set<TestPair>);
	COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multiset<std::string>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multiset<TestPair>);

	// associative containers
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::map<std::string, int>);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::multimap<std::string, int>);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_map<std::string, int>);
	PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::unordered_multimap<std::string, int>);

	// std::array
	COLLECTOR_TEST_FOR_CONTAINER(std::array<std::string, 4>); PAIR_COLLECTOR_TEST_FOR_CONTAINER(std::array<TestPair, 4>);

	#undef COLLECTOR_TEST_FOR_CONTAINER
	#undef PAIR_COLLECTOR_TEST_FOR_CONTAINER
}
