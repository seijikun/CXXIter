#pragma once

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
		friend struct IteratorTrait<SrcMov<TContainer>>;
		friend struct ExactSizeIteratorTrait<SrcMov<TContainer>>;
		using Src = SourceTrait<TContainer>;
	private:
		TContainer container;
		typename Src::IteratorState iter;
	public:
		SrcMov(TContainer&& container) : container(std::move(container)), iter(Src::initIterator(this->container)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TContainer>
	struct IteratorTrait<SrcMov<TContainer>> {
		using Src = SourceTrait<TContainer>;
		// CXXIter Interface
		using Self = SrcMov<TContainer>;
		using Item = typename Src::Item;

		static inline Item next(Self& self) {
			if(!Src::hasNext(self.container, self.iter)) [[unlikely]] { throw IteratorEndedException{}; }
			return std::move(Src::next(self.container, self.iter));
		}
		static inline SizeHint sizeHint(const Self& self) { return Src::sizeHint(self.container); }
	};
	/** @private */
	template<typename TContainer>
	struct ExactSizeIteratorTrait<SrcMov<TContainer>> {
		static inline size_t size(const SrcMov<TContainer>& self) { return self.container.size(); }
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
		friend struct IteratorTrait<SrcRef<TContainer>>;
		friend struct ExactSizeIteratorTrait<SrcRef<TContainer>>;
		using Src = SourceTrait<TContainer>;
	private:
		TContainer& container;
		typename Src::IteratorState iter;
	public:
		SrcRef(TContainer& container) : container(container), iter(Src::initIterator(this->container)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TContainer>
	struct IteratorTrait<SrcRef<TContainer>> {
		using Src = SourceTrait<TContainer>;
		// CXXIter Interface
		using Self = SrcRef<TContainer>;
		using Item = typename Src::Item&;

		static inline Item next(Self& self) {
			if(!Src::hasNext(self.container, self.iter)) [[unlikely]] { throw IteratorEndedException{}; }
			return Src::next(self.container, self.iter);
		}
		static inline SizeHint sizeHint(const Self& self) { return Src::sizeHint(self.container); }
	};
	/** @private */
	template<typename TContainer>
	struct ExactSizeIteratorTrait<SrcRef<TContainer>> {
		static inline size_t size(const SrcRef<TContainer>& self) { return self.container.size(); }
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
		friend struct IteratorTrait<SrcCRef<TContainer>>;
		friend struct ExactSizeIteratorTrait<SrcCRef<TContainer>>;
		using Src = SourceTrait<TContainer>;
	private:
		const TContainer& container;
		typename Src::ConstIteratorState iter;
	public:
		SrcCRef(const TContainer& container) : container(container), iter(Src::initIterator(this->container)) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TContainer>
	struct IteratorTrait<SrcCRef<TContainer>> {
		using Src = SourceTrait<TContainer>;
		// CXXIter Interface
		using Self = SrcCRef<TContainer>;
		using Item = const typename Src::Item&;

		static inline Item next(Self& self) {
			if(!Src::hasNext(self.container, self.iter)) [[unlikely]] { throw IteratorEndedException{}; }
			return Src::next(self.container, self.iter);
		}
		static inline SizeHint sizeHint(const Self& self) { return Src::sizeHint(self.container); }
	};
	/** @private */
	template<typename TContainer>
	struct ExactSizeIteratorTrait<SrcCRef<TContainer>> {
		static inline size_t size(const SrcCRef<TContainer>& self) { return self.container.size(); }
	};

}
