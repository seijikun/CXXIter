#pragma once

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// FILTERMAP
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TFilterMapFn, typename TItem>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] FilterMap : public IterApi<FilterMap<TChainInput, TFilterMapFn, TItem>> {
			friend struct trait::IteratorTrait<FilterMap<TChainInput, TFilterMapFn, TItem>>;
		private:
			TChainInput input;
			TFilterMapFn filterMapFn;
		public:
			FilterMap(TChainInput&& input, TFilterMapFn filterMapFn) : input(std::move(input)), filterMapFn(filterMapFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TFilterMapFn, typename TItem>
	struct trait::IteratorTrait<op::FilterMap<TChainInput, TFilterMapFn, TItem>> {
		using ChainInputIterator = trait::IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::FilterMap<TChainInput, TFilterMapFn, TItem>;
		using Item = TItem;

		static inline IterValue<Item> next(Self& self) {
			while(true) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; }
				std::optional<Item> value(self.filterMapFn(std::forward<InputItem>( item.value() )));
				if(!value) { continue; }
				return *value;
			}
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
	};

}
