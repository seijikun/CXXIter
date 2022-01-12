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
			Item item;
			bool hasNext = constexpr_for<0, INPUT_CNT>([&](auto idx) {
				auto input = std::tuple_element_t<idx, ChainInputIterators>::next( std::get<idx>(self.inputs) );
				if(!input.has_value()) [[unlikely]] { return false; }
				std::get<idx>(item) = input.value();
				return true;
			});
			if(!hasNext) [[unlikely]] { return {}; }
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
