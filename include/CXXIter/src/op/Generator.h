#pragma once

#ifdef CXXITER_HAS_COROUTINE

#include <coroutine>

#include "../Common.h"

namespace CXXIter {

	/**
	 * @brief Generator that C++20 coroutines passed to CXXIter::IterApi::generateFrom() have to return.
	 * This generator supports exceptions, co_yield for producing an arbitrary amount of elements, and can
	 * take references as results from coroutines - as long as they live long enough until used.
	 */
	template<typename T>
	class Generator {
	public:
		using value_type = T;
		struct promise_type;
		using Handle = std::coroutine_handle<promise_type>;

		struct promise_type {
			IterValue<T> currentItem;
			std::exception_ptr exceptionPtr;

			Generator<T> get_return_object() {
				return Generator{Handle::from_promise(*this)};
			}
			static std::suspend_always initial_suspend() noexcept { return {}; }
			static std::suspend_always final_suspend() noexcept { return {}; }
			std::suspend_always yield_value(T value) noexcept {
				currentItem = value;
				return {};
			}

			void unhandled_exception() {
				exceptionPtr = std::current_exception();
			}
		};

		explicit Generator(const Handle coroutine) : m_coroutine{coroutine} {}
		Generator() = default;
		~Generator() {
			if (m_coroutine) {
				m_coroutine.destroy();
			}
		}

		Generator(const Generator&) = delete;
		Generator& operator=(const Generator&) = delete;

		Generator(Generator&& other) noexcept : m_coroutine{other.m_coroutine} {
			other.m_coroutine = {};
		}
		Generator& operator=(Generator&& other) noexcept {
			m_coroutine = other;
			other.m_coroutine = {};
			return *this;
		}

		IterValue<T> next() {
			if(m_coroutine && !m_coroutine.promise().currentItem.has_value()) {
				m_coroutine.resume();
				if(m_coroutine.promise().exceptionPtr) {
					std::rethrow_exception(m_coroutine.promise().exceptionPtr);
				}
			}
			if(m_coroutine.done()) {
				m_coroutine.destroy();
				m_coroutine = {};
				return {};
			}
			// Depends on IterValue<T>'s guarantee, to clear the moved-out-of IterValue
			// irrespecting on the type of T. (std::optional<T> doesn't!)
			//TODO: unit-test for IterValue?
			return std::move(m_coroutine.promise().currentItem);
		}

	private:
		Handle m_coroutine;
	};

	template<typename T, typename TItem>
	concept GeneratorFromFunction = (invocable_byvalue<T, TItem> && is_template_instance_v<std::invoke_result_t<T, TItem>, Generator>);

	// ################################################################################################
	// GENERATEFROM
	// ################################################################################################
	/** @private */
	template<typename TChainInput, typename TGeneratorFn, typename TGenerator>
	class [[nodiscard(CXXITER_CHAINER_NODISCARD_WARNING)]] GenerateFrom : public IterApi<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>> {
		friend struct IteratorTrait<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>>;
		friend struct ExactSizeIteratorTrait<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>>;
	private:
		TChainInput input;
		TGeneratorFn generatorFn;
		std::optional<TGenerator> currentGenerator;
	public:
		GenerateFrom(TChainInput&& input, TGeneratorFn& generatorFn) : input(std::move(input)), generatorFn(generatorFn) {}
	};
	// ------------------------------------------------------------------------------------------------
	/** @private */
	template<typename TChainInput, typename TGeneratorFn, typename TGenerator>
	struct IteratorTrait<GenerateFrom<TChainInput, TGeneratorFn, TGenerator>> {
		using ChainInputIterator = IteratorTrait<TChainInput>;
		using InputItemOwned = typename TChainInput::ItemOwned;
		// CXXIter Interface
		using Self = GenerateFrom<TChainInput, TGeneratorFn, TGenerator>;
		using Item = typename TGenerator::value_type;

		static inline IterValue<Item> next(Self& self) {
			while(true) {
				if(!self.currentGenerator.has_value()) {
					auto item = ChainInputIterator::next(self.input);
					if(!item.has_value()) { return {}; } // reached end
					self.currentGenerator.emplace(self.generatorFn( std::forward<Item>(item.value()) ));
				}

				auto item = self.currentGenerator.value().next();
				if(!item.has_value()) {
					self.currentGenerator.reset();
					continue;
				}
				return item;
			}
		}
		static inline SizeHint sizeHint(const Self& self) { return ChainInputIterator::sizeHint(self.input); }
	};

}

#endif
