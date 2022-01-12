#pragma once

#include <utility>
#include <cstdlib>
#include <algorithm>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// INTERSPERSER
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TSeparatorInput>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Intersperser : public IterApi<Intersperser<TChainInput, TSeparatorInput>> {
		friend struct IteratorTrait<Intersperser<TChainInput, TSeparatorInput>>;
		friend struct ExactSizeIteratorTrait<Intersperser<TChainInput, TSeparatorInput>>;
		enum class IntersperserState { Uninitialized, Item, Separator };
	private:
		TChainInput input;
		TSeparatorInput separatorInput;
		IterValue<typename TChainInput::Item> nextItem;
		IntersperserState intersperserState = IntersperserState::Uninitialized;
	public:
		Intersperser(TChainInput&& input, TSeparatorInput&& separatorInput) : input(std::move(input)), separatorInput(std::move(separatorInput)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TSeparatorInput>
	struct IteratorTrait<Intersperser<TChainInput, TSeparatorInput>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using SeparatorInputIterator = IteratorTrait<TSeparatorInput>;
		// CXXIter Interface
		using Self = Intersperser<TChainInput, TSeparatorInput>;
		using Item = typename ChainInputIterator::Item;

		static inline IterValue<Item> next(Self& self) {
			if(self.intersperserState == Self::IntersperserState::Uninitialized) [[unlikely]] {
				self.nextItem = ChainInputIterator::next(self.input);
				self.intersperserState = Self::IntersperserState::Item;
			}
			if(!self.nextItem.has_value()) [[unlikely]] { return {}; }

			if(self.intersperserState == Self::IntersperserState::Item) {
				self.intersperserState = Self::IntersperserState::Separator;
				auto item = std::move(self.nextItem);
				self.nextItem = ChainInputIterator::next(self.input);
				return item;
			} else {
				self.intersperserState = Self::IntersperserState::Item;
				return SeparatorInputIterator::next(self.separatorInput);
			}
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint input = ChainInputIterator::sizeHint(self.input);
			SizeHint separator = SeparatorInputIterator::sizeHint(self.separatorInput);

			SizeHint result = input;
			if(result.lowerBound > 0) {
				size_t sepCnt = std::min((result.lowerBound - 1), separator.lowerBound);
				result.lowerBound = (SaturatingArithmetic<size_t>(sepCnt) + sepCnt + 1).get();
			}
			if(result.upperBound.value_or(0) > 0) {
				size_t sepCnt = std::min((result.upperBound.value() - 1), separator.upperBound.value_or(SizeHint::INFINITE));
				result.upperBound = (SaturatingArithmetic<size_t>(sepCnt) + sepCnt + 1).get();
			}
			return result;
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, CXXIterExactSizeIterator TSeparatorInput>
	struct ExactSizeIteratorTrait<Intersperser<TChainInput, TSeparatorInput>> {
		static inline size_t size(const Intersperser<TChainInput, TSeparatorInput>& self) {
			return IteratorTrait<Intersperser<TChainInput, TSeparatorInput>>::sizeHint(self).lowerBound;
		}
	};

}
