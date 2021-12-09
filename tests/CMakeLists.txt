enable_testing()
include(GoogleTest)
find_package(GTest REQUIRED)

add_executable(CXXIterTests CXXIter.cpp)
target_link_libraries(CXXIterTests GTest::gtest GTest::Main CXXIter)
target_compile_options(CXXIterTests PRIVATE -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-variable)
