#pragma once

#include <limits>
#include <optional>
#include <algorithm>
#include <type_traits>

namespace CXXIter {

	// ################################################################################################
	// ITERATOR OPTIONAL (supports references)
	// ################################################################################################

	/** @private */
	template<typename TValue>
	class IterValue {};

	/**
	* @brief Container that is used to pass elements throught CXXIter iterator pipelines.
	* @details This is essentially a @c std::optional<> that also supports references (in comparison
	* to the original).
	*/
	template<typename TValue>
	requires (!std::is_reference_v<TValue>)
	class IterValue<TValue> {
		std::optional<TValue> inner;
	public:
		IterValue() {}
		IterValue(const TValue& value) : inner(value) {}
		IterValue(TValue&& value) : inner(std::forward<TValue>(value)) {}
		IterValue<TValue>& operator=(IterValue<TValue>&& o) = default;
		IterValue(IterValue<TValue>&& o) : inner(std::move(o.inner)) {
			o.inner.reset();
		};

		inline const TValue& value() const { return inner.value(); }
		inline TValue& value() { return inner.value(); }
		inline const TValue& value(TValue&& def) const { return inner.value_or(def); }
		inline TValue& value(TValue&& def) { return inner.value_or(def); }

		bool has_value() const { return inner.has_value(); }
		std::optional<TValue> toStdOptional() {
			return std::optional<TValue>(std::move(inner));
		}

		IterValue<TValue>& operator=(TValue&& o) {
			inner = std::forward<TValue>(o);
			return *this;
		}

		template<typename TOutValue, std::invocable<TValue&&> TMapFn>
		IterValue<TOutValue> map(TMapFn mapFn) {
			if(!has_value()) { return {}; }
			return mapFn(std::forward<TValue>(value()));
		}
	};

	template<typename TValue>
	requires std::is_reference_v<TValue>
	class IterValue<TValue> {
		using TValueDeref = std::remove_reference_t<TValue>;
		std::optional<std::reference_wrapper<TValueDeref>> inner;
	public:
		IterValue() {}
		IterValue(TValue value) : inner(value) {}
		IterValue<TValue>& operator=(IterValue<TValue>&& o) = default;
		IterValue(IterValue<TValue>&& o) : inner(std::move(o.inner)) {
			o.inner.reset();
		};

		inline const TValue& value() const { return inner.value(); }
		inline TValue& value() { return inner.value(); }
		inline const TValue& value(TValue&& def) const { return inner.value_or(def); }
		inline TValue& value(TValue&& def) { return inner.value_or(def); }

		bool has_value() const { return inner.has_value(); }
		std::optional<TValueDeref> toStdOptional() {
			if(inner.has_value()) { return inner.value(); }
			return {};
		}

		IterValue<TValue>& operator=(TValue&& o) {
			inner = std::forward<TValue>(o);
			return *this;
		}

		template<typename TOutValue, std::invocable<TValue> TMapFn>
		IterValue<TOutValue> map(TMapFn mapFn) {
			if(!has_value()) { return {}; }
			return mapFn(value());
		}
	};



	// ################################################################################################
	// PUBLIC STRUCTURES AND DEFINITIONS
	// ################################################################################################

	/**
	* @brief SortOrder for the item sorting methods.
	*/
	enum class SortOrder {
		ASCENDING,
		DESCENDING
	};
	/** Shortcut for SortOrder::ASCENDING in the CXXIter namespace */
	static constexpr SortOrder ASCENDING = SortOrder::ASCENDING;
	/** Shortcut for SortOrder::DESCENDING in the CXXIter namespace */
	static constexpr SortOrder DESCENDING = SortOrder::DESCENDING;

	/**
	* @brief Structure holding the bounds of a CXXIter iterator's estimated length.
	* @details This structure contains a lowerBound and an optional upper bound.
	* Both are initialized from the source's length (if any), and subsequently edited
	* by chained iteration accordingly.
	*/
	struct SizeHint {
		constexpr static size_t INFINITE = std::numeric_limits<size_t>::max();

		size_t lowerBound;
		std::optional<size_t> upperBound;

		size_t expectedResultSize(size_t min = 0) const { return std::min(min, upperBound.value_or(lowerBound)); }

		SizeHint(size_t lowerBound = 0, std::optional<size_t> upperBound = {}) : lowerBound(lowerBound), upperBound(upperBound) {}

