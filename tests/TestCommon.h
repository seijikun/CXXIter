#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <CXXIter/CXXIter.h>

using ::testing::ElementsAre;
using ::testing::Pair;


// ################################################################################################
// TEST STRUCTURES
// ################################################################################################

using TestPair = std::pair<std::string, int>;
// define hash for TestPair
namespace std {
	template<>
	struct hash<TestPair> {
		inline std::size_t operator()(const TestPair& v) const {
			std::hash<std::string> firstHash;
			std::hash<int> secondHash;
			return firstHash(v.first) ^ secondHash(v.second);
		}
	};
}

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
// CUSTOM CONTAINER
// ################################################################################################

template<typename TItem> struct CustomContainer {
	using CustomContainerItem = TItem;
	std::vector<CustomContainerItem> input;
	size_t cnt = 0;

	CustomContainer() {}
	CustomContainer(std::initializer_list<TItem>&& initialItems) : input(std::move(initialItems)) {}

	size_t size() const { return input.size(); }
	CustomContainerItem& get(size_t idx) { return input.at(idx); }
	const CustomContainerItem& get(size_t idx) const { return input.at(idx); }
	void append(CustomContainerItem&& item) {
		input.push_back(std::forward<CustomContainerItem>(item));
	}
};

namespace CXXIter {
	// SourceTrait implementation for the CustomContainer
	template<typename TItem> struct SourceTrait<CustomContainer<TItem>> {
		using Item = typename CustomContainer<TItem>::CustomContainerItem;
		using IteratorState = size_t;
		using ConstIteratorState = size_t;

		static inline IteratorState initIterator(CustomContainer<TItem>&) { return 0; }
		static inline ConstIteratorState initIterator(const CustomContainer<TItem>&) { return 0; }

		static inline bool hasNext(CustomContainer<TItem>& container, IteratorState& iter) { return iter < container.size(); }
		static inline bool hasNext(const CustomContainer<TItem>& container, ConstIteratorState& iter) { return iter < container.size(); }

		static inline Item& next(CustomContainer<TItem>& container, IteratorState& iter) { return container.get(iter++); }
		static inline const Item& next(const CustomContainer<TItem>& container, ConstIteratorState& iter) { return container.get(iter++); }
	};

	// Collector implementation for the CustomContainer
	template<typename TChainInput>
	struct Collector<TChainInput, CustomContainer> {
		template<typename Item, typename ItemOwned>
		static CustomContainer<ItemOwned> collect(TChainInput& input) {
			CustomContainer<ItemOwned> container;
			input.forEach([&container](Item&& item) { container.append(std::forward<ItemOwned>(item)); });
			return container;
		}
	};
};