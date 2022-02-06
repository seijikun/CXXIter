#pragma once

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// FILTERMAP
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TFilterMapFn, typename TItem>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] FilterMap : public IterApi<FilterMap<TChainInput, TFilterMapFn, TItem>> {
		friend struct IteratorTrait<FilterMap<TChainInput, TFilterMapFn, TItem>>;
	private:
		TChainInput input;
		TFilterMapFn filterMapFn;
	public:
		FilterMap(TChainInput&& input, TFilterMapFn filterMapFn) : input(std::move(input)), filterMapFn(filterMapFn) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TFilterMapFn, typename TItem>
	struct IteratorTrait<FilterMap<TChainInput, TFilterMapFn, TItem>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = FilterMap<TChainInput, TFilterMapFn, TItem>;
		using Item = TItem;

		static inline Item next(Self& self) {
			while(true) {
				std::optional<Item> filterMappedItem(self.filterMapFn(ChainInputIterator::next(self.input)));
				if(!filterMappedItem.has_value()) { continue; }
				return *filterMappedItem;
			}
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
	};

}
