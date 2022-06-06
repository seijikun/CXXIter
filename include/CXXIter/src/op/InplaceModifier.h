#pragma once

#include <cstdlib>
#include <type_traits>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// INPLACE MODIFIER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TModifierFn>
		requires std::is_object_v<typename trait::Iterator<TChainInput>::Item> || (!std::is_const_v<typename trait::Iterator<TChainInput>::Item>)
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] InplaceModifier : public IterApi<InplaceModifier<TChainInput, TModifierFn>> {
			friend struct trait::Iterator<InplaceModifier<TChainInput, TModifierFn>>;
			friend struct trait::DoubleEndedIterator<InplaceModifier<TChainInput, TModifierFn>>;
			friend struct trait::ExactSizeIterator<InplaceModifier<TChainInput, TModifierFn>>;
		private:
			using InputItem = typename TChainInput::Item;

			TChainInput input;
			TModifierFn modifierFn;
		public:
			InplaceModifier(TChainInput&& input, TModifierFn modifierFn) : input(std::move(input)), modifierFn(modifierFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TModifierFn>
	struct trait::Iterator<op::InplaceModifier<TChainInput, TModifierFn>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		// CXXIter Interface
		using Self = op::InplaceModifier<TChainInput, TModifierFn>;
		using Item = typename ChainInputIterator::Item;

		static inline IterValue<Item> next(Self& self) {
			auto item = ChainInputIterator::next(self.input);
			if(!item.has_value()) [[unlikely]] { return {}; }
			self.modifierFn(item.value());
			return item;
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
		static inline size_t advanceBy(Self& self, size_t n) { return ChainInputIterator::advanceBy(self.input, n); }
	};
	/** @private */
	template<CXXIterDoubleEndedIterator TChainInput, typename TModifierFn>
	struct trait::DoubleEndedIterator<op::InplaceModifier<TChainInput, TModifierFn>> {
		using ChainInputIterator = trait::DoubleEndedIterator<TChainInput>;
		// CXXIter Interface
		using Self = op::InplaceModifier<TChainInput, TModifierFn>;
		using Item = typename ChainInputIterator::Item;

		static inline IterValue<Item> nextBack(Self& self) {
			auto item = ChainInputIterator::nextBack(self.input);
			if(!item.has_value()) [[unlikely]] { return {}; }
			self.modifierFn(item.value());
			return item;
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TItem>
	struct trait::ExactSizeIterator<op::InplaceModifier<TChainInput, TItem>> {
		static inline size_t size(const op::InplaceModifier<TChainInput, TItem>& self) { return trait::ExactSizeIterator<TChainInput>::size(self.input); }
	};

}
