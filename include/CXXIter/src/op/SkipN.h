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
			friend struct trait::ExactSizeIterator<SkipN<TChainInput>>;
			friend struct trait::ContiguousMemoryIterator<SkipN<TChainInput>>;
		private:
			TChainInput input;
			size_t n;
			bool skipEnded = false;
		public:
			constexpr SkipN(TChainInput&& input, size_t n) : input(std::move(input)), n(n) {}
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

		static constexpr inline IterValue<Item> next(Self& self) {
			if(!self.skipEnded) [[unlikely]] { // first call -> skip requested now
				ChainInputIterator::advanceBy(self.input, self.n);
				self.skipEnded = true;
			}
			return ChainInputIterator::next(self.input);
		}
		static constexpr inline SizeHint sizeHint(const Self& self) {
			SizeHint result = ChainInputIterator::sizeHint(self.input);
			result.subtract(self.n);
			return result;
		}
		static constexpr inline size_t advanceBy(Self& self, size_t n) {
			if(!self.skipEnded) [[unlikely]] {
				size_t skippedCnt = ChainInputIterator::advanceBy(self.input, n + self.n);
				self.skipEnded = true;
				if(skippedCnt <= self.n) { return 0; }
				return (skippedCnt - self.n);
			} else {
				return ChainInputIterator::advanceBy(self.input, n);
			}
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput>
	struct trait::ExactSizeIterator<op::SkipN<TChainInput>> {
		static constexpr inline size_t size(const op::SkipN<TChainInput>& self) {
			return trait::Iterator<op::SkipN<TChainInput>>::sizeHint(self).lowerBound;
		}
	};
	/** @private */
	template<CXXIterContiguousMemoryIterator TChainInput>
	struct trait::ContiguousMemoryIterator<op::SkipN<TChainInput>> {
		using ItemPtr = std::add_pointer_t<std::remove_reference_t<typename op::SkipN<TChainInput>::Item>>;
		static constexpr inline ItemPtr currentPtr(op::SkipN<TChainInput>& self) {
			size_t offset = (self.skipEnded) ? 0 : self.n;
			return (trait::ContiguousMemoryIterator<TChainInput>::currentPtr(self.input) + offset);
		}
	};

}
