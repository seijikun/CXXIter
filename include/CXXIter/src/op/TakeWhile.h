#pragma once

#include <utility>
#include <optional>
#include <algorithm>

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// TAKE WHILE
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TTakePredicate>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] TakeWhile : public IterApi<TakeWhile<TChainInput, TTakePredicate>> {
			friend struct trait::Iterator<TakeWhile<TChainInput, TTakePredicate>>;
		private:
			TChainInput input;
			TTakePredicate takePredicate;
		public:
			constexpr TakeWhile(TChainInput&& input, TTakePredicate takePredicate) : input(std::move(input)), takePredicate(takePredicate) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TTakePredicate>
	struct trait::Iterator<op::TakeWhile<TChainInput, TTakePredicate>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::TakeWhile<TChainInput, TTakePredicate>;
		using Item = InputItem;

		static constexpr inline IterValue<Item> next(Self& self) {
			auto item = ChainInputIterator::next(self.input);
			if(!item.has_value()) [[unlikely]] { return {}; }
			// end iterator directly after takePredicate returned false the first time
			if(self.takePredicate(item.value()) == false) { return {}; }
			return item;
		}
		static constexpr inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
		static constexpr inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};

}
