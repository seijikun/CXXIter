#pragma once

#include <type_traits>
#include <optional>

namespace CXXIter {

	// ################################################################################################
	// ITERATOR OPTIONAL (supports references)
	// ################################################################################################

	/**
	 * @brief Container that is used to pass elements through CXXIter's iterator pipelines.
	 * @details This is essentially a @c std::optional<> that also transparently supports references (in comparison
	 * to the original). This is achieved by wrapping reference types in a @c std::reference_wrapper<> before storing
	 * them in the internal @c std::optional<>.
	 */
	template<typename TValue>
	class IterValue {
	private:
		/** Owned (references removed) version of the type that is to be stored in this IterValue. */
		using TValueDeref = std::remove_reference_t<TValue>;
		using TValueStore = std::conditional_t<
			std::is_reference_v<TValue>,
			std::reference_wrapper<TValueDeref>,
			TValue
		>;

		std::optional<TValueStore> inner;

	public:
		/** ctor */
		constexpr IterValue() noexcept {}
		/** ctor */
		constexpr IterValue(TValue value) noexcept requires std::is_reference_v<TValue> : inner(value) {}
		/** ctor */
		constexpr IterValue(const TValueDeref& value) noexcept(std::is_nothrow_copy_constructible_v<TValueDeref>)
				requires (!std::is_reference_v<TValue>) : inner(value) {}
		/** ctor */
		constexpr IterValue(TValueDeref&& value) noexcept(std::is_nothrow_move_constructible_v<TValueDeref>)
				: inner(std::forward<TValueDeref>(value)) {}

		/** Assignment from another IterValue instance. */
		constexpr IterValue& operator=(IterValue&& o) = default;
		/** Assignment from another IterValue instance. */
		constexpr IterValue(IterValue&& o) noexcept(std::is_nothrow_move_constructible_v<TValueDeref>)
				: inner(std::move(o.inner)) {
			o.inner.reset();
		};

		/** Assignment from an instance of the stored type. */
		constexpr auto& operator=(TValueDeref&& o) {
			this->inner = std::forward<decltype(o)>(o);
			return *this;
		}

		/**
		 * @brief Get the contained value (if any).
		 * @throws If this is called when no value is contained.
		 * @return const reference to the contained value.
		 */
		constexpr inline const TValueDeref& value() const { return inner.value(); }
		/**
		 * @brief Get the contained value (if any).
		 * @throws If this is called when no value is contained.
		 * @return reference to the contained value.
		 */
		constexpr inline TValueDeref& value() { return inner.value(); }
		/**
		 * @brief Get the contained value, or alternatively the given @p def if none is present.
		 * @param def Default value to return when this optional does not contain a value.
		 * @return const reference to the contained value (if any), or alternatively the given @p def value.
		 */
		constexpr inline const TValueDeref& value_or(TValueDeref&& def) const noexcept { return inner.value_or(def); }
		/**
		 * @brief Get the contained value, or alternatively the given @p def if none is present.
		 * @param def Default value to return when this optional does not contain a value.
		 * @return reference to the contained value (if any), or alternatively the given @p def value.
		 */
		constexpr inline TValueDeref& value_or(TValueDeref&& def) noexcept { return inner.value_or(def); }

		/**
		 * @brief Get whether this optional IteratorValue contains a value.
		 * @return @c true when this IterValue contains a value, @c false otherwise.
		 */
		constexpr bool has_value() const noexcept { return inner.has_value(); }

		/**
		 * @brief Swap the values within this and another IterValue container.
		 * @param o Other IterValue container to swap contents with.
		 */
		constexpr void swap(IterValue<TValue>& o) noexcept { inner.swap(o.inner); }

		/**
		 * @brief Convert this IterValue to a @c std::optional<> on the owned (no-reference) type.
		 * @details If the contained value is a reference, the value will be copied into the returned @c std::optional<>.
		 * @return @c std::optional<> containing an owned version of the value contained by this IterValue.
		 */
		constexpr std::optional<TValueStore> toStdOptional() noexcept {
			return std::optional<TValueStore>(std::move(this->inner));
		}
		/**
		 * @brief Cast to @c std::optional<>.
		 */
		constexpr operator std::optional<TValueStore>() noexcept { return toStdOptional(); }

		/**
		 * @brief Map this IterValue's contained value (if any) to the requested new @p TOutValue type, using the given @p mapFn
		 * @tparam The requested output type for the mapping operation.
		 * @param mapFn Mapper function that takes this IterValue's contained value (if any) to map it to an instance of @p TOutValue
		 * @return A new IterValue containing the mapped value if this instance contained a value.
		 */
		template<typename TOutValue, std::invocable<TValueDeref&&> TMapFn>
		requires (!std::is_reference_v<TValue>)
		constexpr IterValue<TOutValue> map(TMapFn mapFn) {
			if(!has_value()) { return {}; }
			return mapFn(std::forward<TValueDeref>(value()));
		}

		/**
		 * @brief Map this IterValue's contained value (if any) to the requested new @p TOutValue type, using the given @p mapFn
		 * @tparam The requested output type for the mapping operation.
		 * @param mapFn Mapper function that takes this IterValue's contained value (if any) to map it to an instance of @p TOutValue
		 * @return A new IterValue containing the mapped value if this instance contained a value.
		 */
		template<typename TOutValue, std::invocable<TValue> TMapFn>
		requires std::is_reference_v<TValue>
		constexpr IterValue<TOutValue> map(TMapFn mapFn) {
			if(!this->has_value()) { return {}; }
			return mapFn(this->value());
		}
	};

}
