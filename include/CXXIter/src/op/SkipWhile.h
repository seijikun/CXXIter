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
		std::optional<size_t> cntRequest;
	public:
		SkipWhile(TChainInput&& input, TSkipPredicate skipPredicate, std::optional<size_t> cntRequest = {})
			: input(std::move(input)), skipPredicate(skipPredicate), cntRequest(cntRequest) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TSkipPredicate>
	struct IteratorTrait<SkipWhile<TChainInput, TSkipPredicate>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = SkipWhile<TChainInput, TSkipPredicate>;
		using Item = InputItem;

		static inline IterValue<Item> next(Self& self) {
			while(true) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; }
				if(self.skipEnded) { return item; }
				if(!self.skipPredicate(item.value())) {
					self.skipEnded = true;
					return item;
				}
			}
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint result = ChainInputIterator::sizeHint(self.input);
			if(self.cntRequest.has_value()) {
				result.subtract(self.cntRequest.value());
			} else {
				result.lowerBound = 0;
			}
			return result;
		}
	};

}
