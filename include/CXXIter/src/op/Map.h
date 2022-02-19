#pragma once

#include <utility>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// MAP
	// ################################################################################################
	/** @private */
	namespace op {
		template<typename TChainInput, typename TMapFn, typename TItem>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Map : public IterApi<Map<TChainInput, TMapFn, TItem>> {
			friend struct IteratorTrait<Map<TChainInput, TMapFn, TItem>>;
			friend struct ExactSizeIteratorTrait<Map<TChainInput, TMapFn, TItem>>;
		private:
			TChainInput input;
			TMapFn mapFn;
		public:
			Map(TChainInput&& input, TMapFn mapFn) : input(std::move(input)), mapFn(mapFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TMapFn, typename TItem>
	struct IteratorTrait<op::Map<TChainInput, TMapFn, TItem>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItemOwned = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = op::Map<TChainInput, TMapFn, TItem>;
		using Item = TItem;

		static inline IterValue<Item> next(Self& self) {
			auto item = ChainInputIterator::next(self.input);
			return item.template map<Item, TMapFn&>(self.mapFn);
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TMapFn, typename TItem>
	struct ExactSizeIteratorTrait<op::Map<TChainInput, TMapFn, TItem>> {
		static inline size_t size(const op::Map<TChainInput, TMapFn, TItem>& self) { return ExactSizeIteratorTrait<TChainInput>::size(self.input); }
	};

}
