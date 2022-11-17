#pragma once

#include <type_traits>
#include <functional>

/** @private */
namespace CXXIter::util {
	/** @private */
	namespace {

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
		 * @brief Concept that checks whether the given type T has an implementation of @c std::hash<T>::operator()
		 * @see https://en.cppreference.com/w/cpp/language/constraints
		 */
		template<typename T>
		concept is_hashable = requires(T a) {
			{ std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
		};

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


		// ################################################################################################
		// CONTAINER CONSTRAINTS
		// ################################################################################################

		template<typename TContainer>
		concept ReservableContainer = requires(TContainer container, size_t newSize) {
			container.reserve(newSize);
		};

		/**
		* @brief Concept enforcing a back-insertible container like @c std::vector.
		*/
		template<typename TContainer, typename TItem>
		concept BackInsertableContainer = requires(TContainer& container, TItem item) {
			typename TContainer::value_type;
			container.push_back(item);
		};

		/**
		* @brief Concept enforcing a back-insertible container like @c std::vector.
		*/
		template<template<typename...> typename TContainer, typename TItem, typename... TContainerArgs>
		concept BackInsertableContainerTemplate = BackInsertableContainer<TContainer<TItem, TContainerArgs...>, TItem>;


		/**
		* @brief Concept enforcing an insertible container like @c std::set.
		*/
		template<typename TContainer, typename TItem>
		concept InsertableContainer = requires(TContainer& container, TItem item) {
			typename TContainer::value_type;
			container.insert(item);
		};
		/**
		* @brief Concept enforcing an insertible container like @c std::set.
		*/
		template<template<typename...> typename TContainer, typename TItem, typename... TContainerArgs>
		concept InsertableContainerTemplate = InsertableContainer<TContainer<TItem, TContainerArgs...>, TItem>;

		/**
		* @brief Concept enforcing an associative container like @c std::map.
		*/
		template<typename TContainer, typename TItemKey, typename TItemValue>
		concept AssocContainer = requires(TContainer& container, std::pair<TItemKey, TItemValue> item) {
			typename TContainer::value_type;
			typename TContainer::key_type;
			typename TContainer::mapped_type;
			container.insert(item);
		};
		/**
		* @brief Concept enforcing an associative container like @c std::map.
		*/
		template<template<typename...> typename TContainer, typename TItemKey, typename TItemValue, typename... TContainerArgs>
		concept AssocContainerTemplate = AssocContainer<TContainer<TItemKey, TItemValue, TContainerArgs...>, TItemKey, TItemValue>;

		/**
		 * @brief Concept enforcing a std::array<,> template instantiation
		 */
		template<typename TContainer, typename TItem>
		concept StdArrayContainer = requires(TContainer& container, size_t idx, TItem item) {
			typename TContainer::value_type;
			typename TContainer::size_type;
			{container[idx] = item};
			{container.max_size()} -> std::same_as<typename TContainer::size_type>;
			{container.fill(item)};
		} && !InsertableContainer<TContainer, TItem> && !BackInsertableContainer<TContainer, TItem>;

		/**
		 * @brief Concept enforcing TContainer to be a container based on a contiguous chunk of memory containing
		 * all elements.
		 */
		template<typename TContainer>
		concept ContiguousMemoryContainer = requires(TContainer& container) {
			typename TContainer::value_type;
			typename TContainer::size_type;
			{container.data()} -> std::same_as<typename TContainer::value_type*>;
		};

	}
}
