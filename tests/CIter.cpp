#include <CIter/CIter.h>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <optional>
#include <unordered_set>
#include <unordered_map>

using ::testing::ElementsAre;
using ::testing::Pair;

enum class LifecycleEventType {
	CTOR,
	DTOR,
	MOVECTOR,
	CPYCTOR
};
struct LifecycleEvent {
	const void* ptr;
	LifecycleEventType event;
	/**
	 * @brief Optional pointer determining the memory area used as source for copy/move ctor or assignment.
	 */
	const void* ptrFrom;
	LifecycleEvent(const void* ptr, LifecycleEventType event, const void* ptrFrom = nullptr) : ptr(ptr), event(event), ptrFrom(ptrFrom) {}
};

using LifecycleEvents = std::vector<LifecycleEvent>;

struct LifecycleDebugger {
	bool alive = true;
	std::string heapTest;
	LifecycleEvents& evtLog;

	LifecycleDebugger(std::string heapTest, LifecycleEvents& evtLog) : heapTest(heapTest), evtLog(evtLog) {
		evtLog.push_back(LifecycleEvent(this, LifecycleEventType::CTOR));
	}
	~LifecycleDebugger() {
		evtLog.push_back(LifecycleEvent(this, LifecycleEventType::DTOR));
	}
	LifecycleDebugger(const LifecycleDebugger& o) : heapTest(o.heapTest), evtLog(o.evtLog) {
		evtLog.push_back(LifecycleEvent(this, LifecycleEventType::CPYCTOR, &o));
	}
	LifecycleDebugger& operator=(const LifecycleDebugger& o) = delete;
	LifecycleDebugger(LifecycleDebugger&& o) : heapTest(std::move(o.heapTest)), evtLog(o.evtLog) {
		o.alive = false;
		evtLog.push_back(LifecycleEvent(this, LifecycleEventType::MOVECTOR, &o));
	}
	LifecycleDebugger& operator=(LifecycleDebugger&& o) = delete;
};



// ################################################################################################
// SOURCES
// ################################################################################################

TEST(CIter, srcMove) {
	LifecycleEvents evtLog;

	{ // move
		{
			std::vector<LifecycleDebugger> input;
			input.emplace_back("heapTestString", evtLog);

			auto outVec = CIter::SrcMov(std::move(input))
				.filter([&evtLog](const LifecycleDebugger&) {
					if(evtLog.size() != 2) { throw std::runtime_error("filter()"); }
					return true;
				})
				.filterMap([&evtLog](LifecycleDebugger&& o) -> std::optional<std::string> {
					if(evtLog.size() != 4) { throw std::runtime_error("filterMap()"); }
					return std::move(o.heapTest);
				})
				.collect<std::vector>();
			ASSERT_EQ(outVec.size(), 1);
			ASSERT_EQ(outVec[0], "heapTestString");
		}

		ASSERT_EQ(evtLog.size(), 6);
		ASSERT_EQ(evtLog[0].event, LifecycleEventType::CTOR);
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::MOVECTOR); // move SrcMov -> Filter (tmp1)
		ASSERT_EQ(evtLog[2].event, LifecycleEventType::MOVECTOR); // move Filter -> FilterMap (tmp2)

		ASSERT_EQ(evtLog[3].event, LifecycleEventType::DTOR); // dtor of tmp2
		ASSERT_EQ(evtLog[1].ptr, evtLog[3].ptr);

		ASSERT_EQ(evtLog[4].event, LifecycleEventType::DTOR); // dtor of tmp1
		ASSERT_EQ(evtLog[2].ptr, evtLog[4].ptr);

		ASSERT_EQ(evtLog[5].event, LifecycleEventType::DTOR); // dtor of original input (stored in SrcMov)
		ASSERT_EQ(evtLog[0].ptr, evtLog[5].ptr);
	}
}

