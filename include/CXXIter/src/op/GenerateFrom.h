#pragma once

#ifdef CXXITER_HAS_COROUTINE

#include "../Common.h"
#include "../Generator.h"

namespace CXXIter {

	// ################################################################################################
	// GENERATE FROM
	// ################################################################################################

	template<typename T, typename TItem>
	concept GeneratorFromFunction = (util::invocable_byvalue<T, TItem> && util::is_template_instance_v<std::invoke_result_t<T, TItem>, Generator>);

	namespace op {
		/** @private */
		template<typename TChainInput, typename TGeneratorFn, typename TGenerator>
		class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] GenerateFrom : public IterApi<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>> {
			friend struct trait::IteratorTrait<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>>;
			friend struct trait::ExactSizeIteratorTrait<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>>;
		private:
			TChainInput input;
			TGeneratorFn generatorFn;
			std::optional<TGenerator> currentGenerator;
		public:
			GenerateFrom(TChainInput&& input, TGeneratorFn& generatorFn) : input(std::move(input)), generatorFn(generatorFn) {}
		};
	}
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TGeneratorFn, typename TGenerator>
	struct trait::IteratorTrait<op::GenerateFrom<TChainInput, TGeneratorFn, TGenerator>> {
		using ChainInputIterator = trait::IteratorTrait<TChainInput>;
		using InputItem = typename TChainInput::Item;
		using InputItemOwned = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = op::GenerateFrom<TChainInput, TGeneratorFn, TGenerator>;
		using Item = typename TGenerator::value_type;

		static inline IterValue<Item> next(Self& self) {
			while(true) {
				if(!self.currentGenerator.has_value()) {
					auto item = ChainInputIterator::next(self.input);
					if(!item.has_value()) { return {}; } // reached end
					self.currentGenerator.emplace(self.generatorFn( std::forward<InputItem>(item.value()) ));
				}

				auto item = self.currentGenerator.value().next();
				if(!item.has_value()) {
					self.currentGenerator.reset();
					continue;
				}
				return item;
			}
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};

}

#endif
