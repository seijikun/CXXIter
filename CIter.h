#pragma once

#include <utility>
#include <optional>
#include <functional>
#include <concepts>
#include <string>

/**
 * std container forward declarations, so we don't have to include everything, blowing up compile-time.
 */
namespace std {
	template<typename, typename> class vector;
	template<typename, typename> class list;
	template<typename, typename> class forward_list;
	template<typename, typename> class deque;

	template<typename, typename, typename> struct set;
	template<typename, typename, typename> struct multiset;
	template<typename, typename, typename, typename> struct unordered_set;
	template<typename, typename, typename, typename> struct unordered_multiset;

	template<typename, typename, typename, typename> struct map;
	template<typename, typename, typename, typename> struct multimap;
	template<typename, typename, typename, typename, typename> struct unordered_map;
	template<typename, typename, typename, typename, typename> struct unordered_multimap;
}

namespace CIter {

// ################################################################################################
// FORWARD DECLARATIONS & CONCEPTS
// ################################################################################################

struct LINQIteratorEnd {};


template<typename T>
struct IteratorTrait {};

template<typename T>
concept CIterIterator = (std::is_same_v<typename IteratorTrait<T>::Self, T>);


template<CIterIterator TSelf>
class IterApi;


// ################################################################################################
// SOURCE (MOVE / CONSUME)
// ################################################################################################
template<typename TContainer>
class SrcMov : public IterApi<SrcMov<TContainer>> {
	friend struct IteratorTrait<SrcMov<TContainer>>;
private:
	TContainer container;
	typename TContainer::iterator ptr;
public:
	SrcMov(TContainer&& container) : container(std::move(container)), ptr(this->container.begin()) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TContainer>
struct IteratorTrait<SrcMov<TContainer>> {
	using Self = SrcMov<TContainer>;
	using Item = typename TContainer::value_type;

	static inline Item next(Self& self) {
		if(self.ptr == self.container.end()) { throw LINQIteratorEnd(); }
		return std::move(*self.ptr++);
	}
};



// ################################################################################################
// SOURCE (MUTABLE REFERENCE)
// ################################################################################################
template<typename TContainer>
class SrcRef : public IterApi<SrcRef<TContainer>> {
	friend struct IteratorTrait<SrcRef<TContainer>>;
private:
	TContainer& container;
	typename TContainer::iterator ptr;
public:
	SrcRef(TContainer& container) : container(container), ptr(this->container.begin()) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TContainer>
struct IteratorTrait<SrcRef<TContainer>> {
	using Self = SrcRef<TContainer>;
	using Item = typename TContainer::reference;

	static inline Item next(Self& self) {
		if(self.ptr == self.container.end()) { throw LINQIteratorEnd(); }
		return *self.ptr++;
	}
};



// ################################################################################################
// SOURCE (CONST REFERENCE)
// ################################################################################################
template<typename TContainer>
class SrcCRef : public IterApi<SrcCRef<TContainer>> {
	friend struct IteratorTrait<SrcCRef<TContainer>>;
private:
	const TContainer& container;
	typename TContainer::const_iterator ptr;
public:
	SrcCRef(const TContainer& container) : container(container), ptr(this->container.begin()) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TContainer>
struct IteratorTrait<SrcCRef<TContainer>> {
	using Self = SrcCRef<TContainer>;
	using Item = typename TContainer::const_reference;

	static inline Item next(Self& self) {
		if(self.ptr == self.container.end()) { throw LINQIteratorEnd(); }
		return *self.ptr++;
	}
};



// ################################################################################################
// CASTER
// ################################################################################################
template<typename TChainInput, typename TItem>
requires std::is_object_v<TItem>
class Caster : public IterApi<Caster<TChainInput, TItem>> {
	friend struct IteratorTrait<Caster<TChainInput, TItem>>;
private:
	TChainInput input;
public:
	Caster(TChainInput&& input) : input(std::move(input)) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TChainInput, typename TItem>
requires std::is_object_v<TItem>
struct IteratorTrait<Caster<TChainInput, TItem>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// LINQ Interface
	using Self = Caster<TChainInput, TItem>;
	using Item = TItem;

