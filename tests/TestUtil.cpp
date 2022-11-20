#include <CXXIter/src/util/MaybeUninitialized.h>

#include "TestCommon.h"

using namespace CXXIter;

// ################################################################################################
// MAYBEUNINITIALIZED
// ################################################################################################

struct NoDefaultCtor {
	int flag;

	NoDefaultCtor() = delete;
	NoDefaultCtor(int flag) : flag(flag) {}
};

TEST(CXXIter, MaybeUninitialized) {
	util::MaybeUninitialized<std::array<NoDefaultCtor, 3>> maybeUninitData;
	ASSERT_FALSE(maybeUninitData.isInitialized());

	// init
	maybeUninitData.get()[0] = NoDefaultCtor(1337);
	maybeUninitData.get()[1] = NoDefaultCtor(42);
	maybeUninitData.get()[2] = NoDefaultCtor(69);

	const std::array<NoDefaultCtor, 3>& rawDataPtr = maybeUninitData.get();
	ASSERT_EQ(rawDataPtr[0].flag, 1337);
	ASSERT_EQ(rawDataPtr[1].flag, 42);
	ASSERT_EQ(rawDataPtr[2].flag, 69);

	// compare with normal array
	NoDefaultCtor stdArrayData[3] = { NoDefaultCtor(1337), NoDefaultCtor(42), NoDefaultCtor(69) };
	ASSERT_EQ(0, memcmp(rawDataPtr.data(), stdArrayData, sizeof(NoDefaultCtor[3])));
}
