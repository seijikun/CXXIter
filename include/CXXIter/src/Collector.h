#pragma once

#include <tuple>
#include <type_traits>

#include "Common.h"

namespace CXXIter {

	// ################################################################################################
	// INTO COLLECTOR
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TContainer>
	struct IntoCollector {};
	/** @private */
	template<typename TChainInput, typename TContainer>
	requires BackInsertableContainer<TContainer, typename TChainInput::ItemOwned>
	struct IntoCollector<TChainInput, TContainer> {
		using Item = typename TChainInput::Item;
		static void collectInto(TChainInput& input, TContainer& container) {
			if constexpr(ReservableContainer<TContainer>) {
				container.reserve( container.size() + input.sizeHint().expectedResultSize() );
			}
			input.forEach([&container](Item&& item) { container.push_back( std::forward<Item>(item) ); });
		}
	};
	/** @private */
	template<typename TChainInput, typename TContainer>
	requires (!BackInsertableContainer<TContainer, typename TChainInput::ItemOwned>)
		&& InsertableContainer<TContainer, typename TChainInput::ItemOwned>
	struct IntoCollector<TChainInput, TContainer> {
		using Item = typename TChainInput::Item;
		static void collectInto(TChainInput& input, TContainer& container) {
			if constexpr(ReservableContainer<TContainer>) {
				container.reserve( container.size() + input.sizeHint().expectedResultSize() );
			}
			input.forEach([&container](Item&& item) { container.insert( std::forward<Item>(item) ); });
		}
	};


	// ################################################################################################
	// COLLECTOR
	// ################################################################################################
	/** @private */
	template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
	struct Collector {};
	/** @private */
	template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
	requires BackInsertableContainerTemplate<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>
		|| InsertableContainerTemplate<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>
	struct Collector<TChainInput, TContainer, TContainerArgs...> {
		template<typename Item, typename ItemOwned>
		static TContainer<ItemOwned, TContainerArgs...> collect(TChainInput& input) {
			TContainer<ItemOwned, TContainerArgs...> container;
			IntoCollector<TChainInput, decltype(container)>::collectInto(input, container);
			return container;
		}
	};
	/** @private */
	template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
	requires (!BackInsertableContainerTemplate<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>)
		&& is_pair<typename TChainInput::ItemOwned>
		&& AssocContainerTemplate<TContainer, std::tuple_element_t<0, typename TChainInput::ItemOwned>, std::tuple_element_t<1, typename TChainInput::ItemOwned>, TContainerArgs...>
	struct Collector<TChainInput, TContainer, TContainerArgs...> {
		template<typename Item, typename ItemOwned>
		static auto collect(TChainInput& input) {
			using TKey = std::remove_const_t<std::tuple_element_t<0, typename TChainInput::ItemOwned>>;
			using TValue = std::tuple_element_t<1, typename TChainInput::ItemOwned>;
			TContainer<TKey, TValue, TContainerArgs...> container;
			IntoCollector<TChainInput, decltype(container)>::collectInto(input, container);
			return container;
		}
	};

}
