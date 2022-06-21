#pragma once

#include <utility>
#include <optional>
#include <concepts>
#include <functional>
#include <string>
#include <limits>

#include <unordered_map>
#include <vector>
#include <cmath>

#include "src/Common.h"
#include "src/Generator.h"
#include "src/sources/ContainerSources.h"
#include "src/sources/GeneratorSources.h"
#include "src/Collector.h"
#include "src/op/Alternater.h"
#include "src/op/Caster.h"
#include "src/op/Chainer.h"
#include "src/op/Chunked.h"
#include "src/op/ChunkedExact.h"
#include "src/op/Filter.h"
#include "src/op/FilterMap.h"
#include "src/op/FlagLast.h"
#include "src/op/FlatMap.h"
#include "src/op/GenerateFrom.h"
#include "src/op/GroupBy.h"
#include "src/op/InplaceModifier.h"
#include "src/op/Intersperser.h"
#include "src/op/Map.h"
#include "src/op/Reverse.h"
#include "src/op/SkipN.h"
#include "src/op/SkipWhile.h"
#include "src/op/Sorter.h"
#include "src/op/TakeN.h"
#include "src/op/TakeWhile.h"
#include "src/op/Unique.h"
#include "src/op/Zipper.h"
#include "src/Helpers.h"


/**
 * @brief CXXIter
 */
namespace CXXIter {

// ################################################################################################
// SURFACE-API
// ################################################################################################

/**
 * @brief Public Iterator API surface.
 */
template<CXXIterIterator TSelf>
class IterApi {
public: // Associated types
	/**
	 * @brief Type of the trait::Iterator implemenation for this.
	 */
	using Iterator = trait::Iterator<TSelf>;
	/**
	 * @brief Type of the elements of this iterator. (Can be references)
	 */
	using Item = typename Iterator::Item;
	/**
	 * @brief Owned Type of the elements of this iterator. (References removed).
	 */
	using ItemOwned = std::remove_cvref_t<Item>;

private:
	TSelf* self() { return static_cast<TSelf*>(this); }
	const TSelf* self() const { return static_cast<const TSelf*>(this); }
	static constexpr bool IS_REFERENCE = std::is_lvalue_reference<Item>::value;

public: // Lifecycle Management
	virtual ~IterApi() {}

public: // C++ Iterator API-Surface

	/**
	 * @brief C++ iterator implementation for a CXXIter chain.
	 */
	class iterator {
		friend class IterApi;
		TSelf& self;
		IterValue<Item> element;

		/** end ctor */
		iterator(TSelf& self) : self(self) {}
		/** element ctor */
		iterator(TSelf& self, IterValue<Item>&& element) : self(self), element(std::move(element)) {}


	public:
		iterator& operator++() {
			if(element.has_value()) {
				element = self.next();
			}
			return *this;
		}

		Item& operator*() { return element.value(); }

		bool operator!=(const iterator& o) {
			return (element.has_value() != o.element.has_value());
		}
	};

	/**
	 * @brief begin() method, part of C++'s iterator interface
	 * @return C++ interface on top of this iterator pipeline.
	 */
	iterator begin() { return {*self(), next()}; }

	/**
	 * @brief end() method, part of C++'s iterator interface
	 * @return C++ interface on top of this iterator pipeline.
	 */
	iterator end() { return {*self()}; }

public: // CXXIter API-Surface

	/**
	 * @brief Get the bounds on the remaining length of this iterator, estimated from the source
	 * and all of the chained iterations on it.
	 * @return The estimated bounds on the remaining length of this iterator.
	 */
	SizeHint sizeHint() const {
		return Iterator::sizeHint(*self());
	}

	/**
	 * @brief Get this iterator's exact size.
	 * @note This method only exists if the iterator's exact size is known. Operations like @c IterApi::filter
	 * cause the remaining iterator to have an unknown exact size.
	 * @return This iterator's exact number of elements.
	 *
	 * Usage Example:
	 * - Valid (Exact number of elements is known):
	 * @code
	 * 	size_t size = CXXIter::range<float>(0.0f, 2.0f, 0.25f)
	 * 			.map([](float blub) { return std::to_string(blub); })
	 * 			.sort()
	 * 			.intersperse(CXXIter::empty<std::string>())
	 * 			.size();
	 * 	// size == 8
	 * @endcode
	 * - Invalid, does not compile (Exact number of elements is unknown):
	 * @code
	 * 	size_t size = CXXIter::range<float>(0.0f, 2.0f, 0.25f)
	 * 		.map([](float blub) { return std::to_string(blub); })
	 * 		.sort()
	 * 		.intersperse(CXXIter::empty<std::string>())
	 * 		.flatMap()
	 * 		.size();
	 * @endcode
	 */
	size_t size() const requires CXXIterExactSizeIterator<TSelf> {
		return trait::ExactSizeIterator<TSelf>::size(*self());
	}

	/**
	 * @brief Get the next element from this iterator (if any), wrapped in a CXXIter::IterValue<>.
	 * @note If the returned CXXIter::IterValue is empty, there are no elements left in this iterator.
	 * Calling @c next() again after that is undefined behavior.
	 * @return The next element from this iterator (if any), wrapped in a CXXIter::IterValue<>
	 *
	 * Usage Example:
	 * @code
	 * 	std::optional<float> output = CXXIter::range<float>(1.337f, 2.0f, 0.25f)
	 * 		.next().toStdOptional();
	 *  // output == Some(1.337f);
	 * @endcode
	 */
	IterValue<Item> next() {
		return Iterator::next(*self());
	}

	/**
	 * @brief Advance the iterator by n elements.
	 * @details If possible, this is a O(1) operation. Some iterator pipeline elements make that impossible
	 * though. In these cases, the implementation falls back to pulling n elements and dropping them.
	 * @param n The amount of elements to advance the iterator by.
	 *
	 * Usage Example:
	 * @code
	 *  auto src = CXXIter::range<float>(1.337f, 2.0f, 0.25f, 5.0f, 42.0f);
	 * 	std::optional<float> output = src.next().toStdOptional();
	 *  // output == Some(1.337f);
	 *  src.advanceBy(2);
	 *  output = src.next().toStdOptional();
	 *  // output == Some(5.0f);
	 * @endcode
	 */
	void advanceBy(size_t n) {
		Iterator::advanceBy(*self(), n);
	}

	/**
	 * @brief Get the next element from the back of this iterator (if any), wrapped in a CXXIter::IterValue<>.
	 * @note If the returned CXXIter::IterValue is empty, there are no elements left in this iterator.
	 * Calling @c nextBack() again after that is undefined behavior.
	 * @note This method only exists if the iterator implements the DoubleEndedIterator trait.
	 * @return The next element from the back of this iterator (if any), wrapped in a CXXIter::IterValue<>
	 *
	 * Usage Example:
	 * @code
	 * 	std::optional<float> output = CXXIter::range<float>(1.337f, 2.0f, 0.25f)
	 * 		.nextBack().toStdOptional();
	 *  // output == Some(0.25f);
	 * @endcode
	 */
	IterValue<Item> nextBack() requires CXXIterDoubleEndedIterator<TSelf> {
		return trait::DoubleEndedIterator<TSelf>::nextBack(*self());
	}

