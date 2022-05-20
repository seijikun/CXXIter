#pragma once

#include <cstdlib>
#include <optional>

#include "../Common.h"
#include "../sources/ContainerSources.h"

namespace CXXIter {

	// ################################################################################################
	// SORTER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TCompareFn, bool STABLE>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Sorter : public IterApi<Sorter<TChainInput, TCompareFn, STABLE>> {
			friend struct trait::Iterator<Sorter<TChainInput, TCompareFn, STABLE>>;
			friend struct trait::DoubleEndedIterator<Sorter<TChainInput, TCompareFn, STABLE>>;
			friend struct trait::ExactSizeIterator<Sorter<TChainInput, TCompareFn, STABLE>>;
		private:
			using OwnedInputItem = typename TChainInput::ItemOwned;
			using SortCache = SrcMov<std::vector<OwnedInputItem>>;

			TChainInput input;
			TCompareFn compareFn;
			std::optional<SortCache> sortCache;
		public:
			Sorter(TChainInput&& input, TCompareFn compareFn) : input(std::move(input)), compareFn(compareFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TCompareFn, bool STABLE>
	struct trait::Iterator<op::Sorter<TChainInput, TCompareFn, STABLE>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		using OwnedInputItem = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = op::Sorter<TChainInput, TCompareFn, STABLE>;
		using Item = OwnedInputItem;

		static inline void initSortCache(Self& self) {
			// drain input iterator into sortCache
			std::vector<OwnedInputItem> sortCache;
			while(true) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] { break; }
				sortCache.push_back(std::forward<InputItem>( item.value() ));
			}
			// sort the cache
			if constexpr(STABLE) {
				std::stable_sort(sortCache.begin(), sortCache.end(), self.compareFn);
			} else {
				std::sort(sortCache.begin(), sortCache.end(), self.compareFn);
			}
			self.sortCache.emplace(std::move(sortCache));
		}

		static inline IterValue<Item> next(Self& self) {
			if(!self.sortCache.has_value()) [[unlikely]] { initSortCache(self); }

			using SortCacheIterator = trait::Iterator<typename Self::SortCache>;
			typename Self::SortCache& sortedItems = self.sortCache.value();
			return SortCacheIterator::next(sortedItems);
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};
	/** @private */
	template<CXXIterDoubleEndedIterator TChainInput, typename TCompareFn, bool STABLE>
	struct trait::DoubleEndedIterator<op::Sorter<TChainInput, TCompareFn, STABLE>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		using OwnedInputItem = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = op::Sorter<TChainInput, TCompareFn, STABLE>;
		using Item = OwnedInputItem;

		static inline IterValue<Item> nextBack(Self& self) {
			if(!self.sortCache.has_value()) [[unlikely]] { trait::Iterator<Self>::initSortCache(self); }

			using SortCacheIterator = trait::DoubleEndedIterator<typename Self::SortCache>;
			typename Self::SortCache& sortedItems = self.sortCache.value();
			return SortCacheIterator::nextBack(sortedItems);
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TCompareFn, bool STABLE>
	struct trait::ExactSizeIterator<op::Sorter<TChainInput, TCompareFn, STABLE>> {
		static inline size_t size(const op::Sorter<TChainInput, TCompareFn, STABLE>& self) {
			return trait::ExactSizeIterator<TChainInput>::size(self.input);
		}
	};

}
