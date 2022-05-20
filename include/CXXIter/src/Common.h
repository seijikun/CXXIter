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
		(std::is_same_v<typename trait::IteratorTrait<T>::Self, T>) &&
		requires(typename trait::IteratorTrait<T>::Self& self, const typename trait::IteratorTrait<T>::Self& constSelf) {
			typename trait::IteratorTrait<T>::Self;
			typename trait::IteratorTrait<T>::Item;
			{trait::IteratorTrait<T>::next(self)} -> std::same_as<IterValue<typename trait::IteratorTrait<T>::Item>>;
			{trait::IteratorTrait<T>::sizeHint(constSelf)} -> std::same_as<SizeHint>;
	};

	template<typename T>
	concept CXXIterExactSizeIterator = CXXIterIterator<T> && requires(const typename trait::IteratorTrait<T>::Self& self) {
			{trait::ExactSizeIteratorTrait<T>::size(self)} -> std::same_as<size_t>;
	};

	template<CXXIterIterator TSelf> class IterApi;

	/**
	* @brief Concept that checks whether the given @p TContainer is supported by CXXIter's standard source
	* classes @c CXXIter::SrcMov, @c CXXIter::SrcRef and @c CXXIter::SrcCRef.
	* @details The concept does these checks by testing whether the @c CXXIter::SourceTrait was properly specialized
	* for the given @p TContainer type.
	*
	* @see CXXIter::SourceTrait for further details on this.
	*/
	template<typename TContainer>
	concept SourceContainer = requires(
			TContainer& container,
			const TContainer& constContainer,
			typename trait::SourceTrait<TContainer>::IteratorState& iterState,
			typename trait::SourceTrait<TContainer>::ConstIteratorState& constIterState
		) {
		typename trait::SourceTrait<TContainer>::Item;
		typename trait::SourceTrait<TContainer>::ItemRef;
		typename trait::SourceTrait<TContainer>::ItemConstRef;
		typename trait::SourceTrait<TContainer>::IteratorState;
		typename trait::SourceTrait<TContainer>::ConstIteratorState;

		{trait::SourceTrait<TContainer>::initIterator(container)} -> std::same_as<typename trait::SourceTrait<TContainer>::IteratorState>;
		{trait::SourceTrait<TContainer>::initIterator(constContainer)} -> std::same_as<typename trait::SourceTrait<TContainer>::ConstIteratorState>;

		{trait::SourceTrait<TContainer>::hasNext(container, iterState)} -> std::same_as<bool>;
		{trait::SourceTrait<TContainer>::hasNext(constContainer, constIterState)} -> std::same_as<bool>;

		{trait::SourceTrait<TContainer>::next(container, iterState)} -> std::same_as<typename trait::SourceTrait<TContainer>::ItemRef>;
		{trait::SourceTrait<TContainer>::next(constContainer, constIterState)} -> std::same_as<typename trait::SourceTrait<TContainer>::ItemConstRef>;
	};

}
