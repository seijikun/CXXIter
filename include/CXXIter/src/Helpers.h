#pragma once

#include <tuple>
#include <cstdlib>
#include <optional>
#include <unordered_set>

/**
 * @brief Namespace that contains helper functions providing commonly required functionality
 * when working with CXXIter.
 * @details These functions are small helper functions that can be used in conjunction with the
 * iterator API interface. For example by constructing lambdas for reusable tasks
 * with a clear and concise API, instead of copying the lambda code itself everywhere it is needed.
 */
namespace CXXIter::fn {

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
	 *  // output == {{3, 3.0f}, {1, 2.0f}, {0, 1.0f}, {2, 0.5f}, {4, -42.0f}}
	 * @endcode
	 */
	template<size_t FIELD_IDX>
	auto unzip() {
		return [](const auto& item) { return std::get<FIELD_IDX>(item); };
	}

	/**
	 * @brief Helper to construct a filterMap() lambda that takes a pointer, dyncasts it to the requested type and
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
	 *  // output.size() == 2
	 *  // output[0]->id == "0"
	 *  // output[1]->id == "1"
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

	/**
	 * @brief Helper that constructs a filter() lambda that filters the iterator elements by calling
	 * the given @p filterExtractFn on each element and then checking whether that is contained in the given
	 * list of @p acceptedValues.
	 * @param filterExtractFn Lambda that extracts a value for each iterator element, which is then searched
	 * for in the list of @p acceptedValues.
	 * @param acceptedValues List of comparison-values that are allowed to remain in the iterator.
	 * @return Lambda that can be passed to CXXIter::filter to filter the iterator elements with a given
	 * list of @p acceptedValues.
	 *
	 * Usage Example:
	 * @code
	 *  enum class CakeType { Sacher, ButterCake, CheeseCake, ChocolateCake, StrawberryCake };
	 *  struct Cake {
	 *  	CakeType type;
	 *  	float volume;
	 *  	bool operator==(const Cake& o) const { return o.type == type && o.volume == volume; }
	 *  };
	 *
	 *  std::vector<Cake> input = {
	 *  	{ CakeType::Sacher, 1.33f }, { CakeType::CheeseCake, 5.0f },
	 *  	{ CakeType::ButterCake, 2.33f }, { CakeType::Sacher, 42.0f },
	 *  	{ CakeType::StrawberryCake, 1.6f }, { CakeType::ChocolateCake, 55.0f },
	 *  	{ CakeType::Sacher, 3.63f }, { CakeType::StrawberryCake, 14.0f }
	 *  };
	 *  std::vector<Cake> output = CXXIter::from(input)
	 *  	.filter(CXXIter::fn::filterIsOneOf(
	 *  		[](const Cake& cake) { return cake.type; },
	 *  		{CakeType::Sacher, CakeType::ChocolateCake}
	 *  	))
	 *  	.collect<std::vector>();
	 *  // output == { Cake {CakeType::Sacher, 1.33f}, Cake {CakeType::Sacher, 42.0f},
	 *					Cake {CakeType::ChocolateCake, 55.0f}, Cake {CakeType::Sacher, 3.63f} };
	 * @endcode
	 */
	template<typename TItem, typename TFilterExtractFn>
	auto filterIsOneOf(TFilterExtractFn filterExtractFn, const std::initializer_list<TItem>& acceptedValues) {
		std::unordered_set<TItem> _acceptedValues = acceptedValues;
		return [_acceptedValues = std::move(_acceptedValues), filterExtractFn](const auto& item) {
			return _acceptedValues.contains(filterExtractFn(item));
		};
	}

	/**
	 * @brief Helper that constructs a filter() lambda that filters an iterator's elements by checking whether
	 * they are contained in the given list of @p acceptedValues.
	 * @param acceptedValues List of values that are allowed to remain in the iterator.
	 * @return Lambda that can be passed to CXXIter::filter to filter the iterator elements with a given
	 * list of @p acceptedValues.
	 *
	 * Usage Example:
	 * @code
	 *  enum class CakeType { Sacher, ButterCake, CheeseCake, ChocolateCake, StrawberryCake };
	 *  struct Cake {
	 *  	CakeType type;
	 *  	float volume;
	 *  	bool operator==(const Cake& o) const { return o.type == type && o.volume == volume; }
	 *  };
	 *
	 *  std::vector<Cake> input = {
	 *  	{ CakeType::Sacher, 1.33f }, { CakeType::CheeseCake, 5.0f },
	 *  	{ CakeType::ButterCake, 2.33f }, { CakeType::Sacher, 42.0f },
	 *  	{ CakeType::StrawberryCake, 1.6f }, { CakeType::ChocolateCake, 55.0f },
	 *  	{ CakeType::Sacher, 3.63f }, { CakeType::StrawberryCake, 14.0f }
	 *  };
	 *  std::vector<CakeType> output = CXXIter::from(input)
	 *  		.map([](const Cake& cake) { return cake.type; })
	 *  		.filter(CXXIter::fn::filterIsOneOf({CakeType::Sacher, CakeType::ChocolateCake}))
	 *  		.collect<std::vector>();
	 *  // output == {CakeType::Sacher, CakeType::Sacher, CakeType::ChocolateCake, CakeType::Sacher};
	 * @endcode
	 */
	template<typename TItem>
	auto filterIsOneOf(const std::initializer_list<TItem>& acceptedValues) {
		return filterIsOneOf<TItem>([](const auto& item) { return item; }, acceptedValues);
	}

}
