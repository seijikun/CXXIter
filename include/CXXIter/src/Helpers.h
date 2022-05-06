#pragma once

#include <tuple>
#include <cstdlib>
#include <optional>

namespace CXXIter {

/**
 * @name Helpers
 * These functions are small helper functions that can be used in conjunction with the
 * iterator API interface. For example by constructing lambdas for reusable tasks
 * with a clear and concise API, instead of copying the lambda code itself everywhere it is needed.
 */
//@{

	// ################################################################################################
	// HELPERS
	// ################################################################################################

	/**
	 * @brief Helper to construct a lambda that extracts the `FIELD_IDX`-th element from tuples or pairs passing through the iterator.
	 * @details You can use this shortcut everywhere, where a lambda is required that takes a tuple or pair and returns their `FIELD_IDX`-th element.
	 * @tparam FIELD_IDX Index of the field to extract.
	 * @return Lambda that, given an arbitrary tuple or pair, extracts its `FIELD_IDX`-th element.
	 *
	 * Usage Example:
	 * - Use the tuple's second (idx = 1) field to sort by.
	 * @code
	 *  std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
	 *  std::vector<std::pair<size_t, float>> output = CXXIter::from(input).copied()
	 *		.indexed()
	 *		.sortBy<CXXIter::DESCENDING>(CXXIter::unzip<1>())
	 *		.collect<std::vector>();
	 *  // output = {{3, 3.0f}, {1, 2.0f}, {0, 1.0f}, {2, 0.5f}, {4, -42.0f}}
	 * @endcode
	 */
	template<size_t FIELD_IDX>
	auto unzip() {
		return [](const auto& item) { return std::get<FIELD_IDX>(item); };
	}

	/**
	 * @brief Helper to construct a filterMap lambda that takes a pointer, dyncasts it to the requested type and
	 * returns a @c std::optional<TDynCastTarget> with the result if the cast was successfull.
	 * @details You can use this shortcut everywhere, where a lambda is required that takes an element and
	 * returns an @c std::optional<TDynCastTarget> that is set
	 * only when the @c dynamic_cast() was successful.
	 * @tparam TDynCastTarget Type that the elements that are passed to the generated lambda should be cast to.
	 * @return Lambda that attempts to @c dynamic_cast() a given element to @p TDynCastTarget
	 * and returns a @c std::optional<TDynCastTarget> with the result if the
	 * cast was successful. And @c none if it failed.
	 *
	 * Usage Example:
	 * @code
	 *  struct Parent {
	 *  	virtual ~Parent() {}
	 *  };
	 *  struct Child1 : public Parent {
	 *  	std::string id;
	 *  	Child1(const std::string& id) : id(id) {}
	 *  };
	 *  struct Child2 : public Parent {};
	 *
	 *  std::vector<Parent*> input = { new Parent(), new Child1("0"), new Child1("1"), new Child2() };
	 *  std::vector<Child1*> output = CXXIter::from(input)
	 *  		.filterMap(CXXIter::tryDynCast<Child1*>())
	 *  		.collect<std::vector>();
	 *  ASSERT_EQ(output.size(), 2);
	 *  ASSERT_EQ(output[0]->id, "0");
	 *  ASSERT_EQ(output[1]->id, "1");
	 *  CXXIter::from(output).forEach([](auto ptr) { delete ptr; });
	 * @endcode
	 */
	template<typename TDynCastTarget>
	auto tryDynCast() {
		return [](auto itemPtr) -> std::optional<TDynCastTarget> {
			TDynCastTarget targetPtr = dynamic_cast<TDynCastTarget>(itemPtr);
			if(targetPtr == nullptr) { return {}; }
			return targetPtr;
		};
	}

//}@

}
