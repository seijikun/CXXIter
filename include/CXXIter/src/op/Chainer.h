#pragma once

#include <cstdlib>
#include <utility>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// CHAINER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput1, typename TChainInput2>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Chainer : public IterApi<Chainer<TChainInput1, TChainInput2>> {
			friend struct trait::IteratorTrait<Chainer<TChainInput1, TChainInput2>>;
			friend struct trait::ExactSizeIteratorTrait<Chainer<TChainInput1, TChainInput2>>;
		private:
			TChainInput1 input1;
			TChainInput2 input2;
			size_t inputIdx = 0;
		public:
			Chainer(TChainInput1&& input1, TChainInput2 input2) : input1(std::move(input1)), input2(std::move(input2)) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput1, typename TChainInput2>
	struct trait::IteratorTrait<op::Chainer<TChainInput1, TChainInput2>> {
		using ChainInputIterator1 = trait::IteratorTrait<TChainInput1>;
		using ChainInputIterator2 = trait::IteratorTrait<TChainInput2>;
		using InputItem = typename trait::IteratorTrait<TChainInput1>::Item;
		// CXXIter Interface
		using Self = op::Chainer<TChainInput1, TChainInput2>;
		using Item = InputItem;

		static inline IterValue<Item> next(Self& self) {
			while(true) {
				if(self.inputIdx == 0) {
					auto item = ChainInputIterator1::next(self.input1);
					if(!item.has_value()) [[unlikely]] {
						self.inputIdx = 1;
						continue;
					}
					return item;
				} else {
					auto item = ChainInputIterator2::next(self.input2);
					return item;
				}
			}
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint result = ChainInputIterator1::sizeHint(self.input1);
			result.add(ChainInputIterator2::sizeHint(self.input2));
			return result;
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput1, CXXIterExactSizeIterator TChainInput2>
	struct trait::ExactSizeIteratorTrait<op::Chainer<TChainInput1, TChainInput2>> {
		static inline size_t size(const op::Chainer<TChainInput1, TChainInput2>& self) {
			return trait::IteratorTrait<op::Chainer<TChainInput1, TChainInput2>>::sizeHint(self).lowerBound;
		}
	};

}
