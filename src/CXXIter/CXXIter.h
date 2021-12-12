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

namespace CXXIter {

// ################################################################################################
// ITERATOR OPTIONAL (supports references)
// ################################################################################################
/** @private */
template<typename TValue>
class IterValue {};

/** @private */
template<typename TValue>
requires (!std::is_reference_v<TValue>)
class IterValue<TValue> {
	std::optional<TValue> inner;
public:
	IterValue() {}
	IterValue(const TValue& value) : inner(value) {}
	IterValue(TValue&& value) : inner(std::forward<TValue>(value)) {}
	IterValue<TValue>& operator=(IterValue<TValue>&& o) = default;
	IterValue(IterValue<TValue>&& o) = default;

	const TValue& value() const { return inner.value(); }
	TValue& value() { return inner.value(); }
	const TValue& value(TValue&& def) const { return inner.value_or(def); }
	TValue& value(TValue&& def) { return inner.value_or(def); }

	bool hasValue() const { return inner.has_value(); }

	IterValue<TValue>& operator=(TValue&& o) {
		inner = std::forward<TValue>(o);
		return *this;
	}

	template<typename TOutValue>
	IterValue<TOutValue> map(std::function<TOutValue(TValue&& value)> mapFn) {
		if(!hasValue()) { return {}; }
		return mapFn(std::forward<TValue>(value()));
	}
};

/** @private */
template<typename TValue>
requires std::is_reference_v<TValue>
class IterValue<TValue> {
	using TValueDeref = typename std::remove_reference<TValue>::type;
	std::optional<std::reference_wrapper<TValueDeref>> inner;
public:
	IterValue() {}
	IterValue(TValue&& value) : inner(std::forward<TValue>(value)) {}
	IterValue<TValue>& operator=(IterValue<TValue>&& o) = default;
	IterValue(IterValue<TValue>&& o) = default;

	const TValue& value() const { return inner.value(); }
	TValue& value() { return inner.value(); }
	const TValue& value(TValue&& def) const { return inner.value_or(def); }
	TValue& value(TValue&& def) { return inner.value_or(def); }

	bool hasValue() const { return inner.has_value(); }

	IterValue<TValue>& operator=(TValue&& o) {
		inner = std::forward<TValue>(o);
		return *this;
	}

	template<typename TOutValue>
	IterValue<TOutValue> map(std::function<TOutValue(TValue&& value)> mapFn) {
		if(!hasValue()) { return {}; }
		return mapFn(std::forward<TValue>(value()));
	}
};




// ################################################################################################
// FORWARD DECLARATIONS & CONCEPTS
// ################################################################################################

/** @private */
template<typename T>
struct IteratorTrait {};

/** @private */
template<typename T>
concept CXXIterIterator = (std::is_same_v<typename IteratorTrait<T>::Self, T>);

template<CXXIterIterator TSelf>
class IterApi;

template<typename TContainer> struct SourceTrait {
	using Item = typename TContainer::value_type;
	using IteratorState = typename TContainer::iterator;
	using ConstIteratorState = typename TContainer::const_iterator;

	static inline IteratorState initIterator(TContainer& container) { return container.begin(); }
	static inline ConstIteratorState initIterator(const TContainer& container) { return container.begin(); }

	static inline bool hasNext(TContainer& container, IteratorState& iter) { return (iter != container.end()); }
	static inline bool hasNext(const TContainer& container, ConstIteratorState& iter) { return (iter != container.end()); }

	static inline Item& next(TContainer&, IteratorState& iter) { return (*iter++); }
	static inline const Item& next(const TContainer&, ConstIteratorState& iter) { return (*iter++); }
};

// ################################################################################################
// SOURCE (MOVE / CONSUME)
// ################################################################################################
template<typename TContainer>
class SrcMov : public IterApi<SrcMov<TContainer>> {
	friend struct IteratorTrait<SrcMov<TContainer>>;
	using Src = SourceTrait<TContainer>;
private:
	TContainer container;
	typename Src::IteratorState iter;
public:
	SrcMov(TContainer&& container) : container(std::move(container)), iter(Src::initIterator(this->container)) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TContainer>
struct IteratorTrait<SrcMov<TContainer>> {
	using Src = SourceTrait<TContainer>;
	// CXXIter Interface
	using Self = SrcMov<TContainer>;
	using Item = typename Src::Item;

