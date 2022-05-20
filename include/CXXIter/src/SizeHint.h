#pragma once

#include <optional>
#include <limits>
#include <cmath>

namespace CXXIter {

	/**
	* @brief Structure holding the bounds of a CXXIter iterator's estimated length.
	* @details This structure contains a lowerBound and an optional upper bound.
	* Both are initialized from the source's length (if any), and subsequently edited
	* by chained iteration accordingly.
	*/
	struct SizeHint {
		constexpr static size_t INFINITE = std::numeric_limits<size_t>::max();

		size_t lowerBound;
		std::optional<size_t> upperBound;

		size_t expectedResultSize(size_t min = 0) const { return std::min(min, upperBound.value_or(lowerBound)); }

		SizeHint(size_t lowerBound = 0, std::optional<size_t> upperBound = {}) : lowerBound(lowerBound), upperBound(upperBound) {}

		static std::optional<size_t> upperBoundMax(std::optional<size_t> upperBound1, std::optional<size_t> upperBound2) {
			if(!upperBound1.has_value() || !upperBound2.has_value()) { return {}; } // no upperbound is like Infinity -> higher
			return std::max(upperBound1.value(), upperBound2.value());
		}
		static std::optional<size_t> upperBoundMin(std::optional<size_t> upperBound1, std::optional<size_t> upperBound2) {
			if(!upperBound1.has_value()) { return upperBound2; }
			if(!upperBound2.has_value()) { return upperBound1; }
			return std::min(upperBound1.value(), upperBound2.value());
		}
		void subtract(size_t cnt) {
			lowerBound = (lowerBound > cnt) ? (lowerBound - cnt) : 0;
			if(upperBound) {
				upperBound = (upperBound.value() > cnt) ? (upperBound.value() - cnt) : 0;
			}
		}
		void add(const SizeHint& o) {
			lowerBound += o.lowerBound;
			if(upperBound.has_value() && o.upperBound.has_value()) {
				upperBound = upperBound.value() + o.upperBound.value();
			} else {
				upperBound = {};
			}
		}
	};

}
