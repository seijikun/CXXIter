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
	namespace op {
		template<typename TChainInput, typename TTakePredicate>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] TakeWhile : public IterApi<TakeWhile<TChainInput, TTakePredicate>> {
			friend struct IteratorTrait<TakeWhile<TChainInput, TTakePredicate>>;
		private:
			TChainInput input;
			TTakePredicate takePredicate;
		public:
			TakeWhile(TChainInput&& input, TTakePredicate takePredicate) : input(std::move(input)), takePredicate(takePredicate) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TTakePredicate>
	struct IteratorTrait<op::TakeWhile<TChainInput, TTakePredicate>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::TakeWhile<TChainInput, TTakePredicate>;
		using Item = InputItem;

		static inline IterValue<Item> next(Self& self) {
			auto item = ChainInputIterator::next(self.input);
			if(!item.has_value()) [[unlikely]] { return {}; }
			// end iterator directly after takePredicate returned false the first time
			if(self.takePredicate(item.value()) == false) { return {}; }
			return item;
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
	};

}
