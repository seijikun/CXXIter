#pragma once

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <functional>
#include <concepts>
#include <variant>

namespace CIter {

// ################################################################################################
// CONCEPTS & CONSTRAINTS
// ################################################################################################

template<typename T>
concept BackInsertable = requires(T v, typename T::value_type item) {
	{ v.push_back(item) };
};

template<typename T>
concept Insertable = requires(T v, typename T::value_type item) {
	{ v.insert(item) };
};



template<typename T>
struct is_pair { static constexpr bool value = false; };
template<typename TKey, typename TValue>
struct is_pair<std::pair<TKey, TValue>> { static constexpr bool value = true; };
template <typename T>
inline constexpr bool is_pair_v = is_pair<T>::value;



// ################################################################################################
// PRE-DEFINITIONS
// ################################################################################################

struct LINQIteratorEnd {};

template<typename T>
struct IteratorTrait {};

template<typename TSelf>
class LINQAPI;


// ################################################################################################
// SOURCE (MOVE / CONSUME)
// ################################################################################################
template<typename TContainer>
class SrcMov : public LINQAPI<SrcMov<TContainer>> {
	friend struct IteratorTrait<SrcMov<TContainer>>;
private:
	TContainer container;
	typename TContainer::iterator ptr;
public:
	SrcMov(TContainer&& container) : container(std::move(container)), ptr(this->container.begin()) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TContainer>
struct IteratorTrait <SrcMov<TContainer>> {
	using Self = SrcMov<TContainer>;
	using Item = typename TContainer::value_type;

	static inline Item next(SrcMov<TContainer>& self) {
		if(self.ptr == self.container.end()) { throw LINQIteratorEnd(); }
		return std::move(*self.ptr++);
	}
};



// ################################################################################################
// SOURCE (MUTABLE REFERENCE)
// ################################################################################################
template<typename TContainer>
class SrcRef : public LINQAPI<SrcRef<TContainer>> {
	friend struct IteratorTrait<SrcRef<TContainer>>;
private:
	TContainer& container;
	typename TContainer::iterator ptr;
public:
	SrcRef(TContainer& container) : container(container), ptr(this->container.begin()) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TContainer>
struct IteratorTrait <SrcRef<TContainer>> {
	using Self = SrcRef<TContainer>;
	using Item = typename TContainer::reference;

	static inline Item next(SrcRef<TContainer>& self) {
		if(self.ptr == self.container.end()) { throw LINQIteratorEnd(); }
		return *self.ptr++;
	}
};



// ################################################################################################
// SOURCE (CONST REFERENCE)
// ################################################################################################
template<typename TContainer>
class SrcCRef : public LINQAPI<SrcCRef<TContainer>> {
	friend struct IteratorTrait<SrcCRef<TContainer>>;
private:
	const TContainer& container;
	typename TContainer::const_iterator ptr;
public:
	SrcCRef(const TContainer& container) : container(container), ptr(this->container.begin()) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TContainer>
struct IteratorTrait <SrcCRef<TContainer>> {
	using Self = SrcCRef<TContainer>;
	using Item = typename TContainer::const_reference;

	static inline Item next(SrcCRef<TContainer>& self) {
		if(self.ptr == self.container.end()) { throw LINQIteratorEnd(); }
		return *self.ptr++;
	}
};



// ################################################################################################
// CASTER
// ################################################################################################
template<typename TChainInput, typename TItem>
requires std::is_object_v<TItem>
class Caster : public LINQAPI<Caster<TChainInput, TItem>> {
	friend struct IteratorTrait<Caster<TChainInput, TItem>>;
private:
	TChainInput input;
public:
	Caster(TChainInput&& input) : input(std::move(input)) {}
};
// ------------------------------------------------------------------------------------------------
template<typename TChainInput, typename TItem>
requires std::is_object_v<TItem>
struct IteratorTrait <Caster<TChainInput, TItem>> {
	// LINQ Interface
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using Self = Caster<TChainInput, TItem>;
	using Item = TItem;

	static inline Item next(Caster<TChainInput, Item>& self) {
		return static_cast<Item>( ChainInputIterator::next(self.input) );
	}
};



// ################################################################################################
// FILTER
// ################################################################################################
template<typename TChainInput>
class Filter : public LINQAPI<Filter<TChainInput>> {
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
struct IteratorTrait <Filter<TChainInput>> {
	// LINQ Interface
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using Self = Filter<TChainInput>;
	using Item = typename ChainInputIterator::Item;

