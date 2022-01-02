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

// ################################################################################################
// CONCEPTS & TYPE CONSTRAINTS & TYPE HELPERS
// ################################################################################################

TEST(CXXIter, is_const_reference_v) {
	using TestType = std::vector<int>;

	ASSERT_FALSE(CXXIter::is_const_reference_v<TestType>);
	ASSERT_FALSE(CXXIter::is_const_reference_v<TestType&>);
	ASSERT_FALSE(CXXIter::is_const_reference_v<TestType&&>);

	ASSERT_TRUE(CXXIter::is_const_reference_v<const TestType&>);
}

TEST(CXXIter, are_same_v) {
	static_assert( CXXIter::are_same_v<int, int, int>);
	static_assert( CXXIter::are_same_v<std::vector<int>, std::vector<int>, std::vector<int>>);
	static_assert( CXXIter::are_same_v<float, float, float>);
	static_assert( CXXIter::are_same_v<std::optional<int>, std::optional<int>, std::optional<int>>);
	static_assert(!CXXIter::are_same_v<int, float, int>);
	static_assert(!CXXIter::are_same_v<std::vector<int>, std::vector<int>, std::vector<float>>);
	static_assert(!CXXIter::are_same_v<float, float, int>);
	static_assert(!CXXIter::are_same_v<std::vector<float>, std::optional<int>, std::optional<int>>);
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

	static_assert( CXXIter::BackInsertableContainer<std::vector				, TestValue>);
	static_assert( CXXIter::BackInsertableContainer<std::list				, TestValue>);
	static_assert( CXXIter::BackInsertableContainer<std::deque				, TestValue>);
	static_assert(!CXXIter::BackInsertableContainer<std::set				, TestValue>);
	static_assert(!CXXIter::BackInsertableContainer<std::multiset			, TestValue>);
	static_assert(!CXXIter::BackInsertableContainer<std::unordered_set		, TestValue>);
	static_assert(!CXXIter::BackInsertableContainer<std::unordered_multiset	, TestValue>);
	static_assert(!CXXIter::BackInsertableContainer<std::map				, TestValue>);
	static_assert(!CXXIter::BackInsertableContainer<std::multimap			, TestValue>);
	static_assert(!CXXIter::BackInsertableContainer<std::unordered_map		, TestValue>);
	static_assert(!CXXIter::BackInsertableContainer<std::unordered_multimap	, TestValue>);

	static_assert(!CXXIter::InsertableContainer<std::vector					, TestValue>);
	static_assert(!CXXIter::InsertableContainer<std::list					, TestValue>);
	static_assert(!CXXIter::InsertableContainer<std::deque					, TestValue>);
	static_assert( CXXIter::InsertableContainer<std::set					, TestValue>);
	static_assert( CXXIter::InsertableContainer<std::multiset				, TestValue>);
	static_assert( CXXIter::InsertableContainer<std::unordered_set			, TestValue>);
	static_assert( CXXIter::InsertableContainer<std::unordered_multiset		, TestValue>);
	static_assert(!CXXIter::InsertableContainer<std::map					, TestValue>);
	static_assert(!CXXIter::InsertableContainer<std::multimap				, TestValue>);
	static_assert(!CXXIter::InsertableContainer<std::unordered_map			, TestValue>);
	static_assert(!CXXIter::InsertableContainer<std::unordered_multimap		, TestValue>);

	//static_assert(!CXXIter::AssocContainer<std::vector			,TestKey, TestValue>);
	//static_assert(!CXXIter::AssocContainer<std::list				,TestKey, TestValue>);
	//static_assert(!CXXIter::AssocContainer<std::deque				,TestKey, TestValue>);
	static_assert(!CXXIter::AssocContainer<std::set					,TestKey, TestValue>);
	static_assert(!CXXIter::AssocContainer<std::multiset			,TestKey, TestValue>);
	static_assert(!CXXIter::AssocContainer<std::unordered_set		,TestKey, TestValue>);
	static_assert(!CXXIter::AssocContainer<std::unordered_multiset	,TestKey, TestValue>);
	static_assert( CXXIter::AssocContainer<std::map					,TestKey, TestValue>);
	static_assert( CXXIter::AssocContainer<std::multimap			,TestKey, TestValue>);
	static_assert( CXXIter::AssocContainer<std::unordered_map		,TestKey, TestValue>);
	static_assert( CXXIter::AssocContainer<std::unordered_multimap	,TestKey, TestValue>);
}

TEST(CXXIter, is_pair) {
	static_assert(!CXXIter::is_pair< int >);
	static_assert(!CXXIter::is_pair< float >);
	static_assert(!CXXIter::is_pair< std::vector<int> >);
	static_assert(!CXXIter::is_pair< std::vector<std::pair<int, int>> >);

	static_assert( CXXIter::is_pair< std::pair<int, int> >);
	static_assert( CXXIter::is_pair< std::pair<float, float> >);
	static_assert( CXXIter::is_pair< std::pair<float, int> >);
	static_assert( CXXIter::is_pair< std::pair<int, std::vector<int>> >);
}

TEST(CXXIter, is_optional) {
	static_assert(!CXXIter::is_optional< int >);
	static_assert(!CXXIter::is_optional< float >);
	static_assert(!CXXIter::is_optional< std::vector<int> >);
	static_assert(!CXXIter::is_optional< std::vector<std::optional<int>> >);

	static_assert( CXXIter::is_optional< std::optional<int> >);
	static_assert( CXXIter::is_optional< std::optional<float> >);
	static_assert( CXXIter::is_optional< std::optional<std::string> >);
	static_assert( CXXIter::is_optional< std::optional<std::pair<float, int>> >);
	static_assert( CXXIter::is_optional< std::optional<std::vector<int>> >);
}