	static inline Item next(Self& self) {
		return static_cast<Item>( ChainInputIterator::next(self.input) );
	}
};



// ################################################################################################
// FILTER
// ################################################################################################
template<typename TChainInput>
class Filter : public IterApi<Filter<TChainInput>> {
	friend struct IteratorTrait<Filter<TChainInput>>;
private:
	using InputItem = typename IteratorTrait<TChainInput>::Item;
	using FilterFn = std::function<bool(const InputItem& item)>;

	TChainInput input;
	FilterFn filterFn;
public:
	Filter(TChainInput&& input, FilterFn filterFn) : input(std::move(input)), filterFn(filterFn) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TChainInput>
struct IteratorTrait<Filter<TChainInput>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// LINQ Interface
	using Self = Filter<TChainInput>;
	using Item = typename ChainInputIterator::Item;

	static inline Item next(Self& self) {
		while(true) {
			Item value = ChainInputIterator::next(self.input);
			if(self.filterFn(value)) { return value; }
		}
	}
};



// ################################################################################################
// INPLACE MODIFIER
// ################################################################################################
template<typename TChainInput>
requires std::is_object_v<typename IteratorTrait<TChainInput>::Item> || (!std::is_const_v<typename IteratorTrait<TChainInput>::Item>)
class InplaceModifier : public IterApi<InplaceModifier<TChainInput>> {
	friend struct IteratorTrait<InplaceModifier<TChainInput>>;
private:
	using InputItem = typename IteratorTrait<TChainInput>::Item;
	using ModifierFn = std::function<void(InputItem& item)>;

	TChainInput input;
	ModifierFn modifierFn;
public:
	InplaceModifier(TChainInput&& input, ModifierFn modifierFn) : input(std::move(input)), modifierFn(modifierFn) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TChainInput>
struct IteratorTrait<InplaceModifier<TChainInput>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// LINQ Interface
	using Self = InplaceModifier<TChainInput>;
	using Item = typename ChainInputIterator::Item;

	static inline Item next(Self& self) {
		Item value = ChainInputIterator::next(self.input);
		self.modifierFn(value);
		return value;
	}
};



// ################################################################################################
// MAP
// ################################################################################################
template<typename TChainInput, typename TItem>
class Map : public IterApi<Map<TChainInput, TItem>> {
	friend struct IteratorTrait<Map<TChainInput, TItem>>;
private:
	using InputItem = typename IteratorTrait<TChainInput>::Item;
	using MapFn = std::function<TItem(InputItem&& item)>;

	TChainInput input;
	MapFn mapFn;
public:
	Map(TChainInput&& input, MapFn mapFn) : input(std::move(input)), mapFn(mapFn) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TChainInput, typename TItem>
struct IteratorTrait<Map<TChainInput, TItem>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// LINQ Interface
	using Self = Map<TChainInput, TItem>;
	using Item = TItem;

	static inline Item next(Self& self) {
		return self.mapFn(std::forward<typename ChainInputIterator::Item>( ChainInputIterator::next(self.input) ));
	}
};



// ################################################################################################
// FLATMAP
// ################################################################################################
template<typename TChainInput, typename TItemContainer>
requires (!std::is_reference_v<TItemContainer>)
class FlatMap : public IterApi<FlatMap<TChainInput, TItemContainer>> {
	friend struct IteratorTrait<FlatMap<TChainInput, TItemContainer>>;
private:
	using InputItem = typename IteratorTrait<TChainInput>::Item;
	using FlatMapFn = std::function<TItemContainer(InputItem&& item)>;

	TChainInput input;
	std::optional<SrcMov<TItemContainer>> current;
	FlatMapFn mapFn;
public:
	FlatMap(TChainInput&& input, FlatMapFn mapFn) : input(std::move(input)), mapFn(mapFn) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TChainInput, typename TItemContainer>
struct IteratorTrait<FlatMap<TChainInput, TItemContainer>> {
	using NestedChainIterator = IteratorTrait<SrcMov<TItemContainer>>;
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// LINQ Interface
	using Self = FlatMap<TChainInput, TItemContainer>;
	using Item = typename TItemContainer::value_type;