	static inline Item next(Filter<TChainInput>& self) {
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
class InplaceModifier : public LINQAPI<InplaceModifier<TChainInput>> {
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
struct IteratorTrait <InplaceModifier<TChainInput>> {
	// LINQ Interface
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using Self = InplaceModifier<TChainInput>;
	using Item = typename ChainInputIterator::Item;

	static inline Item next(InplaceModifier<TChainInput>& self) {
		Item value = ChainInputIterator::next(self.input);
		self.modifierFn(value);
		return value;
	}
};



// ################################################################################################
// MAP
// ################################################################################################
template<typename TChainInput, typename TItem>
class Map : public LINQAPI<Map<TChainInput, TItem>> {
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
struct IteratorTrait <Map<TChainInput, TItem>> {
	// LINQ Interface
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using Self = Map<TChainInput, TItem>;
	using Item = TItem;

	static inline Item next(Map<TChainInput, TItem>& self) {
		return self.mapFn(std::forward<typename ChainInputIterator::Item>( ChainInputIterator::next(self.input) ));
	}
};



// ################################################################################################
// FILTERMAP
// ################################################################################################
template<typename TChainInput, typename TItem>
class FilterMap : public LINQAPI<FilterMap<TChainInput, TItem>> {
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
struct IteratorTrait <FilterMap<TChainInput, TItem>> {
	// LINQ Interface
	using ChainInputIterator = IteratorTrait<TChainInput>;
	using Self = FilterMap<TChainInput, TItem>;
	using Item = TItem;

	static inline Item next(FilterMap<TChainInput, TItem>& self) {
		while(true) {
			std::optional<Item> value(self.filterMapFn(std::move( ChainInputIterator::next(self.input) )));
			if(!value) { continue; }
			return *value;
		}
	}
};



// ################################################################################################
// COLLECTOR
// ################################################################################################
template<typename TChainInput, template <typename... TContainerArgs> typename TContainer>
struct Collector {
	template<typename Item, typename ItemOwned>
	requires BackInsertable<TContainer<ItemOwned>> || Insertable<TContainer<ItemOwned>>
	static auto collect(TChainInput& input) {}

	template<typename Item, typename ItemOwned>
	requires BackInsertable<TContainer<ItemOwned>>
	static auto collect(TChainInput& input) {
		TContainer<ItemOwned> container;
		auto inserter = std::back_inserter(container);
		input.forEach([&inserter](Item&& item) { *inserter = std::forward<ItemOwned>(item); });
		return container;
	}

	template<typename Item, typename ItemOwned>
	requires Insertable<TContainer<ItemOwned>>
	auto collect(TChainInput& input) {
		TContainer<ItemOwned> container;
		auto inserter = std::inserter(container, container.end());
		input.forEach([&inserter](Item&& item) { *inserter = std::forward<ItemOwned>(item); });
		return container;
	}

	template<typename Item, typename ItemOwned>
	std::unordered_map<typename std::remove_const<typename ItemOwned::first_type>::type, typename ItemOwned::second_type> collect(TChainInput& input) {
		std::unordered_map<typename std::remove_const<typename ItemOwned::first_type>::type, typename ItemOwned::second_type> container;
		input.forEach([&container](Item&& item) { container[item.first] = item.second; });
		return container;
	}
};




template<typename TSelf>
class LINQAPI {
	TSelf* self() { return dynamic_cast<TSelf*>(this); }

public:
	using Iterator = IteratorTrait<TSelf>;
	using Item = typename Iterator::Item;
	using ItemOwned = typename std::remove_reference<Item>::type;
	static constexpr bool IS_REFERENCE = std::is_reference<Item>::value;

	virtual ~LINQAPI() {}

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

	template<template <typename... TTargetContainerArgs> typename TTargetContainer>
	auto collect() {
		Collector<TSelf, TTargetContainer> collector;
		return collector.template collect<Item, ItemOwned>(*self());
	}


	// ###################
	// CHAINERS
	// ###################
	template<typename TItemOutput>
	Caster<TSelf, TItemOutput> cast() {
		return Caster<TSelf, TItemOutput>(std::move(*self()));
	}

	template<std::invocable<const Item&> TFilterFn>
	Filter<TSelf> filter(TFilterFn filterFn) {
		return Filter<TSelf>(std::move(*self()), filterFn);
	}

	template<std::invocable<Item&&> TMapFn>
	auto map(TMapFn mapFn) {
		return Map<TSelf, decltype(mapFn( std::declval<Item&&>() ))>(std::move(*self()), mapFn);
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
