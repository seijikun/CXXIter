#pragma once

#include <limits>
#include <optional>
#include <algorithm>
#include <type_traits>

#include "IterValue.h"
#include "SizeHint.h"
#include "Traits.h"
#include "util/Constraints.h"

namespace CXXIter {

	// ################################################################################################
	// PUBLIC STRUCTURES AND DEFINITIONS
	// ################################################################################################

	/**
	* @brief SortOrder for the item sorting methods.
	*/
	enum class SortOrder {
		ASCENDING,
		DESCENDING
	};
	/** Shortcut for SortOrder::ASCENDING in the CXXIter namespace */
	static constexpr SortOrder ASCENDING = SortOrder::ASCENDING;
	/** Shortcut for SortOrder::DESCENDING in the CXXIter namespace */
	static constexpr SortOrder DESCENDING = SortOrder::DESCENDING;

	/**
	 * @brief Normalization variant to use while calculating statistics (mean / stddev / ...)
	 */
	enum class StatisticNormalization {
		/**
		 * @brief Use when the mean, variance, stddev is calculated with the COMPLETE population
		 */
		N,
		/**
		 * @brief Use when the mean, variance, stddev is calculated from a sample of the population
		 */
		N_MINUS_ONE
	};



	// ################################################################################################
	// INTERNALS
	// ################################################################################################

	/** @private */
	namespace {
		#define CXXITER_CHAINER_NODISCARD_WARNING "The result of chainer methods needs to be used, otherwise the iterator will not be doing any work."

		template<size_t START, size_t END, typename F>
		constexpr bool constexpr_for(F&& f) {
			if constexpr (START < END) {
				if(f(std::integral_constant<size_t, START>()) == false) { return false; }
				if(constexpr_for<START + 1, END>(f) == false) { return false; }
			}
			return true;
		}

	}

	template<typename T>
	concept CXXIterIterator =
		(std::is_same_v<typename trait::Iterator<T>::Self, T>) &&
		requires(typename trait::Iterator<T>::Self& self, const typename trait::Iterator<T>::Self& constSelf, size_t n) {
			typename trait::Iterator<T>::Self;
			typename trait::Iterator<T>::Item;
			{trait::Iterator<T>::next(self)} -> std::same_as<IterValue<typename trait::Iterator<T>::Item>>;
			{trait::Iterator<T>::sizeHint(constSelf)} -> std::same_as<SizeHint>;
			{trait::Iterator<T>::advanceBy(self, n)} -> std::same_as<size_t>;
	};

	template<typename T>
	concept CXXIterDoubleEndedIterator = CXXIterIterator<T> && requires(typename trait::Iterator<T>::Self& self) {
		{trait::DoubleEndedIterator<T>::nextBack(self)} -> std::same_as<IterValue<typename trait::Iterator<T>::Item>>;
	};

	template<typename T>
	concept CXXIterExactSizeIterator = CXXIterIterator<T> && requires(const typename trait::Iterator<T>::Self& self) {
		{trait::ExactSizeIterator<T>::size(self)} -> std::same_as<size_t>;
	};

	template<CXXIterIterator TSelf> class IterApi;

	/**
	* @brief Concept that checks whether the given @p TContainer is supported by CXXIter's standard source
	* classes CXXIter::SrcMov, CXXIter::SrcRef and CXXIter::SrcCRef.
	* @details The concept does these checks by testing whether the CXXIter::trait::Source was properly specialized
	* for the given @p TContainer type.
	*
	* @see CXXIter::SourceTrait for further details on this.
	*/
	template<typename TContainer>
	concept SourceContainer = requires(
			TContainer& container,
			const TContainer& constContainer,
			typename trait::Source<TContainer>::IteratorState& iterState,
			typename trait::Source<TContainer>::ConstIteratorState& constIterState,
			size_t n
		) {
		typename trait::Source<TContainer>::Item;
		typename trait::Source<TContainer>::ItemRef;
		typename trait::Source<TContainer>::ItemConstRef;
		typename trait::Source<TContainer>::IteratorState;
		typename trait::Source<TContainer>::ConstIteratorState;

		{trait::Source<TContainer>::initIterator(container)} -> std::same_as<typename trait::Source<TContainer>::IteratorState>;
		{trait::Source<TContainer>::initIterator(constContainer)} -> std::same_as<typename trait::Source<TContainer>::ConstIteratorState>;

		{trait::Source<TContainer>::hasNext(container, iterState)} -> std::same_as<bool>;
		{trait::Source<TContainer>::hasNext(constContainer, constIterState)} -> std::same_as<bool>;

		{trait::Source<TContainer>::next(container, iterState)} -> std::same_as<typename trait::Source<TContainer>::ItemRef>;
		{trait::Source<TContainer>::next(constContainer, constIterState)} -> std::same_as<typename trait::Source<TContainer>::ItemConstRef>;

		{trait::Source<TContainer>::skipN(container, iterState, n)} -> std::same_as<size_t>;
		{trait::Source<TContainer>::skipN(container, constIterState, n)} -> std::same_as<size_t>;
	};

	/**
	 * @brief Concept that checks whether the given @p TContainer supports double-ended iteration when using CXXIter's
	 * standard source classes CXXIter::SrcMov, CXXIter::SrcRef and CXXIter::SrcCRef.
	 * @details The concept does these checks by testing whether the optional double-ended part in CXXIter::trait::Source
	 * was properly provided by the specialization for the given @p TContainer type.
	 *
	 * @see CXXIter::Source for further details on this.
	 */
	template<typename TContainer>
	concept DoubleEndedSourceContainer = SourceContainer<TContainer> && requires(
				TContainer& container,
				const TContainer& constContainer,
				typename trait::Source<TContainer>::IteratorState& iterState,
				typename trait::Source<TContainer>::ConstIteratorState& constIterState,
				size_t n
			) {

		{trait::Source<TContainer>::nextBack(container, iterState)} -> std::same_as<typename trait::Source<TContainer>::ItemRef>;
		{trait::Source<TContainer>::nextBack(constContainer, constIterState)} -> std::same_as<typename trait::Source<TContainer>::ItemConstRef>;

		{trait::Source<TContainer>::skipNBack(container, iterState, n)} -> std::same_as<size_t>;
		{trait::Source<TContainer>::skipNBack(container, constIterState, n)} -> std::same_as<size_t>;
	};

}
