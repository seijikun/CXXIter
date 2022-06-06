#pragma once

#include <cstddef>

#include "../Traits.h"

/** @private */
namespace CXXIter::util {

	/**
	 * @private
	 * @brief Internal advanceBy() implementation that eagerly pulls @p n elements from the pipeline.
	 * @param self Iterator pipeline.
	 * @param n Amounts to advance by.
	 * @return Amount of elements by which the iterator was actually advanced.
	 */
	template<typename TSelf>
	static inline size_t advanceByPull(TSelf& self, size_t n) {
		size_t skipN = 0;
		while(skipN < n) {
			trait::Iterator<TSelf>::next(self);
			skipN += 1;
		}
		return skipN;
	}

	/**
	 * @private
	 * @brief Internal advanceBy() implementation that eagerly pulls @p n elements from the pipeline.
	 * @param self Iterator pipeline.
	 * @param n Amounts to advance by.
	 * @return Amount of elements by which the iterator was actually advanced.
	 */
	template<typename TSelf>
	static inline size_t advanceByPullBack(TSelf& self, size_t n) {
		size_t skipN = 0;
		while(skipN < n) {
			trait::Iterator<TSelf>::nextBack(self);
			skipN += 1;
		}
		return skipN;
	}

}
