#pragma once

#include "IterValue.h"
#include "SizeHint.h"

/**
 * @brief Trait namespace. This namespaces contains all public traits that cover all of the iterator's inner workings.
 */
namespace CXXIter::trait {

	// ################################################################################################
	// ITERATOR TRAITS
	// ################################################################################################

	/**
	* @brief Trait, that is used for the chaining and the operation of iterator pipelines.
	* @details This allows making any class or struct iterable, to be able to interact with CXXIter's
	* iterator pipelines. It essentially provides two functions:
	* - One that delivers a hint about the iterator's size after the current element implementing
	*   the trait @c CXXIter::trait::Iterator
	* - Method that allows pulling one element from the iterator pipeline.
	*/
	template<typename T>
	struct Iterator {
		/**
		* @brief Self-Type. This is the type of the struct for which the trait::Iterator is being specialized.
		*/
		using Self = trait::Iterator<T>;
		/**
		* @brief Item-Type. This is the type of elements that can be pulled from this pipeline-element.
		*/
		using Item = void;

		/**
		* @brief Pull one element from the iterator pipeline previous to this pipeline-element.
		* @param self Reference to the instance of the class for which trait::Iterator is being specialized.
		* @return An element (if any) wrapped in the CXXIter::IterValue.
		*/
		static constexpr inline IterValue<Item> next(Self& self) = delete;

		/**
		* @brief Get the bounds on the remaining length of the iterator pipeline until this pipeline-element,
		* estimated from the source and all of the chained iterations until after this pipeline-element.
		* @return The estimated bounds on the remaining length of the iterator pipeline until after this pipeline-element.
		*/
		static constexpr inline SizeHint sizeHint(const Self& self) = delete;

		/**
		 * @brief Advance the iterator by n elements.
		 * @details If possible, this should be forwarded to the previous pipeline element, because then the
		 * pipeline source has the possibility to implement this as a O(1) operation, if supported.
		 * @param n The amount of elements to advance the iterator by.
		 * @return The amount of elements by which the iterator was actually advanced (e.g. if the iterator
		 * did have less than @p n elements remaining)
		 */
		static constexpr inline size_t advanceBy(Self& self, size_t n) = delete;
	};

	/**
	* @brief Trait, that extends iterators for which an exact length is known.
	*/
	template<typename T>
	struct ExactSizeIterator {
		/**
		* @brief Get the iterator's exact number of elements.
		* @param self Reference to the instance of the class for which trait::ExactSizeIterator is being specialized.
		* @return This iterator's exact number of elements.
		*/
		static constexpr inline size_t size(const typename trait::Iterator<T>::Self& self) = delete;
	};

	/**
	 * @brief Trait, that iterators implement whose elements are stored in a contiguous block of memory.
	 * @details This trait allows downstream elements to assume that it is valid to access following
	 * elements by reading from the current element onward.
	 */
	template<typename T>
	struct ContiguousMemoryIterator {
		/**
		 * @brief Pointer type to an item of this iterator.
		 * @details This keeps the const specifier.
		 */
		using ItemPtr = std::add_pointer_t<std::remove_reference_t<typename Iterator<T>::Item>>;
		/**
		 * @brief Get the pointer of the current element.
		 * @param self Reference to the instance of the class for which trait::ExactSizeIterator is being specialized.
		 * @return Pointer of the element that would be returned by the next call to next().
		 */
		static constexpr inline ItemPtr currentPtr(typename trait::Iterator<T>::Self& self) = delete;
	};

	/**
	* @brief Trait that extends the Iterator trait with double-ended functionality.
	* @details Implementing this trait extends the iterator with the functionality to pull elements from the back.
	* This can be arbitrarily mixed. Elements can e.g. be retrieved from front and back alternatingly.
	*/
	template<typename T>
	struct DoubleEndedIterator {
		using Self = typename trait::Iterator<T>::Self;

