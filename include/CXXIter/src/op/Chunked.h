#pragma once

#include <array>
#include <cstdlib>
#include <utility>
#include <type_traits>

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	template<typename TItem, const size_t CHUNK_SIZE>
	using DynamicChunk = std::conditional_t<
		std::is_reference_v<TItem>,
		std::vector<std::reference_wrapper<TItem>>,
		std::vector<TItem>>;

	// ################################################################################################
	// CHUNKED
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, const size_t CHUNK_SIZE>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] Chunked : public IterApi<Chunked<TChainInput, CHUNK_SIZE>> {
			friend struct trait::Iterator<Chunked<TChainInput, CHUNK_SIZE>>;
			friend struct trait::ExactSizeIterator<Chunked<TChainInput, CHUNK_SIZE>>;
		private:
			TChainInput input;
			bool reachedEnd = false;
		public:
			Chunked(TChainInput&& input) : input(std::move(input)) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, const size_t CHUNK_SIZE>
	struct trait::Iterator<op::Chunked<TChainInput, CHUNK_SIZE>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::Chunked<TChainInput, CHUNK_SIZE>;
		using Item = DynamicChunk<InputItem, CHUNK_SIZE>;

		static inline IterValue<Item> next(Self& self) {
			if(self.reachedEnd) [[unlikely]] { return {}; }
			Item chunk;
			chunk.reserve(CHUNK_SIZE);
			for(size_t i = 0; i < CHUNK_SIZE; ++i) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] {
					self.reachedEnd = true;
					if(chunk.size() == 0) { return {}; } // chunk empty
					return chunk;
				} // reached end. Commit chunk if it has items
				chunk.push_back(std::move( item.value() ));
			}
			return chunk;
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint result = ChainInputIterator::sizeHint(self.input);
			result.lowerBound = (result.lowerBound / CHUNK_SIZE) + (result.lowerBound % CHUNK_SIZE == 0 ? 0 : 1);
			if(result.upperBound.has_value()) {
				result.upperBound.value() = (result.upperBound.value() / CHUNK_SIZE) + (result.upperBound.value() % CHUNK_SIZE == 0 ? 0 : 1);
			}
			return result;
		}
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, const size_t CHUNK_SIZE>
	struct trait::ExactSizeIterator<op::Chunked<TChainInput, CHUNK_SIZE>> {
		static inline size_t size(const op::Chunked<TChainInput, CHUNK_SIZE>& self) {
			return trait::ExactSizeIterator<TChainInput>::size(self.input) / CHUNK_SIZE;
		}
	};

}