	static inline Item next(Self& self) {
		while(true) {
			if(!self.current) { // pull new collection from the outer iterator
				self.current = SrcMov(std::move(
					self.mapFn(std::forward<typename ChainInputIterator::Item>( ChainInputIterator::next(self.input) ))
				));
			}
			try { // if the outer iterator yielded a collection, take from it until we reach the end
				return NestedChainIterator::next(*self.current);
			} catch (LINQIteratorEnd) { // inner collection ended, unset current cache
				self.current.reset();
			}
		}
	}
};



// ################################################################################################
// FILTERMAP
// ################################################################################################
template<typename TChainInput, typename TItem>
class FilterMap : public IterApi<FilterMap<TChainInput, TItem>> {
	friend struct IteratorTrait<FilterMap<TChainInput, TItem>>;
private:
	using InputItem = typename IteratorTrait<TChainInput>::Item;
	using FilterMapFn = std::function<std::optional<TItem>(InputItem&& item)>;

	TChainInput input;
	FilterMapFn filterMapFn;
public:
	FilterMap(TChainInput&& input, FilterMapFn filterMapFn) : input(std::move(input)), filterMapFn(filterMapFn) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TChainInput, typename TItem>
struct IteratorTrait<FilterMap<TChainInput, TItem>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// LINQ Interface
	using Self = FilterMap<TChainInput, TItem>;
	using Item = TItem;

	static inline Item next(Self& self) {
		while(true) {
			std::optional<Item> value(self.filterMapFn(std::move( ChainInputIterator::next(self.input) )));
			if(!value) { continue; }
			return *value;
		}
	}
};




// ################################################################################################
// ZIPPER
// ################################################################################################
template<typename TChainInput1, typename TChainInput2>
class Zipper : public IterApi<Zipper<TChainInput1, TChainInput2>> {
	friend struct IteratorTrait<Zipper<TChainInput1, TChainInput2>>;
private:
	TChainInput1 input1;
	TChainInput2 input2;
public:
	Zipper(TChainInput1&& input1, TChainInput2 input2) : input1(std::move(input1)), input2(std::move(input2)) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TChainInput1, typename TChainInput2>
struct IteratorTrait<Zipper<TChainInput1, TChainInput2>> {
	using ChainInputIterator1 = IteratorTrait<TChainInput1>;
	using ChainInputIterator2 = IteratorTrait<TChainInput2>;
	using InputItem1 = typename IteratorTrait<TChainInput1>::Item;
	using InputItem2 = typename IteratorTrait<TChainInput2>::Item;
	// LINQ Interface
	using Self = Zipper<TChainInput1, TChainInput2>;
	using Item = std::pair<InputItem1, InputItem2>;

	static inline Item next(Self& self) {
		return {
			ChainInputIterator1::next(self.input1),
			ChainInputIterator2::next(self.input2)
		};
	}
};



// ################################################################################################
// COLLECTOR
// ################################################################################################
namespace collectors {
	template<typename TChainInput, template<typename...> typename TContainer, typename... TContainerArgs>
	struct BackInsertCollector {
		template<typename Item, typename ItemOwned>
		static TContainer<ItemOwned, TContainerArgs...> collect(TChainInput& input) {
			TContainer<ItemOwned, TContainerArgs...> container;
			auto inserter = std::back_inserter(container);
			input.forEach([&inserter](Item&& item) { *inserter = std::forward<ItemOwned>(item); });
			return container;
		}
	};

	template<typename TChainInput, template<typename...> typename TContainer, typename... TContainerArgs>
	struct InsertCollector {
		template<typename Item, typename ItemOwned>
		static TContainer<ItemOwned, TContainerArgs...> collect(TChainInput& input) {
			TContainer<ItemOwned, TContainerArgs...> container;
			auto inserter = std::inserter(container, container.end());
			input.forEach([&inserter](Item&& item) { *inserter = std::forward<ItemOwned>(item); });
			return container;
		}
	};

