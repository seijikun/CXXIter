#pragma once

#include <utility>
#include <optional>

#include "../Common.h"
#include "../sources/ContainerSources.h"

namespace CXXIter {

	// ################################################################################################
	// FLATMAP
	// ################################################################################################
	/** @private */
	namespace op {
		template<typename TChainInput, typename TFlatMapFn, typename TItemContainer>
		requires (!std::is_reference_v<TItemContainer>)
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] FlatMap : public IterApi<FlatMap<TChainInput, TFlatMapFn, TItemContainer>> {
			friend struct IteratorTrait<FlatMap<TChainInput, TFlatMapFn, TItemContainer>>;
		private:
			TChainInput input;
			std::optional<SrcMov<TItemContainer>> current;
			TFlatMapFn mapFn;
		public:
			FlatMap(TChainInput&& input, TFlatMapFn mapFn) : input(std::move(input)), mapFn(mapFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TFlatMapFn, typename TItemContainer>
	struct IteratorTrait<op::FlatMap<TChainInput, TFlatMapFn, TItemContainer>> {
		using NestedChainIterator = IteratorTrait<SrcMov<TItemContainer>>;
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItem = typename ChainInputIterator::Item;
		// CXXIter Interface
		using Self = op::FlatMap<TChainInput, TFlatMapFn, TItemContainer>;
		using Item = typename TItemContainer::value_type;

		static inline IterValue<Item> next(Self& self) {
			while(true) {
				if(!self.current) { // pull new container from the outer iterator
					auto item = ChainInputIterator::next(self.input);
					if(!item.has_value()) [[unlikely]] { return {}; } // end of iteration
					self.current = SrcMov(std::move(
						self.mapFn(std::forward<InputItem>( item.value() ))
					));
				}

				// if the outer iterator yielded a container, take from it until we reach the end
				auto item = NestedChainIterator::next(*self.current);
				if(item.has_value()) [[likely]] { // inner yielded a usable item
					return item.value();
				} else [[unlikely]] {
					self.current.reset(); // inner container ended, unset current cache
				}
			}
		}
		static inline SizeHint sizeHint(const Self&) { return SizeHint(); }
	};

}
