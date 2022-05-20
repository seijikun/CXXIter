#pragma once

#include <cstdlib>
#include <utility>
#include <type_traits>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// CASTER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, typename TItem>
		requires std::is_object_v<TItem>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Caster : public IterApi<Caster<TChainInput, TItem>> {
			friend struct trait::IteratorTrait<Caster<TChainInput, TItem>>;
			friend struct trait::ExactSizeIteratorTrait<Caster<TChainInput, TItem>>;
		private:
			TChainInput input;
		public:
			Caster(TChainInput&& input) : input(std::move(input)) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TItem>
	requires std::is_object_v<TItem>
	struct trait::IteratorTrait<op::Caster<TChainInput, TItem>> {
		using ChainInputIterator = trait::IteratorTrait<TChainInput>;
		// CXXIter Interface
		using Self = op::Caster<TChainInput, TItem>;
		using Item = TItem;

		static inline IterValue<Item> next(Self& self) {
			auto item = ChainInputIterator::next(self.input);
			return item.template map<Item>([](auto&& item) { return static_cast<Item>(item); });
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TItem>
	struct trait::ExactSizeIteratorTrait<op::Caster<TChainInput, TItem>> {
		static inline size_t size(const op::Caster<TChainInput, TItem>& self) { return trait::ExactSizeIteratorTrait<TChainInput>::size(self.input); }
	};

}
