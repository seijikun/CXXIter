#pragma once

#include <tuple>
#include <type_traits>

#include "Common.h"

namespace CXXIter {

	// ################################################################################################
	// COLLECTOR
	// ################################################################################################
	/** @private */
	template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
	struct Collector {};
	/** @private */
	template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
	requires BackInsertableContainer<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>
	struct Collector<TChainInput, TContainer, TContainerArgs...> {
		template<typename Item, typename ItemOwned>
		static TContainer<ItemOwned, TContainerArgs...> collect(TChainInput& input) {
			TContainer<ItemOwned, TContainerArgs...> container;
			if constexpr(ReservableContainer<TContainer<ItemOwned, TContainerArgs...>>) {
				container.reserve( input.sizeHint().expectedResultSize() );
			}
			input.forEach([&container](Item&& item) { container.push_back( std::forward<Item>(item) ); });
			return container;
		}
	};
	/** @private */
	template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
	requires (!BackInsertableContainer<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>)
		&& is_pair<typename TChainInput::ItemOwned>
		&& AssocContainer<TContainer, std::tuple_element_t<0, typename TChainInput::ItemOwned>, std::tuple_element_t<1, typename TChainInput::ItemOwned>, TContainerArgs...>
	struct Collector<TChainInput, TContainer, TContainerArgs...> {
		template<typename Item, typename ItemOwned>
		static auto collect(TChainInput& input) {
			using TKey = std::remove_const_t<std::tuple_element_t<0, typename TChainInput::ItemOwned>>;
			using TValue = std::tuple_element_t<1, typename TChainInput::ItemOwned>;
			TContainer<TKey, TValue, TContainerArgs...> container;
			input.forEach([&container](Item&& item) { container.insert( std::forward<Item>(item) ); });
			return container;
		}
	};
	/** @private */
	template<typename TChainInput, template <typename...> typename TContainer, typename... TContainerArgs>
	requires (!BackInsertableContainer<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>)
		&& InsertableContainer<TContainer, typename TChainInput::ItemOwned, TContainerArgs...>
	struct Collector<TChainInput, TContainer, TContainerArgs...> {
		template<typename Item, typename ItemOwned>
		static TContainer<ItemOwned, TContainerArgs...> collect(TChainInput& input) {
			TContainer<ItemOwned, TContainerArgs...> container;
			if constexpr(ReservableContainer<TContainer<ItemOwned, TContainerArgs...>>) {
				container.reserve( input.sizeHint().expectedResultSize() );
			}
			input.forEach([&container](Item&& item) { container.insert( std::forward<Item>(item) ); });
			return container;
		}
	};

}
