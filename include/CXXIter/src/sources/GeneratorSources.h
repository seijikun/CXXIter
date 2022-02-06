#pragma once

#include <cstdlib>
#include <optional>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// GENERATOR EMPTY
	// ################################################################################################
	/** @private */
	template<typename TItem>
	class Empty : public IterApi<Empty<TItem>> {
		friend struct IteratorTrait<Empty<TItem>>;
		friend struct ExactSizeIteratorTrait<Empty<TItem>>;
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TItem>
	struct IteratorTrait<Empty<TItem>> {
		// CXXIter Interface
		using Self = Empty<TItem>;
		using Item = TItem;

		static inline Item next(Self&) { throw IteratorEndedException{}; }
		static inline SizeHint sizeHint(const Self&) { return SizeHint(0, 0); }
	};
	/** @private */
	template<typename TItem>
	struct ExactSizeIteratorTrait<Empty<TItem>> {
		static inline size_t size(const Empty<TItem>&) { return 0; }
	};


	// ################################################################################################
	// GENERATOR FUNCTION
	// ################################################################################################
	/** @private */
	template<typename TItem, typename TGeneratorFn> //TODO: port to IterValue, to also support references as items?
	class FunctionGenerator : public IterApi<FunctionGenerator<TItem, TGeneratorFn>> {
		friend struct IteratorTrait<FunctionGenerator<TItem, TGeneratorFn>>;
		friend struct ExactSizeIteratorTrait<FunctionGenerator<TItem, TGeneratorFn>>;
	private:
		TGeneratorFn generatorFn;
	public:
		FunctionGenerator(TGeneratorFn generatorFn) : generatorFn(generatorFn) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TItem, typename TGeneratorFn>
	struct IteratorTrait<FunctionGenerator<TItem, TGeneratorFn>> {
		// CXXIter Interface
		using Self = FunctionGenerator<TItem, TGeneratorFn>;
		using Item = TItem;

		static inline Item next(Self& self) {
			auto item = self.generatorFn();
			if(!item.has_value()) [[unlikely]] { throw IteratorEndedException{}; }
			return item.value();
		}
		static inline SizeHint sizeHint(const Self&) { return SizeHint(); }
	};



	// ################################################################################################
	// GENERATOR REPEAT
	// ################################################################################################
	/** @private */
	template<typename TItem>
	class Repeater : public IterApi<Repeater<TItem>> {
		friend struct IteratorTrait<Repeater<TItem>>;
		friend struct ExactSizeIteratorTrait<Repeater<TItem>>;
	private:
		TItem item;
		std::optional<size_t> repetitions;
		size_t repetitionsRemaining;
	public:
		Repeater(const TItem& item, std::optional<size_t> repetitions) : item(item), repetitions(repetitions), repetitionsRemaining(repetitions.value_or(0)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TItem>
	struct IteratorTrait<Repeater<TItem>> {
		// CXXIter Interface
		using Self = Repeater<TItem>;
		using Item = TItem;

		static inline Item next(Self& self) {
			if(self.repetitions.has_value()) {
				if(self.repetitionsRemaining == 0) { throw IteratorEndedException{}; }
				self.repetitionsRemaining -= 1;
			}
			return self.item;
		}
		static inline SizeHint sizeHint(const Self& self) {
			return SizeHint(
				self.repetitions.value_or(SizeHint::INFINITE),
				self.repetitions
			);
		}
	};
	/** @private */
	template<typename TItem>
	struct ExactSizeIteratorTrait<Repeater<TItem>> {
		static inline size_t size(const Repeater<TItem>& self) { return IteratorTrait<Repeater<TItem>>::sizeHint(self).lowerBound; }
	};



	// ################################################################################################
	// GENERATOR RANGE
	// ################################################################################################
	/** @private */
	template<typename TValue>
	class Range : public IterApi<Range<TValue>> {
		friend struct IteratorTrait<Range<TValue>>;
		friend struct ExactSizeIteratorTrait<Range<TValue>>;
	private:
		TValue current;
		TValue from;
		TValue to;
		TValue step;
	public:
		Range(TValue from, TValue to, TValue step) : current(from), from(from), to(to), step(step) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TValue>
	struct IteratorTrait<Range<TValue>> {
		// CXXIter Interface
		using Self = Range<TValue>;
		using Item = TValue;

		static inline Item next(Self& self) {
			if(self.current > self.to) [[unlikely]] { throw IteratorEndedException{}; }
			TValue current = self.current;
			self.current += self.step;
			return current;
		}
		static inline SizeHint sizeHint(const Self& self) {
			size_t cnt = static_cast<size_t>((self.to - self.from) / self.step) + 1;
			return SizeHint(cnt, cnt);
		}
	};
	/** @private */
	template<typename TItem>
	struct ExactSizeIteratorTrait<Range<TItem>> {
		static inline size_t size(const Range<TItem>& self) { return IteratorTrait<Range<TItem>>::sizeHint(self).lowerBound; }
	};



#ifdef CXXITER_HAS_COROUTINE
	// ################################################################################################
	// COROUTINE GENERATOR
	// ################################################################################################
	/** @private */
	template<typename TGenerator>
	class CoroutineGenerator : public IterApi<CoroutineGenerator<TGenerator>> {
		friend struct IteratorTrait<CoroutineGenerator<TGenerator>>;
		friend struct ExactSizeIteratorTrait<CoroutineGenerator<TGenerator>>;
	private:
		TGenerator generator;
	public:
		CoroutineGenerator(TGenerator&& generator) : generator(std::forward<TGenerator>(generator)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TGeneratorFn>
	struct IteratorTrait<CoroutineGenerator<TGeneratorFn>> {
		// CXXIter Interface
		using Self = CoroutineGenerator<TGeneratorFn>;
		using Item = typename TGeneratorFn::value_type;

		static inline Item next(Self& self) {
			auto item = self.generator.next();
			if(!item.has_value()) { throw IteratorEndedException{}; }
			return item.value();
		}
		static inline SizeHint sizeHint(const Self&) { return SizeHint(); }
	};
#endif

}
