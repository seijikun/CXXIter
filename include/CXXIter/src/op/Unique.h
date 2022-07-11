#pragma once

#include <unordered_set>

#include "../Common.h"
#include "../sources/ContainerSources.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// UNIQUE
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TMapFn>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Unique : public IterApi<Unique<TChainInput, TMapFn>> {
			friend struct trait::Iterator<Unique<TChainInput, TMapFn>>;
		private:
			using OwnedInputItem = typename TChainInput::ItemOwned;

			TChainInput input;
			TMapFn mapFn;
			std::unordered_set<OwnedInputItem> uniqueCache;
		public:
			constexpr Unique(TChainInput&& input, TMapFn mapFn) : input(std::move(input)), mapFn(mapFn) {
				uniqueCache.reserve(this->input.sizeHint().expectedResultSize());
			}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TMapFn>
	struct trait::Iterator<op::Unique<TChainInput, TMapFn>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		using OwnedInputItem = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = op::Unique<TChainInput, TMapFn>;
		using Item = InputItem;

		static constexpr inline IterValue<Item> next(Self& self) {
			while(true) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; } // reached end of input

				auto itemUniqueValue = self.mapFn(item.value());
				if(self.uniqueCache.contains(itemUniqueValue)) { continue; } // uniqueness-property violated, ignore item
				self.uniqueCache.insert(std::move(itemUniqueValue));
				return item;
			}
		}
		static constexpr inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(0, input.upperBound);
		}
		static constexpr inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};

}
