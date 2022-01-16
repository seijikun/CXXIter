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
// SOURCES
// ################################################################################################
TEST(CXXIter, srcMove) { // move
	{ // sizeHint
		std::vector<int> input = {1, 3, 3, 7};
		auto sizeHint = CXXIter::from(input).sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
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
			CustomContainer<LifecycleDebugger> input({ LifecycleDebugger("heapTestString", evtLog) });

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
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::CPYCTOR); // initializer list -> CustomContainer
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
	{
		std::vector<int> input = {1, 2, 3};
		auto iter = CXXIter::from(std::move(input));
		static_assert(std::is_same_v<decltype(iter), CXXIter::SrcMov<std::vector<int>>>);
		int output = iter
				.map([](int item) { return item * 2; })
				.mean().value();
		ASSERT_EQ(output, 4);
	}
}

TEST(CXXIter, srcConstRef) { // const references
	{ // sizeHint
		std::vector<int> input = {1, 3, 3, 7};
		auto sizeHint = CXXIter::from(input).sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
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
			CustomContainer<LifecycleDebugger> input({ LifecycleDebugger("heapTestString", evtLog) });

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
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::CPYCTOR); // initializer list -> CustomContainer
		ASSERT_EQ(evtLog[2].event, LifecycleEventType::DTOR); // dtor in initializer list
		ASSERT_EQ(evtLog[0].ptr, evtLog[2].ptr);

		ASSERT_EQ(evtLog[3].event, LifecycleEventType::DTOR); // dtor in CustomContainer
		ASSERT_EQ(evtLog[1].ptr, evtLog[3].ptr);
	}
	{
		std::vector<int> input = {1, 2, 3};
		const std::vector<int>& inputCRef = input;
		auto iter = CXXIter::from(inputCRef);
		static_assert(std::is_same_v<decltype(iter), CXXIter::SrcCRef<std::vector<int>>>);
		int output = iter
				.map([](int item) { return item * 2; })
				.mean().value();
		ASSERT_EQ(output, 4);
	}
}

TEST(CXXIter, srcRef) { // mutable references (move out of heapTest)
	{ // sizeHint
		std::vector<int> input = {1, 3, 3, 7};
		auto sizeHint = CXXIter::from(input).sizeHint();
		ASSERT_EQ(sizeHint.lowerBound, input.size());
		ASSERT_EQ(sizeHint.upperBound.value(), input.size());
	}
	{ // std::vector
		LifecycleEvents evtLog;
		{
			std::vector<LifecycleDebugger> input;
			input.emplace_back("heapTestString", evtLog);

			std::vector<std::string> outVec = CXXIter::SrcRef(input)
					.filter([](const LifecycleDebugger&){ return true; })
					.map([](LifecycleDebugger&& o) { return std::move(o.heapTest); })
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
			CustomContainer<LifecycleDebugger> input({ LifecycleDebugger("heapTestString", evtLog) });

			std::vector<std::string> outVec = CXXIter::SrcRef(input)
					.filter([](const LifecycleDebugger&){ return true; })
					.map([](LifecycleDebugger&& o) { return std::move(o.heapTest); })
					.collect<std::vector>();
			ASSERT_EQ(outVec.size(), 1);
			ASSERT_EQ(outVec[0], "heapTestString");
			ASSERT_EQ(input.get(0).heapTest, ""); // we only moved out of the string
			ASSERT_TRUE(input.get(0).alive); // ^
		}

		ASSERT_EQ(evtLog.size(), 4);
		ASSERT_EQ(evtLog[0].event, LifecycleEventType::CTOR); // ctor in initializer list
		ASSERT_EQ(evtLog[1].event, LifecycleEventType::CPYCTOR); // initializer list -> CustomContainer
		ASSERT_EQ(evtLog[2].event, LifecycleEventType::DTOR); // dtor in initializer list
		ASSERT_EQ(evtLog[0].ptr, evtLog[2].ptr);

		ASSERT_EQ(evtLog[3].event, LifecycleEventType::DTOR); // dtor in CustomContainer
		ASSERT_EQ(evtLog[1].ptr, evtLog[3].ptr);
	}
	{
		std::vector<int> input = {1, 2, 3};
		std::vector<int>& inputRef = input;
		auto iter = CXXIter::from(inputRef);
		static_assert(std::is_same_v<decltype(iter), CXXIter::SrcRef<std::vector<int>>>);
		int output = iter
				.map([](int item) { return item * 2; })
				.mean().value();
		ASSERT_EQ(output, 4);
	}
}

