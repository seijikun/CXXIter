#pragma once

#include <cstdlib>
#include <utility>
#include <type_traits>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// CASTER
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TItem>
	requires std::is_object_v<TItem>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Caster : public IterApi<Caster<TChainInput, TItem>> {
		friend struct IteratorTrait<Caster<TChainInput, TItem>>;
		friend struct ExactSizeIteratorTrait<Caster<TChainInput, TItem>>;
	private:
		TChainInput input;
	public:
		Caster(TChainInput&& input) : input(std::move(input)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TItem>
	requires std::is_object_v<TItem>
	struct IteratorTrait<Caster<TChainInput, TItem>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		// CXXIter Interface
		using Self = Caster<TChainInput, TItem>;
		using Item = TItem;

		static inline Item next(Self& self) {
			return static_cast<Item>(ChainInputIterator::next(self.input));
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TItem>
	struct ExactSizeIteratorTrait<Caster<TChainInput, TItem>> {
		static inline size_t size(const Caster<TChainInput, TItem>& self) { return ExactSizeIteratorTrait<TChainInput>::size(self.input); }
	};

}
