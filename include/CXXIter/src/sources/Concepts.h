#pragma once

#include "../Traits.h"

namespace CXXIter::concepts {

	/**
		* @brief Concept that checks whether the given @p TContainer is supported by CXXIter's standard source
		* classes CXXIter::SrcMov, CXXIter::SrcRef and CXXIter::SrcCRef.
		* @details The concept does these checks by testing whether the CXXIter::trait::Source was properly specialized
		* for the given @p TContainer type.
		*
		* @see CXXIter::SourceTrait for further details on this.
		*/
	template<typename TContainer>
	concept SourceContainer = requires(
		TContainer& container,
		const TContainer& constContainer,
		typename trait::Source<TContainer>::IteratorState& iterState,
		typename trait::Source<TContainer>::ConstIteratorState& constIterState,
		size_t n
		) {
		typename trait::Source<TContainer>::Item;
		typename trait::Source<TContainer>::ItemRef;
		typename trait::Source<TContainer>::ItemConstRef;
		typename trait::Source<TContainer>::IteratorState;
		typename trait::Source<TContainer>::ConstIteratorState;

		{trait::Source<TContainer>::initIterator(container)} -> std::same_as<typename trait::Source<TContainer>::IteratorState>;
		{trait::Source<TContainer>::initIterator(constContainer)} -> std::same_as<typename trait::Source<TContainer>::ConstIteratorState>;

		{trait::Source<TContainer>::hasNext(container, iterState)} -> std::same_as<bool>;
		{trait::Source<TContainer>::hasNext(constContainer, constIterState)} -> std::same_as<bool>;

		{trait::Source<TContainer>::next(container, iterState)} -> std::same_as<typename trait::Source<TContainer>::ItemRef>;
		{trait::Source<TContainer>::next(constContainer, constIterState)} -> std::same_as<typename trait::Source<TContainer>::ItemConstRef>;

		{trait::Source<TContainer>::peekNext(container, iterState)} -> std::same_as<typename trait::Source<TContainer>::ItemRef>;
		{trait::Source<TContainer>::peekNext(constContainer, constIterState)} -> std::same_as<typename trait::Source<TContainer>::ItemConstRef>;

		{trait::Source<TContainer>::skipN(container, iterState, n)} -> std::same_as<size_t>;
		{trait::Source<TContainer>::skipN(container, constIterState, n)} -> std::same_as<size_t>;
	};

	/**
		 * @brief Concept that checks whether the given @p TContainer supports double-ended iteration when using CXXIter's
		 * standard source classes CXXIter::SrcMov, CXXIter::SrcRef and CXXIter::SrcCRef.
		 * @details The concept does these checks by testing whether the optional double-ended part in CXXIter::trait::Source
		 * was properly provided by the specialization for the given @p TContainer type.
		 *
		 * @see CXXIter::Source for further details on this.
		 */
	template<typename TContainer>
	concept DoubleEndedSourceContainer = SourceContainer<TContainer> && requires(
		TContainer& container,
		const TContainer& constContainer,
		typename trait::Source<TContainer>::IteratorState& iterState,
		typename trait::Source<TContainer>::ConstIteratorState& constIterState,
		size_t n
		) {

		{trait::Source<TContainer>::nextBack(container, iterState)} -> std::same_as<typename trait::Source<TContainer>::ItemRef>;
		{trait::Source<TContainer>::nextBack(constContainer, constIterState)} -> std::same_as<typename trait::Source<TContainer>::ItemConstRef>;

		{trait::Source<TContainer>::skipNBack(container, iterState, n)} -> std::same_as<size_t>;
		{trait::Source<TContainer>::skipNBack(container, constIterState, n)} -> std::same_as<size_t>;
	};

}
