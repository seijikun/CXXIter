#pragma once

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// FILTER
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TFilterFn>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Filter : public IterApi<Filter<TChainInput, TFilterFn>> {
		friend struct IteratorTrait<Filter<TChainInput, TFilterFn>>;
		friend struct ExactSizeIteratorTrait<Filter<TChainInput, TFilterFn>>;
	private:
		using InputItem = typename TChainInput::Item;

		TChainInput input;
		TFilterFn filterFn;
	public:
		Filter(TChainInput&& input, TFilterFn filterFn) : input(std::move(input)), filterFn(filterFn) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TFilterFn>
	struct IteratorTrait<Filter<TChainInput, TFilterFn>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		// CXXIter Interface
		using Self = Filter<TChainInput, TFilterFn>;
		using Item = typename ChainInputIterator::Item;

		static inline IterValue<Item> next(Self& self) {
			while(true) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; }
				if(self.filterFn(item.value())) { return item; }
			}
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
	};

}