TEST(CIter, srcConstRef) {
	LifecycleEvents evtLog;
	{ // const references
		{
			std::vector<LifecycleDebugger> input;
			input.emplace_back("heapTestString", evtLog);

			std::vector<std::string> outVec = CIter::SrcCRef(input)
					.filter([](const LifecycleDebugger&){ return true; })
					.map([](const LifecycleDebugger& o) { return o.heapTest; })
					.collect<std::vector>();
			ASSERT_EQ(outVec.size(), 1);
			ASSERT_EQ(outVec[0], "heapTestString");
			ASSERT_EQ(input[0].heapTest, "heapTestString");
			ASSERT_TRUE(input[0].alive);
		}

		ASSERT_EQ(evtLog.size(), 2);
		ASSERT_EQ(evtLog[0].event, LifecycleEventType::CTOR);
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::DTOR);
		ASSERT_EQ(evtLog[0].ptr, evtLog[1].ptr);
	}
}

TEST(CIter, srcRef) {
	LifecycleEvents evtLog;

	{ // mutable references (move out of heapTest)
		{
			std::vector<LifecycleDebugger> input;
			input.emplace_back("heapTestString", evtLog);

			std::vector<std::string> outVec = CIter::SrcRef(input)
					.filter([](const LifecycleDebugger& o){ return true; })
					.map([](LifecycleDebugger& o) { return std::move(o.heapTest); })
					.collect<std::vector>();
			ASSERT_EQ(outVec.size(), 1);
			ASSERT_EQ(outVec[0], "heapTestString");
			ASSERT_EQ(input[0].heapTest, ""); // we only moved out of the string
			ASSERT_TRUE(input[0].alive); // ^
		}

		ASSERT_EQ(evtLog.size(), 2);
		ASSERT_EQ(evtLog[0].event, LifecycleEventType::CTOR);
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::DTOR);
		ASSERT_EQ(evtLog[0].ptr, evtLog[1].ptr);
	}
}




// ################################################################################################
// CHAINERS
// ################################################################################################

TEST(CIter, cast) {
	std::vector<float> input = {1.35, 56.123};
	std::vector<double> output = CIter::from(input)
			.cast<double>()
			.collect<std::vector>();
	ASSERT_EQ(input.size(), output.size());
	for(size_t i = 0; i < input.size(); ++i) { ASSERT_NEAR(input[i], output[i], 0.000005); }
}

TEST(CIter, filter) {
	{
		std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
		std::vector<int> output = CIter::from(input)
				.filter([](int item) { return (item % 2) == 0; })
				.collect<std::vector>();
		ASSERT_EQ(4, output.size());
		ASSERT_THAT(output, ElementsAre(2, 4, 6, 8));
	}
	{
		std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
		std::vector<int> output = CIter::SrcMov(std::move(input))
				.filter([](int item) { return (item % 2) == 0; })
				.collect<std::vector>();
		ASSERT_EQ(4, output.size());
		ASSERT_THAT(output, ElementsAre(2, 4, 6, 8));
	}
}

TEST(CIter, map) {
	{
		std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
		std::unordered_set<int> output = CIter::from(input)
				.map([](const auto& pair) { return pair.first; })
				.collect<std::unordered_set>();
		for(const int& item : output) { ASSERT_TRUE(input.contains(item)); }
	}
	{
		std::vector<int> input = {1337, 42};
		std::unordered_map<int, std::string> output = CIter::from(input)
				.map([](int i) { return std::make_pair(i, std::to_string(i)); }) // construct pair
				.collect<std::unordered_map>(); // collect into map
		for(const int& item : input) {
			ASSERT_TRUE(output.contains(item));
			ASSERT_EQ(output[item], std::to_string(item));
		}
	}
}

TEST(CIter, modify) {
	std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
	std::unordered_map<int, std::string> output = CIter::from(input)
			.modify([](auto& keyValue) { keyValue.second = "-" + keyValue.second; }) // modify input
			.collect<std::unordered_map>(); // copy to output
	for(const auto& item : input) {
		ASSERT_TRUE(output.contains(item.first));
		ASSERT_EQ(item.second, input[item.first]);
		ASSERT_TRUE(item.second.starts_with("-"));
	}
}