	static inline IterValue<Item> next(Self& self) {
		if(!Src::hasNext(self.container, self.iter)) { return {}; }
		return std::move(Src::next(self.container, self.iter));
	}
};



// ################################################################################################
// SOURCE (MUTABLE REFERENCE)
// ################################################################################################
template<typename TContainer>
class SrcRef : public IterApi<SrcRef<TContainer>> {
	friend struct IteratorTrait<SrcRef<TContainer>>;
	using Src = SourceTrait<TContainer>;
private:
	TContainer& container;
	typename Src::IteratorState iter;
public:
	SrcRef(TContainer& container) : container(container), iter(Src::initIterator(this->container)) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TContainer>
struct IteratorTrait<SrcRef<TContainer>> {
	using Src = SourceTrait<TContainer>;
	// CXXIter Interface
	using Self = SrcRef<TContainer>;
	using Item = typename Src::Item&;

	static inline IterValue<Item> next(Self& self) {
		if(!Src::hasNext(self.container, self.iter)) { return {}; }
		return Src::next(self.container, self.iter);
	}
};



// ################################################################################################
// SOURCE (CONST REFERENCE)
// ################################################################################################
template<typename TContainer>
class SrcCRef : public IterApi<SrcCRef<TContainer>> {
	friend struct IteratorTrait<SrcCRef<TContainer>>;
	using Src = SourceTrait<TContainer>;
private:
	const TContainer& container;
	typename Src::ConstIteratorState iter;
public:
	SrcCRef(const TContainer& container) : container(container), iter(Src::initIterator(this->container)) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TContainer>
struct IteratorTrait<SrcCRef<TContainer>> {
	using Src = SourceTrait<TContainer>;
	// CXXIter Interface
	using Self = SrcCRef<TContainer>;
	using Item = const typename Src::Item&;

	static inline IterValue<Item> next(Self& self) {
		if(!Src::hasNext(self.container, self.iter)) { return {}; }
		return Src::next(self.container, self.iter);
	}
};



// ################################################################################################
// CASTER
// ################################################################################################
/** @private */
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
/** @private */
template<typename TChainInput, typename TItem>
requires std::is_object_v<TItem>
struct IteratorTrait<Caster<TChainInput, TItem>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// CXXIter Interface
	using Self = Caster<TChainInput, TItem>;
	using Item = TItem;

	static inline IterValue<Item> next(Self& self) {
		auto item = ChainInputIterator::next(self.input);
		return item.template map<Item>([](auto&& item) { return static_cast<Item>(item); });
	}
};



// ################################################################################################
// FILTER
// ################################################################################################
/** @private */
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
/** @private */
template<typename TChainInput>
struct IteratorTrait<Filter<TChainInput>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// CXXIter Interface
	using Self = Filter<TChainInput>;
	using Item = typename ChainInputIterator::Item;

	static inline IterValue<Item> next(Self& self) {
		while(true) {
			auto item = ChainInputIterator::next(self.input);
			if(!item.hasValue()) { return {}; }
			if(self.filterFn(item.value())) { return item; }
		}
	}
};



// ################################################################################################
// INPLACE MODIFIER
// ################################################################################################
/** @private */
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
/** @private */
template<typename TChainInput>
struct IteratorTrait<InplaceModifier<TChainInput>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// CXXIter Interface
	using Self = InplaceModifier<TChainInput>;
	using Item = typename ChainInputIterator::Item;

	static inline IterValue<Item> next(Self& self) {
		auto item = ChainInputIterator::next(self.input);
		if(!item.hasValue()) { return {}; }
		self.modifierFn(item.value());
		return item;
	}
};



// ################################################################################################
// MAP
// ################################################################################################
/** @private */
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
/** @private */
template<typename TChainInput, typename TItem>
struct IteratorTrait<Map<TChainInput, TItem>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using InputItem = typename ChainInputIterator::Item;
	// CXXIter Interface
	using Self = Map<TChainInput, TItem>;
	using Item = TItem;

	static inline IterValue<Item> next(Self& self) {
		auto item = ChainInputIterator::next(self.input);
		return item.template map<Item>(self.mapFn);
	}
};



// ################################################################################################
// FLATMAP
// ################################################################################################
/** @private */
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
/** @private */
template<typename TChainInput, typename TItemContainer>
struct IteratorTrait<FlatMap<TChainInput, TItemContainer>> {
	using NestedChainIterator = IteratorTrait<SrcMov<TItemContainer>>;
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using InputItem = typename ChainInputIterator::Item;
	// CXXIter Interface
	using Self = FlatMap<TChainInput, TItemContainer>;
	using Item = typename TItemContainer::value_type;

