#pragma once

#include <utility>

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// MAP
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] FlagLast : public IterApi<FlagLast<TChainInput>> {
			friend struct trait::Iterator<FlagLast<TChainInput>>;
			friend struct trait::DoubleEndedIterator<FlagLast<TChainInput>>;
			friend struct trait::ExactSizeIterator<FlagLast<TChainInput>>;

			using InputItem = typename TChainInput::Item;
		private:
			TChainInput input;
			bool initialized = false;
			IterValue<InputItem> nextValue;
		public:
			FlagLast(TChainInput&& input) : input(std::move(input)) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput>
	struct trait::Iterator<op::FlagLast<TChainInput>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::FlagLast<TChainInput>;
		using Item = std::pair<InputItem, bool>;

		static inline IterValue<Item> next(Self& self) {
			if(!self.initialized) [[unlikely]] {
				// initialize by populating nextValue with the first element from the input iterator
				self.nextValue = ChainInputIterator::next(self.input);
				self.initialized = true;
			}
			// next value is empty - the previous element was the last one.
			if(!self.nextValue.has_value()) [[unlikely]] { return {}; }

			// Return the current nextValue, but fetch the next one from the input and flag the
			// returned element with a boolean that specifies whether there was a further element.
			Item item = { std::forward<InputItem>(self.nextValue.value()), false };
			self.nextValue = ChainInputIterator::next(self.input);
			item.second = !self.nextValue.has_value();
			return item;
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput>
	struct trait::ExactSizeIterator<op::FlagLast<TChainInput>> {
		static inline size_t size(const op::FlagLast<TChainInput>& self) { return trait::ExactSizeIterator<TChainInput>::size(self.input); }
	};

}
