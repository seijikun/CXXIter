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
}

TEST(CXXIter, srcRef) { // mutable references (move out of heapTest)
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
}
