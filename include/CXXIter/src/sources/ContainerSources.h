#pragma once

#include <memory>

#include "../Common.h"

namespace CXXIter {

	// ################################################################################################
	// SOURCE (MOVE / CONSUME)
	// ################################################################################################

	/**
	 * @brief CXXIter iterator source that takes over the input item source, and moves its items
	 * through the element stream, essentially "consuming" them.
	 */
	template<typename TContainer>
	requires SourceContainer<std::remove_cvref_t<TContainer>>
	class SrcMov : public IterApi<SrcMov<TContainer>> {
		friend struct trait::Iterator<SrcMov<TContainer>>;
		friend struct trait::DoubleEndedIterator<SrcMov<TContainer>>;
		friend struct trait::ExactSizeIterator<SrcMov<TContainer>>;
		using Src = trait::Source<TContainer>;
	private:
		std::unique_ptr<TContainer> container;
		typename Src::IteratorState iter;
	public:
		SrcMov(TContainer&& container) : container(std::make_unique<TContainer>(std::move(container))), iter(Src::initIterator(*this->container)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TContainer>
	struct trait::Iterator<SrcMov<TContainer>> {
		using Src = trait::Source<TContainer>;
		// CXXIter Interface
		using Self = SrcMov<TContainer>;
		using Item = typename Src::Item;

		static constexpr inline IterValue<Item> next(Self& self) {
			if(!Src::hasNext(*self.container, self.iter)) [[unlikely]] { return {}; }
			return std::move(Src::next(*self.container, self.iter));
		}
		static constexpr inline SizeHint sizeHint(const Self& self) { return Src::sizeHint(*self.container); }
		static constexpr inline size_t advanceBy(Self& self, size_t n) {
			return Src::skipN(*self.container, self.iter, n);
		}
	};
	/** @private */
	template<typename TContainer>
	requires DoubleEndedSourceContainer<std::remove_cvref_t<TContainer>>
	struct trait::DoubleEndedIterator<SrcMov<TContainer>> {
		using Src = trait::Source<TContainer>;
		using Item = typename Src::Item;

		// CXXIter Interface
		static constexpr inline IterValue<Item> nextBack(SrcMov<TContainer>& self) {
			if(!Src::hasNext(*self.container, self.iter)) [[unlikely]] { return {}; }
			return std::move(Src::nextBack(*self.container, self.iter));
		}
	};
	/** @private */
	template<typename TContainer>
	struct trait::ExactSizeIterator<SrcMov<TContainer>> {
		static constexpr inline size_t size(const SrcMov<TContainer>& self) { return self.container->size(); }
	};



	// ################################################################################################
	// SOURCE (MUTABLE REFERENCE)
	// ################################################################################################

	/**
	 * @brief CXXIter iterator source that mutably borrows the input item source, and passes mutable
	 * references to the items of the source through the iterator.
	 * @details This allows the iterator to modify the items while leaving them in the original
	 * item source.
	 */
	template<typename TContainer>
	requires SourceContainer<std::remove_cvref_t<TContainer>>
	class SrcRef : public IterApi<SrcRef<TContainer>> {
		friend struct trait::Iterator<SrcRef<TContainer>>;
		friend struct trait::DoubleEndedIterator<SrcRef<TContainer>>;
		friend struct trait::ExactSizeIterator<SrcRef<TContainer>>;
		using Src = trait::Source<TContainer>;
	private:
		TContainer& container;
		typename Src::IteratorState iter;
	public:
		SrcRef(TContainer& container) : container(container), iter(Src::initIterator(this->container)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TContainer>
	struct trait::Iterator<SrcRef<TContainer>> {
		using Src = trait::Source<TContainer>;
		// CXXIter Interface
		using Self = SrcRef<TContainer>;
		using Item = typename Src::ItemRef;

		static constexpr inline IterValue<Item> next(Self& self) {
			if(!Src::hasNext(self.container, self.iter)) [[unlikely]] { return {}; }
			return Src::next(self.container, self.iter);
		}
		static constexpr inline SizeHint sizeHint(const Self& self) { return Src::sizeHint(self.container); }
		static constexpr inline size_t advanceBy(Self& self, size_t n) {
			return Src::skipN(self.container, self.iter, n);
		}
	};
	/** @private */
	template<typename TContainer>
	requires DoubleEndedSourceContainer<std::remove_cvref_t<TContainer>>
	struct trait::DoubleEndedIterator<SrcRef<TContainer>> {
		using Src = trait::Source<TContainer>;
		using Item = typename Src::ItemRef;

		// CXXIter Interface
		static constexpr inline IterValue<Item> nextBack(SrcRef<TContainer>& self) {
			if(!Src::hasNext(self.container, self.iter)) [[unlikely]] { return {}; }
			return Src::nextBack(self.container, self.iter);
		}
	};
	/** @private */
	template<typename TContainer>
	struct trait::ExactSizeIterator<SrcRef<TContainer>> {
		static constexpr inline size_t size(const SrcRef<TContainer>& self) { return self.container.size(); }
	};



	// ################################################################################################
	// SOURCE (CONST REFERENCE)
	// ################################################################################################

	/**
	 * @brief CXXIter iterator source that immutably borrows the input item source, and passes immutable
	 * references to the items of the source through the iterator.
	 * @details This guarantees the original source to stay untouched & unmodified.
	 */
	template<typename TContainer>
	requires SourceContainer<std::remove_cvref_t<TContainer>>
	class SrcCRef : public IterApi<SrcCRef<TContainer>> {
		friend struct trait::Iterator<SrcCRef<TContainer>>;
		friend struct trait::DoubleEndedIterator<SrcCRef<TContainer>>;
		friend struct trait::ExactSizeIterator<SrcCRef<TContainer>>;
		using Src = trait::Source<TContainer>;
	private:
		const TContainer& container;
		typename Src::ConstIteratorState iter;
	public:
		constexpr SrcCRef(const TContainer& container) : container(container), iter(Src::initIterator(this->container)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TContainer>
	struct trait::Iterator<SrcCRef<TContainer>> {
		using Src = trait::Source<TContainer>;
		// CXXIter Interface
		using Self = SrcCRef<TContainer>;
		using Item = typename Src::ItemConstRef;

		static constexpr inline IterValue<Item> next(Self& self) {
			if(!Src::hasNext(self.container, self.iter)) [[unlikely]] { return {}; }
			return Src::next(self.container, self.iter);
		}
		static constexpr inline SizeHint sizeHint(const Self& self) { return Src::sizeHint(self.container); }
		static constexpr inline size_t advanceBy(Self& self, size_t n) {
			return Src::skipN(self.container, self.iter, n);
		}
	};
	/** @private */
	template<typename TContainer>
	requires DoubleEndedSourceContainer<std::remove_cvref_t<TContainer>>
	struct trait::DoubleEndedIterator<SrcCRef<TContainer>> {
		using Src = trait::Source<TContainer>;
		using Item = typename Src::ItemConstRef;

		// CXXIter Interface
		static constexpr inline IterValue<Item> nextBack(SrcCRef<TContainer>& self) {
			if(!Src::hasNext(self.container, self.iter)) [[unlikely]] { return {}; }
			return Src::nextBack(self.container, self.iter);
		}
	};
	/** @private */
	template<typename TContainer>
	struct trait::ExactSizeIterator<SrcCRef<TContainer>> {
		static constexpr inline size_t size(const SrcCRef<TContainer>& self) { return self.container.size(); }
	};

}
