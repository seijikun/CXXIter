#include <vector>
#include <string>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <unordered_set>
#include <unordered_map>

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

TEST(CXXIter, owned_t) {
	using TestType = std::vector<int>;

	static_assert(std::is_same_v<TestType, CXXIter::owned_t<TestType>>);
	static_assert(std::is_same_v<TestType, CXXIter::owned_t<TestType&>>);
	static_assert(std::is_same_v<TestType, CXXIter::owned_t<TestType&&>>);
	static_assert(std::is_same_v<TestType, CXXIter::owned_t<const TestType&>>);

	static_assert(!std::is_same_v<TestType, TestType&>);
	static_assert(!std::is_same_v<TestType, TestType&&>);
	static_assert(!std::is_same_v<TestType, const TestType&>);
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
