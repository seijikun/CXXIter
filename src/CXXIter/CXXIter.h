#pragma once

#include <utility>
#include <optional>
#include <concepts>

#include <unordered_map>
#include <vector>

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

	inline const TValue& value() const { return inner.value(); }
	inline TValue& value() { return inner.value(); }
	inline const TValue& value(TValue&& def) const { return inner.value_or(def); }
	inline TValue& value(TValue&& def) { return inner.value_or(def); }

	bool hasValue() const { return inner.has_value(); }

	IterValue<TValue>& operator=(TValue&& o) {
		inner = std::forward<TValue>(o);
		return *this;
	}

	template<typename TOutValue, std::invocable<TValue&&> TMapFn>
	IterValue<TOutValue> map(TMapFn mapFn) {
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
	IterValue(TValue value) : inner(value) {}
	IterValue<TValue>& operator=(IterValue<TValue>&& o) = default;
	IterValue(IterValue<TValue>&& o) = default;

	inline const TValue& value() const { return inner.value(); }
	inline TValue& value() { return inner.value(); }
	inline const TValue& value(TValue&& def) const { return inner.value_or(def); }
	inline TValue& value(TValue&& def) { return inner.value_or(def); }

	bool hasValue() const { return inner.has_value(); }

	IterValue<TValue>& operator=(TValue&& o) {
		inner = std::forward<TValue>(o);
		return *this;
	}

	template<typename TOutValue, std::invocable<TValueDeref&&> TMapFn>
	IterValue<TOutValue> map(TMapFn mapFn) {
		if(!hasValue()) { return {}; }
		return mapFn(std::forward<TValueDeref>(value()));
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

/** @private */
template<CXXIterIterator TSelf> class IterApi;

/** @private */
template<typename T> concept is_pair = requires(T pair) {
	typename T::first_type;
	typename T::second_type;
	{std::get<typename T::first_type>(pair)} -> std::convertible_to<typename T::first_type>;
	{std::get<typename T::second_type>(pair)} -> std::convertible_to<typename T::second_type>;
};
/** @private */
template<template<typename...> typename TContainer, typename TItem, typename... TContainerArgs>
concept BackInsertableCollection = requires(TContainer<TItem, TContainerArgs...> container, TItem item) {
	typename decltype(container)::value_type;
	container.push_back(item);
};
/** @private */
template<template<typename...> typename TContainer, typename TItem, typename... TContainerArgs>
concept InsertableCollection = requires(TContainer<TItem, TContainerArgs...> container, TItem item) {
	typename decltype(container)::value_type;
	container.insert(item);
};
/** @private */
template<template<typename...> typename TContainer, typename TItemKey, typename TItemValue, typename... TContainerArgs>
concept AssocCollection = requires(TContainer<TItemKey, TItemValue, TContainerArgs...> container, std::pair<TItemKey, TItemValue> item) {
	typename decltype(container)::value_type;
	typename decltype(container)::key_type;
	typename decltype(container)::mapped_type;
	container.insert(item);
};

/**
 * @brief SourceTrait, that all containers supported as CXXIter source has to be specialized for.
 * @details If you want to add support for your own containers, create a template specialization
 * for this SourceTrait, for your container class.
 *
 * This is the default implementation supporting all STL containers.
 */
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

template<typename TContainer>
concept SourceContainer = requires(
		TContainer& container,
		const TContainer& constContainer,
		typename SourceTrait<TContainer>::IteratorState& iterState,
		typename SourceTrait<TContainer>::ConstIteratorState& constIterState
	) {
	typename SourceTrait<TContainer>::Item;
	typename SourceTrait<TContainer>::IteratorState;
	typename SourceTrait<TContainer>::ConstIteratorState;

	{SourceTrait<TContainer>::initIterator(container)} -> std::same_as<typename SourceTrait<TContainer>::IteratorState>;
	{SourceTrait<TContainer>::initIterator(constContainer)} -> std::same_as<typename SourceTrait<TContainer>::ConstIteratorState>;

	{SourceTrait<TContainer>::hasNext(container, iterState)} -> std::same_as<bool>;
	{SourceTrait<TContainer>::hasNext(constContainer, constIterState)} -> std::same_as<bool>;

	{SourceTrait<TContainer>::next(container, iterState)} -> std::same_as<typename SourceTrait<TContainer>::Item&>;
	{SourceTrait<TContainer>::next(constContainer, constIterState)} -> std::same_as<const typename SourceTrait<TContainer>::Item&>;
};

// ################################################################################################
// SOURCE (MOVE / CONSUME)
// ################################################################################################
template<typename TContainer>
requires SourceContainer<typename std::remove_reference<TContainer>::type>
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
template<typename TChainInput, typename TFilterFn>
class Filter : public IterApi<Filter<TChainInput, TFilterFn>> {
	friend struct IteratorTrait<Filter<TChainInput, TFilterFn>>;
private:
	using InputItem = typename TChainInput::Item;

	TChainInput input;
	TFilterFn filterFn;
public:
	Filter(TChainInput&& input, TFilterFn filterFn) : input(std::move(input)), filterFn(filterFn) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TChainInput, typename TFilterFn>
struct IteratorTrait<Filter<TChainInput, TFilterFn>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// CXXIter Interface
	using Self = Filter<TChainInput, TFilterFn>;
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
template<typename TChainInput, typename TModifierFn>
requires std::is_object_v<typename IteratorTrait<TChainInput>::Item> || (!std::is_const_v<typename IteratorTrait<TChainInput>::Item>)
class InplaceModifier : public IterApi<InplaceModifier<TChainInput, TModifierFn>> {
	friend struct IteratorTrait<InplaceModifier<TChainInput, TModifierFn>>;
private:
	using InputItem = typename TChainInput::Item;

	TChainInput input;
	TModifierFn modifierFn;
public:
	InplaceModifier(TChainInput&& input, TModifierFn modifierFn) : input(std::move(input)), modifierFn(modifierFn) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TChainInput, typename TModifierFn>
struct IteratorTrait<InplaceModifier<TChainInput, TModifierFn>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	// CXXIter Interface
	using Self = InplaceModifier<TChainInput, TModifierFn>;
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
template<typename TChainInput, typename TMapFn, typename TItem>
class Map : public IterApi<Map<TChainInput, TMapFn, TItem>> {
	friend struct IteratorTrait<Map<TChainInput, TMapFn, TItem>>;
private:
	TChainInput input;
	TMapFn mapFn;
public:
	Map(TChainInput&& input, TMapFn mapFn) : input(std::move(input)), mapFn(mapFn) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TChainInput, typename TMapFn, typename TItem>
struct IteratorTrait<Map<TChainInput, TMapFn, TItem>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using InputItemOwned = typename TChainInput::ItemOwned;
	// CXXIter Interface
	using Self = Map<TChainInput, TMapFn, TItem>;
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
template<typename TChainInput, typename TFlatMapFn, typename TItemContainer>
requires (!std::is_reference_v<TItemContainer>)
class FlatMap : public IterApi<FlatMap<TChainInput, TFlatMapFn, TItemContainer>> {
	friend struct IteratorTrait<FlatMap<TChainInput, TFlatMapFn, TItemContainer>>;
private:
	TChainInput input;
	std::optional<SrcMov<TItemContainer>> current;
	TFlatMapFn mapFn;
public:
	FlatMap(TChainInput&& input, TFlatMapFn mapFn) : input(std::move(input)), mapFn(mapFn) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TChainInput, typename TFlatMapFn, typename TItemContainer>
struct IteratorTrait<FlatMap<TChainInput, TFlatMapFn, TItemContainer>> {
	using NestedChainIterator = IteratorTrait<SrcMov<TItemContainer>>;
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using InputItem = typename ChainInputIterator::Item;
	// CXXIter Interface
	using Self = FlatMap<TChainInput, TFlatMapFn, TItemContainer>;
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
template<typename TChainInput, typename TFilterMapFn, typename TItem>
class FilterMap : public IterApi<FilterMap<TChainInput, TFilterMapFn, TItem>> {
	friend struct IteratorTrait<FilterMap<TChainInput, TFilterMapFn, TItem>>;
private:
	using InputItemOwned = typename TChainInput::ItemOwned;

	TChainInput input;
	TFilterMapFn filterMapFn;
public:
	FilterMap(TChainInput&& input, TFilterMapFn filterMapFn) : input(std::move(input)), filterMapFn(filterMapFn) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TChainInput, typename TFilterMapFn, typename TItem>
struct IteratorTrait<FilterMap<TChainInput, TFilterMapFn, TItem>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using InputItemOwned = typename TChainInput::ItemOwned;
	// CXXIter Interface
	using Self = FilterMap<TChainInput, TFilterMapFn, TItem>;
	using Item = TItem;

	static inline IterValue<Item> next(Self& self) {
		while(true) {
			auto item = ChainInputIterator::next(self.input);
			if(!item.hasValue()) { return {}; }
			std::optional<Item> value(self.filterMapFn(std::forward<InputItemOwned>( item.value() )));
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
// GROUP BY
// ################################################################################################
/** @private */
template<typename TChainInput, typename TGroupIdentifierFn, typename TGroupIdent>
class GroupBy : public IterApi<GroupBy<TChainInput, TGroupIdentifierFn, TGroupIdent>> {
	friend struct IteratorTrait<GroupBy<TChainInput, TGroupIdentifierFn, TGroupIdent>>;
private:
	using OwnedInputItem = typename TChainInput::ItemOwned;
	using GroupCache = SrcMov<std::unordered_map<TGroupIdent, std::vector<OwnedInputItem>>>;

	TChainInput input;
	TGroupIdentifierFn groupIdentFn;
	std::optional<GroupCache> groupCache;
public:
	GroupBy(TChainInput&& input, TGroupIdentifierFn groupIdentFn) : input(std::move(input)), groupIdentFn(groupIdentFn) {}
};
// ------------------------------------------------------------------------------------------------
/** @private */
template<typename TChainInput, typename TGroupIdentifierFn, typename TGroupIdent>
struct IteratorTrait<GroupBy<TChainInput, TGroupIdentifierFn, TGroupIdent>> {
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using OwnedInputItem = typename TChainInput::ItemOwned;
	// CXXIter Interface
	using Self = GroupBy<TChainInput, TGroupIdentifierFn, TGroupIdent>;
	using Item = std::pair<const TGroupIdent, std::vector<OwnedInputItem>>;

	static inline IterValue<Item> next(Self& self) {
		// we have to drain the input in order to be able to calculate the groups
		// so we do that on the first invocation, and then yield from the calculated result.
		if(!self.groupCache.has_value()) [[unlikely]] {
			std::unordered_map<TGroupIdent, std::vector<OwnedInputItem>> groupCache;
			while(true) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.hasValue()) { break; } // group cache building complete
				TGroupIdent itemGroup = self.groupIdentFn(item.value());
				if(groupCache.contains(itemGroup)) {
					groupCache[itemGroup].push_back(item.value());
				} else {
					groupCache[itemGroup] = { item.value() };
				}
			}
			self.groupCache.emplace(std::move(groupCache));
		}

		using GroupCacheIterator = IteratorTrait<typename Self::GroupCache>;
		typename Self::GroupCache& groupedItems = self.groupCache.value();
		return GroupCacheIterator::next(groupedItems);
	}
};



// ################################################################################################
// COLLECTOR
// ################################################################################################
/** @private */
template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
struct Collector {};
/** @private */
template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
requires BackInsertableCollection<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>
struct Collector<TChainInput, TContainer, TContainerArgs...> {
	template<typename Item, typename ItemOwned>
	static TContainer<ItemOwned, TContainerArgs...> collect(TChainInput& input) {
		TContainer<ItemOwned, TContainerArgs...> container;
		auto inserter = std::back_inserter(container);
		input.forEach([&inserter](Item&& item) { *inserter = item; });
		return container;
	}
};
/** @private */
template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
requires (!BackInsertableCollection<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>)
	&& is_pair<typename TChainInput::ItemOwned>
	&& AssocCollection<TContainer, typename TChainInput::ItemOwned::first_type, typename TChainInput::ItemOwned::second_type, TContainerArgs...>
struct Collector<TChainInput, TContainer, TContainerArgs...> {
	template<typename Item, typename ItemOwned>
	static auto collect(TChainInput& input) {
		TContainer<typename std::remove_const<typename ItemOwned::first_type>::type, typename ItemOwned::second_type, TContainerArgs...> container;
		auto inserter = std::inserter(container, container.end());
		input.forEach([&inserter](Item&& item) { *inserter = item; });
		return container;
	}
};
/** @private */
template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
requires (!BackInsertableCollection<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>)
	&& InsertableCollection<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>
struct Collector<TChainInput, TContainer, TContainerArgs...> {
	template<typename Item, typename ItemOwned>
	static TContainer<ItemOwned, TContainerArgs...> collect(TChainInput& input) {
		TContainer<ItemOwned, TContainerArgs...> container;
		auto inserter = std::inserter(container, container.end());
		input.forEach([&inserter](Item&& item) { *inserter = item; });
		return container;
	}
};


// ################################################################################################
// SURFACE-API
// ################################################################################################

template<CXXIterIterator TSelf>
class IterApi {
public: // Associated types
	/**
	 * @brief Type of the IteratorTrait implemenation for this.
	 */
	using Iterator = IteratorTrait<TSelf>;
	/**
	 * @brief Type of the elements of this iterator. (Can be references)
	 */
	using Item = typename Iterator::Item;
	/**
	 * @brief Owned Type of the elements of this iterator. (References removed).
	 */
	using ItemOwned = typename std::remove_reference<Item>::type;

private:
	TSelf* self() { return static_cast<TSelf*>(this); }
	static constexpr bool IS_REFERENCE = std::is_lvalue_reference<Item>::value;

public:
	virtual ~IterApi() {}

	// ###################
	// CONSUMERS
	// ###################
	/**
	 * @brief Consumer that calls the given function @p useFn for each of the elements in this iterator.
	 * @note This consumes the iterator.
	 * @param useFn Function called for each of the elements in this iterator.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::vector<std::string> output;
	 * 	CXXIter::from(input)
	 * 			.forEach([&output](std::string& item) {
	 * 				output.push_back(std::forward<std::string>(item));
	 * 			});
	 * @endcode
	 */
	template<typename TUseFn>
	void forEach(TUseFn useFn) {
		while(true) {
			auto item = Iterator::next(*self());
			if(!item.hasValue()) { return; }
			useFn(std::forward<Item>( item.value() ));
		}
	}

	/**
	 * @brief Consumer that collects all elements from this iterator in a new collection of type @p TTargetContainer
	 * @note This consumes the iterator.
	 * @tparam TTargetContainer Type-Template for the target container that the elements from this iterator should
	 * be collected into. The first template parameter of this Type-Template has to take the type of the elements.
	 * @tparam TTargetContainerArgs... Optional additional type attributes to pass on to the target container. These
	 * are appended to the item value type, which is automatically supplied.
	 * @return An instance of @p TTargetContainer with all the elements of this iterator collected into.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::vector<std::string> output = CXXIter::from(input)
	 * 		.collect<std::vector>();
	 * @endcode
	 *
	 * With Additional container type parameters:
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::vector<std::string, std::allocator<std::string>> output = CXXIter::from(input)
	 * 		.collect<std::vector, std::allocator<std::string>>();
	 * @endcode
	 */
	template<template <typename...> typename TTargetContainer, typename... TTargetContainerArgs>
	auto collect() {
		return Collector<TSelf, TTargetContainer, TTargetContainerArgs...>::template collect<Item, ItemOwned>(*self());
	}

	/**
	 * @brief Consumer that executes the given @p foldFn for each item in this iterator, to apply
	 * to a working value, which is passed on and passed as second argument to the next call to @p foldFn.
	 * @note This consumes the iterator.
	 * @param startValue The initial value of the working value passed to @p foldFn.
	 * @param foldFn Function called for each element in this iterator, passed the current workingValue and
	 * an element from this iterator.
	 * @return The workingValue from the last call to @p foldFn for the last element from this iterator.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<double> input = {1.331335363800390, 1.331335363800390, 1.331335363800390, 1.331335363800390};
	 * 	double output = CXXIter::from(input)
	 * 		.fold(1.0, [](double& workingValue, double item) {
	 * 			workingValue *= item;
	 * 		});
	 *	// output ~ 3.141592653589793
	 * @endcode
	 */
	template<typename TResult, std::invocable<TResult&, Item&&> FoldFn>
	TResult fold(TResult startValue, FoldFn foldFn) {
		TResult result = startValue;
		forEach([&result, &foldFn](Item&& item) { foldFn(result, std::forward<Item>(item)); });
		return result;
	}

	/**
	 * @brief Consumer that counts the elements in this iterator.
	 * @note This consumes the iterator.
	 * @return The amount of elements in this iterator
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {42, 1337, 52};
	 * 	size_t output = CXXIter::from(input).count();
	 * 	// output == 3
	 * 	std::vector<int> input2 = {};
	 * 	size_t output2 = CXXIter::from(input2).count();
	 * 	// output == 0
	 * @endcode
	 */
	size_t count() {
		return fold((size_t)0, [](size_t& cnt, auto&&) { cnt += 1; });
	}

	/**
	 * @brief Consumer that calculates the sum of all elements from this iterator.
	 * @note This consumes the iterator.
	 * @param startValue Starting value from which to start the sum.
	 * @return The sum of all elements from this iterator, or @p startValue if this
	 * iterator had no elements.
	 *
	 * Usage Example:
	 * - Non-empty iterator with default startValue
	 * @code
	 * 	std::vector<int> input = {42, 1337, 52};
	 * 	int output = CXXIter::from(input).sum();
	 * 	// output == 1431
	 * @endcode
	 * - Non-Empty iterator with custom startValue of 29906
	 * @code
	 * 	std::vector<int> input = {42, 1337, 52};
	 * 	int output = CXXIter::from(input).sum(29906);
	 * 	// output == 31337
	 * @endcode
	 * - Empty iterator with default startValue
	 * @code
	 * 	std::vector<int> input = {};
	 * 	int output = CXXIter::from(input).sum();
	 * 	// output == 0
	 * @endcode
	 * - Empty iterator with custom startValue
	 * @code
	 * 	std::vector<int> input = {};
	 * 	int output = CXXIter::from(input).sum(31337);
	 * 	// output == 31337
	 * @endcode
	 */
	template<typename TResult = ItemOwned>
	requires requires(TResult res, Item item) { { res += item }; }
	TResult sum(TResult startValue = TResult()) {
		return fold(startValue, [](TResult& res, Item&& item) { res += item; });
	}

	/**
	 * @brief Consumer that yields the smallest element from this iterator.
	 * @note This consumes the iterator.
	 * @return The smallest element of this iterator (if any).
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *	std::vector<int> input = {42, 1337, 52};
	 *	std::optional<int> output = CXXIter::from(input).min();
	 *	// output == Some(42)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<int> input = {};
	 *	std::optional<int> output = CXXIter::from(input).min();
	 *	// output == None
	 * @endcode
	 */
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

	/**
	 * @brief Consumer that yields the largest element from this iterator.
	 * @note This consumes the iterator.
	 * @return The largest element of this iterator (if any).
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *	std::vector<int> input = {42, 1337, 52};
	 *	std::optional<int> output = CXXIter::from(input).max();
	 *	// output == Some(1337)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<int> input = {};
	 *	std::optional<int> output = CXXIter::from(input).max();
	 *	// output == None
	 * @endcode
	 */
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

	/**
	 * @brief Consumer that yields the last element of this iterator.
	 * @note This consumes the iterator.
	 * @return The last element of this iterator (if any).
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *	std::vector<int> input = {42, 1337, 52};
	 *	std::optional<int> output = CXXIter::from(input).last();
	 *	// output == Some(52)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<int> input = {};
	 *	std::optional<int> output = CXXIter::from(input).last();
	 *	// output == None
	 * @endcode
	 */
	template<typename _unused = ItemOwned>
	std::optional<ItemOwned> last() {
		std::optional<ItemOwned> tmp;
		forEach([&tmp](Item&& item) { tmp = item; });
		return tmp;
	}


	// ###################
	// CHAINERS
	// ###################
	/**
	 * @brief Constructs a new iterator that casts the elements of this iterator to the type requested by @p TItemOutput.
	 * @details This iterator applies the requested type cast to @p TItemOutput using @c static_cast<>.
	 * @tparam TItemOutput Type to cast the elements of this iterator to.
	 * @return A new iterator that casts all elements from this iterator to the requested type @p TItemOutput.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<float> input = {1.35, 56.123};
	 * 	std::vector<double> output = CXXIter::from(input)
	 * 			.cast<double>()
	 * 			.collect<std::vector>();
	 * @endcode
	 */
	template<typename TItemOutput>
	Caster<TSelf, TItemOutput> cast() {
		return Caster<TSelf, TItemOutput>(std::move(*self()));
	}

	/**
	 * @brief Constructs a new iterator that copies the elements of this iterator.
	 * @details This function essentially converts an iterator that is passing elements by
	 * reference, to an iterator that is passing elements by value midway.
	 * @return A new iterator that is passing copies of the original input elements by value.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::string> input = {"inputString1", "inputString2"};
	 * 	std::vector<std::string> output = CXXIter::from(input)
	 * 		.copied() // clone values, now working with owned copies instead of references to input
	 * 		.modify([](std::string& item) { item[item.size() - 1] += 1; }) // modify copies, input untouched
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	auto copied() {
		return map([](ItemOwned&& item) -> ItemOwned {
			ItemOwned copy = item;
			return copy;
		});
	}

	/**
	 * @brief Constructs a new iterator that only contains the elements from this iterator, for
	 * which the given @p filterFn returned @c true.
	 * @param filterFn Function that decides which element of this iterator to yield in the
	 * newly created iterator.
	 * @return Iterator that only returns the elements for which the @p filterFn returns @c true.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
	 * 	std::vector<int> output = CXXIter::from(input)
	 * 		.filter([](int item) { return (item % 2) == 0; })
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	template<std::invocable<const ItemOwned&> TFilterFn>
	Filter<TSelf, TFilterFn> filter(TFilterFn filterFn) {
		return Filter<TSelf, TFilterFn>(std::move(*self()), filterFn);
	}

	/**
	 * @brief Creates an iterator that uses the given @p mapFn to map each element from this
	 * iterator to elements of the new iterator.
	 * @details This pulls a new value from this iterator, maps it to a new value (can have
	 * a completely new type) using the given @p mapFn and then yields that as new item for
	 * thew newly created iterator.
	 * @param mapFn Function that maps items from this iterator to a new value.
	 * @return New iterator that maps the values from this iterator to new values, using the
	 * given @p mapFn.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {1337, 42};
	 * 	std::unordered_map<int, std::string> output = CXXIter::from(input)
	 * 		.map([](int i) { return std::make_pair(i, std::to_string(i)); }) // construct pair
	 * 		.collect<std::unordered_map>(); // collect into map
	 * @endcode
	 */
	template<std::invocable<ItemOwned&&> TMapFn>
	auto map(TMapFn mapFn) {
		return Map<TSelf, TMapFn, decltype(mapFn( std::declval<ItemOwned&&>() ))>(std::move(*self()), mapFn);
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
	template<std::invocable<Item&&> TFlatMapFn>
	auto flatMap(TFlatMapFn mapFn = [](Item&& item) { return item; }) {
		return FlatMap<TSelf, TFlatMapFn, decltype(mapFn( std::declval<Item&&>() ))>(std::move(*self()), mapFn);
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
	InplaceModifier<TSelf, TModifierFn> modify(TModifierFn modifierFn) {
		return InplaceModifier<TSelf, TModifierFn>(std::move(*self()), modifierFn);
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
	template<std::invocable<ItemOwned&&> TFilterMapFn>
	auto filterMap(TFilterMapFn filterMapFn) {
		using ResultType = typename decltype ( filterMapFn( std::declval<ItemOwned&&>() ) )::value_type;
		return FilterMap<TSelf, TFilterMapFn, ResultType>(std::move(*self()), filterMapFn);
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
	auto skip(size_t cnt) {
		return filter([cnt](const ItemOwned&) mutable {
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
	auto skipWhile(TSkipPredicate skipPredicate) {
		bool skipDone = false;
		return filter([skipPredicate, skipDone](const ItemOwned& value) mutable {
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
	auto take(size_t cnt) {
		return filter([cnt](const ItemOwned&) mutable {
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
	auto takeWhile(TTakePredicate takePredicate) {
		bool takeDone = false;
		return filter([takePredicate, takeDone](const ItemOwned& value) mutable {
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

	/**
	 * @brief Groups the elements of this iterator according to the values returned by the given @p groupidentFn.
	 * @param groupIdentFn Function called for each element from this iterator, to determine the grouping value,
	 * that is then used to identify the group an item belongs to.
	 * @return New iterator whose elements are the calculated groups from the values of this iterator, in the form
	 * of a @c std::pair<> with the group identifier as first value, and a @c std::vector of all values in the group
	 * as second value.
	 * @attention GroupBy requires to first drain the input iterator, before being able to supply a single element.
	 * This leads to additional memory usage.
	 *
	 * Usage Example:
	 * @code
	 *	struct CakeMeasurement {
	 *		std::string cakeType;
	 *		float cakeWeight;
	 *		bool operator==(const CakeMeasurement& o) const {
	 *			return cakeType == o.cakeType && cakeWeight == o.cakeWeight;
	 *		}
	 *	};
	 *	std::vector<CakeMeasurement> input = { {"ApplePie", 1.3f}, {"Sacher", 0.5f}, {"ApplePie", 1.8f} };
	 *	std::unordered_map<std::string, std::vector<CakeMeasurement>>  output = CXXIter::from(input)
	 *		.groupBy([](const CakeMeasurement& item) { return item.cakeType; })
	 *		.collect<std::unordered_map>();
	 * @endcode
	 */
	template<std::invocable<const Item&> TGroupIdentifierFn>
	auto groupBy(TGroupIdentifierFn groupIdentFn) {
		using TGroupIdent = typename std::remove_reference<decltype ( groupIdentFn( std::declval<const ItemOwned&>() ) )>::type;
		return GroupBy<TSelf, TGroupIdentifierFn, TGroupIdent>(std::move(*self()), groupIdentFn);
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
template<typename TContainer>
requires SourceContainer<typename std::remove_reference<TContainer>::type>
SrcMov<TContainer> from(TContainer&& container) {
	return SrcMov<TContainer>(std::forward<TContainer>(container));
}

/**
 * @brief Construct a CXXIter mutable-reference source from the given container.
 * @details This constructs a mutable-reference source. This allows the iterator
 * to modify the elements in the array.
 * @param container Container to construct a CXXIter source from.
 * @return CXXIter mutable-reference source from the given container.
 */
template<typename TContainer>
requires SourceContainer<typename std::remove_reference<TContainer>::type>
SrcRef<TContainer> from(TContainer& container) {
	return SrcRef<TContainer>(container);
}

/**
 * @brief Construct a CXXIter const-reference source from the given container.
 * @details This constructs a const-reference source. This guarantees the
 * given container to stay untouched.
 * @param container Container to construct a CXXIter source from.
 * @return CXXIter const-reference source from the given container.
 */
template<typename TContainer>
requires SourceContainer<typename std::remove_reference<TContainer>::type>
SrcCRef<TContainer> from(const TContainer& container) {
	return SrcMov<TContainer>(container);
}

}
