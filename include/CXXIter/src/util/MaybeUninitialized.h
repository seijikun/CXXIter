#pragma once

#include <cstdint>

namespace CXXIter::util {

	/**
	 * @private
	 * @brief MaybeUninitialized data container.
	 * @details This container is similar to @c std::optional<> in that it allows one to either contain
	 * a value or not contain a value. Though the difference is that the data can be used as if it was
	 * properly initialized without ever calling the constructor. That also makes this inherently unsafe
	 * and should not be used with types that do work in the constructor.
	 */
	template<typename TValue>
	class MaybeUninitialized {
	private:
		bool initialized = false;
		// buffer exactly large enough to contain TValue
		uint8_t rawData[sizeof(TValue)];

	public:
		/**
		 * @brief Mark the data contained in this container as @p initialized.
		 * @param initialized Whether the data in this container should be marked as initialized.
		 */
		constexpr void setInitialized(bool initialized = true) {
			this->initialized = initialized;
		}
		/**
		 * @brief Get whether the data in this container was marked as initialized.
		 * @return Whether the data in this container was marked as initialized.
		 */
		constexpr bool isInitialized() const { return initialized; }

		/**
		 * @brief Get a reference to the data in this container.
		 * @return Reference to the data in this container.
		 */
		constexpr const TValue& get() const {
			return *reinterpret_cast<const TValue*>(rawData);
		}

		/**
		 * @brief Get a reference to the data in this container.
		 * @return Reference to the data in this container.
		 */
		constexpr TValue& get() {
			return *reinterpret_cast<TValue*>(rawData);
		}

	};

}