TEST(CIter, skip) {
	std::vector<int> input = {42, 42, 42, 42, 1337};
	std::vector<int> output = CIter::from(input)
			.skip(3) // skip first 3 values
			.collect<std::vector>();
	ASSERT_EQ(output.size(), 2);
	ASSERT_THAT(output, ElementsAre(42, 1337));
}

TEST(CIter, skipWhile) {
	std::vector<int> input = {42, 42, 42, 42, 1337, 42};
	std::vector<int> output = CIter::from(input)
			.skipWhile([](const int value) { return (value == 42); }) // skip leading 42s
			.collect<std::vector>();
	ASSERT_EQ(output.size(), 2);
	ASSERT_THAT(output, ElementsAre(1337, 42));
}

TEST(CIter, take) {
	{
		std::vector<int> input = {42, 57, 64, 128, 1337, 10};
		std::vector<int> output = CIter::from(input)
				.take(3) // take first 3 values
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_THAT(output, ElementsAre(42, 57, 64));
	}
	{
		std::string input = "test";
		std::string output = CIter::from(input)
				.take(3)
				.collect<std::basic_string>();
		ASSERT_EQ(output.size(), 3);
		ASSERT_EQ(output, "tes");
	}
}

TEST(CIter, takeWhile) {
	std::vector<int> input = {42, 57, 64, 128, 1337, 10};
	std::vector<int> output = CIter::from(input)
			.takeWhile([](const int value) { return (value < 1000); }) // take until first item > 1000
			.collect<std::vector>();
	ASSERT_EQ(output.size(), 4);
	ASSERT_THAT(output, ElementsAre(42, 57, 64, 128));
}

TEST(CIter, flatMap) {
	std::vector<std::pair<std::string, std::vector<int>>> input = {{"first pair", {1337, 42}}, {"second pair", {6, 123, 7888}}};
	std::vector<int> output = CIter::from(std::move(input))
			.flatMap([](auto&& item) { return std::get<1>(item); })
			.collect<std::vector>();
	ASSERT_EQ(output.size(), 5);
	ASSERT_THAT(output, ElementsAre(1337, 42, 6, 123, 7888));
}

TEST(CIter, zip) {
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<int> input2 = {1337, 42};
		std::vector<std::pair<std::string, int>> output = CIter::from(input1).copied()
				.zip(CIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), input1.size());
		ASSERT_THAT(output, ElementsAre(Pair("1337", 1337), Pair("42", 42)));
	}
	{
		std::vector<std::string> input1 = {"1337", "42"};
		std::vector<int> input2 = {1337, 42, 80};
		std::vector<std::pair<std::string, int>> output = CIter::from(input1).copied()
				.zip(CIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre(Pair("1337", 1337), Pair("42", 42)));
	}
	{
		std::vector<std::string> input1 = {"1337", "42", "80"};
		std::vector<int> input2 = {1337, 42};
		std::vector<std::pair<std::string, int>> output = CIter::from(input1).copied()
				.zip(CIter::from(input2).copied())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_THAT(output, ElementsAre(Pair("1337", 1337), Pair("42", 42)));
	}
}

TEST(CIter, count) {
	{
		std::vector<int> input = {42, 1337, 52};
		size_t output = CIter::from(input).count();
		ASSERT_EQ(output, 3);
	}
	{
		std::vector<int> input = {};
		size_t output = CIter::from(input).count();
		ASSERT_EQ(output, 0);
	}
}

TEST(CIter, sum) {
	{
		std::vector<int> input = {42, 1337, 52};
		int output = CIter::from(input).sum();
		ASSERT_EQ(output, 1431);
	}
	{
		std::vector<int> input = {};
		int output = CIter::from(input).sum();
		ASSERT_EQ(output, 0);
	}
}

TEST(CIter, last) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CIter::from(input).last();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 52);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CIter::from(input).last();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CIter, min) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CIter::from(input).min();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 42);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CIter::from(input).min();
		ASSERT_FALSE(output.has_value());
	}
}

TEST(CIter, max) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CIter::from(input).max();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 1337);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CIter::from(input).max();
		ASSERT_FALSE(output.has_value());
	}
}
