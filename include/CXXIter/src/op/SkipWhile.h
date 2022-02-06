#pragma once

#include <cstdlib>
#include <optional>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// SKIP WHILE
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TSkipPredicate>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] SkipWhile : public IterApi<SkipWhile<TChainInput, TSkipPredicate>> {
		friend struct IteratorTrait<SkipWhile<TChainInput, TSkipPredicate>>;
	private:
		TChainInput input;
		TSkipPredicate skipPredicate;
		bool skipEnded = false;
	public:
		SkipWhile(TChainInput&& input, TSkipPredicate skipPredicate) : input(std::move(input)), skipPredicate(skipPredicate) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TSkipPredicate>
	struct IteratorTrait<SkipWhile<TChainInput, TSkipPredicate>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		// CXXIter Interface
		using Self = SkipWhile<TChainInput, TSkipPredicate>;
		using Item = typename TChainInput::Item;

		static inline Item next(Self& self) {
			while(true) {
				Item item = ChainInputIterator::next(self.input);
				if(self.skipEnded) { return item; }
				if(!self.skipPredicate(item)) {
					self.skipEnded = true;
					return item;
				}
			}
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
	};

}
