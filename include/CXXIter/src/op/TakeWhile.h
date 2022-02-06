#pragma once

#include <utility>
#include <optional>
#include <algorithm>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// TAKE WHILE
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TTakePredicate>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] TakeWhile : public IterApi<TakeWhile<TChainInput, TTakePredicate>> {
		friend struct IteratorTrait<TakeWhile<TChainInput, TTakePredicate>>;
	private:
		TChainInput input;
		TTakePredicate takePredicate;
	public:
		TakeWhile(TChainInput&& input, TTakePredicate takePredicate) : input(std::move(input)), takePredicate(takePredicate) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TTakePredicate>
	struct IteratorTrait<TakeWhile<TChainInput, TTakePredicate>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		// CXXIter Interface
		using Self = TakeWhile<TChainInput, TTakePredicate>;
		using Item = typename TChainInput::Item;

		static inline Item next(Self& self) {
			Item item = ChainInputIterator::next(self.input);
			// end iterator directly after takePredicate returned false the first time
			if(self.takePredicate(item) == false) { throw IteratorEndedException{}; }
			return item;
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
	};

}
