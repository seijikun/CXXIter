#include <array>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <optional>

#include "TestCommon.h"

using namespace CXXIter;

// ################################################################################################
// CONCEPTS & TYPE CONSTRAINTS & TYPE HELPERS
// ################################################################################################

TEST(CXXIter, IterValue) {
	{ // Move out of IterValue has to clear source
		{
			IterValue<size_t> src = 1337;
			IterValue<size_t> dst = std::move(src);
			ASSERT_FALSE(src.has_value());
			ASSERT_EQ(dst.value(), 1337);
		}
		{
			IterValue<std::string> src = std::string("1337");
			IterValue<std::string> dst = std::move(src);
			ASSERT_FALSE(src.has_value());
			ASSERT_EQ(dst.value(), "1337");
		}
		{
			IterValue<size_t> src = 1337;
			IterValue<size_t> dst(std::move(src));
			ASSERT_FALSE(src.has_value());
			ASSERT_EQ(dst.value(), 1337);
		}
		{
			IterValue<std::string> src = std::string("1337");
			IterValue<std::string> dst(std::move(src));
			ASSERT_FALSE(src.has_value());
			ASSERT_EQ(dst.value(), "1337");
		}
	}
}

TEST(CXXIter, invocable_byvalue) {
	auto fnRValueRef = [](std::string&&) {};
	auto fnLValueRef = [](std::string&) {};
	auto fnByValue = [](std::string) {};


	static_assert(!CXXIter::util::invocable_byvalue<typeof(fnRValueRef),	std::string>);
	static_assert(!CXXIter::util::invocable_byvalue<typeof(fnLValueRef),	std::string>);
	// should also forbid const lvalue ref - but seems impossible
	static_assert( CXXIter::util::invocable_byvalue<typeof(fnByValue),	std::string>);
}

TEST(CXXIter, is_const_reference_v) {
	using TestType = std::vector<int>;

	ASSERT_FALSE(CXXIter::util::is_const_reference_v<TestType>);
	ASSERT_FALSE(CXXIter::util::is_const_reference_v<TestType&>);
	ASSERT_FALSE(CXXIter::util::is_const_reference_v<TestType&&>);

	ASSERT_TRUE(CXXIter::util::is_const_reference_v<const TestType&>);
}

TEST(CXXIter, are_same_v) {
	static_assert( CXXIter::util::are_same_v<int, int, int>);
	static_assert( CXXIter::util::are_same_v<std::vector<int>, std::vector<int>, std::vector<int>>);
	static_assert( CXXIter::util::are_same_v<float, float, float>);
	static_assert( CXXIter::util::are_same_v<std::optional<int>, std::optional<int>, std::optional<int>>);
	static_assert(!CXXIter::util::are_same_v<int, float, int>);
	static_assert(!CXXIter::util::are_same_v<std::vector<int>, std::vector<int>, std::vector<float>>);
	static_assert(!CXXIter::util::are_same_v<float, float, int>);
	static_assert(!CXXIter::util::are_same_v<std::vector<float>, std::optional<int>, std::optional<int>>);
}

TEST(CXXIter, CXXIterIterator) {
	using TestSrc = std::vector<int>;

	static_assert( CXXIter::CXXIterIterator< CXXIter::SrcRef<TestSrc> >);
	static_assert( CXXIter::CXXIterIterator< CXXIter::SrcMov<TestSrc> >);
	static_assert( CXXIter::CXXIterIterator< CXXIter::SrcCRef<TestSrc> >);
	static_assert(!CXXIter::CXXIterIterator< std::vector<int> >);
	static_assert(!CXXIter::CXXIterIterator< std::vector<float> >);
	static_assert(!CXXIter::CXXIterIterator< std::vector<CXXIter::SrcCRef<TestSrc>> >);
	static_assert(!CXXIter::CXXIterIterator< int >);
	static_assert(!CXXIter::CXXIterIterator< void >);
}

