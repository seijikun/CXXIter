#pragma once

#include <type_traits>
#include <optional>

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
		operator std::optional<TValue>() { return toStdOptional(); }

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
		operator std::optional<TValueDeref>() { return toStdOptional(); }

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

}
