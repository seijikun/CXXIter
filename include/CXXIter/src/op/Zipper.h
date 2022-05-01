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
	namespace op {
		/** @private */
		template<typename TChainInput1, template<typename...> typename TZipContainer, typename... TChainInputs>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Zipper : public IterApi<Zipper<TChainInput1, TZipContainer, TChainInputs...>> {
			friend struct IteratorTrait<Zipper<TChainInput1, TZipContainer, TChainInputs...>>;
			friend struct ExactSizeIteratorTrait<Zipper<TChainInput1, TZipContainer, TChainInputs...>>;
		private:
			struct source_ended_exception {};
			std::tuple<TChainInput1, TChainInputs...> inputs;
		public:
			Zipper(TChainInput1&& input1, TChainInputs&&... inputs) : inputs( std::forward_as_tuple(std::move(input1), std::move(inputs)...) ) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput1, template<typename...> typename TZipContainer, typename... TChainInputs>
	struct IteratorTrait<op::Zipper<TChainInput1, TZipContainer, TChainInputs...>> {
		using ChainInputIterators = std::tuple<IteratorTrait<TChainInput1>, IteratorTrait<TChainInputs>...>;
		static constexpr size_t INPUT_CNT = 1 + sizeof...(TChainInputs);
		// CXXIter Interface
		using Self = op::Zipper<TChainInput1, TZipContainer, TChainInputs...>;
		using Item = TZipContainer<typename TChainInput1::Item, typename TChainInputs::Item...>;

		static inline IterValue<Item> next(Self& self) {
			auto getElementFromChainInput = [&]<size_t IDX>(std::integral_constant<size_t, IDX>) -> std::tuple_element_t<IDX, Item> {
				auto input = std::tuple_element_t<IDX, ChainInputIterators>::next( std::get<IDX>(self.inputs) );
				if(!input.has_value()) [[unlikely]] {
					throw typename Self::source_ended_exception{};
				}
				return input.value();
			};
			auto constructZipped = [&]<size_t... IDX>(std::integer_sequence<size_t, IDX...>) -> Item {
				return { getElementFromChainInput(std::integral_constant<size_t, IDX>())... };
			};

			try {
				return constructZipped(std::make_index_sequence<INPUT_CNT>{});
			} catch(typename Self::source_ended_exception) {
				return {};
			}
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
	struct ExactSizeIteratorTrait<op::Zipper<TChainInput1, TZipContainer, TChainInputs...>> {
		static inline size_t size(const op::Zipper<TChainInput1, TZipContainer, TChainInputs...>& self) {
			return IteratorTrait<op::Zipper<TChainInput1, TZipContainer, TChainInputs...>>::sizeHint(self).lowerBound;
		}
	};

}
