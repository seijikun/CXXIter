#pragma once

#include <array>
#include <cstdlib>
#include <utility>
#include <type_traits>

#include "../Common.h"

namespace CXXIter {

	template<typename TItem, const size_t CHUNK_SIZE>
	using ExactChunk = std::conditional_t<
		std::is_reference_v<TItem>,
		std::array<std::reference_wrapper<TItem>, CHUNK_SIZE>,
		std::array<TItem, CHUNK_SIZE>>;

	// ################################################################################################
	// CHUNKED EXACT
	// ################################################################################################
	namespace op {
		/** @private */
		template<typename TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] ChunkedExact : public IterApi<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
			friend struct trait::IteratorTrait<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>>;
			friend struct trait::ExactSizeIteratorTrait<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>>;
		private:
			TChainInput input;
			std::optional<ExactChunk<typename TChainInput::Item, CHUNK_SIZE>> chunk;
		public:
			ChunkedExact(TChainInput&& input) : input(std::move(input)) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
	struct trait::IteratorTrait<op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
	private:
		static constexpr size_t LOAD_SIZE = std::min(CHUNK_SIZE, STEP_SIZE);
		static constexpr size_t SHIFT_SIZE = (STEP_SIZE < CHUNK_SIZE) ? (CHUNK_SIZE - STEP_SIZE) : 0;
		static constexpr size_t LOAD_START = SHIFT_SIZE;
		static constexpr size_t SKIP_SIZE = (STEP_SIZE > CHUNK_SIZE) ? (STEP_SIZE - CHUNK_SIZE) : 0;

	public:
		using ChainInputIterator = trait::IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>;
		using Item = ExactChunk<InputItem, CHUNK_SIZE>;

		static inline IterValue<Item> next(Self& self) {
			if(!self.chunk.has_value()) [[unlikely]] {
				// initial loading
				self.chunk = Item();
				for(size_t i = 0; i < CHUNK_SIZE; ++i) {
					auto item = ChainInputIterator::next(self.input);
					if(!item.has_value()) [[unlikely]] { return {}; } // reached end. Chunk needs to be full to commit!
					(*self.chunk)[i] = std::move( item.value() );
				}
				return *self.chunk;
			}

			// if step-size is greater than chunk-size, we need to skip some values
			for(size_t i = 0; i < SKIP_SIZE; ++i) {
				ChainInputIterator::next(self.input);
			}

			// if step-size is smaller than chunk-size, we have to shift out the first couple of items
			// so we can push new ones in the back
			for(size_t i = 0; i < SHIFT_SIZE; ++i) {
				(*self.chunk)[i] = std::move((*self.chunk)[STEP_SIZE + i]);
			}

			// load new items
			for(size_t i = 0; i < LOAD_SIZE; ++i) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; } // reached end. Chunk needs to be full to commit!
				(*self.chunk)[LOAD_START + i] = std::move( item.value() );
			}
			return *self.chunk;
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint result = ChainInputIterator::sizeHint(self.input);
			result.lowerBound = (result.lowerBound >= CHUNK_SIZE) ? ((result.lowerBound - CHUNK_SIZE) / STEP_SIZE + 1) : 0;
			if(result.upperBound.has_value()) {
				result.upperBound.value() = (result.upperBound.value() >= CHUNK_SIZE) ? ((result.upperBound.value() - CHUNK_SIZE) / STEP_SIZE + 1) : 0;
			}
			return result;
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
	struct trait::ExactSizeIteratorTrait<op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
		static inline size_t size(const op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>& self) {
			return trait::IteratorTrait<op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>>::sizeHint(self).lowerBound;
		}
	};

}
