#pragma once

#include <cstdlib>
#include <optional>

#include "../Common.h"
#include "../sources/ContainerSources.h"

namespace CXXIter {

	// ################################################################################################
	// SORTER
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Reverse : public IterApi<Reverse<TChainInput>> {
			friend struct trait::Iterator<Reverse<TChainInput>>;
			friend struct trait::DoubleEndedIterator<Reverse<TChainInput>>;
			friend struct trait::ExactSizeIterator<Reverse<TChainInput>>;

			using InputItem = typename TChainInput::Item;

			struct NoReverseCache {};
			using ReverseCacheContainer = std::conditional_t<
					std::is_reference_v<InputItem>,
					std::vector<std::reference_wrapper<InputItem>>,
					std::vector<InputItem>>;
			using ReverseCache = SrcMov<ReverseCacheContainer>;

		private:
			// If the underlying iterator pipeline is double-ended, we don't need to use the reverseCache
			static constexpr bool USE_CACHE = !CXXIterDoubleEndedIterator<TChainInput>;
			TChainInput input;
			std::conditional_t<USE_CACHE, std::optional<ReverseCache>, NoReverseCache> reverseCache;

		public:
			Reverse(TChainInput&& input) : input(std::move(input)) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput>
	struct trait::Iterator<op::Reverse<TChainInput>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::Reverse<TChainInput>;
		using Item = InputItem;

		static void initReverseCache(Self& self) {
			// drain input iterator into reverse cache
			typename Self::ReverseCacheContainer reverseCache;
			reverseCache.reserve(self.input.sizeHint().expectedResultSize());
			while(true) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) { break; }
				reverseCache.push_back(std::move(item.value()));
			}
			self.reverseCache = typename Self::ReverseCache(std::move(reverseCache));
		}

		static inline IterValue<Item> next(Self& self) {
			if constexpr(Self::USE_CACHE) {
				if(!self.reverseCache.has_value()) [[unlikely]] { initReverseCache(self); }
				return trait::DoubleEndedIterator<typename Self::ReverseCache>::nextBack(self.reverseCache.value());
			} else {
				return trait::DoubleEndedIterator<TChainInput>::nextBack(self.input);
			}
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};
	/** @private */
	template<CXXIterDoubleEndedIterator TChainInput>
	struct trait::DoubleEndedIterator<op::Reverse<TChainInput>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::Reverse<TChainInput>;
		using Item = InputItem;

		static inline IterValue<Item> nextBack(Self& self) {
			if constexpr(Self::USE_CACHE) {
				if(!self.reverseCache.has_value()) [[unlikely]] { initReverseCache(self); }
				return trait::Iterator<typename Self::ReverseCache>::next(self.reverseCache.value());
			} else {
				return ChainInputIterator::next(self.input);
			}
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput>
	struct trait::ExactSizeIterator<op::Reverse<TChainInput>> {
		static inline size_t size(const op::Reverse<TChainInput>& self) {
			return trait::ExactSizeIterator<TChainInput>::size(self.input);
		}
	};

}
