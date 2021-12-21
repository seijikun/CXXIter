include(GoogleTest)
find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

file(GLOB_RECURSE CXXITER_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Test*.cpp)
add_executable(CXXIterTests ${CXXITER_TEST_SOURCES} TestCommon.h)
target_link_libraries(CXXIterTests GTest::gtest GTest::Main CXXIter)
target_compile_options(CXXIterTests PRIVATE -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-variable)
gtest_discover_tests(CXXIterTests)

add_executable(CXXIterBenchmark Benchmark.cpp)
target_link_libraries(CXXIterBenchmark benchmark::benchmark CXXIter)
target_compile_options(CXXIterBenchmark PRIVATE -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-variable -Ofast -march=native)
