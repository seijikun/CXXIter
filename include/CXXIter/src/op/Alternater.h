#pragma once

#include <tuple>
#include <cstdlib>
#include <optional>
#include <limits>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// ALTERNATER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput1, typename... TChainInputs>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Alternater : public IterApi<Alternater<TChainInput1, TChainInputs...>> {
			friend struct trait::Iterator<Alternater<TChainInput1, TChainInputs...>>;
			friend struct trait::ExactSizeIterator<Alternater<TChainInput1, TChainInputs...>>;
		private:
			static constexpr size_t BATCH_SIZE = 1 + sizeof...(TChainInputs);
			std::tuple<TChainInput1, TChainInputs...> inputs;
			std::array<IterValue<typename TChainInput1::Item>, BATCH_SIZE> currentBatch;
			size_t batchElementIdx = BATCH_SIZE;
		public:
			Alternater(TChainInput1&& input1, TChainInputs&&... inputs) : inputs( std::forward_as_tuple(std::move(input1), std::move(inputs)...) ) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput1, typename... TChainInputs>
	struct trait::Iterator<op::Alternater<TChainInput1, TChainInputs...>> {
		using ChainInputIterators = std::tuple<Iterator<TChainInput1>, trait::Iterator<TChainInputs>...>;
		static constexpr size_t INPUT_CNT = 1 + sizeof...(TChainInputs);
		// CXXIter Interface
		using Self = op::Alternater<TChainInput1, TChainInputs...>;
		using Item = typename TChainInput1::Item;

		static inline IterValue<Item> next(Self& self) {
			if(self.batchElementIdx == Self::BATCH_SIZE) [[unlikely]] { // returned all elements from the batch -> retrieve new batch
				constexpr_for<0, INPUT_CNT>([&](auto idx) {
					self.currentBatch[idx] = std::tuple_element_t<idx, ChainInputIterators>::next( std::get<idx>(self.inputs) );
					return true;
				});
				self.batchElementIdx = 0;
			}
			return std::move(self.currentBatch[self.batchElementIdx++]);
		}
		static inline SizeHint sizeHint(const Self& self) {
			size_t lowerBoundMin = std::numeric_limits<size_t>::max();
			std::optional<size_t> upperBoundMin = {};
			size_t minIdx = 0;
			constexpr_for<0, INPUT_CNT>([&](auto idx) {
				SizeHint tmp = std::tuple_element_t<idx, ChainInputIterators>::sizeHint( std::get<idx>(self.inputs) );
				if(lowerBoundMin > tmp.lowerBound) {
					minIdx = idx;
					lowerBoundMin = tmp.lowerBound;
				}
				upperBoundMin = SizeHint::upperBoundMin(upperBoundMin, tmp.upperBound);
				return true;
			});
			// smallest input iterator defines base length. The amount of elements then is that
			// length, multiplied by the amount of input iterators + the index of the shortest
			if(upperBoundMin.has_value()) { upperBoundMin.value() = upperBoundMin.value() * Self::BATCH_SIZE + minIdx; }
			SizeHint result = SizeHint(
				lowerBoundMin * Self::BATCH_SIZE + minIdx,
				upperBoundMin
			);
			return result;
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput1, CXXIterExactSizeIterator... TChainInputs>
	struct trait::ExactSizeIterator<op::Alternater<TChainInput1, TChainInputs...>> {
		static inline size_t size(const op::Alternater<TChainInput1, TChainInputs...>& self) {
			return trait::Iterator<op::Alternater<TChainInput1, TChainInputs...>>::sizeHint(self).lowerBound;
		}
	};

}
