#pragma once

#include <array>
#include <cstdlib>
#include <utility>
#include <type_traits>

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// CHUNKED EXACT
	// ################################################################################################
	namespace op {
		/** @private */
		template<CXXIterContiguousMemoryIterator TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] ChunkedExactPtr : public IterApi<ChunkedExactPtr<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
			friend struct trait::Iterator<ChunkedExactPtr<TChainInput, CHUNK_SIZE, STEP_SIZE>>;
			friend struct trait::ExactSizeIterator<ChunkedExactPtr<TChainInput, CHUNK_SIZE, STEP_SIZE>>;

			static_assert(STEP_SIZE > 0, "STEP_SIZE has to be at least 1");

		private:
			TChainInput input;
			size_t chunkCnt = 0;
			size_t remaining = 0;
		public:
			constexpr ChunkedExactPtr(TChainInput&& input) : input(std::move(input)) {
				chunkCnt = (this->input.size() - CHUNK_SIZE) / STEP_SIZE + 1;
				remaining = chunkCnt;
			}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
	struct trait::Iterator<op::ChunkedExactPtr<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
		using ChainInputIterator = trait::Iterator<TChainInput>;
		using InputItem = typename TChainInput::Item;
		using InputItemOwned = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = op::ChunkedExactPtr<TChainInput, CHUNK_SIZE, STEP_SIZE>;
		using Item = const InputItemOwned*;

		static constexpr inline IterValue<Item> next(Self& self) {
			if(self.remaining == 0) [[unlikely]] {
				return {};
			}

			// next ptr
			auto item = ChainInputIterator::next(self.input);
			Item itemPtr = &item.value();
			ChainInputIterator::advanceBy(self.input, STEP_SIZE - 1);
			self.remaining -= 1;
			return itemPtr;
		}
		static constexpr inline SizeHint sizeHint(const Self& self) {
			SizeHint result(self.chunkCnt, self.chunkCnt);
			return result;
		}
		static constexpr inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
	/** @private */
	template<CXXIterExactSizeIterator TChainInput, const size_t CHUNK_SIZE, const size_t STEP_SIZE>
	struct trait::ExactSizeIterator<op::ChunkedExactPtr<TChainInput, CHUNK_SIZE, STEP_SIZE>> {
		static constexpr inline size_t size(const op::ChunkedExactPtr<TChainInput, CHUNK_SIZE, STEP_SIZE>& self) {
			return self.chunkCnt;
		}
	};

}
