#pragma once

#include <tuple>
#include <cstdlib>
#include <optional>
#include <algorithm>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// ZIPPER
	// ################################################################################################
	/** @private */
	template<typename TChainInput1, template<typename...> typename TZipContainer, typename... TChainInputs>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Zipper : public IterApi<Zipper<TChainInput1, TZipContainer, TChainInputs...>> {
		friend struct IteratorTrait<Zipper<TChainInput1, TZipContainer, TChainInputs...>>;
		friend struct ExactSizeIteratorTrait<Zipper<TChainInput1, TZipContainer, TChainInputs...>>;
	private:
		std::tuple<TChainInput1, TChainInputs...> inputs;
	public:
		Zipper(TChainInput1&& input1, TChainInputs&&... inputs) : inputs( std::forward_as_tuple(std::move(input1), std::move(inputs)...) ) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput1, template<typename...> typename TZipContainer, typename... TChainInputs>
	struct IteratorTrait<Zipper<TChainInput1, TZipContainer, TChainInputs...>> {
		using ChainInputIterators = std::tuple<IteratorTrait<TChainInput1>, IteratorTrait<TChainInputs>...>;
		static constexpr size_t INPUT_CNT = 1 + sizeof...(TChainInputs);
		// CXXIter Interface
		using Self = Zipper<TChainInput1, TZipContainer, TChainInputs...>;
		using Item = TZipContainer<typename TChainInput1::Item, typename TChainInputs::Item...>;

		static inline IterValue<Item> next(Self& self) {
			// extremely hugly hack to be able to support references in tuples and pairs!
			// tuples and pairs with references in them do not have a default ctor(), so we have to initialize
			// it in one go and decide whether that succeeded later on.
			static uint8_t rawDefaultValues[sizeof(Item)] = {0};
			static Item& defaultValues = *reinterpret_cast<Item*>(rawDefaultValues);

			bool success = true;
			auto getElementFromChainInput = [&]<size_t IDX>(std::integral_constant<size_t, IDX>) -> std::tuple_element_t<IDX, Item> {
				auto input = std::tuple_element_t<IDX, ChainInputIterators>::next( std::get<IDX>(self.inputs) );
				if(!input.has_value()) [[unlikely]] {
					success = false;
					// required as fallback for elements without default ctor(), such as references
					return std::get<IDX>(defaultValues);
				}
				return input.value();
			};
			auto constructZipped = [&]<size_t... IDX>(std::integer_sequence<size_t, IDX...>) -> Item {
				return { getElementFromChainInput(std::integral_constant<size_t, IDX>())... };
			};

			Item item = constructZipped(std::make_index_sequence<INPUT_CNT>{});
			if(!success) { return {}; }
			return item;
		}
		static inline SizeHint sizeHint(const Self& self) {
			size_t lowerBoundMin = std::numeric_limits<size_t>::max();
			std::optional<size_t> upperBoundMin = {};
			constexpr_for<0, INPUT_CNT>([&](auto idx) {
				SizeHint tmp = std::tuple_element_t<idx, ChainInputIterators>::sizeHint( std::get<idx>(self.inputs) );
				lowerBoundMin = std::min(lowerBoundMin, tmp.lowerBound);
				upperBoundMin = SizeHint::upperBoundMin(upperBoundMin, tmp.upperBound);
				return true;
			});
			return SizeHint(lowerBoundMin, upperBoundMin);
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput1, template<typename...> typename TZipContainer, CXXIterExactSizeIterator... TChainInputs>
	struct ExactSizeIteratorTrait<Zipper<TChainInput1, TZipContainer, TChainInputs...>> {
		static inline size_t size(const Zipper<TChainInput1, TZipContainer, TChainInputs...>& self) {
			return IteratorTrait<Zipper<TChainInput1, TZipContainer, TChainInputs...>>::sizeHint(self).lowerBound;
		}
	};

}
