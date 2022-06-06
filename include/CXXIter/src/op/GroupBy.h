#pragma once

#include <vector>
#include <utility>
#include <optional>
#include <unordered_map>

#include "../Common.h"
#include "../sources/ContainerSources.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// GROUP BY
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TGroupIdentifierFn, typename TGroupIdent>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] GroupBy : public IterApi<GroupBy<TChainInput, TGroupIdentifierFn, TGroupIdent>> {
			friend struct trait::Iterator<GroupBy<TChainInput, TGroupIdentifierFn, TGroupIdent>>;
		private:
			using OwnedInputItem = typename TChainInput::ItemOwned;
			using GroupCache = SrcMov<std::unordered_map<TGroupIdent, std::vector<OwnedInputItem>>>;

			TChainInput input;
			TGroupIdentifierFn groupIdentFn;
			std::optional<GroupCache> groupCache;
		public:
			GroupBy(TChainInput&& input, TGroupIdentifierFn groupIdentFn) : input(std::move(input)), groupIdentFn(groupIdentFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TGroupIdentifierFn, typename TGroupIdent>
	struct trait::Iterator<op::GroupBy<TChainInput, TGroupIdentifierFn, TGroupIdent>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using OwnedInputItem = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = op::GroupBy<TChainInput, TGroupIdentifierFn, TGroupIdent>;
		using Item = std::pair<const TGroupIdent, std::vector<OwnedInputItem>>;

		static inline IterValue<Item> next(Self& self) {
			// we have to drain the input in order to be able to calculate the groups
			// so we do that on the first invocation, and then yield from the calculated result.
			if(!self.groupCache.has_value()) [[unlikely]] {
				std::unordered_map<TGroupIdent, std::vector<OwnedInputItem>> groupCache;
				while(true) {
					auto item = ChainInputIterator::next(self.input);
					if(!item.has_value()) [[unlikely]] { break; } // group cache building complete
					TGroupIdent itemGroup = self.groupIdentFn(item.value());
					if(groupCache.contains(itemGroup)) {
						groupCache[itemGroup].push_back(item.value());
					} else {
						groupCache[itemGroup] = { item.value() };
					}
				}
				self.groupCache.emplace(std::move(groupCache));
			}

			using GroupCacheIterator = trait::Iterator<typename Self::GroupCache>;
			typename Self::GroupCache& groupedItems = self.groupCache.value();
			return GroupCacheIterator::next(groupedItems);
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			return SizeHint(1, input.upperBound);
		}
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};

}
