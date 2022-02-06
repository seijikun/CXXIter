#pragma once

#include <utility>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// TAKE WHILE
	// ################################################################################################
	/** @private */
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
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput>
	struct IteratorTrait<TakeN<TChainInput>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		// CXXIter Interface
		using Self = TakeN<TChainInput>;
		using Item = typename TChainInput::Item;

		static inline Item next(Self& self) {
			if(self.remaining == 0) [[unlikely]] { throw IteratorEndedException{}; }
			Item item = ChainInputIterator::next(self.input);
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
	struct ExactSizeIteratorTrait<TakeN<TChainInput>> {
		static inline size_t size(const TakeN<TChainInput>& self) {
			return IteratorTrait<TakeN<TChainInput>>::sizeHint(self).lowerBound;
		}
	};
}
