#pragma once

#include <array>
#include <cstdlib>
#include <utility>
#include <type_traits>

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	template<typename TItem, const size_t CHUNK_SIZE>
	using ExactChunk = std::conditional_t<
		std::is_reference_v<TItem>,
		std::array<std::reference_wrapper<std::remove_reference_t<TItem>>, CHUNK_SIZE>,
		std::array<TItem, CHUNK_SIZE>>;

	// ################################################################################################
	// CHUNKED EXACT
	// ################################################################################################
	namespace op {
		// Empty definition
		// ------------------------------------------------
		/** @private */
		template<typename TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
		class ChunkedExact {};


		// Implementation for non-contiguous memory sources
		// ------------------------------------------------
		/** @private */
		template<typename TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
		requires (!CXXIterContiguousMemoryIterator<TChainInput>)
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE> : public IterApi<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
			friend struct trait::Iterator<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>>;
			friend struct trait::ExactSizeIterator<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>>;
		private:
			struct source_ended_exception {};
			TChainInput input;
			std::optional<ExactChunk<typename TChainInput::Item, CHUNK_SIZE>> chunk;
		public:
			constexpr ChunkedExact(TChainInput&& input) : input(std::move(input)) {}
		};

		// Implementation for contiguous memory sources
		// ------------------------------------------------
		/** @private */
		template<typename TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
		requires CXXIterContiguousMemoryIterator<TChainInput>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE> : public IterApi<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
			friend struct trait::Iterator<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>>;
			friend struct trait::ExactSizeIterator<ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>>;
		private:
			TChainInput input;
			size_t remaining = 0;
		public:
			constexpr ChunkedExact(TChainInput&& input) : input(std::move(input)) {
				remaining = (this->input.size() - CHUNK_SIZE) / STEP_SIZE + 1;
			}
		};
	}


	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
	struct trait::Iterator<op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>> {

		static_assert(STEP_SIZE > 0, "STEP_SIZE has to be at least 1");

	private:
		static constexpr size_t LOAD_SIZE = std::min(CHUNK_SIZE, STEP_SIZE);
		static constexpr size_t SHIFT_SIZE = (STEP_SIZE < CHUNK_SIZE) ? (CHUNK_SIZE - STEP_SIZE) : 0;
		static constexpr size_t LOAD_START = SHIFT_SIZE;
		static constexpr size_t SKIP_SIZE = (STEP_SIZE > CHUNK_SIZE) ? (STEP_SIZE - CHUNK_SIZE) : 0;

		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		using InputItemOwned = typename TChainInput::ItemOwned;

		static constexpr bool IS_CONTIGUOUS = CXXIterContiguousMemoryIterator<TChainInput>;

	public:
		// CXXIter Interface
		using Self = op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>;
		// If the source is contiguous, copy the const specifier from the InputItem
		// If the source is non-contiguous, always add a const specifier, since we pass a reference
		//    to our internal chunk buffer, and changing that doesn't make much sense.
		using ItemOwned = std::conditional_t<IS_CONTIGUOUS,
		copy_const_from<InputItem, ExactChunk<InputItemOwned, CHUNK_SIZE>>,
		const ExactChunk<InputItem, CHUNK_SIZE>>;
		using Item = ItemOwned&;

		// non-contiguous
		static constexpr inline IterValue<Item> next(Self& self) requires (!IS_CONTIGUOUS) {
			auto getElementFromChainInput = [&]<size_t IDX>(std::integral_constant<size_t, IDX>) -> typename ItemOwned::value_type {
				auto input = ChainInputIterator::next(self.input);
				if(!input.has_value()) [[unlikely]] {
					throw typename Self::source_ended_exception{};
				}
				return input.value();
			};
			auto initializeChunk = [&]<size_t... IDX>(auto& chunkOptional, std::integer_sequence<size_t, IDX...>) {
				chunkOptional = { getElementFromChainInput(std::integral_constant<size_t, IDX>())... };
			};

			if(!self.chunk.has_value()) [[unlikely]] {
				// initial loading
				initializeChunk(self.chunk, std::make_index_sequence<CHUNK_SIZE>{});
				return *self.chunk;
			}

			auto& chunk = *self.chunk;

			// if step-size is greater than chunk-size, we need to skip some values
			ChainInputIterator::advanceBy(self.input, SKIP_SIZE);

			// if step-size is smaller than chunk-size, we have to shift out the first couple of items
			// so we can push new ones in the back
			for(size_t i = 0; i < SHIFT_SIZE; ++i) {
				chunk[i] = std::move(chunk[STEP_SIZE + i]);
			}

			// load new items
			for(size_t i = 0; i < LOAD_SIZE; ++i) {
				auto item = ChainInputIterator::next(self.input);
				if(!item.has_value()) [[unlikely]] { return {}; } // reached end. Chunk needs to be full to commit!
				chunk[LOAD_START + i] = item.value();
			}
			return chunk;
		}

		// contiguous
		static constexpr inline IterValue<Item> next(Self& self) requires IS_CONTIGUOUS {
			if(self.remaining == 0) [[unlikely]] {
				return {};
			}

			// next ptr
			auto itemPtr = ContiguousMemoryIterator<TChainInput>::currentPtr(self.input);
			ChainInputIterator::advanceBy(self.input, STEP_SIZE);
			self.remaining -= 1;
			return (Item)(*itemPtr);
		}


		static constexpr inline SizeHint sizeHint(const Self& self) {
			SizeHint result = ChainInputIterator::sizeHint(self.input);
			result.lowerBound = (result.lowerBound >= CHUNK_SIZE) ? ((result.lowerBound - CHUNK_SIZE) / STEP_SIZE + 1) : 0;
			if(result.upperBound.has_value()) {
				result.upperBound.value() = (result.upperBound.value() >= CHUNK_SIZE) ? ((result.upperBound.value() - CHUNK_SIZE) / STEP_SIZE + 1) : 0;
			}
			return result;
		}
		static constexpr inline size_t advanceBy(Self& self, size_t n) {
			if constexpr(IS_CONTIGUOUS) {
				ChainInputIterator::advanceBy(self.input, n * STEP_SIZE);
			} else {
				return util::advanceByPull(self, n);
			}
		}
	};

	/** @private */
	template<CXXIterExactSizeIterator TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
	struct trait::ExactSizeIterator<op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
		static constexpr inline size_t size(const op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>& self) {
			return trait::Iterator<op::ChunkedExact<TChainInput, CHUNK_SIZE, STEP_SIZE>>::sizeHint(self).lowerBound;
		}
	};

}
