#pragma once

#include <cstdlib>
#include <optional>

#include "../Common.h"
#include "../sources/ContainerSources.h"

namespace CXXIter {

	// ################################################################################################
	// SORTER
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TCompareFn, bool STABLE>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Sorter : public IterApi<Sorter<TChainInput, TCompareFn, STABLE>> {
		friend struct IteratorTrait<Sorter<TChainInput, TCompareFn, STABLE>>;
		friend struct ExactSizeIteratorTrait<Sorter<TChainInput, TCompareFn, STABLE>>;
	private:
		using OwnedInputItem = typename TChainInput::ItemOwned;
		using SortCache = SrcMov<std::vector<OwnedInputItem>>;

		TChainInput input;
		TCompareFn compareFn;
		std::optional<SortCache> sortCache;
	public:
		Sorter(TChainInput&& input, TCompareFn compareFn) : input(std::move(input)), compareFn(compareFn) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TCompareFn, bool STABLE>
	struct IteratorTrait<Sorter<TChainInput, TCompareFn, STABLE>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		using OwnedInputItem = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = Sorter<TChainInput, TCompareFn, STABLE>;
		using Item = OwnedInputItem;

		static inline IterValue<Item> next(Self& self) {
			if(!self.sortCache.has_value()) {
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

			using SortCacheIterator = IteratorTrait<typename Self::SortCache>;
			typename Self::SortCache& sortedItems = self.sortCache.value();
			return SortCacheIterator::next(sortedItems);
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TCompareFn, bool STABLE>
	struct ExactSizeIteratorTrait<Sorter<TChainInput, TCompareFn, STABLE>> {
		static inline size_t size(const Sorter<TChainInput, TCompareFn, STABLE>& self) {
			return ExactSizeIteratorTrait<TChainInput>::size(self.input);
		}
	};

}
