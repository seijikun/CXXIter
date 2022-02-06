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
	// CASTER
	// ################################################################################################
	/** @private */
	template<typename TChainInput, const size_t CHUNK_SIZE>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] ChunkedExact : public IterApi<ChunkedExact<TChainInput, CHUNK_SIZE>> {
		friend struct IteratorTrait<ChunkedExact<TChainInput, CHUNK_SIZE>>;
		friend struct ExactSizeIteratorTrait<ChunkedExact<TChainInput, CHUNK_SIZE>>;
	private:
		TChainInput input;
	public:
		ChunkedExact(TChainInput&& input) : input(std::move(input)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, const size_t CHUNK_SIZE>
	struct IteratorTrait<ChunkedExact<TChainInput, CHUNK_SIZE>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		// CXXIter Interface
		using Self = ChunkedExact<TChainInput, CHUNK_SIZE>;
		using Item = ExactChunk<InputItem, CHUNK_SIZE>;

		static inline Item next(Self& self) {
			Item chunk; //TODO: can we get rid of this for-loop here?
			for(size_t i = 0; i < CHUNK_SIZE; ++i) {
				chunk[i] = ChainInputIterator::next(self.input);
			}
			return chunk;
		}
		static inline SizeHint sizeHint(const Self& self) {
			SizeHint result = ChainInputIterator::sizeHint(self.input);
			result.lowerBound /= CHUNK_SIZE;
			if(result.upperBound.has_value()) {
				result.upperBound.value() /= CHUNK_SIZE;
			}
			return result;
		}
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, const size_t CHUNK_SIZE>
	struct ExactSizeIteratorTrait<ChunkedExact<TChainInput, CHUNK_SIZE>> {
		static inline size_t size(const ChunkedExact<TChainInput, CHUNK_SIZE>& self) { return ExactSizeIteratorTrait<TChainInput>::size(self.input) / CHUNK_SIZE; }
	};

}
