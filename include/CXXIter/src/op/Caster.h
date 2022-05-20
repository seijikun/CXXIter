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
			friend struct trait::Iterator<Caster<TChainInput, TItem>>;
			friend struct trait::DoubleEndedIterator<Caster<TChainInput, TItem>>;
			friend struct trait::ExactSizeIterator<Caster<TChainInput, TItem>>;
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
	struct trait::Iterator<op::Caster<TChainInput, TItem>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
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
	template<CXXIterDoubleEndedIterator TChainInput, typename TItem>
	requires std::is_object_v<TItem>
	struct trait::DoubleEndedIterator<op::Caster<TChainInput, TItem>> {
		using ChainInputIterator = trait::DoubleEndedIterator<TChainInput>;
		using Self = op::Caster<TChainInput, TItem>;
		using Item = TItem;

		static inline IterValue<TItem> nextBack(Self& self) {
			auto item = ChainInputIterator::nextBack(self.input);
			return item.template map<Item>([](auto&& item) { return static_cast<Item>(item); });
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, typename TItem>
	struct trait::ExactSizeIterator<op::Caster<TChainInput, TItem>> {
		static inline size_t size(const op::Caster<TChainInput, TItem>& self) { return trait::ExactSizeIterator<TChainInput>::size(self.input); }
	};

}
