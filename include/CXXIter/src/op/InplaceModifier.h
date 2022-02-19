#pragma once

#include <cstdlib>
#include <type_traits>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// INPLACE MODIFIER
	// ################################################################################################
	/** @private */
	namespace op {
		template<typename TChainInput, typename TModifierFn>
		requires std::is_object_v<typename IteratorTrait<TChainInput>::Item> || (!std::is_const_v<typename IteratorTrait<TChainInput>::Item>)
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] InplaceModifier : public IterApi<InplaceModifier<TChainInput, TModifierFn>> {
			friend struct IteratorTrait<InplaceModifier<TChainInput, TModifierFn>>;
			friend struct ExactSizeIteratorTrait<InplaceModifier<TChainInput, TModifierFn>>;
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
	struct IteratorTrait<op::InplaceModifier<TChainInput, TModifierFn>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
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
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TItem>
	struct ExactSizeIteratorTrait<op::InplaceModifier<TChainInput, TItem>> {
		static inline size_t size(const op::InplaceModifier<TChainInput, TItem>& self) { return ExactSizeIteratorTrait<TChainInput>::size(self.input); }
	};

}
