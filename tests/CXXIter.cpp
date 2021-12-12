#include <CXXIter/CXXIter.h>

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
template<typename TItem, const size_t COUNT> struct CustomContainer {
	using CustomContainerItem = TItem;
	std::unique_ptr<std::array<CustomContainerItem, COUNT>> input = nullptr;

	CustomContainer(std::array<CustomContainerItem, COUNT>&& initial) : input(new std::array<CustomContainerItem, COUNT>(std::move(initial))) {}

	size_t size() const { return input->size(); }
	CustomContainerItem& get(size_t idx) { return input->at(idx); }
	const CustomContainerItem& get(size_t idx) const { return input->at(idx); }
};

namespace CXXIter { // SourceTrait implementation for the CustomContainer
	template<typename TItem, const size_t COUNT> struct SourceTrait<CustomContainer<TItem, COUNT>> {
		using Item = typename CustomContainer<TItem, COUNT>::CustomContainerItem;
		using IteratorState = size_t;
		using ConstIteratorState = size_t;

		static inline IteratorState initIterator(CustomContainer<TItem, COUNT>&) { return 0; }
		static inline ConstIteratorState initIterator(const CustomContainer<TItem, COUNT>&) { return 0; }

		static inline bool hasNext(CustomContainer<TItem, COUNT>& container, IteratorState& iter) { return iter < container.size(); }
		static inline bool hasNext(const CustomContainer<TItem, COUNT>& container, ConstIteratorState& iter) { return iter < container.size(); }

		static inline Item& next(CustomContainer<TItem, COUNT>& container, IteratorState& iter) { return container.get(iter++); }
		static inline const Item& next(const CustomContainer<TItem, COUNT>& container, ConstIteratorState& iter) { return container.get(iter++); }
	};
};

