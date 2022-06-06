#pragma once

#include <cstdlib>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// SKIP WHILE
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] SkipN : public IterApi<SkipN<TChainInput>> {
			friend struct trait::Iterator<SkipN<TChainInput>>;
		private:
			TChainInput input;
			size_t n;
			bool skipEnded = false;
		public:
			SkipN(TChainInput&& input, size_t n) : input(std::move(input)), n(n) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput>
	struct trait::Iterator<op::SkipN<TChainInput>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::SkipN<TChainInput>;
		using Item = InputItem;

		static inline IterValue<Item> next(Self& self) {
			if(!self.skipEnded) [[unlikely]] { // first call -> skip requested now
				for(size_t i = 0; i < self.n; ++i) {
					auto item = ChainInputIterator::next(self.input);
					if(!item.has_value()) [[unlikely]] { return {}; }
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
		static inline size_t advanceBy(Self& self, size_t n) { return ChainInputIterator::advanceBy(self.input, n); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput>
	struct trait::ExactSizeIterator<op::SkipN<TChainInput>> {
		static inline size_t size(const op::SkipN<TChainInput>& self) {
			return trait::Iterator<op::SkipN<TChainInput>>::sizeHint(self).lowerBound;
		}
	};

}
