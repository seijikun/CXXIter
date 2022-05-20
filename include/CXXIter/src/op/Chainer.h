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
			friend struct trait::Iterator<Chainer<TChainInput1, TChainInput2>>;
			friend struct trait::DoubleEndedIterator<Chainer<TChainInput1, TChainInput2>>;
			friend struct trait::ExactSizeIterator<Chainer<TChainInput1, TChainInput2>>;
		private:
			TChainInput1 input1;
			TChainInput2 input2;
			bool input1Ended = false;
			bool input2Ended = false;
		public:
			Chainer(TChainInput1&& input1, TChainInput2 input2) : input1(std::move(input1)), input2(std::move(input2)) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput1, typename TChainInput2>
	struct trait::Iterator<op::Chainer<TChainInput1, TChainInput2>> {
		using ChainInputIterator1 = trait::Iterator<TChainInput1>;
		using ChainInputIterator2 = trait::Iterator<TChainInput2>;
		using InputItem = typename trait::Iterator<TChainInput1>::Item;
		// CXXIter Interface
		using Self = op::Chainer<TChainInput1, TChainInput2>;
		using Item = InputItem;

		static inline IterValue<Item> next(Self& self) {
			while(true) {
				if(self.input1Ended == false) {
					auto item = ChainInputIterator1::next(self.input1);
					if(!item.has_value()) [[unlikely]] {
						self.input1Ended = true;
						continue;
					}
					return item;
				} else {
					auto item = ChainInputIterator2::next(self.input2);
					if(!item.has_value()) [[unlikely]] { self.input2Ended = true; }
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
	template<CXXIterDoubleEndedIterator TChainInput1, CXXIterDoubleEndedIterator TChainInput2>
	struct trait::DoubleEndedIterator<op::Chainer<TChainInput1, TChainInput2>> {
		using ChainInputIterator1 = trait::DoubleEndedIterator<TChainInput1>;
		using ChainInputIterator2 = trait::DoubleEndedIterator<TChainInput2>;
		using InputItem = typename trait::Iterator<TChainInput1>::Item;
		// CXXIter Interface
		using Self = op::Chainer<TChainInput1, TChainInput2>;
		using Item = InputItem;

		static inline IterValue<Item> nextBack(Self& self) {
			while(true) {
				if(self.input2Ended == false) {
					auto item = ChainInputIterator2::nextBack(self.input2);
					if(!item.has_value()) [[unlikely]] {
						self.input2Ended = true;
						continue;
					}
					return item;
				} else {
					auto item = ChainInputIterator1::nextBack(self.input1);
					if(!item.has_value()) [[unlikely]] { self.input1Ended = true; }
					return item;
				}
			}
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput1, CXXIterExactSizeIterator TChainInput2>
	struct trait::ExactSizeIterator<op::Chainer<TChainInput1, TChainInput2>> {
		static inline size_t size(const op::Chainer<TChainInput1, TChainInput2>& self) {
			return trait::Iterator<op::Chainer<TChainInput1, TChainInput2>>::sizeHint(self).lowerBound;
		}
	};

}
