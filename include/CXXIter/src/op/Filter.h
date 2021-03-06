#pragma once

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// FILTER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TFilterFn>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Filter : public IterApi<Filter<TChainInput, TFilterFn>> {
			friend struct trait::Iterator<Filter<TChainInput, TFilterFn>>;
			friend struct trait::DoubleEndedIterator<Filter<TChainInput, TFilterFn>>;
		private:
			using InputItem = typename TChainInput::Item;

			TChainInput input;
			TFilterFn filterFn;
		public:
			constexpr Filter(TChainInput&& input, TFilterFn filterFn) : input(std::move(input)), filterFn(filterFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TFilterFn>
	struct trait::Iterator<op::Filter<TChainInput, TFilterFn>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		// CXXIter Interface
		using Self = op::Filter<TChainInput, TFilterFn>;
		using Item = typename ChainInputIterator::Item;

		static constexpr inline IterValue<Item> next(Self& self) {
			while(true) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; }
				if(self.filterFn(item.value())) { return item; }
			}
		}
		static constexpr inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
		static constexpr inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
	/** @private */
	template<CXXIterDoubleEndedIterator TChainInput, typename TFilterFn>
	struct trait::DoubleEndedIterator<op::Filter<TChainInput, TFilterFn>> {
		using ChainInputIterator = trait::DoubleEndedIterator<TChainInput>;
		// CXXIter Interface
		using Self = op::Filter<TChainInput, TFilterFn>;
		using Item = typename ChainInputIterator::Item;

		static constexpr inline IterValue<Item> nextBack(Self& self) {
			while(true) {
				auto item = ChainInputIterator::nextBack(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; }
				if(self.filterFn(item.value())) { return item; }
			}
		}
	};

}