		static std::optional<size_t> upperBoundMax(std::optional<size_t> upperBound1, std::optional<size_t> upperBound2) {
			if(!upperBound1.has_value() || !upperBound2.has_value()) { return {}; } // no upperbound is like Infinity -> higher
			return std::max(upperBound1.value(), upperBound2.value());
		}
		static std::optional<size_t> upperBoundMin(std::optional<size_t> upperBound1, std::optional<size_t> upperBound2) {
			if(!upperBound1.has_value()) { return upperBound2; }
			if(!upperBound2.has_value()) { return upperBound1; }
			return std::min(upperBound1.value(), upperBound2.value());
		}
		void subtract(size_t cnt) {
			lowerBound = (lowerBound > cnt) ? (lowerBound - cnt) : 0;
			if(upperBound) {
				upperBound = (upperBound.value() > cnt) ? (upperBound.value() - cnt) : 0;
			}
		}
		void add(const SizeHint& o) {
			lowerBound += o.lowerBound;
			if(upperBound.has_value() && o.upperBound.has_value()) {
				upperBound = upperBound.value() + o.upperBound.value();
			} else {
				upperBound = {};
			}
		}
	};

	// ################################################################################################
	// INTERNALS
	// ################################################################################################

	/** @private */
	namespace {
		#define CXXITER_CHAINER_NODISCARD_WARNING "The result of chainer methods needs to be used, otherwise the iterator will not be doing any work."

		template<size_t START, size_t END, typename F>
		constexpr bool constexpr_for(F&& f) {
			if constexpr (START < END) {
				if(f(std::integral_constant<size_t, START>()) == false) { return false; }
				if(constexpr_for<START + 1, END>(f) == false) { return false; }
			}
			return true;
		}

		//TODO: unit-tests
		template<typename T>
		struct SaturatingArithmetic {
			T value;
			SaturatingArithmetic(T value) : value(value) {}
			T get() const { return value; }

			SaturatingArithmetic<T> operator+(T o) {
				T res = value + o;
				res |= -(res < value);
				return res;
			}
			SaturatingArithmetic<T> operator-(T o) {
				T res = value - o;
				res &= -(res <= value);
				return res;
			}
			SaturatingArithmetic<T> operator/(T o) {
				return value / o;
			}
		};

		template <class T, class... Ts>
		constexpr bool are_same_v = (std::is_same_v<T, Ts> && ...);

		/**
		 * @brief Constraint that checks whether the given type @p T is an instantiation of
		 * the template class @p U.
		 */
		template <typename T, template <typename> typename U>
		constexpr bool is_template_instance_v = false;
		template <typename T, template <typename> typename U>
		constexpr bool is_template_instance_v<U<T>, U> = true;

		/**
		 * @brief Concept that checks whether the given invocable type @p TFn is a function-type
		 * that accepts the parameter of types @p TArgs by value.
		 * @details This concepts only accepts @p TFn if it takes the argument types by value. Neither
		 * taking the parameters as rvalue references, nor as lvalue references is allowed.
		 */
		template<typename TFn, typename... TArgs>
		concept invocable_byvalue = requires(TFn fn, TArgs... args) {
			{ fn(args...) }; // disallows TArgs&&
			{ fn(std::forward<TArgs>(args)...) }; // disallows TArgs&
		};

		template<typename T>
		inline constexpr bool is_const_reference_v = std::is_const_v<std::remove_reference_t<T>>;

		template<typename T> concept is_pair = requires(T pair) {
			{std::get<0>(pair)} -> std::convertible_to< std::tuple_element_t<0, T> >;
			{std::get<1>(pair)} -> std::convertible_to< std::tuple_element_t<1, T> >;
		} && std::tuple_size_v<T> == 2;

		template<typename T> concept is_optional = requires(T optional, typename T::value_type value) {
			typename T::value_type;
			{optional.value()} -> std::convertible_to<typename T::value_type>;
			{optional.value_or(value)} -> std::convertible_to<typename T::value_type>;
			{optional = value};
		};

		template<typename TContainer>
		concept ReservableContainer = requires(TContainer container, size_t newSize) {
			container.reserve(newSize);
		};

		/**
		* @brief Concept enforcing a back-insertible container like @c std::vector.
		*/
		template<template<typename...> typename TContainer, typename TItem, typename... TContainerArgs>
		concept BackInsertableContainer = requires(TContainer<TItem, TContainerArgs...> container, TItem item) {
			typename decltype(container)::value_type;
			container.push_back(item);
		};