	template<typename TChainInput, template<typename...> typename TContainer, typename... TContainerArgs>
	struct AssocCollector {
		template<typename Item, typename ItemOwned>
		static auto collect(TChainInput& input) {
			TContainer<typename std::remove_const<typename ItemOwned::first_type>::type, typename ItemOwned::second_type, TContainerArgs...> container;
			input.forEach([&container](Item&& item) { container[item.first] = item.second; });
			return container;
		}
	};
}

template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
struct Collector {};

#define DEFINE_COLLECTOR_IMPL(_CONTAINER_, _COLLECTOR_) \
	template<typename TChainInput, typename... TContainerArgs> \
	struct Collector<TChainInput, _CONTAINER_, TContainerArgs...> : public collectors::_COLLECTOR_<TChainInput, _CONTAINER_, TContainerArgs...> {}

DEFINE_COLLECTOR_IMPL(std::vector, BackInsertCollector);
DEFINE_COLLECTOR_IMPL(std::list, BackInsertCollector);
DEFINE_COLLECTOR_IMPL(std::deque, BackInsertCollector);
DEFINE_COLLECTOR_IMPL(std::basic_string, BackInsertCollector);

DEFINE_COLLECTOR_IMPL(std::set, BackInsertCollector);
DEFINE_COLLECTOR_IMPL(std::multiset, BackInsertCollector);
DEFINE_COLLECTOR_IMPL(std::unordered_set, InsertCollector);
DEFINE_COLLECTOR_IMPL(std::unordered_multiset, InsertCollector);

DEFINE_COLLECTOR_IMPL(std::map, AssocCollector);
DEFINE_COLLECTOR_IMPL(std::multimap, AssocCollector);
DEFINE_COLLECTOR_IMPL(std::unordered_map, AssocCollector);
DEFINE_COLLECTOR_IMPL(std::unordered_multimap, AssocCollector);



// ################################################################################################
// SURFACE-API
// ################################################################################################

template<CIterIterator TSelf>
class IterApi {
	TSelf* self() { return dynamic_cast<TSelf*>(this); }

public:
	using Iterator = IteratorTrait<TSelf>;
	using Item = typename Iterator::Item;
	using ItemOwned = typename std::remove_reference<Item>::type;
	static constexpr bool IS_REFERENCE = std::is_reference<Item>::value;

	virtual ~IterApi() {}

	// ###################
	// CONSUMERS
	// ###################
	template<typename TUseFn>
	void forEach(TUseFn useFn) {
		try {
			while(true) {
				useFn(std::forward<Item>( Iterator::next(*self()) ));
			}
		} catch (LINQIteratorEnd) {}
	}

	template<template <typename...> typename TTargetContainer, typename... TTargetContainerArgs>
	auto collect() {
		return Collector<TSelf, TTargetContainer, TTargetContainerArgs...>::template collect<Item, ItemOwned>(*self());
	}

	template<typename TResult, std::invocable<TResult&, Item&&> FoldFn>
	TResult fold(TResult startValue, FoldFn foldFn) {
		TResult result = startValue;
		forEach([&result, &foldFn](Item&& item) { foldFn(result, std::forward<Item>(item)); });
		return result;
	}

	size_t count() {
		return fold((size_t)0, [](size_t& cnt, auto&&) { cnt += 1; });
	}

	template<typename TResult = ItemOwned>
	requires requires(TResult res, Item item) { { res += item }; }
	TResult sum(TResult startValue = TResult()) {
		return fold(startValue, [](TResult& res, Item&& item) { res += item; });
	}

	template<typename TResult = ItemOwned>
	requires requires(TResult res, ItemOwned item) {
		{ item < res };
		{ res = item };
	}
	std::optional<TResult> min() {
		try {
			TResult result = Iterator::next(*self());
			forEach([&result](Item&& item) {
				if(item < result) { result = item; }
			});
			return result;
		} catch (LINQIteratorEnd) {
			return {};
		}
	}

	template<typename TResult = ItemOwned>
	requires requires(TResult res, ItemOwned item) {
		{ item > res };
		{ res = item };
	}
	std::optional<TResult> max() {
		try {
			TResult result = Iterator::next(*self());
			forEach([&result](Item&& item) {
				if(item > result) { result = item; }
			});
			return result;
		} catch (LINQIteratorEnd) {
			return {};
		}
	}