TEST(CXXIter, stlCollections) {
	using TestKey = int;
	using TestValue = std::string;

	static_assert( CXXIter::util::BackInsertableContainerTemplate<std::vector				, TestValue>);
	static_assert( CXXIter::util::BackInsertableContainerTemplate<std::list				, TestValue>);
	static_assert( CXXIter::util::BackInsertableContainerTemplate<std::deque				, TestValue>);
	static_assert(!CXXIter::util::BackInsertableContainerTemplate<std::set				, TestValue>);
	static_assert(!CXXIter::util::BackInsertableContainerTemplate<std::multiset			, TestValue>);
	static_assert(!CXXIter::util::BackInsertableContainerTemplate<std::unordered_set		, TestValue>);
	static_assert(!CXXIter::util::BackInsertableContainerTemplate<std::unordered_multiset	, TestValue>);
	static_assert(!CXXIter::util::BackInsertableContainerTemplate<std::map				, TestValue>);
	static_assert(!CXXIter::util::BackInsertableContainerTemplate<std::multimap			, TestValue>);
	static_assert(!CXXIter::util::BackInsertableContainerTemplate<std::unordered_map		, TestValue>);
	static_assert(!CXXIter::util::BackInsertableContainerTemplate<std::unordered_multimap	, TestValue>);

	static_assert(!CXXIter::util::InsertableContainerTemplate<std::vector					, TestValue>);
	static_assert(!CXXIter::util::InsertableContainerTemplate<std::list					, TestValue>);
	static_assert(!CXXIter::util::InsertableContainerTemplate<std::deque					, TestValue>);
	static_assert( CXXIter::util::InsertableContainerTemplate<std::set					, TestValue>);
	static_assert( CXXIter::util::InsertableContainerTemplate<std::multiset				, TestValue>);
	static_assert( CXXIter::util::InsertableContainerTemplate<std::unordered_set			, TestValue>);
	static_assert( CXXIter::util::InsertableContainerTemplate<std::unordered_multiset		, TestValue>);
	static_assert(!CXXIter::util::InsertableContainerTemplate<std::map					, TestValue>);
	static_assert(!CXXIter::util::InsertableContainerTemplate<std::multimap				, TestValue>);
	static_assert(!CXXIter::util::InsertableContainerTemplate<std::unordered_map			, TestValue>);
	static_assert(!CXXIter::util::InsertableContainerTemplate<std::unordered_multimap		, TestValue>);

	//static_assert(!CXXIter::util::AssocContainerTemplate<std::vector			,TestKey, TestValue>);
	//static_assert(!CXXIter::util::AssocContainerTemplate<std::list				,TestKey, TestValue>);
	//static_assert(!CXXIter::util::AssocContainerTemplate<std::deque				,TestKey, TestValue>);
	static_assert(!CXXIter::util::AssocContainerTemplate<std::set					,TestKey, TestValue>);
	static_assert(!CXXIter::util::AssocContainerTemplate<std::multiset			,TestKey, TestValue>);
	static_assert(!CXXIter::util::AssocContainerTemplate<std::unordered_set		,TestKey, TestValue>);
	static_assert(!CXXIter::util::AssocContainerTemplate<std::unordered_multiset	,TestKey, TestValue>);
	static_assert( CXXIter::util::AssocContainerTemplate<std::map					,TestKey, TestValue>);
	static_assert( CXXIter::util::AssocContainerTemplate<std::multimap			,TestKey, TestValue>);
	static_assert( CXXIter::util::AssocContainerTemplate<std::unordered_map		,TestKey, TestValue>);
	static_assert( CXXIter::util::AssocContainerTemplate<std::unordered_multimap	,TestKey, TestValue>);

	// std::array concept
	static_assert( CXXIter::util::StdArrayContainer<std::array<TestValue, 3>,						TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::vector<TestValue>,						TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::list<TestValue>,							TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::deque<TestValue>,						TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::set<TestValue>,							TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::multiset<TestValue>,						TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::unordered_set<TestValue>,				TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::unordered_multiset<TestValue>,			TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::map<TestKey, TestValue>,					TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::multimap<TestKey, TestValue>,			TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::unordered_map<TestKey, TestValue>,		TestValue>);
	static_assert(!CXXIter::util::StdArrayContainer<std::unordered_multimap<TestKey, TestValue>,	TestValue>);
}

TEST(CXXIter, is_pair) {
	static_assert(!CXXIter::util::is_pair< int >);
	static_assert(!CXXIter::util::is_pair< float >);
	static_assert(!CXXIter::util::is_pair< std::vector<int> >);
	static_assert(!CXXIter::util::is_pair< std::vector<std::pair<int, int>> >);

	static_assert( CXXIter::util::is_pair< std::pair<int, int> >);
	static_assert( CXXIter::util::is_pair< std::pair<float, float> >);
	static_assert( CXXIter::util::is_pair< std::pair<float, int> >);
	static_assert( CXXIter::util::is_pair< std::pair<int, std::vector<int>> >);
}

TEST(CXXIter, is_optional) {
	static_assert(!CXXIter::util::is_optional< int >);
	static_assert(!CXXIter::util::is_optional< float >);
	static_assert(!CXXIter::util::is_optional< std::vector<int> >);
	static_assert(!CXXIter::util::is_optional< std::vector<std::optional<int>> >);

	static_assert( CXXIter::util::is_optional< std::optional<int> >);
	static_assert( CXXIter::util::is_optional< std::optional<float> >);
	static_assert( CXXIter::util::is_optional< std::optional<std::string> >);
	static_assert( CXXIter::util::is_optional< std::optional<std::pair<float, int>> >);
	static_assert( CXXIter::util::is_optional< std::optional<std::vector<int>> >);
}