	static inline IterValue<Item> next(Self& self) {
		while(true) {
			if(!self.current) { // pull new collection from the outer iterator
				auto item = ChainInputIterator::next(self.input);
				if(!item.hasValue()) { return {}; } // end of iteration
				self.current = SrcMov(std::move(
					self.mapFn(std::forward<InputItem>( item.value() ))
				));
			}

			// if the outer iterator yielded a collection, take from it until we reach the end
			auto item = NestedChainIterator::next(*self.current);
			if(item.hasValue()) { // inner yielded a usable item
				return item.value();
			} else {
				self.current.reset(); // inner collection ended, unset current cache
			}
		}
	}
};



// ################################################################################################
// FILTERMAP
// ################################################################################################
/** @private */
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
/** @private */
template<typename TChainInput, typename TItem>
struct IteratorTrait<FilterMap<TChainInput, TItem>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using InputItem = typename IteratorTrait<TChainInput>::Item;
	// CXXIter Interface
	using Self = FilterMap<TChainInput, TItem>;
	using Item = TItem;

	static inline IterValue<Item> next(Self& self) {
		while(true) {
			auto item = ChainInputIterator::next(self.input);
			if(!item.hasValue()) { return {}; }
			std::optional<Item> value(self.filterMapFn(std::forward<InputItem>( item.value() )));
			if(!value) { continue; }
			return *value;
		}
	}
};




// ################################################################################################
// ZIPPER
// ################################################################################################
/** @private */
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
/** @private */
template<typename TChainInput1, typename TChainInput2>
struct IteratorTrait<Zipper<TChainInput1, TChainInput2>> {
	using ChainInputIterator1 = IteratorTrait<TChainInput1>;
	using ChainInputIterator2 = IteratorTrait<TChainInput2>;
	using InputItem1 = typename IteratorTrait<TChainInput1>::Item;
	using InputItem2 = typename IteratorTrait<TChainInput2>::Item;
	// CXXIter Interface
	using Self = Zipper<TChainInput1, TChainInput2>;
	using Item = std::pair<InputItem1, InputItem2>;

	static inline IterValue<Item> next(Self& self) {
		auto item1 = ChainInputIterator1::next(self.input1);
		auto item2 = ChainInputIterator2::next(self.input2);
		if(!item1.hasValue()) { return {}; }
		if(!item2.hasValue()) { return {}; }
		return std::make_pair(item1.value(), item2.value());
	}
};



// ################################################################################################
// COLLECTOR
// ################################################################################################
/** @private */
namespace collectors {
	/** @private */
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

	/** @private */
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

	/** @private */
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

/** @private */
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

template<CXXIterIterator TSelf>
class IterApi {
public: // Associated types
	using Iterator = IteratorTrait<TSelf>;
	using Item = typename Iterator::Item;
	using ItemOwned = typename std::remove_reference<Item>::type;

private:
	TSelf* self() { return dynamic_cast<TSelf*>(this); }
	static constexpr bool IS_REFERENCE = std::is_lvalue_reference<Item>::value;

public:
	virtual ~IterApi() {}

	// ###################
	// CONSUMERS
	// ###################
	template<typename TUseFn>
	void forEach(TUseFn useFn) {
		while(true) {
			auto item = Iterator::next(*self());
			if(!item.hasValue()) { return; }
			useFn(std::forward<Item>( item.value() ));
		}
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
		auto item = Iterator::next(*self());
		if(!item.hasValue()) { return {}; }
		TResult result = item.value();
		forEach([&result](Item&& item) {
			if(item < result) { result = item; }
		});
		return result;
	}

