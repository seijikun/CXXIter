#pragma once

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// FILTERMAP
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TFilterMapFn, typename TItem>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] FilterMap : public IterApi<FilterMap<TChainInput, TFilterMapFn, TItem>> {
			friend struct trait::Iterator<FilterMap<TChainInput, TFilterMapFn, TItem>>;
			friend struct trait::DoubleEndedIterator<FilterMap<TChainInput, TFilterMapFn, TItem>>;
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
	struct trait::Iterator<op::FilterMap<TChainInput, TFilterMapFn, TItem>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
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
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
	/** @private */
	template<CXXIterDoubleEndedIterator TChainInput, typename TFilterMapFn, typename TItem>
	struct trait::DoubleEndedIterator<op::FilterMap<TChainInput, TFilterMapFn, TItem>> {
		using ChainInputIterator = trait::DoubleEndedIterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::FilterMap<TChainInput, TFilterMapFn, TItem>;
		using Item = TItem;

		static inline IterValue<Item> nextBack(Self& self) {
			while(true) {
				auto item = ChainInputIterator::nextBack(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; }
				std::optional<Item> value(self.filterMapFn(std::forward<InputItem>( item.value() )));
				if(!value) { continue; }
				return *value;
			}
		}
	};

}
