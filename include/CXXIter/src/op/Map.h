#pragma once

#include <utility>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// MAP
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TMapFn, typename TItem>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Map : public IterApi<Map<TChainInput, TMapFn, TItem>> {
			friend struct trait::Iterator<Map<TChainInput, TMapFn, TItem>>;
			friend struct trait::DoubleEndedIterator<Map<TChainInput, TMapFn, TItem>>;
			friend struct trait::ExactSizeIterator<Map<TChainInput, TMapFn, TItem>>;
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
	struct trait::Iterator<op::Map<TChainInput, TMapFn, TItem>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
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
	template<CXXIterDoubleEndedIterator TChainInput, typename TMapFn, typename TItem>
	struct trait::DoubleEndedIterator<op::Map<TChainInput, TMapFn, TItem>> {
		using ChainInputIterator = trait::DoubleEndedIterator<TChainInput>;
		using InputItemOwned = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = op::Map<TChainInput, TMapFn, TItem>;
		using Item = TItem;

		static inline IterValue<Item> nextBack(Self& self) {
			auto item = ChainInputIterator::nextBack(self.input);
			return item.template map<Item, TMapFn&>(self.mapFn);
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TMapFn, typename TItem>
	struct trait::ExactSizeIterator<op::Map<TChainInput, TMapFn, TItem>> {
		static inline size_t size(const op::Map<TChainInput, TMapFn, TItem>& self) { return trait::ExactSizeIterator<TChainInput>::size(self.input); }
	};

}