TEST(CXXIter, srcMove) { // move
	{ // std::vector
		LifecycleEvents evtLog;
		{
			std::vector<LifecycleDebugger> input;
			input.emplace_back("heapTestString", evtLog);

			auto outVec = CXXIter::SrcMov(std::move(input))
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
	{ // CustomContainer
		LifecycleEvents evtLog;
		{
			CustomContainer<LifecycleDebugger, 1> input({ LifecycleDebugger("heapTestString", evtLog) });

			auto outVec = CXXIter::SrcMov(std::move(input))
				.filter([&evtLog](const LifecycleDebugger&) {
					if(evtLog.size() != 4) { throw std::runtime_error("filter()"); }
					return true;
				})
				.filterMap([&evtLog](LifecycleDebugger&& o) -> std::optional<std::string> {
					if(evtLog.size() != 6) { throw std::runtime_error("filterMap()"); }
					return std::move(o.heapTest);
				})
				.collect<std::vector>();
			ASSERT_EQ(outVec.size(), 1);
			ASSERT_EQ(outVec[0], "heapTestString");
		}

		ASSERT_EQ(evtLog.size(), 8);
		ASSERT_EQ(evtLog[0].event, LifecycleEventType::CTOR); // ctor in initializer list
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::MOVECTOR); // move initializer list -> CustomContainer
		ASSERT_EQ(evtLog[2].event, LifecycleEventType::DTOR); // dtor in initializer list
		ASSERT_EQ(evtLog[0].ptr, evtLog[2].ptr);

		ASSERT_EQ(evtLog[3].event, LifecycleEventType::MOVECTOR); // move SrcMov -> Filter (tmp1)
		ASSERT_EQ(evtLog[4].event, LifecycleEventType::MOVECTOR); // move Filter -> FilterMap (tmp2)

		ASSERT_EQ(evtLog[5].event, LifecycleEventType::DTOR); // dtor of tmp2
		ASSERT_EQ(evtLog[3].ptr, evtLog[5].ptr);

		ASSERT_EQ(evtLog[6].event, LifecycleEventType::DTOR); // dtor of tmp1
		ASSERT_EQ(evtLog[4].ptr, evtLog[6].ptr);

		ASSERT_EQ(evtLog[7].event, LifecycleEventType::DTOR); // dtor of original input (stored in SrcMov)
		ASSERT_EQ(evtLog[1].ptr, evtLog[7].ptr);
	}
}

TEST(CXXIter, srcConstRef) { // const references
	{ // std::vector
		LifecycleEvents evtLog;
		{
			std::vector<LifecycleDebugger> input;
			input.emplace_back("heapTestString", evtLog);

			std::vector<std::string> outVec = CXXIter::SrcCRef(input)
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
	{ // CustomContainer
		LifecycleEvents evtLog;
		{
			CustomContainer<LifecycleDebugger, 1> input({ LifecycleDebugger("heapTestString", evtLog) });

			std::vector<std::string> outVec = CXXIter::SrcCRef(input)
					.filter([](const LifecycleDebugger&){ return true; })
					.map([](const LifecycleDebugger& o) { return o.heapTest; })
					.collect<std::vector>();
			ASSERT_EQ(outVec.size(), 1);
			ASSERT_EQ(outVec[0], "heapTestString");
			ASSERT_EQ(input.get(0).heapTest, "heapTestString");
			ASSERT_TRUE(input.get(0).alive);
		}

		ASSERT_EQ(evtLog.size(), 4);
		ASSERT_EQ(evtLog[0].event, LifecycleEventType::CTOR); // ctor in initializer list
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::MOVECTOR); // move initializer list -> CustomContainer
		ASSERT_EQ(evtLog[2].event, LifecycleEventType::DTOR); // dtor in initializer list
		ASSERT_EQ(evtLog[0].ptr, evtLog[2].ptr);

		ASSERT_EQ(evtLog[3].event, LifecycleEventType::DTOR); // dtor in CustomContainer
		ASSERT_EQ(evtLog[1].ptr, evtLog[3].ptr);
	}
}

TEST(CXXIter, srcRef) { // mutable references (move out of heapTest)
	{ // std::vector
		LifecycleEvents evtLog;
		{
			std::vector<LifecycleDebugger> input;
			input.emplace_back("heapTestString", evtLog);

			std::vector<std::string> outVec = CXXIter::SrcRef(input)
					.filter([](const LifecycleDebugger&){ return true; })
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
	{ // std::vector
		LifecycleEvents evtLog;
		{
			CustomContainer<LifecycleDebugger, 1> input({ LifecycleDebugger("heapTestString", evtLog) });

			std::vector<std::string> outVec = CXXIter::SrcRef(input)
					.filter([](const LifecycleDebugger&){ return true; })
					.map([](LifecycleDebugger& o) { return std::move(o.heapTest); })
					.collect<std::vector>();
			ASSERT_EQ(outVec.size(), 1);
			ASSERT_EQ(outVec[0], "heapTestString");
			ASSERT_EQ(input.get(0).heapTest, ""); // we only moved out of the string
			ASSERT_TRUE(input.get(0).alive); // ^
		}

		ASSERT_EQ(evtLog.size(), 4);
		ASSERT_EQ(evtLog[0].event, LifecycleEventType::CTOR); // ctor in initializer list
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::MOVECTOR); // move initializer list -> CustomContainer
		ASSERT_EQ(evtLog[2].event, LifecycleEventType::DTOR); // dtor in initializer list
		ASSERT_EQ(evtLog[0].ptr, evtLog[2].ptr);

		ASSERT_EQ(evtLog[3].event, LifecycleEventType::DTOR); // dtor in CustomContainer
		ASSERT_EQ(evtLog[1].ptr, evtLog[3].ptr);
	}
}




// ################################################################################################
// CHAINERS
// ################################################################################################
TEST(CXXIter, cast) {
	std::vector<float> input = {1.35, 56.123};
	std::vector<double> output = CXXIter::from(input)
			.cast<double>()
			.collect<std::vector>();
	ASSERT_EQ(input.size(), output.size());
	for(size_t i = 0; i < input.size(); ++i) { ASSERT_NEAR(input[i], output[i], 0.000005); }
}

TEST(CXXIter, copied) {
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

TEST(CXXIter, map) {
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

TEST(CXXIter, skip) {
	std::vector<int> input = {42, 42, 42, 42, 1337};
	std::vector<int> output = CXXIter::from(input)
			.skip(3) // skip first 3 values
			.collect<std::vector>();
	ASSERT_EQ(output.size(), 2);
	ASSERT_THAT(output, ElementsAre(42, 1337));
}

TEST(CXXIter, skipWhile) {
	std::vector<int> input = {42, 42, 42, 42, 1337, 42};
	std::vector<int> output = CXXIter::from(input)
			.skipWhile([](const int value) { return (value == 42); }) // skip leading 42s
			.collect<std::vector>();
	ASSERT_EQ(output.size(), 2);
	ASSERT_THAT(output, ElementsAre(1337, 42));
}

TEST(CXXIter, take) {
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
	std::vector<int> input = {42, 57, 64, 128, 1337, 10};
	std::vector<int> output = CXXIter::from(input)
			.takeWhile([](const int value) { return (value < 1000); }) // take until first item > 1000
			.collect<std::vector>();
	ASSERT_EQ(output.size(), 4);
	ASSERT_THAT(output, ElementsAre(42, 57, 64, 128));
}

TEST(CXXIter, flatMap) {
	std::vector<std::pair<std::string, std::vector<int>>> input = {{"first pair", {1337, 42}}, {"second pair", {6, 123, 7888}}};
	std::vector<int> output = CXXIter::from(std::move(input))
			.flatMap([](auto&& item) { return std::get<1>(item); }) // flatten the std::vector<int> from the pair
			.collect<std::vector>(); // collect into vector containing {1337, 42, 6, 123, 7888}
	ASSERT_EQ(output.size(), 5);
	ASSERT_THAT(output, ElementsAre(1337, 42, 6, 123, 7888));
}

TEST(CXXIter, zip) {
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

TEST(CXXIter, collect) {
	// additional container type parameters
	std::vector<std::string> input = {"1337", "42", "64"};
	std::vector<std::string, std::allocator<std::string>> output = CXXIter::from(input)
			.collect<std::vector, std::allocator<std::string>>();
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

TEST(CXXIter, last) {
	{
		std::vector<int> input = {42, 1337, 52};
		std::optional<int> output = CXXIter::from(input).last();
		ASSERT_TRUE(output.has_value());
		ASSERT_EQ(output.value(), 52);
	}
	{
		std::vector<int> input = {};
		std::optional<int> output = CXXIter::from(input).last();
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