		/**
		* @brief Pull the next last element from the iterator pipeline previous to this pipeline-element.
		* @param self Reference to the instance of the class for which trait::DoubleEndedIterator is being specialized.
		* @return An element (if any) wrapped in the CXXIter::IterValue.
		*/
		static constexpr inline IterValue<typename Iterator<T>::Item> nextBack(Self& self) = delete;
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
	template<typename TContainer> struct Source {
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
		struct IteratorState {
			/** Iteration range left (start) */
			typename TContainer::iterator left;
			/** Iteration range right (end) */
			typename TContainer::iterator right;
		};

		/**
		 * @brief Type of the state structure stored in CXXIter's source classes, used to keep track of the iteration progress.
		 * @details This is used for @c CXXIter::SrcCRef
		 */
		struct ConstIteratorState {
			/** Iteration range left (start) */
			typename TContainer::const_iterator left;
			/** Iteration range right (end) */
			typename TContainer::const_iterator right;
		};

		/**
		 * @brief Report a size hint for a source on the given @p container.
		 * @details This injects information about the source's size (element count) into the iterator API.
		 * @param container Container for which to generate a size hint.
		 * @return A size hint for the given @p container.
		 */
		static constexpr inline SizeHint sizeHint(const TContainer& container) { return SizeHint(container.size(), container.size()); }

		/**
		 * @brief Return an initial @c IteratorState instance for iteration on the given @p container.
		 * @details This is stored within CXXIter's source classes, to hold the iteration's state.
		 * This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		 * @param container Container on which the source runs.
		 * @return Instance of @c IteratorState
		 */
		static constexpr inline IteratorState initIterator(TContainer& container) { return {container.begin(), container.end()}; }
		/**
		 * @brief Return an initial @c IteratorState instance for iteration on the given @p container.
		 * @details This is stored within CXXIter's source classes, to hold the iteration's state.
		 * This is used for @c CXXIter::SrcCRef
		 * @param container Container on which the source runs.
		 * @return Instance of @c ConstIteratorState
		 */
		static constexpr inline ConstIteratorState initIterator(const TContainer& container) { return {container.begin(), container.end()}; }

		/**
		 * @brief Checks whether there is a next item in the iteration with the given @p iter state on the given @p container.
		 * @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @return @c true when there is another item available, @c false otherwise.
		 */
		static constexpr inline bool hasNext([[maybe_unused]] TContainer& container, IteratorState& iter) { return (iter.left != iter.right); }
		/**
		 * @brief Checks whether there is a next item in the iteration with the given @p iter state on the given @p container.
		 * @details This is used for @c CXXIter::SrcCRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @return @c true when there is another item available, @c false otherwise.
		 */
		static constexpr inline bool hasNext([[maybe_unused]] const TContainer& container, ConstIteratorState& iter) { return (iter.left != iter.right); }

		/**
		 * @brief Return the next item in the iteration with the given @p iter state on the given @p container.
		 * @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @return The next item from the current iteration.
		 */
		static constexpr inline ItemRef next([[maybe_unused]] TContainer& container, IteratorState& iter) { return (*iter.left++); }
		/**
		 * @brief Return the next item in the iteration with the given @p iter state on the given @p container.
		 * @details This is used for @c CXXIter::SrcCRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @return The next item from the current iteration.
		 */
		static constexpr inline ItemConstRef next([[maybe_unused]] const TContainer& container, ConstIteratorState& iter) { return (*iter.left++); }

		/**
		 * @brief Return the next item in the iteration with the given @p iter state on the given @p container, without advancing.
		 * @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @return The next item from the current iteration.
		 * @attention Calling this when @c hasNext() returns @c false is undefined behavior!
		 */
		static constexpr inline ItemRef peekNext([[maybe_unused]] TContainer& container, IteratorState& iter) { return (*iter.left); }
		/**
		 * @brief Return the next item in the iteration with the given @p iter state on the given @p container, without advancing.
		 * @details This is used for @c CXXIter::SrcCRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @return The next item from the current iteration.
		 * @attention Calling this when @c hasNext() returns @c false is undefined behavior!
		 */
		static constexpr inline ItemConstRef peekNext([[maybe_unused]] const TContainer& container, ConstIteratorState& iter) { return (*iter.left); }

		/**
		 * @brief Skip the next @p n elements from the container.
		 * @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @param n The amount of elements to skip.
		 * @return The amount of actually skipped elements (e.g. if there were less than @p n elements remaining).
		 */
		static constexpr inline size_t skipN([[maybe_unused]] const TContainer& container, IteratorState& iter, size_t n) {
			size_t skipN = std::min(n, static_cast<size_t>(std::distance(iter.left, iter.right)));
			iter.left += skipN;
			return skipN;
		}
		/**
		 * @brief Skip the next @p n elements from the container.
		 * @details This is used for @c CXXIter::SrcCRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @param n The amount of elements to skip.
		 * @return The amount of actually skipped elements (e.g. if there were less than @p n elements remaining).
		 */
		static constexpr inline size_t skipN([[maybe_unused]] const TContainer& container, ConstIteratorState& iter, size_t n) {
			size_t skipN = std::min(n, static_cast<size_t>(std::distance(iter.left, iter.right)));
			iter.left += skipN;
			return skipN;
		}

	/**
	 * @name Double-Ended iterator functionality (optional)
	 */
	//@{
		/**
		 * @brief Return the next item from the back of the iteration with the given @p iter state on the given @p container.
		 * @note Implementing this is optional, since not all containers can support this.
		 * @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @return The next item from the back of the current iteration.
		 */
		static constexpr inline ItemRef nextBack([[maybe_unused]] TContainer& container, IteratorState& iter) { return (*--iter.right); }
		/**
		 * @brief Return the next item from the back of the iteration with the given @p iter state on the given @p container.
		 * @note Implementing this is optional, since not all containers can support this.
		 * @details This is used for @c CXXIter::SrcCRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @return The next item from the back of the current iteration.
		 */
		static constexpr inline ItemConstRef nextBack([[maybe_unused]] const TContainer& container, ConstIteratorState& iter) { return (*--iter.right); }

		/**
		 * @brief Skip the next n elements from the back of this iterator.
		 * @details This is used for @c CXXIter::SrcMov and @c CXXIter::SrcRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @param n The amount of elements to skip.
		 * @return The amount of actually skipped elements (e.g. if there were less than @p n elements remaining).
		 */
		static constexpr inline size_t skipNBack([[maybe_unused]] const TContainer& container, IteratorState& iter, size_t n) {
			size_t skipN = std::min(n, static_cast<size_t>(std::distance(iter.left, iter.right)));
			iter.right -= skipN;
			return skipN;
		}
		/**
		 * @brief Skip the next n elements from the back of this iterator.
		 * @details This is used for @c CXXIter::SrcCRef
		 * @param container Container on which the current iteration is running.
		 * @param iter The current iteration's state structure.
		 * @param n The amount of elements to skip.
		 * @return The amount of actually skipped elements (e.g. if there were less than @p n elements remaining).
		 */
		static constexpr inline size_t skipNBack([[maybe_unused]] const TContainer& container, ConstIteratorState& iter, size_t n) {
			size_t skipN = std::min(n, static_cast<size_t>(std::distance(iter.left, iter.right)));
			iter.right -= skipN;
			return skipN;
		}
	//}@

	};

}
