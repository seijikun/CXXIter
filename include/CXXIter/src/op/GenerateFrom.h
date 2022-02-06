#pragma once

#ifdef CXXITER_HAS_COROUTINE

#include "../Common.h"
#include "../Generator.h"

namespace CXXIter {

	// ################################################################################################
	// GENERATE FROM
	// ################################################################################################

	template<typename T, typename TItem>
	concept GeneratorFromFunction = (invocable_byvalue<T, TItem> && is_template_instance_v<std::invoke_result_t<T, TItem>, Generator>);

	/** @private */
	template<typename TChainInput, typename TGeneratorFn, typename TGenerator>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] GenerateFrom : public IterApi<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>> {
		friend struct IteratorTrait<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>>;
		friend struct ExactSizeIteratorTrait<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>>;
	private:
		TChainInput input;
		TGeneratorFn generatorFn;
		std::optional<TGenerator> currentGenerator;
	public:
		GenerateFrom(TChainInput&& input, TGeneratorFn& generatorFn) : input(std::move(input)), generatorFn(generatorFn) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TGeneratorFn, typename TGenerator>
	struct IteratorTrait<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItemOwned = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = GenerateFrom<TChainInput, TGeneratorFn, TGenerator>;
		using Item = typename TGenerator::value_type;

		static inline Item next(Self& self) {
			while(true) {
				if(!self.currentGenerator.has_value()) {
					self.currentGenerator.emplace(self.generatorFn(ChainInputIterator::next(self.input)));
				}

				auto item = self.currentGenerator.value().next();
				if(!item.has_value()) {
					self.currentGenerator.reset();
					continue;
				}
				return item.value();
			}
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};

}

#endif
