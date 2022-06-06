#pragma once

#include <cstdlib>
#include <optional>

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// SKIP WHILE
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TSkipPredicate>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] SkipWhile : public IterApi<SkipWhile<TChainInput, TSkipPredicate>> {
			friend struct trait::Iterator<SkipWhile<TChainInput, TSkipPredicate>>;
		private:
			TChainInput input;
			TSkipPredicate skipPredicate;
			bool skipEnded = false;
		public:
			SkipWhile(TChainInput&& input, TSkipPredicate skipPredicate) : input(std::move(input)), skipPredicate(skipPredicate) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TSkipPredicate>
	struct trait::Iterator<op::SkipWhile<TChainInput, TSkipPredicate>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::SkipWhile<TChainInput, TSkipPredicate>;
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
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};

}