	template<typename TResult = ItemOwned>
	requires requires(TResult res, ItemOwned item) {
		{ item > res };
		{ res = item };
	}
	std::optional<TResult> max() {
		auto item = Iterator::next(*self());
		if(!item.hasValue()) { return {}; }
		TResult result = item.value();
		forEach([&result](Item&& item) {
			if(item > result) { result = item; }
		});
		return result;
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

	/**
	 * @brief Creates an iterator that works like map(), but flattens nested containers.
	 * @details This works by pulling elements from this iterator, passing them to the given
	 * @p mapFn, and then taking the returned values to turn them into iterators themselves,
	 * to merge them into the stream of the resulting iterator.
	 * This only resolves one layer of nesting, and values returned by @p mapFn have to
	 * be supported by CXXIter (by a fitting @c SourceTrait implementation).
	 * @param mapFn Function that returns a nesting container, that should be merged into the returned
	 * iterator's stream.
	 * @return New iterator that pulls values from this iterator, maps them to a nested container, which
	 * is then flattened into the new iterator's stream of elements.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::pair<std::string, std::vector<int>>> input = {{"first pair", {1337, 42}}, {"second pair", {6, 123, 7888}}};
	 * 	std::vector<int> output = CXXIter::from(std::move(input))
	 * 		.flatMap([](auto&& item) { return std::get<1>(item); }) // flatten the std::vector<int> from the pair
	 * 		.collect<std::vector>(); // collect into vector containing {1337, 42, 6, 123, 7888}
	 * @endcode
	 */
	template<std::invocable<Item&&> TFlatMapFn = std::function<Item(Item&&)>>
	auto flatMap(TFlatMapFn mapFn = [](Item&& item) { return item; }) {
		return FlatMap<TSelf, decltype(mapFn( std::declval<Item&&>() ))>(std::move(*self()), mapFn);
	}

	/**
	 * @brief Allows to inspect and modify each item in-place, that passes through this iterator.
	 * @details This can be used instead of a map() with the same type as input and output.
	 * @param modifierFn Function that is called for each item that passes through this iterator.
	 * @return Iterator that forwards the items of this iterator, after they have been inspected
	 * and potentially modified by the @p modifierFn.
	 *
	 * Usage Example:
	 * @code
	 * 	std::unordered_map<int, std::string> input = { {1337, "1337"}, {42, "42"} };
	 * 	std::unordered_map<int, std::string> output = CXXIter::from(input)
	 * 		.modify([](auto& keyValue) { keyValue.second = "-" + keyValue.second; }) // modify input
	 * 		.collect<std::unordered_map>(); // copy to output
	 * @endcode
	 */
	template<std::invocable<Item&> TModifierFn>
	InplaceModifier<TSelf> modify(TModifierFn modifierFn) {
		return InplaceModifier<TSelf>(std::move(*self()), modifierFn);
	}

	/**
	 * @brief Creates a new iterator that filters and maps items from this iterator.
	 * @param filterMapFn Function that maps the incomming items to an optional mapped value.
	 * If it returns an empty @c std::optional<> the element is filtered. If it returns an
	 * @c std::optional<> with a value, that item is yielded from the resulting iterator.
	 * @return Iterator that yields only the items for which the given @c filterMapFn returned a
	 * mapped value.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
	 * 	std::vector<int> output = CXXIter::from(input)
	 * 		.filterMap([](int item) -> std::optional<int> {
	 * 			if(item % 2 == 0) { return (item + 3); }
	 * 			return {};
	 * 		})
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	template<std::invocable<Item&&> TFilterMapFn>
	auto filterMap(TFilterMapFn filterMapFn) {
		using ResultType = typename decltype ( filterMapFn( std::declval<Item&&>() ) )::value_type;
		return FilterMap<TSelf, ResultType>(std::move(*self()), filterMapFn);
	}

	/**
	 * @brief Creates an iterator that skips the first @p cnt elements from this iterator, before it
	 * yields the remaining items.
	 * @param cnt Amount of elements to skip from this iterator.
	 * @return A new iterator that skips @p cnt elements from this iterator, before yielding the remaining items.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {42, 42, 42, 42, 1337};
	 * 	std::vector<int> output = CXXIter::from(input)
	 * 		.skip(3) // skip first 3 values
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	Filter<TSelf> skip(size_t cnt) {
		return filter([cnt](const Item&) mutable {
			if(cnt != 0) { cnt -= 1; return false; }
			return true;
		});
	}

	/**
	 * @brief Creates an iterator that skips the first elements of this iterator, for which the
	 * given @p skipPredicate returns @c true.
	 * @details The @p skipPredicate is only called until it returned @c false for the first time,
	 * after that its job is done.
	 * @param skipPredicate Predicate that determines the items whether an item at the beginning
	 * of this iterator should be skipped (@c true). Should return @c false for the first item
	 * yielded from the resulted iterator.
	 * @return A new iterator that skips the frist elements from this iterator, until the given
	 * @p skipPredicate returns @c false for the first time. It then yields all remaining items of this
	 * iterator.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {42, 42, 42, 42, 1337, 42};
	 * 	std::vector<int> output = CXXIter::from(input)
	 * 		.skipWhile([](const int value) { return (value == 42); }) // skip leading 42s
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	template<std::invocable<const Item&> TSkipPredicate>
	Filter<TSelf> skipWhile(TSkipPredicate skipPredicate) {
		bool skipDone = false;
		return filter([skipPredicate, skipDone](const Item& value) mutable {
			if(skipDone) { return true; }
			skipDone = !skipPredicate(value);
			return skipDone;
		});
	}

	/**
	 * @brief Creates an iterator that yields at most the first @p cnt elements from this iterator.
	 * @param cnt Amount of elements to yield from this iterator.
	 * @return A new iterator that yields only at most the first @p cnt elements from this iterator.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {42, 57, 64, 128, 1337, 10};
	 * 	std::vector<int> output = CXXIter::from(input)
	 * 		.take(3) // take first 3 values
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	Filter<TSelf> take(size_t cnt) {
		return filter([cnt](const Item&) mutable {
			if(cnt != 0) { cnt -= 1; return true; }
			return false;
		});
	}

	/**
	 * @brief Creates an iterator that yields the first elements of this iterator, for which the
	 * given @p takePredicate returns @c true.
	 * @details The @p takePredicate is only called until it returned @c false for the first time,
	 * after that its job is done.
	 * @param takePredicate Predicate that determines the items returned by the newly constructed
	 * iterator. After this predicate yielded @c false for the first time, the new iterator ends.
	 * @return A new iterator that yields the first couple elements from this iterator, until
	 * the given predicate returns @c false for the first time.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {42, 57, 64, 128, 1337, 10};
	 * 	std::vector<int> output = CXXIter::from(input)
	 * 		.takeWhile([](const int value) { return (value < 1000); }) // take until first item > 1000
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	template<std::invocable<const Item&> TTakePredicate>
	Filter<TSelf> takeWhile(TTakePredicate takePredicate) {
		bool takeDone = false;
		return filter([takePredicate, takeDone](const Item& value) mutable {
			if(takeDone) { return false; }
			takeDone = !takePredicate(value);
			return !takeDone;
		});
	}

	/**
	 * @brief "Zips up" two CXXIter iterators into a single iterator over pairs from both iterators.
	 * @details Constructs new iterator that iterates over @c std::pair<> instances where values from this
	 * iterator are put in the first value, and values from the given @p otherIterator become the second values.
	 * The resulting iterator is only as long as the shorter of both zipped iterators.
	 * @param otherIterator Second iterator zipped against this iterator.
	 * @return New iterator that zips together this iteratore and the given @p otherIterator into a new iterator
	 * over @c std::pair<> for both zipped iterator's values.
	 *
	 * Usage Example:
	 * @code
	 *	std::vector<std::string> input1 = {"1337", "42"};
	 *	std::vector<int> input2 = {1337, 42};
	 *	std::vector<std::pair<std::string, int>> output = CXXIter::from(input1).copied()
	 *		.zip(CXXIter::from(input2).copied())
	 *		.collect<std::vector>();
	 * @endcode
	 */
	template<typename TOtherIterator>
	requires (!std::is_reference_v<typename IteratorTrait<TOtherIterator>::Item> && !IS_REFERENCE)
	Zipper<TSelf, TOtherIterator> zip(TOtherIterator&& otherIterator) {
		return Zipper<TSelf, TOtherIterator>(std::move(*self()), std::forward<TOtherIterator>(otherIterator));
	}
};



// ################################################################################################
// CONVENIENT ENTRY POINTS
// ################################################################################################

/**
 * @brief Construct a CXXIter move source from the given container.
 * @details This constructs a move source, which will move the items from the
 * container into the iterator.
 * @param container Container to construct a CXXIter source from.
 * @return CXXIter move source from the given container.
 */
template<typename TContainer> SrcMov<TContainer> from(TContainer&& container) {
	return SrcMov<TContainer>(std::forward<TContainer>(container));
}

/**
 * @brief Construct a CXXIter mutable-reference source from the given container.
 * @details This constructs a mutable-reference source. This allows the iterator
 * to modify the elements in the array.
 * @param container Container to construct a CXXIter source from.
 * @return CXXIter mutable-reference source from the given container.
 */
template<typename TContainer> SrcRef<TContainer> from(TContainer& container) {
	return SrcRef<TContainer>(container);
}

/**
 * @brief Construct a CXXIter const-reference source from the given container.
 * @details This constructs a const-reference source. This guarantees the
 * given container to stay untouched.
 * @param container Container to construct a CXXIter source from.
 * @return CXXIter const-reference source from the given container.
 */
template<typename TContainer> SrcCRef<TContainer> from(const TContainer& container) {
	return SrcMov<TContainer>(container);
}

}
