#pragma once

#include <cstdlib>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// SKIP WHILE
	// ################################################################################################
	/** @private */
	template<typename TChainInput>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] SkipN : public IterApi<SkipN<TChainInput>> {
		friend struct IteratorTrait<SkipN<TChainInput>>;
	private:
		TChainInput input;
		size_t n;
		bool skipEnded = false;
	public:
		SkipN(TChainInput&& input, size_t n) : input(std::move(input)), n(n) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput>
	struct IteratorTrait<SkipN<TChainInput>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		// CXXIter Interface
		using Self = SkipN<TChainInput>;
		using Item = typename TChainInput::Item;

		static inline Item next(Self& self) {
			if(!self.skipEnded) [[unlikely]] { // first call -> skip requested now
				for(size_t i = 0; i < self.n; ++i) {
					ChainInputIterator::next(self.input);
				}
				self.skipEnded = true;
			}
			return ChainInputIterator::next(self.input);
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint result = ChainInputIterator::sizeHint(self.input);
			result.subtract(self.n);
			return result;
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput>
	struct ExactSizeIteratorTrait<SkipN<TChainInput>> {
		static inline size_t size(const SkipN<TChainInput>& self) {
			return IteratorTrait<SkipN<TChainInput>>::sizeHint(self).lowerBound;
		}
	};

}