TEST(CXXIter, empty) {
	CXXIter::IterValue<std::string> output = CXXIter::empty<std::string>().next();
	ASSERT_FALSE(output.has_value());
}

TEST(CXXIter, fromFn) {
	size_t generatorState = 0;
	std::function<std::optional<size_t>()> generatorFn = [generatorState]() mutable {
		return (generatorState++);
	};
	std::vector<size_t> output = CXXIter::fromFn(generatorFn)
			.take(100)
			.collect<std::vector>();
	ASSERT_EQ(output.size(), 100);
	for(size_t i = 0; i < 100; ++i) { ASSERT_EQ(output[i], i); }
}

#ifdef CXXITER_HAS_COROUTINE
TEST(CXXIter, generate) {
	{
		std::vector<std::string> output = CXXIter::generate(
			[]() -> CXXIter::Generator<std::string> {
				for(size_t i = 0; i < 1000; ++i) {
					co_yield std::to_string(i);
				}
			}
		).collect<std::vector>();
		ASSERT_EQ(output.size(), 1000);
		for(size_t i = 0; i < 1000; ++i) { ASSERT_EQ(output[i], std::to_string(i)); }
	}
	{
		std::vector<std::string> output = CXXIter::generate(
			[]() -> CXXIter::Generator<std::string> {
				co_yield std::to_string(0);
			}
		).collect<std::vector>();
		ASSERT_EQ(output.size(), 1);
		ASSERT_THAT(output, ElementsAre("0"));
	}
}
#endif

TEST(CXXIter, repeat) {
	{ // sizeHint
		{
			auto sizeHint = CXXIter::repeat(5.0f, 3).sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 3);
			ASSERT_EQ(sizeHint.upperBound.value(), 3);
		}
		{
			auto sizeHint = CXXIter::repeat(5.0f).sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, CXXIter::SizeHint::INFINITE);
			ASSERT_FALSE(sizeHint.upperBound.has_value());
		}
	}
	{
		std::vector<int> item = {1, 3, 3, 7};
		std::vector<int> output = CXXIter::repeat(item, 3)
				.flatMap()
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 3 * 4);
		ASSERT_THAT(output, ElementsAre(1, 3, 3, 7, 1, 3, 3, 7, 1, 3, 3, 7));
	}
}

TEST(CXXIter, range) {
	{ // sizeHint
		{
			auto sizeHint = CXXIter::range(0, 7, 2).sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 4);
			ASSERT_EQ(sizeHint.upperBound.value(), 4);
		}
		{
			auto sizeHint = CXXIter::range(0.0f, 1.1f, 0.25f).sizeHint();
			ASSERT_EQ(sizeHint.lowerBound, 5);
			ASSERT_EQ(sizeHint.upperBound.value(), 5);
		}
	}
	{
		std::vector<int> output = CXXIter::range(0, 7, 2)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 4);
		ASSERT_THAT(output, ElementsAre(0, 2, 4, 6));
	}
	{
		std::vector<int> output = CXXIter::range(1, 7, 2)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 4);
		ASSERT_THAT(output, ElementsAre(1, 3, 5, 7));
	}
	{
		std::vector<float> output = CXXIter::range(0.0f, 1.1f, 0.25f)
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 5);
		ASSERT_THAT(output, ElementsAre(0.0f, 0.25f, 0.5f, 0.75f, 1.0f));
	}
}

TEST(CXXIter, next) {
	std::vector<std::string> input = {"42", "1337"};
	auto iter = CXXIter::from(input);
	auto output0 = iter.next();
	ASSERT_TRUE(output0.has_value());
	ASSERT_EQ(output0.value(), "42");
	auto output1 = iter.next();
	ASSERT_TRUE(output1.has_value());
	ASSERT_EQ(output1.value(), "1337");
	auto output2 = iter.next();
	ASSERT_FALSE(output2.has_value());
}
