#pragma once

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// FILTER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TFilterFn>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Filter : public IterApi<Filter<TChainInput, TFilterFn>> {
			friend struct trait::IteratorTrait<Filter<TChainInput, TFilterFn>>;
			friend struct trait::ExactSizeIteratorTrait<Filter<TChainInput, TFilterFn>>;
		private:
			using InputItem = typename TChainInput::Item;

			TChainInput input;
			TFilterFn filterFn;
		public:
			Filter(TChainInput&& input, TFilterFn filterFn) : input(std::move(input)), filterFn(filterFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TFilterFn>
	struct trait::IteratorTrait<op::Filter<TChainInput, TFilterFn>> {
		using ChainInputIterator = trait::IteratorTrait<TChainInput>;
		// CXXIter Interface
		using Self = op::Filter<TChainInput, TFilterFn>;
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
