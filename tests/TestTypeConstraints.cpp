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

	static_assert( CXXIter::BackInsertableCollection<std::vector				, TestValue>);
	static_assert( CXXIter::BackInsertableCollection<std::list					, TestValue>);
	static_assert( CXXIter::BackInsertableCollection<std::deque					, TestValue>);
	static_assert(!CXXIter::BackInsertableCollection<std::set					, TestValue>);
	static_assert(!CXXIter::BackInsertableCollection<std::multiset				, TestValue>);
	static_assert(!CXXIter::BackInsertableCollection<std::unordered_set			, TestValue>);
	static_assert(!CXXIter::BackInsertableCollection<std::unordered_multiset	, TestValue>);
	static_assert(!CXXIter::BackInsertableCollection<std::map					, TestValue>);
	static_assert(!CXXIter::BackInsertableCollection<std::multimap				, TestValue>);
	static_assert(!CXXIter::BackInsertableCollection<std::unordered_map			, TestValue>);
	static_assert(!CXXIter::BackInsertableCollection<std::unordered_multimap	, TestValue>);

	static_assert(!CXXIter::InsertableCollection<std::vector					, TestValue>);
	static_assert(!CXXIter::InsertableCollection<std::list						, TestValue>);
	static_assert(!CXXIter::InsertableCollection<std::deque						, TestValue>);
	static_assert( CXXIter::InsertableCollection<std::set						, TestValue>);
	static_assert( CXXIter::InsertableCollection<std::multiset					, TestValue>);
	static_assert( CXXIter::InsertableCollection<std::unordered_set				, TestValue>);
	static_assert( CXXIter::InsertableCollection<std::unordered_multiset		, TestValue>);
	static_assert(!CXXIter::InsertableCollection<std::map						, TestValue>);
	static_assert(!CXXIter::InsertableCollection<std::multimap					, TestValue>);
	static_assert(!CXXIter::InsertableCollection<std::unordered_map				, TestValue>);
	static_assert(!CXXIter::InsertableCollection<std::unordered_multimap		, TestValue>);

	//static_assert(!CXXIter::AssocCollection<std::vector			,TestKey, TestValue>);
	//static_assert(!CXXIter::AssocCollection<std::list				,TestKey, TestValue>);
	//static_assert(!CXXIter::AssocCollection<std::deque			,TestKey, TestValue>);
	static_assert(!CXXIter::AssocCollection<std::set				,TestKey, TestValue>);
	static_assert(!CXXIter::AssocCollection<std::multiset			,TestKey, TestValue>);
	static_assert(!CXXIter::AssocCollection<std::unordered_set		,TestKey, TestValue>);
	static_assert(!CXXIter::AssocCollection<std::unordered_multiset	,TestKey, TestValue>);
	static_assert( CXXIter::AssocCollection<std::map				,TestKey, TestValue>);
	static_assert( CXXIter::AssocCollection<std::multimap			,TestKey, TestValue>);
	static_assert( CXXIter::AssocCollection<std::unordered_map		,TestKey, TestValue>);
	static_assert( CXXIter::AssocCollection<std::unordered_multimap	,TestKey, TestValue>);
}
