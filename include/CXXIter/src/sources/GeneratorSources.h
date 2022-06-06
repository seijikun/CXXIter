#pragma once

#include <cstdlib>
#include <optional>

#include "../Common.h"
#include "../util/TraitImpl.h"

namespace CXXIter {

	// ################################################################################################
	// GENERATOR EMPTY
	// ################################################################################################
	/** @private */
	template<typename TItem>
	class Empty : public IterApi<Empty<TItem>> {
		friend struct trait::Iterator<Empty<TItem>>;
		friend struct trait::ExactSizeIterator<Empty<TItem>>;
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TItem>
	struct trait::Iterator<Empty<TItem>> {
		// CXXIter Interface
		using Self = Empty<TItem>;
		using Item = TItem;

		static inline IterValue<Item> next(Self&) { return {}; }
		static inline SizeHint sizeHint(const Self&) { return SizeHint(0, 0); }
		static inline size_t advanceBy(Self& self, size_t n) { return 0; }
	};
	/** @private */
	template<typename TItem>
	struct trait::ExactSizeIterator<Empty<TItem>> {
		static inline size_t size(const Empty<TItem>&) { return 0; }
	};


	// ################################################################################################
	// GENERATOR FUNCTION
	// ################################################################################################
	/** @private */
	template<typename TItem, typename TGeneratorFn>
	class FunctionGenerator : public IterApi<FunctionGenerator<TItem, TGeneratorFn>> {
		friend struct trait::Iterator<FunctionGenerator<TItem, TGeneratorFn>>;
		friend struct trait::ExactSizeIterator<FunctionGenerator<TItem, TGeneratorFn>>;
	private:
		TGeneratorFn generatorFn;
	public:
		FunctionGenerator(TGeneratorFn generatorFn) : generatorFn(generatorFn) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TItem, typename TGeneratorFn>
	struct trait::Iterator<FunctionGenerator<TItem, TGeneratorFn>> {
		// CXXIter Interface
		using Self = FunctionGenerator<TItem, TGeneratorFn>;
		using Item = TItem;

		static inline IterValue<Item> next(Self& self) {
			auto item = self.generatorFn();
			if(!item.has_value()) [[unlikely]] { return {}; }
			return item.value();
		}
		static inline SizeHint sizeHint(const Self&) { return SizeHint(); }
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};



	// ################################################################################################
	// GENERATOR REPEAT
	// ################################################################################################
	/** @private */
	template<typename TItem>
	class Repeater : public IterApi<Repeater<TItem>> {
		friend struct trait::Iterator<Repeater<TItem>>;
		friend struct trait::ExactSizeIterator<Repeater<TItem>>;
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
	struct trait::Iterator<Repeater<TItem>> {
		// CXXIter Interface
		using Self = Repeater<TItem>;
		using Item = TItem;

		static inline IterValue<Item> next(Self& self) {
			if(self.repetitions.has_value()) {
				if(self.repetitionsRemaining == 0) { return {}; }
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
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
	/** @private */
	template<typename TItem>
	struct trait::ExactSizeIterator<Repeater<TItem>> {
		static inline size_t size(const Repeater<TItem>& self) { return trait::Iterator<Repeater<TItem>>::sizeHint(self).lowerBound; }
	};



	// ################################################################################################
	// GENERATOR RANGE
	// ################################################################################################
	/** @private */
	template<typename TValue>
	class Range : public IterApi<Range<TValue>> {
		friend struct trait::Iterator<Range<TValue>>;
		friend struct trait::ExactSizeIterator<Range<TValue>>;
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
	struct trait::Iterator<Range<TValue>> {
		// CXXIter Interface
		using Self = Range<TValue>;
		using Item = TValue;

		static inline IterValue<Item> next(Self& self) {
			if(self.current > self.to) [[unlikely]] { return {}; }
			TValue current = self.current;
			self.current += self.step;
			return current;
		}
		static inline SizeHint sizeHint(const Self& self) {
			size_t cnt = static_cast<size_t>((self.to - self.from) / self.step) + 1;
			return SizeHint(cnt, cnt);
		}
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
	/** @private */
	template<typename TItem>
	struct trait::ExactSizeIterator<Range<TItem>> {
		static inline size_t size(const Range<TItem>& self) { return trait::Iterator<Range<TItem>>::sizeHint(self).lowerBound; }
	};



#ifdef CXXITER_HAS_COROUTINE
	// ################################################################################################
	// COROUTINE GENERATOR
	// ################################################################################################
	/** @private */
	template<typename TGenerator>
	class CoroutineGenerator : public IterApi<CoroutineGenerator<TGenerator>> {
		friend struct trait::Iterator<CoroutineGenerator<TGenerator>>;
		friend struct trait::ExactSizeIterator<CoroutineGenerator<TGenerator>>;
	private:
		TGenerator generator;
	public:
		CoroutineGenerator(TGenerator&& generator) : generator(std::forward<TGenerator>(generator)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TGeneratorFn>
	struct trait::Iterator<CoroutineGenerator<TGeneratorFn>> {
		// CXXIter Interface
		using Self = CoroutineGenerator<TGeneratorFn>;
		using Item = typename TGeneratorFn::value_type;

		static inline IterValue<Item> next(Self& self) {
			return self.generator.next();
		}
		static inline SizeHint sizeHint(const Self&) { return SizeHint(); }
		static inline size_t advanceBy(Self& self, size_t n) { return util::advanceByPull(self, n); }
	};
#endif

}