	template<typename _unused = ItemOwned>
	std::optional<ItemOwned> last() {
		std::optional<ItemOwned> tmp;
		forEach([&tmp](Item&& item) { tmp = item; });
		return tmp;
	}


	// ###################
	// CHAINERS
	// ###################
	template<typename TItemOutput>
	Caster<TSelf, TItemOutput> cast() {
		return Caster<TSelf, TItemOutput>(std::move(*self()));
	}

	Map<TSelf, ItemOwned> copied() {
		return Map<TSelf, ItemOwned>(std::move(*self()), [](Item&& item) -> ItemOwned {
			ItemOwned copy = item;
			return copy;
		});
	}

	template<std::invocable<const Item&> TFilterFn>
	Filter<TSelf> filter(TFilterFn filterFn) {
		return Filter<TSelf>(std::move(*self()), filterFn);
	}

	template<std::invocable<Item&&> TMapFn>
	auto map(TMapFn mapFn) {
		return Map<TSelf, decltype(mapFn( std::declval<Item&&>() ))>(std::move(*self()), mapFn);
	}

	template<std::invocable<Item&&> TFlatMapFn = std::function<Item(Item&&)>>
	auto flatMap(TFlatMapFn mapFn = [](Item&& item) { return item; }) {
		return FlatMap<TSelf, decltype(mapFn( std::declval<Item&&>() ))>(std::move(*self()), mapFn);
	}

	template<std::invocable<Item&> TModifierFn>
	InplaceModifier<TSelf> modify(TModifierFn modifierFn) {
		return InplaceModifier<TSelf>(std::move(*self()), modifierFn);
	}

	template<std::invocable<Item&&> TFilterMapFn, typename = typename std::enable_if<!IS_REFERENCE, TFilterMapFn>::type>
	auto filterMap(TFilterMapFn filterMapFn) {
		using ResultType = typename decltype ( filterMapFn( std::declval<Item&&>() ) )::value_type;
		return FilterMap<TSelf, ResultType>(std::move(*self()), filterMapFn);
	}

	Filter<TSelf> skip(size_t cnt) {
		return filter([cnt](const Item&) mutable {
			if(cnt != 0) { cnt -= 1; return false; }
			return true;
		});
	}

	template<std::invocable<const Item&> TSkipPredicate>
	Filter<TSelf> skipWhile(TSkipPredicate skipPredicate) {
		bool skipDone = false;
		return filter([skipPredicate, skipDone](const Item& value) mutable {
			if(skipDone) { return true; }
			skipDone = !skipPredicate(value);
			return skipDone;
		});
	}

	Filter<TSelf> take(size_t cnt) {
		return filter([cnt](const Item&) mutable {
			if(cnt != 0) { cnt -= 1; return true; }
			return false;
		});
	}

	template<std::invocable<const Item&> TTakePredicate>
	Filter<TSelf> takeWhile(TTakePredicate takePredicate) {
		bool takeDone = false;
		return filter([takePredicate, takeDone](const Item& value) mutable {
			if(takeDone) { return false; }
			takeDone = !takePredicate(value);
			return !takeDone;
		});
	}

	template<typename TOtherIterator>
	requires (!std::is_reference_v<typename IteratorTrait<TOtherIterator>::Item> && !IS_REFERENCE)
	Zipper<TSelf, TOtherIterator> zip(TOtherIterator&& otherIterator) {
		return Zipper<TSelf, TOtherIterator>(std::move(*self()), std::forward<TOtherIterator>(otherIterator));
	}
};



// ################################################################################################
// CONVENIENT ENTRY POINTS
// ################################################################################################

template<typename TContainer> SrcMov<TContainer> from(TContainer&& container) {
	return SrcMov<TContainer>(std::forward<TContainer>(container));
}

template<typename TContainer> SrcRef<TContainer> from(TContainer& container) {
	return SrcRef<TContainer>(container);
}

template<typename TContainer> SrcCRef<TContainer> from(const TContainer& container) {
	return SrcMov<TContainer>(container);
}

}
