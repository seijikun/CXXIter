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

	template<typename T>
	concept CXXIterContiguousMemoryIterator = CXXIterExactSizeIterator<T>
		&& requires(typename trait::Iterator<T>::Self& self) {
		typename trait::ContiguousMemoryIterator<T>::ItemPtr;
		{trait::ContiguousMemoryIterator<T>::currentPtr(self)} -> std::same_as<typename trait::ContiguousMemoryIterator<T>::ItemPtr>;
	};

	template<CXXIterIterator TSelf> class IterApi;

}
