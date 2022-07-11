#pragma once

#include <utility>
#include <cstdlib>
#include <algorithm>

#include "../Common.h"
#include "../util/SaturatingArithmetic.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// INTERSPERSER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TSeparatorInput>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Intersperser : public IterApi<Intersperser<TChainInput, TSeparatorInput>> {
			friend struct trait::Iterator<Intersperser<TChainInput, TSeparatorInput>>;
			friend struct trait::ExactSizeIterator<Intersperser<TChainInput, TSeparatorInput>>;
			enum class IntersperserState { Uninitialized, Item, Separator };
		private:
			TChainInput input;
			TSeparatorInput separatorInput;
			IterValue<typename TChainInput::Item> nextItem;
			IntersperserState intersperserState = IntersperserState::Uninitialized;
		public:
			Intersperser(TChainInput&& input, TSeparatorInput&& separatorInput) : input(std::move(input)), separatorInput(std::move(separatorInput)) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TSeparatorInput>
	struct trait::Iterator<op::Intersperser<TChainInput, TSeparatorInput>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using SeparatorInputIterator = trait::Iterator<TSeparatorInput>;
		// CXXIter Interface
		using Self = op::Intersperser<TChainInput, TSeparatorInput>;
		using Item = typename ChainInputIterator::Item;

		static inline IterValue<Item> next(Self& self) {
			if(self.intersperserState == Self::IntersperserState::Uninitialized) [[unlikely]] {
				self.nextItem = ChainInputIterator::next(self.input);
				self.intersperserState = Self::IntersperserState::Item;
			}
			if(!self.nextItem.has_value()) [[unlikely]] { return {}; }

			if(self.intersperserState == Self::IntersperserState::Item) {
				self.intersperserState = Self::IntersperserState::Separator;
				IterValue<Item> item;
				item.swap(self.nextItem);
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
				result.lowerBound = (util::SaturatingArithmetic<size_t>(sepCnt) + sepCnt + 1).get();
			}
			if(result.upperBound.value_or(0) > 0) {
				size_t sepCnt = std::min((result.upperBound.value() - 1), separator.upperBound.value_or(SizeHint::INFINITE));
				result.upperBound = (util::SaturatingArithmetic<size_t>(sepCnt) + sepCnt + 1).get();
			}
			return result;
		}
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, CXXIterExactSizeIterator TSeparatorInput>
	struct trait::ExactSizeIterator<op::Intersperser<TChainInput, TSeparatorInput>> {
		static inline size_t size(const op::Intersperser<TChainInput, TSeparatorInput>& self) {
			return trait::Iterator<op::Intersperser<TChainInput, TSeparatorInput>>::sizeHint(self).lowerBound;
		}
	};

}
