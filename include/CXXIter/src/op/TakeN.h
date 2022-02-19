#pragma once

#include <utility>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// TAKE WHILE
	// ################################################################################################
	/** @private */
	namespace op {
		template<typename TChainInput>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] TakeN : public IterApi<TakeN<TChainInput>> {
			friend struct IteratorTrait<TakeN<TChainInput>>;
		private:
			TChainInput input;
			size_t n;
			size_t remaining;
		public:
			TakeN(TChainInput&& input, size_t n) : input(std::move(input)), n(n), remaining(n) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput>
	struct IteratorTrait<op::TakeN<TChainInput>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::TakeN<TChainInput>;
		using Item = InputItem;

		static inline IterValue<Item> next(Self& self) {
			if(self.remaining == 0) [[unlikely]] { return {}; }
			auto item = ChainInputIterator::next(self.input);
			if(!item.has_value()) [[unlikely]] { return {}; }
			self.remaining -= 1;
			return item;
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(
				std::min(input.lowerBound, self.n),
				SizeHint::upperBoundMin(input.upperBound, self.n)
			);
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput>
	struct ExactSizeIteratorTrait<op::TakeN<TChainInput>> {
		static inline size_t size(const op::TakeN<TChainInput>& self) {
			return IteratorTrait<op::TakeN<TChainInput>>::sizeHint(self).lowerBound;
		}
	};
}