		/**
		* @brief Concept enforcing an insertible container like @c std::set.
		*/
		template<template<typename...> typename TContainer, typename TItem, typename... TContainerArgs>
		concept InsertableContainer = requires(TContainer<TItem, TContainerArgs...> container, TItem item) {
			typename decltype(container)::value_type;
			container.insert(item);
		};

		/**
		* @brief Concept enforcing an associative container like @c std::map.
		*/
		template<template<typename...> typename TContainer, typename TItemKey, typename TItemValue, typename... TContainerArgs>
		concept AssocContainer = requires(TContainer<TItemKey, TItemValue, TContainerArgs...> container, std::pair<TItemKey, TItemValue> item) {
			typename decltype(container)::value_type;
			typename decltype(container)::key_type;
			typename decltype(container)::mapped_type;
			container.insert(item);
		};
	}

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
		* @brief Self-Type. This is the type of the struct for which the IteratorTrait is being specialized.
		*/
		using Self = IteratorTrait<T>;
		/**
		* @brief Item-Type. This is the type of elements that can be pulled from this pipeline-element.
		*/
		using Item = void;

		/**
		* @brief Pull one element from the iterator pipeline previous to this pipeline-element.
		* @param self Reference to the instance of the class for which IteratorTrait is being specialized.
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
		* @param self Reference to the instance of the class for which ExactSizeIteratorTrait is being specialized.
		* @return This iterator's exact number of elements.
		*/
		static inline size_t size(const typename IteratorTrait<T>::Self& self) = delete;
	};

	template<typename T>
	concept CXXIterIterator =
		(std::is_same_v<typename IteratorTrait<T>::Self, T>) &&
		requires(typename IteratorTrait<T>::Self& self, const typename IteratorTrait<T>::Self& constSelf) {
			typename IteratorTrait<T>::Self;
			typename IteratorTrait<T>::Item;
			{IteratorTrait<T>::next(self)} -> std::same_as<IterValue<typename IteratorTrait<T>::Item>>;
			{IteratorTrait<T>::sizeHint(constSelf)} -> std::same_as<SizeHint>;
	};

	template<typename T>
	concept CXXIterExactSizeIterator = CXXIterIterator<T> && requires(const typename IteratorTrait<T>::Self& self) {
			{ExactSizeIteratorTrait<T>::size(self)} -> std::same_as<size_t>;
	};

	template<CXXIterIterator TSelf> class IterApi;

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

	/**
	* @brief Concept that checks whether the given @p TContainer is supported by CXXIter's standard source
	* classes @c CXXIter::SrcMov, @c CXXIter::SrcRef and @c CXXIter::SrcCRef.
	* @details The concept does these checks by testing whether the @c CXXIter::SourceTrait was properly specialized
	* for the given @p TContainer type.
	*
	* @see CXXIter::SourceTrait for further details on this.
	*/
	template<typename TContainer>
	concept SourceContainer = requires(
			TContainer& container,
			const TContainer& constContainer,
			typename SourceTrait<TContainer>::IteratorState& iterState,
			typename SourceTrait<TContainer>::ConstIteratorState& constIterState
		) {
		typename SourceTrait<TContainer>::Item;
		typename SourceTrait<TContainer>::ItemRef;
		typename SourceTrait<TContainer>::ItemConstRef;
		typename SourceTrait<TContainer>::IteratorState;
		typename SourceTrait<TContainer>::ConstIteratorState;

		{SourceTrait<TContainer>::initIterator(container)} -> std::same_as<typename SourceTrait<TContainer>::IteratorState>;
		{SourceTrait<TContainer>::initIterator(constContainer)} -> std::same_as<typename SourceTrait<TContainer>::ConstIteratorState>;

		{SourceTrait<TContainer>::hasNext(container, iterState)} -> std::same_as<bool>;
		{SourceTrait<TContainer>::hasNext(constContainer, constIterState)} -> std::same_as<bool>;

		{SourceTrait<TContainer>::next(container, iterState)} -> std::same_as<typename SourceTrait<TContainer>::ItemRef>;
		{SourceTrait<TContainer>::next(constContainer, constIterState)} -> std::same_as<typename SourceTrait<TContainer>::ItemConstRef>;
	};

}
