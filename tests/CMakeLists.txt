enable_testing()
include(GoogleTest)
find_package(GTest REQUIRED)

add_executable(CIterTests CIter.cpp)
target_link_libraries(CIterTests GTest::gtest GTest::Main CIter)
target_compile_options(CIterTests PRIVATE -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-variable)
