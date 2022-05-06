#include <vector>

#include "TestCommon.h"

// ################################################################################################
// HELPERS
// ################################################################################################
TEST(CXXIter, unzip) {
	std::vector<float> input = {1.0f, 2.0f, 0.5f, 3.0f, -42.0f};
	std::vector<std::pair<size_t, float>> output = CXXIter::from(input).copied()
		.indexed()
		.sortBy<CXXIter::DESCENDING>(CXXIter::unzip<1>())
		.collect<std::vector>();
	ASSERT_EQ(input.size(), output.size());
	ASSERT_THAT(output, ElementsAre(Pair(3, 3.0f), Pair(1, 2.0f), Pair(0, 1.0f), Pair(2, 0.5f), Pair(4, -42.0f)));
}

TEST(CXXIter, tryDynCast) {
	struct Parent {
		virtual ~Parent() {}
	};
	struct Child1 : public Parent {
		std::string id;
		Child1(const std::string& id) : id(id) {}
	};
	struct Child2 : public Parent {};

	{ // normal
		std::vector<Parent*> input = { new Parent(), new Child1("0"), new Child1("1"), new Child2() };
		std::vector<Child1*> output = CXXIter::from(input)
				.filterMap(CXXIter::tryDynCast<Child1*>())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_EQ(output[0]->id, "0");
		ASSERT_EQ(output[1]->id, "1");
		CXXIter::from(output).forEach([](auto ptr) { delete ptr; });
	}
	{ // const
		std::vector<const Parent*> input = { new Parent(), new Child1("0"), new Child1("1"), new Child2() };
		std::vector<const Child1*> output = CXXIter::from(input)
				.filterMap(CXXIter::tryDynCast<const Child1*>())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_EQ(output[0]->id, "0");
		ASSERT_EQ(output[1]->id, "1");
		CXXIter::from(output).forEach([](auto ptr) { delete ptr; });
	}
	{ // volatile
		std::vector<volatile Parent*> input = { new Parent(), new Child1("0"), new Child1("1"), new Child2() };
		std::vector<volatile Child1*> output = CXXIter::from(input)
				.filterMap(CXXIter::tryDynCast<volatile Child1*>())
				.collect<std::vector>();
		ASSERT_EQ(output.size(), 2);
		ASSERT_EQ(const_cast<std::string&>(output[0]->id), "0");
		ASSERT_EQ(const_cast<std::string&>(output[1]->id), "1");
		CXXIter::from(output).forEach([](auto ptr) { delete ptr; });
	}
}
