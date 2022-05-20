#pragma once

#include "IterValue.h"
#include "SizeHint.h"

namespace CXXIter::trait {

	// ################################################################################################
	// ITERATOR TRAITS
	// ################################################################################################

	/**
	* @brief Trait, that is used for the chaining and the operation of iterator pipelines.
	* @details This allows making any class or struct iterable, to be able to interact with CXXIter's
	* iterator pipelines. It essentially provides two functions:
	* - One that delivers a hint about the iterator's size after the current element implementing
	*   the CXXIter::IteratorTrait
	* - Method that allows pulling one element from the iterator pipeline.
	*/
	template<typename T>
	struct IteratorTrait {
		/**
		* @brief Self-Type. This is the type of the struct for which the trait::IteratorTrait is being specialized.
		*/
		using Self = trait::IteratorTrait<T>;
		/**
		* @brief Item-Type. This is the type of elements that can be pulled from this pipeline-element.
		*/
		using Item = void;

		/**
		* @brief Pull one element from the iterator pipeline previous to this pipeline-element.
		* @param self Reference to the instance of the class for which trait::IteratorTrait is being specialized.
		* @return An element (if any) wrapped in the CXXIter::IterValue.
		*/
		static inline IterValue<Item> next(Self& self) = delete;

		/**
		* @brief Get the bounds on the remaining length of the iterator pipeline until this pipeline-element,
		* estimated from the source and all of the chained iterations until after this pipeline-element.
		* @return The estimated bounds on the remaining length of the iterator pipeline until after this pipeline-element.
		*/
		static inline SizeHint sizeHint(const Self& self) = delete;
	};

	/**
	* @brief Trait, that extends iterators for which an exact length is known.
	*/
	template<typename T>
	struct ExactSizeIteratorTrait {
		/**
		* @brief Get the iterator's exact number of elements.
		* @param self Reference to the instance of the class for which trait::ExactSizeIteratorTrait is being specialized.
		* @return This iterator's exact number of elements.
		*/
		static inline size_t size(const typename trait::IteratorTrait<T>::Self& self) = delete;
	};


	// ################################################################################################
	// SOURCE TRAITS
	// ################################################################################################

	/**
	* @brief SourceTrait, that is used by CXXIter's standard source classes @c CXXIter::SrcMov, @c CXXIter::SrcRef and @c CXXIter::SrcCRef.
	* @details If you want to add support for your own containers to these sources, and thus to @c CXXIter::from() calls,
	* create a template specialization of @c CXXIter::SourceTrait, for your container class.
	*
	* This is the default implementation supporting all STL containers.
	*/
	template<typename TContainer> struct SourceTrait {
		/**
		* @brief Type of the item @p TContainer holds and provides for the iterator.
		*/
		using Item = typename TContainer::value_type;
		/**
		* @brief Type of the item @p TContainer holds and provides for the iterator, in referenced form.
		*/
		using ItemRef = typename TContainer::reference;
		/**
		* @brief Type of the item @p TContainer holds and provides for the iterator, in const referenced form.
		*/
		using ItemConstRef = typename TContainer::const_reference;
		/**
		* @brief Type of the state structure stored in CXXIter's source classes, used to keep track of the iteration progress.
		* @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		*/
		using IteratorState = typename TContainer::iterator;
		/**
		* @brief Type of the state structure stored in CXXIter's source classes, used to keep track of the iteration progress.
		* @details This is used for @c CXXIter::SrcCRef
		*/
		using ConstIteratorState = typename TContainer::const_iterator;

		/**
		* @brief Report a size hint for a source on the given @p container.
		* @details This injects information about the source's size (element count) into the iterator API.
		* @param container Container for which to generate a size hint.
		* @return A size hint for the given @p container.
		*/
		static inline SizeHint sizeHint(const TContainer& container) { return SizeHint(container.size(), container.size()); }

		/**
		* @brief Return an initial @c IteratorState instance for iteration on the given @p container.
		* @details This is stored within CXXIter's source classes, to hold the iteration's state.
		* This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		* @param container Container on which the source runs.
		* @return Instance of @c IteratorState
		*/
		static inline IteratorState initIterator(TContainer& container) { return container.begin(); }
		/**
		* @brief Return an initial @c IteratorState instance for iteration on the given @p container.
		* @details This is stored within CXXIter's source classes, to hold the iteration's state.
		* This is used for @c CXXIter::SrcCRef
		* @param container Container on which the source runs.
		* @return Instance of @c ConstIteratorState
		*/
		static inline ConstIteratorState initIterator(const TContainer& container) { return container.begin(); }

		/**
		* @brief Checks whether there is a next item in the iteration with the given @p iter state on the given @p container.
		* @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		* @param container Container on which the current iteration is running.
		* @param iter The current iteration's state structure.
		* @return @c true when there is another item available, @c false otherwise.
		*/
		static inline bool hasNext(TContainer& container, IteratorState& iter) { return (iter != container.end()); }
		/**
		* @brief Checks whether there is a next item in the iteration with the given @p iter state on the given @p container.
		* @details This is used for @c CXXIter::SrcCRef
		* @param container Container on which the current iteration is running.
		* @param iter The current iteration's state structure.
		* @return @c true when there is another item available, @c false otherwise.
		*/
		static inline bool hasNext(const TContainer& container, ConstIteratorState& iter) { return (iter != container.end()); }

		/**
		* @brief Return the next item in the iteration with the given @p iter state on the given @p container.
		* @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		* @param container Container on which the current iteration is running.
		* @param iter The current iteration's state structure.
		* @return The next item from the current iteration.
		*/
		static inline ItemRef next([[maybe_unused]] TContainer& container, IteratorState& iter) { return (*iter++); }
		/**
		* @brief Return the next item in the iteration with the given @p iter state on the given @p container.
		* @details This is used for @c CXXIter::SrcCRef
		* @param container Container on which the current iteration is running.
		* @param iter The current iteration's state structure.
		* @return The next item from the current iteration.
		*/
		static inline ItemConstRef next([[maybe_unused]] const TContainer& container, ConstIteratorState& iter) { return (*iter++); }
	};

}