	// ###################
	// CONSUMERS
	// ###################
	/**
	 * @name Consumers
	 */
//@{

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
			if(!item.has_value()) [[unlikely]] { return; }
			useFn(std::forward<Item>( item.value() ));
		}
	}

	/**
	 * @brief Consumer that collects all elements from this iterator in a new container of type @p TTargetContainer
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
	 * @brief Consumer that collects all elements from this iterator in a new container of type @p TTargetContainer
	 * @note This consumes the iterator.
	 * @tparam TTargetContainer Fully qualified type of the target container to collect the items of this iterator into.
	 * @return An instance of @p TTargetContainer with all the elements of this iterator collected into.
	 *
	 * Usage Example:
	 * - std::vector<std::string>
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::vector<std::string> output = CXXIter::from(input)
	 * 		.collect<std::vector<std::string>>();
	 * @endcode
	 *
	 * - std::vector<std::string> with explicitly defined allocator
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::vector<std::string, std::allocator<std::string>> output = CXXIter::from(input)
	 * 		.collect<std::vector<std::string, std::allocator<std::string>>>();
	 * @endcode
	 *
	 * - std::array<std::string, 3>
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::array<std::string, 3> output = CXXIter::from(input)
	 * 		.collect<std::array<std::string, 3>>();
	 * @endcode
	 */
	template<typename TTargetContainer>
	TTargetContainer collect() {
		TTargetContainer container;
		collectInto(container);
		return container;
	}

	/**
	 * @brief Consumer that collects all elements from this iterator into the given @p container.
	 * @note This consumes the iterator.
	 * @param container to collect this iterator's elements into.
	 * @details Before appending this iterator's elements to the elements already present in the
	 * given @p container, the collector tries to resize the @p container to its current size + this
	 * iterator's expected amount of items.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::vector<std::string> output = {"prevElement"};
	 *	CXXIter::from(input).collectInto(output);
	 *	// output == {"prevElement", "1337", "42", "64"}
	 * @endcode
	 */
	template<typename TTargetContainer>
	void collectInto(TTargetContainer& container) {
		IntoCollector<TSelf, TTargetContainer>::collectInto(*self(), container);
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
	 * @brief Tests if all elements of this iterator match the given @p predicateFn.
	 * @note This consumes the iterator.
	 * @param predicateFn Predicate to test all items of this iterator against.
	 * @return @c true when the given @p predicateFn returned @c true for all elements of this
	 * iterator, @c false otherwise.
	 *
	 * Usage Example:
	 * (Using the following predicate)
	 * @code
	 * 	auto intAsBoolFn = [](uint32_t item) -> bool { return (item != 0); };
	 * @endcode
	 *
	 * - For cases where the predicate does not return @c true for all elements:
	 * @code
	 * 	std::vector<uint32_t> input = { 1, 1, 1, 0 };
	 * 	bool output = CXXIter::from(input).copied().all(intAsBoolFn);
	 * 	// output == false
	 *
	 * 	std::vector<uint32_t> input = { 0, 1, 1, 1 };
	 * 	bool output = CXXIter::from(input).copied().all(intAsBoolFn);
	 * 	// output == false
	 *
	 * 	std::vector<uint32_t> input = { 0, 0, 1, 1 };
	 * 	bool output = CXXIter::from(input).copied().all(intAsBoolFn);
	 * 	// output == false
	 * @endcode
	 *
	 * - For cases where the predicate does return @c true for all elements:
	 * @code
	 * 	std::vector<uint32_t> input = { 1, 1, 1, 1 };
	 * 	bool output = CXXIter::from(input).copied().all(intAsBoolFn);
	 * 	// output == true
	 * @endcode
	 */
	template<std::invocable<const ItemOwned&> TPredicateFn>
	requires std::same_as<std::invoke_result_t<TPredicateFn, const ItemOwned&>, bool>
	bool all(TPredicateFn predicateFn) {
		return !skipWhile(predicateFn).next().has_value();
	}

	/**
	 * @brief Tests if all elements of this iterator yield the value @c true when casted to @c bool.
	 * @note This consumes the iterator.
	 * @details This is an overload of all(TPredicateFn) for item types that support
	 * being casted to @c bool.
	 * @return @c true when all elements of this iterator yielded the value @c true when casted to
	 * a @c bool, @c false otherwise.
	 *
	 * Usage Example:
	 * - For cases where not all elements of this iterator evaluate to @c true when casted to @c bool.
	 * @code
	 * 	std::vector<bool> input = { true, true, true, false };
	 * 	bool output = CXXIter::from(input).copied().all();
	 * 	// output == false
	 *
	 * 	std::vector<bool> input = { false, true, true, true };
	 * 	bool output = CXXIter::from(input).copied().all();
	 * 	// output == false
	 *
	 * 	std::vector<bool> input = { true, true, false, true };
	 * 	bool output = CXXIter::from(input).copied().all();
	 * 	// output == false
	 * @endcode
	 *
	 * - For cases where all elements of this iterator evaluate to @c true when casted to @c bool.
	 * @code
	 * 	std::vector<bool> input = { true, true, true, true };
	 * 	bool output = CXXIter::from(input).copied().all();
	 * 	// output == true
	 * @endcode
	 */
	bool all() requires requires(const ItemOwned& item) {
		{static_cast<bool>(item)};
	} {
		return all([](const auto& item) -> bool { return item; });
	}

	/**
	 * @brief Tests if any of the elements of this iterator match the given @p predicateFn.
	 * @note This consumes the iterator.
	 * @param predicateFn Predicate to test all items of this iterator against.
	 * @return @c true when the given @p predicateFn returned @c true for any of the elements
	 * of this iterator, @c false otherwise.
	 *
	 * Usage Example:
	 * (Using the following predicate)
	 * @code
	 * 	auto intAsBoolFn = [](uint32_t item) -> bool { return (item != 0); };
	 * @endcode
	 *
	 * - For the case where the @p predicateFn returns @c false for all elements:
	 * @code
	 * 	std::vector<uint32_t> input = { 0, 0, 0, 0 };
	 * 	bool output = CXXIter::from(input).copied().any(intAsBoolFn);
	 * 	// output == false
	 * @endcode
	 * - For the case where the @p predicateFn returns @c true for any of the elements:
	 * @code
	 * 	std::vector<uint32_t> input = { 0, 1, 1, 1 };
	 * 	bool output = CXXIter::from(input).copied().any(intAsBoolFn);
	 * 	// output == true
	 *
	 * 	std::vector<uint32_t> input = { 0, 0, 0, 1 };
	 * 	bool output = CXXIter::from(input).copied().any(intAsBoolFn);
	 * 	// output == true
	 *
	 * 	std::vector<uint32_t> input = { 1, 1, 1, 1 };
	 * 	bool output = CXXIter::from(input).copied().any(intAsBoolFn);
	 * 	// output == true
	 * @endcode
	 */
	template<std::invocable<const ItemOwned&> TPredicateFn>
	bool any(TPredicateFn predicateFn) {
		return filter(predicateFn).next().has_value();
	}

	/**
	 * @brief Tests if any of the elements of this iterator yield the value @c true when casted to @c bool.
	 * @note This consumes the iterator.
	 * @details This is an overload of any(TPredicateFn) for item types that support
	 * being casted to @c bool.
	 * @return @c true when any of the elements of this iterator yielded the value @c true when casted to
	 * a @c bool, @c false otherwise.
	 *
	 * Usage Example:
	 * - For cases where none of the elements yields the value @c true when casted to @c bool.
	 * @code
	 * 	std::vector<bool> input = { false, false, false, false };
	 * 	bool output = CXXIter::from(input).copied().any();
	 * 	// output == false
	 * @endcode
	 *
	 * - For cases where any of the elements yields the value @c true when casted to @c bool.
	 * @code
	 * 	std::vector<bool> input = { false, true, true, true };
	 * 	bool output = CXXIter::from(input).copied().any();
	 * 	// output == true
	 *
	 * 	std::vector<bool> input = { true, false, false, false };
	 * 	bool output = CXXIter::from(input).copied().any();
	 * 	// output == true
	 *
	 * 	std::vector<bool> input = { true, true, true, true };
	 * 	bool output = CXXIter::from(input).copied().any();
	 * 	// output == true
	 * @endcode
	 */
	bool any() requires requires(const ItemOwned& item) {
		{static_cast<bool>(item)};
	} {
		return any([](const auto& item) -> bool { return item; });
	}

	/**
	 * @brief Search for the given @p searchItem within the items of this iterator, and return the index of the first item
	 * from the iterator that is equal to the given @p searchItem.
	 * @param searchItem Item to search for in the iterator.
	 * @return Index of the given @p searchItem in the iterator, if found.
	 *
	 * Usage Example:
	 * - When item is found in the iterator:
	 * @code
	 *	std::vector<int> input = {42, 1337, 52};
	 *	std::optional<size_t> output = CXXIter::from(input).findIdx(1337);
	 *	// output == Some(1)
	 * @endcode
	 * - When item is not found in the iterator:
	 * @code
	 *	std::vector<int> input = {"42", "1337", "52"};
	 *	std::optional<size_t> output = CXXIter::from(input).findIdx("not found");
	 *	// output == None
	 * @endcode
	 */
	std::optional<size_t> findIdx(const ItemOwned& searchItem) 	requires requires(const ItemOwned& searchItem, const Item& item) {
		{searchItem == item} -> std::same_as<bool>;
	} {
		return findIdx([&searchItem](const ItemOwned& item) {
			return (searchItem == item);
		});
	}


	/**
	 * @brief Search for the iterator with the given @p findFn, and return the index of the element from this iterator,
	 * for which the @p findFn returned @c true the first time.
	 * @param findFn Lambda invoked for each element of this stream, to determined whether it is the item that is searched for.
	 * @return Index of the first element from this stream, for which the invocation to the given @p findFn returned @c true.
	 *
	 * Usage Example:
	 * - When item is found in the iterator:
	 * @code
	 * 	std::vector<int> input = {1337, 31337, 41, 43, 42, 64};
	 * 	std::optional<size_t> output = CXXIter::from(input)
	 * 		.findIdx([](int item) { return (item % 2 == 0); });
	 * 	// output == Some(4)
	 * @endcode
	 * - When item is not found in the iterator:
	 * @code
	 * 	std::vector<int> input = {1337, 31337, 41, 43};
	 * 	std::optional<size_t> output = CXXIter::from(input)
	 * 		.findIdx([](int item) { return (item % 2 == 0); });
	 * 	// output == None
	 * @endcode
	 */
	template<std::invocable<const ItemOwned&> TFindFn>
	std::optional<size_t> findIdx(TFindFn findFn) {
		size_t idx = 0;
		while(true) {
			auto item = Iterator::next(*self());
			if(!item.has_value()) [[unlikely]] { return {}; }
			if(findFn(item.value())) [[unlikely]] { return idx; }
			idx += 1;
		}
	}

	/**
	 * @brief Searches for an element of this iterator, that satisfies the given @p findFn predicate.
	 * @param findFn Predicate used to search for an element in this iterator.
	 * @return An CXXIter::IterValue containing the first element, for which the @p findFn predicate
	 * returned @c true (if any), otherwise empty.
	 *
	 * Usage Example:
	 * - When item is found in the iterator:
	 * @code
	 * 	std::vector<std::string> input = {"42", "1337", "52"};
	 * 	CXXIter::IterValue<std::string&> output = CXXIter::from(input)
	 * 			.find([](const std::string& item) {
	 * 				return item.size() == 4;
	 * 			});
	 * 	// output == Some("1337"&)
	 * @endcode
	 * - When item is not found in the iterator:
	 * @code
	 * 	std::vector<std::string> input = {"42", "1337", "52"};
	 * 	CXXIter::IterValue<std::string&> output = CXXIter::from(input)
	 * 			.find([](const std::string& item) {
	 * 				return item.size() == 3;
	 * 			});
	 * 	// output == None
	 * @endcode
	 */
	template<std::invocable<const ItemOwned&> TFindFn>
	IterValue<Item> find(TFindFn findFn) {
		return filter(findFn).next();
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
	 * @brief Consumer that counts the elements in this iterator, for which the given @p predicateFn
	 * returns @c true.
	 * @note This consumes the iterator.
	 * @param predicateFn Predicate that is run for each element of this iterator, to determine whether it
	 * should contribute to the resulting count.
	 * @return The amount of elements in this iterator for which the given @p predicateFn returned @c true.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	 * 	size_t output = CXXIter::from(input)
	 * 			.count([](int item){ return (item % 2 == 0); });
	 * // output == 5
	 * @endcode
	 */
	template<std::invocable<const ItemOwned&> TPredicateFn>
	size_t count(TPredicateFn predicateFn) {
		return fold((size_t)0, [&predicateFn](size_t& cnt, auto&& item) {
			if(predicateFn(item)) { cnt += 1; }
		});
	}

	/**
	 * @brief Consumer that counts the occurences of @p countItem within this iterator.
	 * @note This consumes the iterator.
	 * @param countItem Item for which to count the amount of occurences within this iterator.
	 * @return The number of occurences of @p countItem within this iterator.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	 * 	size_t output = CXXIter::from(input)
	 * 			.map([](int item) { return (item % 2 == 0); })
	 * 			.count(true);
	 * 	// output == 5
	 * @endcode
	 */
	size_t count(const ItemOwned& countItem)
	requires requires(const ItemOwned& countItem, Item&& item) {
		{countItem == item};
	} {
		return fold((size_t)0, [&countItem](size_t& cnt, auto&& item) {
			if(item == countItem) { cnt += 1; }
		});
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
	 * @brief Consumer that concatenates the elements of this iterator to a large @c std::string , where
	 * each element is separated by the specified @p separator.
	 * @note This consumes the iterator.
	 * @note This method is only available for iterators whose elements are @c std::string . If that is not
	 * the case, convert your items to @c std::string s first, using a method like @c map().
	 * @return The resulting string concatenation of all items of this iterator.
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
	std::string stringJoin(const std::string& separator) requires std::is_same_v<ItemOwned, std::string> {
		std::string result;
		forEach([&result, &separator](const std::string& item) {
			if(result.size() > 0) [[likely]] { result += separator + item; }
			else [[unlikely]] { result = item; }
		});
		return result;
	}

	/**
	 * @brief Consumer that calculates the mean of all elements of this iterator.
	 * @details The mean is calculated by first summing up all elements, and then
	 * dividing through the number of elements counted while summing.
	 * @note This consumes the iterator.
	 * @param sumStart Optional starting point for the sum of all items. Normally uses default ctor of @p TResult.
	 * @return The mean of all elements of this iterator.
	 * @tparam NORM Type of the statistical normalization variant to use for the
	 * calculation. @see StatisticNormalization
	 * @tparam TResult Type of the mean-calculation's result. This is also the type used
	 * for the sum of all elements.
	 * @tparam TCount Type the element counter is converted into, before dividing the sum
	 * by. This can be necessary, if TResult is a complex object that only supports the
	 * division operator for e.g. double.
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *	std::vector<float> input = {1.0f, 2.0f, 3.0f};
	 *	std::optional<float> output = CXXIter::from(input).mean();
	 *	// output == Some(2.0f)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<float> input = {};
	 *	std::optional<float> output = CXXIter::from(input).mean();
	 *	// output == None
	 * @endcode
	 */
	template<StatisticNormalization NORM = StatisticNormalization::N, typename TResult = ItemOwned, typename TCount = ItemOwned>
	std::optional<TResult> mean(TResult sumStart = TResult()) {
		size_t cnt = 0;
		TResult result = fold(sumStart, [&cnt](TResult& res, Item&& item) {
			cnt += 1;
			res += item;
		});
		if(cnt > 0) {
			if constexpr(NORM == StatisticNormalization::N) {
				return result / static_cast<TCount>(cnt);
			} else {
				return result / static_cast<TCount>(cnt - 1);
			}
		}
		return {};
	}

	/**
	 * @brief Consumer that calculates the variance of all elements of this iterator.
	 * @details The variance is calculated by incrementally calculating a sum and
	 * a squared sum of all elements. Then from there, the mean and then the variance
	 * are calculated.
	 * @note This consumes the iterator.
	 * @return The variance of all elements of this iterator.
	 * @tparam NORM Type of the statistical normalization variant to use for the
	 * calculation. @see StatisticNormalization
	 * @tparam TResult Type of the variance-calculation's result. This is also the type used
	 * for the sum of all elements.
	 * @tparam TCount Type the element counter is converted into, before dividing the sum and
	 * the squared sum by. This can be necessary, if TResult is a complex object that only supports
	 * the division operator for e.g. double.
	 * @see https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
	 *
	 * Usage Example:
	 * - For iterators with at least 2 values:
	 * @code
	 *	std::vector<float> input = {2.0f, 4.0f, 4.0f, 4.0f, 5.0f, 5.0f, 7.0f, 9.0f};
	 *	std::optional<float> output = CXXIter::from(input).variance();
	 *	// output == Some(4.0f)
	 * @endcode
	 * - For iterators with at least 2 values with (N-1) norm:
	 * @code
	 *	std::vector<float> input = {1.0f, 2.0f, 3.0f};
	 *	std::optional<float> output = CXXIter::from(input)
	 *		.variance<CXXIter::StatisticNormalization::N_MINUS_ONE>();
	 *	// output == Some(1.0f)
	 * @endcode
	 * - For iterators with less than 2 values (not defined):
	 * @code
	 *	std::vector<float> input = {42.0f};
	 *	std::optional<float> output = CXXIter::from(input).variance();
	 *	// output == None
	 * @endcode
	 */
	template<StatisticNormalization NORM = StatisticNormalization::N, typename TResult = ItemOwned, typename TCount = ItemOwned>
	std::optional<TResult> variance() {
		TResult sumSquare = TResult();
		TResult sum = TResult();
		size_t cnt = 0;
		forEach([&sumSquare, &sum, &cnt](Item&& item) {
			sum += item;
			sumSquare += (item * item);
			cnt += 1;
		});
		if(cnt >= 2) {
			if constexpr(NORM == StatisticNormalization::N) {
				TResult E1 = (sumSquare / static_cast<TCount>(cnt));
				TResult E2 = (sum / static_cast<TCount>(cnt));
				return E1 - (E2 * E2);
			} else {
				TResult E1 = (sum * sum / static_cast<TCount>(cnt));
				return (sumSquare - E1) / static_cast<TCount>(cnt - 1);
			}
		}
		return {};
	}

	/**
	 * @brief Consumer that calculates the standard deviation of all elements of this iterator.
	 * @details The standard deviation is calculated using @see variance().
	 * @note This consumes the iterator.
	 * @return The standard deviation of all elements of this iterator.
	 * @tparam NORM Type of the statistical normalization variant to use for the
	 * calculation. @see StatisticNormalization
	 * @tparam TResult Type of the stddev-calculation's result. This is also the type used
	 * for the sum of all elements.
	 * @tparam TCount Type the element counter is converted into, before dividing the sum and
	 * the squared sum by. This can be necessary, if TResult is a complex object that only supports
	 * the division operator for e.g. double.
	 *
	 * Usage Example:
	 * - For iterators with at least 2 values:
	 * @code
	 *	std::vector<float> input = {2.0f, 4.0f, 4.0f, 4.0f, 5.0f, 5.0f, 7.0f, 9.0f};
	 *	std::optional<float> output = CXXIter::from(input).stddev();
	 *	// output == Some(2.0f)
	 * @endcode
	 * - For iterators with at least 2 values with (N-1) norm:
	 * @code
	 *	std::vector<float> input = {1.0f, 2.0f, 3.0f};
	 *	std::optional<float> output = CXXIter::from(input)
	 *		.stddev<CXXIter::StatisticNormalization::N_MINUS_ONE>();
	 *	// output == Some(1.0f)
	 * @endcode
	 * - For iterators with less than 2 values (not defined):
	 * @code
	 *	std::vector<float> input = {42.0f};
	 *	std::optional<float> output = CXXIter::from(input).stddev();
	 *	// output == None
	 * @endcode
	 */
	template<StatisticNormalization NORM = StatisticNormalization::N, typename TResult = ItemOwned, typename TCount = ItemOwned>
	std::optional<TResult> stddev() {
		std::optional<TResult> result = variance<NORM, TResult, TCount>();
		if(result.has_value()) { return std::sqrt(result.value()); }
		return {};
	}

	/**
	 * @brief Consumer that yields the smallest element from this iterator.
	 * @note This consumes the iterator.
	 * @return A CXXIter::IterValue optional either containing the smallest element of this iterator (if any),
	 * or empty otherwise.
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *	std::vector<int> input = {42, 1337, 52};
	 *	std::optional<int> output = CXXIter::from(input)
	 *		.min().toStdOptional();
	 *	// output == Some(42)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<int> input = {};
	 *	std::optional<int> output = CXXIter::from(input)
	 *		.min().toStdOptional();
	 *	// output == None
	 * @endcode
	 */
	IterValue<Item> min() {
		return minBy([](auto&& item) { return item; });
	}

	/**
	 * @brief Consumer that yields the index of the smallest element within this iterator.
	 * @note This consumes the iterator.
	 * @return Index of the smallest element within the input iterator (if any).
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *	std::vector<int> input = {1337, 42, 52};
	 *	std::optional<size_t> output = CXXIter::from(input).minIdx();
	 *	// output == Some(1)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<int> input = {};
	 *	std::optional<size_t> output = CXXIter::from(input).minIdx();
	 *	// output == None
	 * @endcode
	 */
	std::optional<size_t> minIdx() {
		return minIdxBy([](auto&& item) { return item; });
	}

	/**
	 * @brief Consumer that yields the largest element from this iterator.
	 * @note This consumes the iterator.
	 * @return A CXXIter::IterValue optional either containing the largest element of this iterator (if any),
	 * or empty otherwise.
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *	std::vector<int> input = {42, 1337, 52};
	 *	std::optional<int> output = CXXIter::from(input)
	 *		.max().toStdOptional();
	 *	// output == Some(1337)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<int> input = {};
	 *	std::optional<int> output = CXXIter::from(input)
	 *		.max().toStdOptional();
	 *	// output == None
	 * @endcode
	 */
	IterValue<Item> max() {
		return maxBy([](auto&& item) { return item; });
	}

	/**
	 * @brief Consumer that yields the index of the largest element within this iterator.
	 * @note This consumes the iterator.
	 * @return Index of the largest element within the input iterator (if any).
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *	std::vector<int> input = {42, 1337, 52};
	 *	std::optional<size_t> output = CXXIter::from(input).maxIdx();
	 *	// output == Some(1)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<int> input = {};
	 *	std::optional<size_t> output = CXXIter::from(input).maxIdx();
	 *	// output == None
	 * @endcode
	 */
	std::optional<size_t> maxIdx() {
		return maxIdxBy([](auto&& item) { return item; });
	}

	/**
	 * @brief Consumer that yields the smallest element from this iterator. Comparison of items is done
	 * using the comparison values returned by invoking the given @p compValueExtractFn on each element.
	 * @note This consumes the iterator.
	 * @param compValueExtractFn Function that, given an element from the input iterator as parameter returns
	 * the value by which the item should be compared to others.
	 * @return A CXXIter::IterValue optional either containing the smallest element of this iterator (if any),
	 * or empty otherwise.
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 * 	std::vector<std::string> input = {"smol", "middle", "largeString"};
	 * 	std::optional<std::string> output = CXXIter::from(input)
	 * 		.minBy([](const std::string& str) { return str.size(); })
	 *		.toStdOptional();
	 *	// output == Some("smol")
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 * 	std::vector<std::string> input = {};
	 * 	std::optional<std::string> output = CXXIter::from(input)
	 * 		.minBy([](const std::string& str) { return str.size(); })
	 *		.toStdOptional();
	 *	// output == None
	 * @endcode
	 */
	template<typename TCompValueExtractFn>
	requires requires(const std::invoke_result_t<TCompValueExtractFn, Item&&>& a, std::remove_cvref_t<decltype(a)> ownedA) {
		{ a < a };
		{ ownedA = ownedA };
	}
	IterValue<Item> minBy(TCompValueExtractFn compValueExtractFn) {
		IterValue<Item> result = Iterator::next(*self());
		if(!result.has_value()) { return {}; }
		auto resultValue = compValueExtractFn(std::forward<Item>(result.value()));
		forEach([&result, &resultValue, &compValueExtractFn](Item&& item) {
			auto itemValue = compValueExtractFn(std::forward<Item>(item));
			if(itemValue < resultValue) {
				result = item;
				resultValue = itemValue;
			}
		});
		return result;
	}

	/**
	 * @brief Consumer that yields the index of the smallest element from this iterator. Comparison of items is done
	 * using the comparison values returned by invoking the given @p compValueExtractFn on each element.
	 * @note This consumes the iterator.
	 * @param compValueExtractFn Function that, given an element from the input iterator as parameter, returns
	 * the value by which the item should be compared to others.
	 * @return Index of the smallest element within the input iterator (if any).
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *  const std::vector<std::string> input = {"middle", "smol", "largeString"};
	 *  std::optional<size_t> output = CXXIter::SrcCRef(input)
	 *  	.minIdxBy([](const std::string& str) { return str.size(); });
	 *  // output = Some(1)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *  const std::vector<std::string> input = {};
	 *  std::optional<size_t> output = CXXIter::SrcCRef(input)
	 *  	.minIdxBy([](const std::string& str) { return str.size(); });
	 *  // output = None
	 * @endcode
	 */
	template<typename TCompValueExtractFn>
	requires requires(const std::invoke_result_t<TCompValueExtractFn, Item&&>& a) {
		{ a < a };
	}
	std::optional<size_t> minIdxBy(TCompValueExtractFn compValueExtractFn) {
		IterValue<Item> tmp = Iterator::next(*self());
		if(!tmp.has_value()) { return {}; }
		size_t iterationIdx = 1, minIdx = 0;
		auto minValue = compValueExtractFn(std::forward<Item>(tmp.value()));
		forEach([iterationIdx, &minIdx, &minValue, &compValueExtractFn](Item&& item) mutable {
			auto itemValue = compValueExtractFn(std::forward<Item>(item));
			if(itemValue < minValue) {
				minValue = itemValue;
				minIdx = iterationIdx;
			}
			iterationIdx += 1;
		});
		return minIdx;
	}

	/**
	 * @brief Consumer that yields the largest element from this iterator. Comparison of items is done
	 * using the comparison values returned by invoking the given @p compValueExtractFn on each element.
	 * @note This consumes the iterator.
	 * @param compValueExtractFn Function that, given an element from the input iterator as parameter returns
	 * the value by which the item should be compared to others.
	 * @return A CXXIter::IterValue optional either containing the largest element of this iterator (if any),
	 * or empty otherwise.
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 * 	std::vector<std::string> input = {"smol", "middle", "largeString"};
	 * 	std::optional<std::string> output = CXXIter::from(input)
	 * 		.maxBy([](const std::string& str) { return str.size(); })
	 *		.toStdOptional();
	 *	// output == Some("largeString")
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 * 	std::vector<std::string> input = {};
	 * 	std::optional<std::string> output = CXXIter::from(input)
	 * 		.maxBy([](const std::string& str) { return str.size(); })
	 *		.toStdOptional();
	 *	// output == None
	 * @endcode
	 */
	template<typename TMaxValueExtractFn>
	requires requires(const std::invoke_result_t<TMaxValueExtractFn, Item&&>& a, std::remove_cvref_t<decltype(a)> ownedA) {
		{ a > a };
		{ ownedA = ownedA };
	}
	IterValue<Item> maxBy(TMaxValueExtractFn compValueExtractFn) {
		IterValue<Item> result = Iterator::next(*self());
		if(!result.has_value()) { return {}; }
		auto resultValue = compValueExtractFn(std::forward<Item>(result.value()));
		forEach([&result, &resultValue, &compValueExtractFn](Item&& item) {
			auto itemValue = compValueExtractFn(std::forward<Item>(item));
			if(itemValue > resultValue) {
				result = item;
				resultValue = itemValue;
			}
		});
		return result;
	}

	/**
	 * @brief Consumer that yields the index of the largest element from this iterator. Comparison of items is done
	 * using the comparison values returned by invoking the given @p TCompValueExtractFn on each element.
	 * @note This consumes the iterator.
	 * @param compValueExtractFn Function that, given an element from the input iterator as parameter, returns
	 * the value by which the item should be compared to others.
	 * @return Index of the largest element within the input iterator (if any).
	 *
	 * Usage Example:
	 * - For a non-empty iterator
	 * @code
	 *  const std::vector<std::string> input = {"middle", "largeString", "smol"};
	 *  std::optional<size_t> output = CXXIter::SrcCRef(input)
	 *  	.maxIdxBy([](const std::string& str) { return str.size(); });
	 *  // output = Some(1)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *  const std::vector<std::string> input = {};
	 *  std::optional<size_t> output = CXXIter::SrcCRef(input)
	 *  	.maxIdxBy([](const std::string& str) { return str.size(); });
	 *  // output = None
	 * @endcode
	 */
	template<typename TMaxValueExtractFn>
	requires requires(const std::invoke_result_t<TMaxValueExtractFn, Item&&>& a, std::remove_cvref_t<decltype(a)> ownedA) {
		{ a > a };
		{ ownedA = ownedA };
	}
	std::optional<size_t> maxIdxBy(TMaxValueExtractFn compValueExtractFn) {
		IterValue<Item> tmp = Iterator::next(*self());
		if(!tmp.has_value()) { return {}; }
		size_t iterationIdx = 1, maxIdx = 0;
		auto maxValue = compValueExtractFn(std::forward<Item>(tmp.value()));
		forEach([iterationIdx, &maxIdx, &maxValue, &compValueExtractFn](Item&& item) mutable {
			auto itemValue = compValueExtractFn(std::forward<Item>(item));
			if(itemValue > maxValue) {
				maxValue = itemValue;
				maxIdx = iterationIdx;
			}
			iterationIdx += 1;
		});
		return maxIdx;
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
	 *	std::optional<int> output = CXXIter::from(input)
	 *		.last()
	 *		.toStdOptional();
	 *	// output == Some(52)
	 * @endcode
	 * - For an empty iterator:
	 * @code
	 *	std::vector<int> input = {};
	 *	std::optional<int> output = CXXIter::from(input)
	 *		.last()
	 *		.toStdOptional();
	 *	// output == None
	 * @endcode
	 */
	IterValue<Item> last() {
		IterValue<Item> tmp;
		forEach([&tmp](Item&& item) { tmp = item; });
		return tmp;
	}

	/**
	 * @brief Return the @p{n}-th element from this iterator (if available).
	 * @param n Index of the element to return from this iterator.
	 * @return The @p{n}-th element from this iterator.
	 *
	 * Usage Example:
	 * - When the n-th element exists:
	 * @code
	 * 	std::vector<int> input = {42, 1337, 52};
	 * 	std::optional<int> output = CXXIter::from(input).nth(1).toStdOptional();
	 *	// output == Some(1337)
	 * @endcode
	 * - When the n-th element does not exist:
	 * @code
	 * 	std::vector<int> input = {42, 1337, 52};
	 * 	std::optional<int> output = CXXIter::from(input).nth(10).toStdOptional();
	 *	// output == None
	 * @endcode
	 */
	IterValue<Item> nth(size_t n) {
		return skip(n).next();
	}
//@}


	// ###################
	// CHAINERS
	// ###################
	/**
	 * @name Chainers
	 */
//@{

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
	op::Caster<TSelf, TItemOutput> cast() {
		return op::Caster<TSelf, TItemOutput>(std::move(*self()));
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
		return map([](const ItemOwned& item) -> ItemOwned {
			ItemOwned copy = item;
			return copy;
		});
	}

	/**
	 * @brief Constructs a new iterator that tags each element of this iterator with the corresponding index,
	 * stored in a @c std::pair.
	 * @return A new iterator whose elements are @c std::pair with an element index in the first, and the
	 * original iterator's corresponding element in the second slot.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::vector<std::pair<size_t, std::string&>> output = CXXIter::from(input)
	 * 		.indexed()
	 * 		.collect<std::vector>();
	 *	// output == {{0, "1337"}, {1, "42"}, {2, "64"}}
	 * @endcode
	 */
	auto indexed() {
		size_t idx = 0;
		return map([idx](Item&& item) mutable -> std::pair<size_t, Item> {
			return std::pair<size_t, Item>(idx++, std::forward<Item>(item));
		});
	}

	/**
	 * @brief Constructs a new iterator that tags each element with a boolean value specifying whether the
	 * element is the last one in the iterator. Boolean and actual iterator element are stored in a @c std::pair.
	 * @return A new iterator whose elements are @c std::pair with the iterator element in the first, and a boolean
	 * flag specifying whether the element will be the last one in the second slot.
	 *
	 * Usage Example:
	 * - Flag last element in filtered iterator
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "420", "64"};
	 * 	std::vector<std::pair<std::string&, bool>> output = CXXIter::from(input)
	 * 		.filter([](const std::string& el) { return el.size() >= 3; })
	 * 		.flagLast()
	 * 		.collect<std::vector>();
	 *	// output == {{"1337", false}, {"420", true}}
	 * @endcode
	 * - Use last flag to filter (remove last element from iterator)
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42", "64"};
	 * 	std::vector<std::pair<std::string&, bool>> output = CXXIter::from(input)
	 * 			.flagLast()
	 * 			.filter([](const std::pair<std::string&, bool>& el) { return !el.second; })
	 * 			.collect<std::vector>();
	 *	// output == {{"1337", false}, {"42", false}}
	 * @endcode
	 */
	op::FlagLast<TSelf> flagLast() {
		return op::FlagLast<TSelf>(std::move(*self()));
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
	op::Filter<TSelf, TFilterFn> filter(TFilterFn filterFn) {
		return op::Filter<TSelf, TFilterFn>(std::move(*self()), filterFn);
	}

	/**
	 * @brief Constructs a new iterator that only contains every element of the input iterator only once.
	 * @details This variant checks whether the data returned by the given @p mapFn when invoked with the input's
	 * item is unique. This requires the data returned by @p mapFn to be hashable using @c std::hash.
	 * @param mapFn Function that maps the input's element to data that should be used in the uniqueness-check.
	 * @return Iterator that does not contain duplicate elements from the input iterator's elements.
	 * @attention Unique requires extra data storage to remember what items it has already seen. This
	 * leads to additional memory usage.
	 *
	 * Usage Example:
	 * @code
	 *  std::vector<double> input = {1.0, 1.0, 1.5, 1.4, 2.0, 2.1, 2.99, 3.25, 4.5};
	 *  std::vector<double> output = CXXIter::from(input)
	 * 		.unique([](double item) { return std::floor(item); })
	 * 		.copied()
	 * 		.collect<std::vector>();
	 *  // output == { 1.0, 2.0, 3.25, 4.5 }
	 * @endcode
	 */
	template<std::invocable<const ItemOwned&> TMapFn>
	requires util::is_hashable<std::invoke_result_t<TMapFn, const ItemOwned&>>
	op::Unique<TSelf, TMapFn> unique(TMapFn mapFn) {
		return op::Unique<TSelf, TMapFn>(std::move(*self()), mapFn);
	}

	/**
	 * @brief Constructs a new iterator that only contains every element of the input iterator only once.
	 * @details This variant uses the input elements directly for the uniqueness-comparison.
	 * This requires the input elements to be hashable using @c std::hash.
	 * @return Iterator that does not contain duplicate elements from the input iterator's elements.
	 * @attention Unique requires extra data storage to remember what items it has already seen. This
	 * leads to additional memory usage.
	 *
	 * Usage Example:
	 * @code
	 *  std::vector<double> input = {1.0, 1.0, 1.5, 1.4, 2.0, 2.1, 2.99, 3.25, 4.5};
	 *  std::vector<double> output = CXXIter::from(input)
	 * 		.unique()
	 * 		.copied()
	 * 		.collect<std::vector>();
	 *  // output == { 1.0, 1.5, 1.4, 2.0, 2.1, 2.99, 3.25, 4.5 }
	 * @endcode
	 */
	auto unique() {
		return unique([](const auto& item) { return item; });
	}

	/**
	 * @brief Constructs a new iterator that provides the elements of this iterator in reverse order.
	 * @attention If the underlying iterator implements DoubleEndedIterator it is going to be used and this operation
	 * will not have an additional cost. If the underlying iterator does not implement DoubleEndedIterator, it will
	 * be emulated by first draining the iterator into a container and then providing those elements in reverse.
	 * This leads to additional memory usage.
	 * @return Iterator that provides the elements of this iterator in reverse order.
	 *
	 * Usage Example:
	 * @code
	 *  std::vector<size_t> input = {1, 42, 2, 1337, 3, 4, 69, 5, 6, 5};
	 *  std::vector<size_t> output = CXXIter::from(input).copied()
	 *		.reverse()
	 *		.collect<std::vector>();
	 *  // output == { 5, 6, 5, 69, 4, 3, 1337, 2, 42, 1 }
	 * @endcode
	 */
	op::Reverse<TSelf> reverse() {
		return op::Reverse<TSelf>(std::move(*self()));
	}

	/**
	 * @brief Create new iterator that collects elements from this iterator in chunks of size up to @p CHUNK_SIZE, which then
	 * constitue the elements of the new iterator.
	 * @details Chunks are of up to @p CHUNK_SIZE elements in size. If the amount of items in the iterator are not dividable
	 * by the requested @p CHUNK_SIZE the last chunk will be smaller.
	 * @tparam CHUNK_SIZE Amount of elements from this iterator, that get collected to one chunk.
	 * @return New iterator that contains chunks of up to the requested size, containing elements from this iterator as elements.
	 *
	 * Usage Example:
	 * - If the amount of elements of the input can be evenly divided up into the requested @p CHUNK_SIZE :
	 * @code
	 * 	std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
	 * 	auto output = CXXIter::from(input)
	 * 			.copied()
	 * 			.chunked<3>()
	 * 			.collect<std::vector>();
	 * 	// output == { {1337, 42, 512}, {31337, 69, 5}, {1, 2, 3} }
	 * @endcode
	 * - If the amount of elements of the input can **not** be evenly divided up into the requested @p CHUNK_SIZE :
	 * @code
	 * 	std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
	 * 	auto output = CXXIter::from(input)
	 * 			.copied()
	 * 			.chunked<3>()
	 * 			.collect<std::vector>();
	 * 	// output == { {1337, 42, 512}, {31337, 69, 5}, {1, 2} }
	 * @endcode
	 */
	template<const size_t CHUNK_SIZE>
	op::Chunked<TSelf, CHUNK_SIZE> chunked() {
		return op::Chunked<TSelf, CHUNK_SIZE>(std::move(*self()));
	}

	/**
	 * @brief Create new iterator that collects elements from this iterator in exact-sized chunks of @p CHUNK_SIZE, which then
	 * constitue the elements of the new iterator.
	 * @details A chunk is only committed in the new iterator, after it was filled completely. That means, that if the amount
	 * of elements in this iterator do not evenly divide up to @p CHUNK_SIZE sized chunks, the last couple of elements that
	 * fail to fill a complete chunk will be dropped.
	 * @tparam CHUNK_SIZE Amount of elements from this iterator, that get collected to one chunk.
	 * @tparam STEP_SIZE Controls the step-size between chunks. Per default, this is set to the same as @p CHUNK_SIZE, so the produced
	 * chunks are directly adjacent. If this is set to a value smaler than @p CHUNK_SIZE, the generated chunks will overlap. If
	 * this is set to a value higher than @p CHUNK_SIZE, the generated chunks will have gaps in between (those items are dropped).
	 * @return New iterator that contains exact-sized (@p CHUNK_SIZE) chunks of elements from this iterator as elements.
	 *
	 * Usage Example:
	 * - If the amount of elements of the input can be evenly divided up into the requested @p CHUNK_SIZE :
	 * @code
	 * 	std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2, 3};
	 * 	auto output = CXXIter::from(input)
	 * 			.copied()
	 * 			.chunkedExact<3>()
	 * 			.collect<std::vector>();
	 * 	// output == { {1337, 42, 512}, {31337, 69, 5}, {1, 2, 3} }
	 * @endcode
	 * - If the amount of elements of the input can **not** be evenly divided up into the requested @p CHUNK_SIZE :
	 * @code
	 * 	std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1, 2};
	 * 	auto output = CXXIter::from(input)
	 * 			.copied()
	 * 			.chunkedExact<3>()
	 * 			.collect<std::vector>();
	 * 	// output == { {1337, 42, 512}, {31337, 69, 5} }
	 * @endcode
	 * - Overlapping chunks (STEP_SIZE < CHUNK_SIZE):
	 * @code
	 * 	std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5};
	 * 	auto output = CXXIter::from(input)
	 * 			.copied()
	 * 			.chunkedExact<3, 1>()
	 * 			.collect<std::vector>();
	 * 	// output == { {1337, 42, 512}, {42, 512, 31337}, {512, 31337, 69}, {31337, 69, 5} }
	 * @endcode
	 * - Gapped Chunks (STEP_SIZE > CHUNK_SIZE):
	 * @code
	 * 	std::vector<size_t> input = {1337, 42, 512, 31337, 69, 5, 1};
	 * 	auto output = CXXIter::from(input)
	 * 			.copied()
	 * 			.chunkedExact<3, 4>()
	 * 			.collect<std::vector>();
	 * 	// output == { {1337, 42, 512}, {69, 5, 1} }
	 * @endcode
	 */
	template<const size_t CHUNK_SIZE, const size_t STEP_SIZE = CHUNK_SIZE>
	op::ChunkedExact<TSelf, CHUNK_SIZE, STEP_SIZE> chunkedExact() {
		return op::ChunkedExact<TSelf, CHUNK_SIZE, STEP_SIZE>(std::move(*self()));
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
	template<std::invocable<Item&&> TMapFn>
	auto map(TMapFn mapFn) {
		using TMapFnResult = std::invoke_result_t<TMapFn, Item&&>;
		return op::Map<TSelf, TMapFn, TMapFnResult>(std::move(*self()), mapFn);
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
	auto flatMap(TFlatMapFn mapFn) {
		using TFlatMapFnResult = std::invoke_result_t<TFlatMapFn, Item&&>;
		return op::FlatMap<TSelf, TFlatMapFn, TFlatMapFnResult>(std::move(*self()), mapFn);
	}

#ifdef CXXITER_HAS_COROUTINE
	/**
	 * @brief Creates a new iterator containing the items that the given generator produces for each element
	 * in this iterator.
	 * @details Conceptually, this method is very similar to flatMap() since it allows to take one element
	 * from this iterator, and returning an arbitrary amount of new elements into the resulting iterator.
	 * A big difference is, that with generateFrom(), elements can be produced on the fly using c++20
	 * coroutines, while with flatMap() they need to be present in a supported container at once - taking up memory.
	 * The given @p generatorFn is run for each element in this iterator, producing a generator.
	 * This generator is then driven to completion, piping every element it produced into the resulting iterator.
	 * @param generatorFn Generator function that is executed for each element of this iterator. This function
	 * can use co_yield to produce as many elements as it wants. Its return value has to be explicitly specified
	 * as CXXIter::Generator with the generated type as template parameter.
	 * @note Returning references from the generator is supported. Make sure your references stay valid until
	 * they are read, though.
	 * @attention Special care has to be taken with respect to the argument types of the given @p generatorFn.
	 * The generator must take the elements of the stream by-value (copied). If the elements in the stream are
	 * moved through the stream, the @p generatorFn must take them as their owned type. If the elements are
	 * passed as references through the stream, the @p generatorFn can take them as references.
	 * If you are getting spurious SEGFAULTs - check your parameter types!
	 *
	 * Usage Example:
	 *
	 * The example shows a generator that repeats the strings from the source, depending on the string's
	 * lengths. Special attention in these examples should be mainly on the parameter types, as well as the
	 * explicitly specified return values of the given generator functions.
	 *
	 * - Using generateFrom() with a move source, that passes elements by move
	 *   (generator clones elements and passes them on as owned clones)\n
	 * Here, the type of the elements passed through the iterator are owned @c std::string by move.
	 * So the type the generator has to take as parameter is an owned @p std::string.
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42"};
	 * 	std::vector<std::string> output = CXXIter::from(std::move(input))
	 * 			.generateFrom([](std::string item) -> CXXIter::Generator<std::string> {
	 * 				for(size_t i = 0; i < item.size(); ++i) {
	 * 					co_yield item;
	 * 				}
	 * 			})
	 * 			.collect<std::vector>();
	 * 	// output == { "1337", "1337", "1337", "1337", "42", "42" }
	 * @endcode
	 * - Using generateFrom() with a reference source, that passes elements as references
	 *   (generator clones elements and passes them on as owned clones)\n
	 * Here, the type of the elements passed through the iterator are const @c std::string references.
	 * So the type the generator takes can either be a const @c std::string reference (because they don't
	 * reference something temporary, but are references from the permanent source) - or as an owned @p std::string.
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42"};
	 * 	std::vector<std::string> output = CXXIter::from(input)
	 * 			.generateFrom([](const std::string& item) -> CXXIter::Generator<std::string> {
	 * 				for(size_t i = 0; i < item.size(); ++i) {
	 * 					co_yield item;
	 * 				}
	 * 			})
	 * 			.collect<std::vector>();
	 * 	// output == { "1337", "1337", "1337", "1337", "42", "42" }
	 * @endcode
	 * - Using generateFrom() with a reference source, that passes elements as references
	 *   (generator clones references to elements - and passes on the copied references)\n
	 * @code
	 * 	std::vector<std::string> input = {"1337", "42"};
	 * 	std::vector<std::string> output = CXXIter::from(input)
	 * 			.generateFrom([](const std::string& item) -> CXXIter::Generator<const std::string&> {
	 * 				for(size_t i = 0; i < item.size(); ++i) {
	 * 					co_yield item;
	 * 				}
	 * 			})
	 * 			.collect<std::vector>();
	 * 	// output == { "1337", "1337", "1337", "1337", "42", "42" }
	 * @endcode
	 */
	template<GeneratorFromFunction<Item> TGeneratorFn>
	auto generateFrom(TGeneratorFn generatorFn) {
		using TGeneratorFnResult = std::invoke_result_t<TGeneratorFn, Item>;
		return op::GenerateFrom<TSelf, TGeneratorFn, TGeneratorFnResult>(std::move(*self()), generatorFn);
	}
#endif

	/**
	 * @brief Creates an iterator that flattens the iterable elements of this iterator.
	 * @details This works by pulling elements from this iterator, turning them into iterators
	 * themselves, and merging them into the stream of the resulting iterator.
	 * This only resolves one layer of nesting, and the elements of this iterator have to
	 * be supported by CXXIter (by a fitting @c SourceTrait implementation).
	 * @return New iterator that pulls values from this iterator, and flattens the contained
	 * iterable into the new iterator's stream.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::vector<int>> input = {{1337, 42}, {6, 123, 7888}};
	 * 	std::vector<int> output = CXXIter::from(std::move(input))
	 * 		.flatMap()
	 * 		.collect<std::vector>(); // collect into vector containing {1337, 42, 6, 123, 7888}
	 * @endcode
	 */
	auto flatMap() {
		return flatMap([](Item&& item) { return item; });
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
	op::InplaceModifier<TSelf, TModifierFn> modify(TModifierFn modifierFn) {
		return op::InplaceModifier<TSelf, TModifierFn>(std::move(*self()), modifierFn);
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
	requires util::is_optional<std::invoke_result_t<TFilterMapFn, ItemOwned&&>>
	auto filterMap(TFilterMapFn filterMapFn) {
		using TFilterMapFnResult = typename std::invoke_result_t<TFilterMapFn, ItemOwned&&>::value_type;
		return op::FilterMap<TSelf, TFilterMapFn, TFilterMapFnResult>(std::move(*self()), filterMapFn);
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
	op::SkipN<TSelf> skip(size_t cnt) {
		return op::SkipN<TSelf>(std::move(*self()), cnt);
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
	op::SkipWhile<TSelf, TSkipPredicate> skipWhile(TSkipPredicate skipPredicate) {
		return op::SkipWhile<TSelf, TSkipPredicate>(std::move(*self()), skipPredicate);
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
	op::TakeN<TSelf> take(size_t cnt) {
		return op::TakeN<TSelf>(std::move(*self()), cnt);
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
	requires std::is_same_v<std::invoke_result_t<TTakePredicate, const Item&>, bool>
	auto takeWhile(TTakePredicate takePredicate) {
		return op::TakeWhile<TSelf, TTakePredicate>(std::move(*self()), takePredicate);
	}

	/**
	 * @brief Creates an iterator with the requested @p stepWidth from this iterator.
	 * @details A step width of @c 1 is a NO-OP, a step width of @c 2 means that every second
	 * element is skipped. The first element is always returned, irrespecting of the requested @p stepWidth.
	 * @param step Step width with which elements from this iterator are yielded.
	 * @return New iterator with the requested @p stepWidth
	 *
	 * Usage Example:
	 * - Step width of 1 (No-Op):
	 * @code
	 * 	std::vector<int> input = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	 * 	std::vector<int> output = CXXIter::from(input)
	 * 			.stepBy(1)
	 * 			.collect<std::vector>();
	 *	// output == {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
	 * @endcode
	 * - Step width of 2:
	 * @code
	 * 	std::vector<int> input = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	 * 	std::vector<int> output = CXXIter::from(input)
	 * 			.stepBy(2)
	 * 			.collect<std::vector>();
	 *	// output == {0, 2, 4, 6, 8, 10}
	 * @endcode
	 */
	auto stepBy(size_t stepWidth) {
		//TODO: better SizeHints?
		size_t idx = 0;
		return filter([idx, stepWidth](const ItemOwned&) mutable {
			return (idx++ % stepWidth) == 0;
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
	op::Zipper<TSelf, std::pair, TOtherIterator> zip(TOtherIterator&& otherIterator) {
		return op::Zipper<TSelf, std::pair, TOtherIterator>(std::move(*self()), std::forward<TOtherIterator>(otherIterator));
	}

	/**
	 * @brief "Zips up" an arbitrary amount of CXXIter iterators into a single iterator over @c std::tuple<> from both iterators.
	 * @details Constructs new iterator that iterates over @c std::tuple<> instances where values from this
	 * iterator are put in the first value, and values from the given @p otherIterators are stored after that in order.
	 * The resulting iterator is only as long as the shortest of all iterators part of the zip.
	 * @param otherIterators Other iterators zipped against this iterator.
	 * @return New iterator that zips together this iteratore and the given @p otherIterators into a new iterator
	 * over @c std::tuple<> for all of the zipped iterator's values.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::string> input1 = {"1337", "42"};
	 * 	std::vector<int> input2 = {1337, 42, 80};
	 * 	std::vector<float> input3 = {1337.0f, 42.0f, 64.0f};
	 * 	std::vector<std::tuple<std::string, int, float>> output = CXXIter::from(input1).copied()
	 * 			.zipTuple(CXXIter::from(input2).copied(), CXXIter::from(input3).copied())
	 * 			.collect<std::vector>();
	 *	// output == { {"1337", 1337, 1337.0f}, {"42", 42, 42.0f} }
	 * @endcode
	 */
	template<typename... TOtherIterators>
	requires (CXXIterIterator<TOtherIterators> && ...)
			&& (!std::disjunction_v< std::is_reference<typename trait::Iterator<TOtherIterators>::Item>... > && !IS_REFERENCE)
	op::Zipper<TSelf, std::tuple, TOtherIterators...> zipTuple(TOtherIterators&&... otherIterators) {
		return op::Zipper<TSelf, std::tuple, TOtherIterators...>(std::move(*self()), std::forward<TOtherIterators>(otherIterators)...);
	}

	/**
	 * @brief Chains this iterator with the given @p otherIterator, resulting in a new iterator that first yields
	 * the elements of this iterator, and then the ones from the @p otherIterator.
	 * @param otherIterator Other iterator whose elements should be "appended" to the elements of this iterator.
	 * @return New iterator that consists of a chain of this iterator with the given @p otherIterator.
	 * @note For this to work, the elements' types of this iterator and the given @p otherIterator have to be identical.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<std::string> input1 = {"1337", "42"};
	 * 	std::vector<std::string> input2 = {"31337", "64"};
	 * 	std::vector<std::string> output = CXXIter::from(input1).copied()
	 * 			.chain(CXXIter::from(input2).copied())
	 * 			.collect<std::vector>();
	 *	// output == {"1337", "42", "31337", "64"}
	 * @endcode
	 */
	template<typename TOtherIterator>
	requires std::is_same_v<Item, typename TOtherIterator::Item>
	op::Chainer<TSelf, TOtherIterator> chain(TOtherIterator&& otherIterator) {
		return op::Chainer<TSelf, TOtherIterator>(std::move(*self()), std::forward<TOtherIterator>(otherIterator));
	}

	/**
	 * @brief Alternating the elements of this iterator with the ones from the other given iterator(s).
	 * @details Everytime an element is polled from the iterator resulting from this call, an element from the
	 * current input iterator is forwarded. Then, the current input iterator is switched to the next input.
	 * The resulting iterator ends, when the currently active input has no more elements.
	 * @param otherIterators An arbitrary amount of iterators to alternate the elements of this iterator with.
	 * @return A new iterator that interweaves the elements from this iterator and all the given iterators in order.
	 *
	 * Usage Example:
	 * @code
	 * 	std::vector<int> input1 = {1, 4, 7};
	 * 	std::vector<int> input2 = {2, 5};
	 * 	std::vector<int> input3 = {3, 6, 9};
	 * 	std::vector<int> output = CXXIter::from(input1)
	 * 		.alternate(CXXIter::from(input2), CXXIter::from(input3))
	 * 		.collect<std::vector>();
	 *	// output == {1, 2, 3, 4, 5, 6, 7}
	 * @endcode
	 */
	template<typename... TOtherIterators>
	requires (CXXIterIterator<TOtherIterators> && ...)
			&& (util::are_same_v<Item, typename TOtherIterators::Item...>)
	op::Alternater<TSelf, TOtherIterators...> alternate(TOtherIterators&&... otherIterators) {
		return op::Alternater<TSelf, TOtherIterators...>(std::move(*self()), std::forward<TOtherIterators>(otherIterators)...);
	}

	/**
	 * @brief Draw elements from the given @p otherIterator and use the returned elements as separators between
	 * the elements of this iterator.
	 * @details This draws one element "into the future" of this iterator, in order to determine if another
	 * separator element from the given @p otherIterator is required.
	 * The resulting iterator ends if either this iterator or the @p otherIterator has no more elements to pull.
	 * The resulting iterator will always start and end on an element from this iterator.
	 * @param otherIterator Iterator whose elements will be inserted as separator elements between the elements
	 * of this iterator.
	 * @return New iterator that uses the given @p otherIterator's elements as separators between this iterator's
	 * elements.
	 *
	 * Usage Example:
	 * - Using infinite separator iterator (int)
	 * @code
	 * 	std::vector<int> input = { 1, 2, 3, 4, 5, 6 };
	 * 	std::vector<int> output = CXXIter::from(input).copied()
	 * 		.intersperse(CXXIter::repeat(0))
	 * 		.collect<std::vector>();
	 * 	// output == {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6}
	 * @endcode
	 * - Using infinite separator iterator (string)
	 * @code
	 * 	std::vector<std::string> input = { "Apple", "Orange", "Cake" };
	 * 	std::vector<std::string> output = CXXIter::from(input).copied()
	 * 		.intersperse(CXXIter::repeat<std::string>(", "))
	 * 		.collect<std::vector>();
	 * 	// output == {"Apple", ", ", "Orange", ", ", "Cake"}
	 * @endcode
	 * - Using finite separator iterator that ends earlier than source iterator
	 * @code
	 * 	std::vector<int> input = { 1, 2, 3, 4, 5, 6 };
	 * 	std::vector<int> output = CXXIter::from(input).copied()
	 * 		.intersperse(CXXIter::range(100, 102, 1))
	 * 		.collect<std::vector>();
	 * 	// output == {1, 100, 2, 101, 3, 102, 4}
	 * @endcode
	 */
	template<typename TOtherIterator>
	requires (std::is_same_v<Item, typename TOtherIterator::Item>)
	op::Intersperser<TSelf, TOtherIterator> intersperse(TOtherIterator&& otherIterator) {
		return op::Intersperser<TSelf, TOtherIterator>(std::move(*self()), std::forward<TOtherIterator>(otherIterator));
	}

	/**
	 * @brief Groups the elements of this iterator according to the values returned by the given @p groupidentFn.
	 * @param groupIdentFn Function called for each element from this iterator, to determine the grouping value,
	 * that is then used to identify the group an item belongs to. The type returned by this function has to
	 * implement @c std::hash<>.
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
	requires util::is_hashable<std::invoke_result_t<TGroupIdentifierFn, const Item&>>
	auto groupBy(TGroupIdentifierFn groupIdentFn) {
		using TGroupIdent = std::remove_cvref_t<std::invoke_result_t<TGroupIdentifierFn, const ItemOwned&>>;
		return op::GroupBy<TSelf, TGroupIdentifierFn, TGroupIdent>(std::move(*self()), groupIdentFn);
	}

	/**
	 * @brief Creates a new iterator that takes the items from this iterator, and passes them on sorted, using
	 * the supplied @p compareFn.
	 * @param compareFn Compare function used for the sorting of items.
	 * @return New iterator that returns the items of this iterator sorted using the given @p compareFn.
	 * @attention Sorter requires to first drain the input iterator, before being able to supply a single element.
	 * This leads to additional memory usage.
	 * @tparam STABLE If @c true, uses @c std::stable_sort internally, if @c false uses @c std::sort
	 *
	 * Usage Example:
	 * - Sorting in ascending order using a custom comparer:
	 * @code
	 * 	std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
	 * 	std::vector<float> output = CXXIter::from(input)
	 * 		.sort<false>([](const float& a, const float& b) {
	 * 			return (a < b);
	 * 		})
	 * 		.collect<std::vector>();
	 * @endcode
	 * - Sorting in descending order using a custom comparer:
	 * @code
	 * 	std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
	 * 	std::vector<float> output = CXXIter::from(input)
	 * 		.sort<false>([](const float& a, const float& b) {
	 * 			return (a > b);
	 * 		})
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	template<bool STABLE, std::invocable<const ItemOwned&, const ItemOwned&> TCompareFn>
	auto sort(TCompareFn compareFn) {
		return op::Sorter<TSelf, TCompareFn, STABLE>(std::move(*self()), compareFn);
	}

	/**
	 * @brief Creates a new iterator that takes the items from this iterator, and passes them on sorted.
	 * @note This variant of sort() requires the items to support comparison operators.
	 * @return New iterator that returns the items of this iterator sorted.
	 * @attention Sorter requires to first drain the input iterator, before being able to supply a single element.
	 * This leads to additional memory usage.
	 * @tparam ORDER Decides the sort order of the resulting iterator.
	 * @tparam STABLE If @c true, uses @c std::stable_sort internally, if @c false uses @c std::sort
	 *
	 * Usage Example:
	 * - Sorting in ascending order using a custom comparer:
	 * @code
	 * 	std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
	 * 	std::vector<float> output = CXXIter::from(input)
	 * 		.sort<CXXIter::ASCENDING, false>()
	 * 		.collect<std::vector>();
	 * @endcode
	 * - Sorting in descending order using a custom comparer:
	 * @code
	 * 	std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
	 * 	std::vector<float> output = CXXIter::from(input)
	 * 		.sort<CXXIter::DESCENDING, false>()
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	template<SortOrder ORDER = SortOrder::ASCENDING, bool STABLE = false>
	requires requires(const ItemOwned& a) { { a < a }; { a > a }; }
	auto sort() {
		return sort<STABLE>([](const ItemOwned& a, const ItemOwned& b) {
			if constexpr(ORDER == SortOrder::ASCENDING) {
				return (a < b);
			} else {
				return (a > b);
			}
		});
	}

	/**
	 * @brief Creates a new iterator that takes the items from this iterator, and passes them on sorted.
	 * @details In comparison to sort(), which either uses a custom comparator or the items themselves
	 * for the sort operation, this variant takes a @p sortValueExtractFn, which extracts a value for
	 * each item in this iterator, that should be used for sorting comparisons.
	 * @return New iterator that returns the items of this iterator sorted.
	 * @attention Sorter requires to first drain the input iterator, before being able to supply a single element.
	 * This leads to additional memory usage.
	 * @tparam ORDER Decides the sort order of the resulting iterator.
	 * @tparam STABLE If @c true, uses @c std::stable_sort internally, if @c false uses @c std::sort
	 *
	 * Usage Example:
	 * - Sorting the items(strings) in ascending order of their length:
	 * @code
	 * 	std::vector<std::string> input = {"test1", "test2", "test23", "test", "tes"};
	 * 	std::vector<std::string> output = CXXIter::from(input)
	 * 		.sortBy<CXXIter::ASCENDING, true>([](const std::string& item) { return item.size(); })
	 * 		.collect<std::vector>();
	 * @endcode
	 * - Sorting the items(strings) in descending order of their length:
	 * @code
	 * 	std::vector<std::string> input = {"test1", "test2", "test23", "test", "tes"};
	 * 	std::vector<std::string> output = CXXIter::from(input)
	 * 		.sortBy<CXXIter::DESCENDING, true>([](const std::string& item) { return item.size(); })
	 * 		.collect<std::vector>();
	 * @endcode
	 */
	template<SortOrder ORDER = SortOrder::ASCENDING, bool STABLE = false, std::invocable<const ItemOwned&> TSortValueExtractFn>
	requires requires(const std::invoke_result_t<TSortValueExtractFn, const ItemOwned&>& a) {
		{ a < a }; { a > a };
	}
	auto sortBy(TSortValueExtractFn sortValueExtractFn) {
		return sort<STABLE>([sortValueExtractFn](const ItemOwned& a, const ItemOwned& b) {
			if constexpr(ORDER == SortOrder::ASCENDING) {
				return (sortValueExtractFn(a) < sortValueExtractFn(b));
			} else {
				return (sortValueExtractFn(a) > sortValueExtractFn(b));
			}
		});
	}
//@}
};



// ################################################################################################
// CONVENIENT ENTRY POINTS
// ################################################################################################

/**
 * @brief Construct a CXXIter move source from the given container.
 * @details This constructs a move source, which will move the items from the
 * given @p container into the iterator.
 * @param container Container to construct a CXXIter source from.
 * @return CXXIter move source from the given container.
 */
template<typename TContainer>
requires (!std::is_reference_v<TContainer> && !util::is_const_reference_v<TContainer> && SourceContainer<TContainer>)
SrcMov<std::remove_cvref_t<TContainer>> from(TContainer&& container) {
	return SrcMov<std::remove_cvref_t<TContainer>>(std::forward<TContainer>(container));
}

/**
 * @brief Construct a CXXIter mutable-reference source from the given container.
 * @details This constructs a mutable-reference source. This allows the iterator
 * to modify the elements in the given @p container.
 * @param container Container to construct a CXXIter source from.
 * @return CXXIter mutable-reference source from the given container.
 */
template<typename TContainer>
requires (!std::is_reference_v<TContainer> && !util::is_const_reference_v<TContainer> && SourceContainer<TContainer>)
SrcRef<std::remove_cvref_t<TContainer>> from(TContainer& container) {
	return SrcRef<std::remove_cvref_t<TContainer>>(container);
}

/**
 * @brief Construct a CXXIter const-reference source from the given container.
 * @details This constructs a const-reference source. This guarantees the
 * given @p container to stay untouched.
 * @param container Container to construct a CXXIter source from.
 * @return CXXIter const-reference source from the given container.
 */
template<typename TContainer>
requires (!std::is_reference_v<TContainer> && !util::is_const_reference_v<TContainer> && SourceContainer<TContainer>)
SrcCRef<std::remove_cvref_t<TContainer>> from(const TContainer& container) {
	return SrcCRef<std::remove_cvref_t<TContainer>>(container);
}

/**
 * @brief Constructs an empty iterator yielding no items.
 * @return An empty iterator that yields no items.
 *
 * Usage Example:
 * @code
 * 	CXXIter::IterValue<std::string> output = CXXIter::empty<std::string>()
 * 		.next();
 * 	// output == None
 * @endcode
 */
template<typename TItem>
Empty<TItem> empty() { return Empty<TItem>(); }

/**
 * @brief Generator source that takes a @p generatorFn, each invocation of which produces one
 * element for the resulting iterator.
 * @param generatorFn Generator that returns an optional value. If the optional is None, the resulting
 * iterator ends.
 * @return CXXIter iterator whose elements are produced by the calls to the given @p generatorFn.
 * @details You could for example also use this to pull messages from a socket.
 *
 * Usage Example:
 * - Simple endless generator producing monotonically increasing numbers
 * @code
 * 	size_t generatorState = 0;
 * 	std::function<std::optional<size_t>()> generatorFn = [generatorState]() mutable {
 * 		return (generatorState++);
 * 	};
 * 	std::vector<size_t> output = CXXIter::fromFn(generatorFn)
 * 		.take(100)
 * 		.collect<std::vector>();
 *	// output == {0, 1, 2, 3, ..., 99}
 * @endcode
 */
template<std::invocable<> TGeneratorFn>
requires util::is_optional<std::invoke_result_t<TGeneratorFn>>
auto fromFn(TGeneratorFn generatorFn) {
	using TGeneratorFnResult = typename std::invoke_result_t<TGeneratorFn>::value_type;
	return FunctionGenerator<TGeneratorFnResult, TGeneratorFn>(generatorFn);
}

#ifdef CXXITER_HAS_COROUTINE
/**
 * @brief Generator source that produces a new iterator over the elements produced by the
 * given @p generatorFn - which is a c++20 coroutine yielding elements using @c co_yield.
 * @param generatorFn C++20 generator coroutine function yielding elements using @c co_yield.
 * The function must produces a generator of type CXXIter::Generator whose template parameter
 * is set to the produced element type.
 * @return CXXIter iterator over the elements produced by the c++20 coroutine given in @p generatorFn.
 *
 * Usage Example:
 * - Simple generator example producing all integer numbers from 0 to 1000 as @p std::string
 * @code
 * 	std::vector<std::string> output = CXXIter::generate(
 * 		[]() -> CXXIter::Generator<std::string> {
 * 			for(size_t i = 0; i < 1000; ++i) {
 * 				co_yield std::to_string(i);
 * 			}
 * 		}
 * 	).collect<std::vector>();
 *	// output == {0, 1, 2, ..., 1000}
 * @endcode
 */
template<GeneratorFunction TGeneratorFn>
auto generate(TGeneratorFn generatorFn) {
	using TGenerator = typename std::invoke_result_t<TGeneratorFn>;
	TGenerator generator = generatorFn();
	return CoroutineGenerator<TGenerator>(std::forward<TGenerator>(generator));
}
#endif

/**
 * @brief Construct a CXXIter iterator, by repeating the given @p item @p cnt times.
 * @param item Item to use as repeated element of the generated element.
 * @param cnt Optional amount of repetitions of @p item the generated iterator should consist of.
 * If none, the iterator will repeat the item forever.
 * @return CXXIter iterator that returns the given @p item @p cnt times.
 *
 * Usage Example:
 * @code
 * 	std::vector<int> item = {1, 3, 3, 7};
 * 	std::vector<int> output = CXXIter::repeat(item, 3)
 * 		.flatMap()
 * 		.collect<std::vector>();
 *	// output == {1, 3, 3, 7, 1, 3, 3, 7, 1, 3, 3, 7}
 * @endcode
 */
template<typename TItem>
Repeater<TItem> repeat(const TItem& item, std::optional<size_t> cnt = {}) {
	return Repeater<TItem>(item, cnt);
}

/**
 * @brief Construct a CXXIter iterator that yields all elements in the range between
 * [@p from, @p to] (inclusive both edges), using the given @p step between elements.
 * @param from Start of the range of elements to generate.
 * @param to End of the range of elements to generate.
 * @param step Stepwidth to use between the generated elements.
 * @return CXXIter iterator returning elements from the requested range [@p from, @p to]
 * using the given @p step width.
 *
 * Usage Example:
 * - For an integer type:
 * @code
 * 	std::vector<int> output = CXXIter::range(1, 7, 2)
 * 		.collect<std::vector>();
 * 	// output == {1, 3, 5, 7}
 * @endcode
 * - For a float type
 * @code
 * 	std::vector<float> output = CXXIter::range(0.0f, 1.1f, 0.25f)
 * 		.collect<std::vector>();
 * 	// output == {0.0f, 0.25f, 0.5f, 0.75f, 1.0f}
 * @endcode
 */
template<typename TValue>
Range<TValue> range(TValue from, TValue to, TValue step = 1) {
	return Range<TValue>(from, to, step);
}

}
