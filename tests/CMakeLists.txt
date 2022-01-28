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

# CODE COVERAGE
function(add_coverage_for EXE_TARGET_NAME)
	message(STATUS "Compiling target ${EXE_TARGET_NAME} with CodeCoverage instrumentation")
	# configure build of target to include coverage instrumentation
	target_compile_options(${EXE_TARGET_NAME} PRIVATE -O0 -g -fprofile-arcs -ftest-coverage -fno-inline -fno-inline-small-functions -fno-default-inline)
	target_link_options(${EXE_TARGET_NAME} PRIVATE -fprofile-arcs -ftest-coverage)

	get_target_property(TSOURCES ${EXE_TARGET_NAME} SOURCES)
	get_target_property(TSOURCE_DIR ${EXE_TARGET_NAME} SOURCE_DIR)
	get_target_property(TBIN_DIR ${EXE_TARGET_NAME} BINARY_DIR)
	set(TOBJECTS_DIR ${TBIN_DIR}/CMakeFiles/${EXE_TARGET_NAME}.dir)

	# register target that runs the target executable to generate gcov-gcda files
	add_custom_target(${EXE_TARGET_NAME}-gcovrun
		COMMAND ${TBIN_DIR}/${EXE_TARGET_NAME}
		DEPENDS ${EXE_TARGET_NAME}
		WORKING_DIRECTORY ${TBIN_DIR}
		COMMENT "Run ${EXE_TARGET_NAME}-executable to produce coverage report files"
	)

	set(COVERAGE_REPORT_FILE ${TBIN_DIR}/${EXE_TARGET_NAME}_coverage.html)
	find_program(GCOVR_BIN gcovr)
	add_custom_command(OUTPUT ${COVERAGE_REPORT_FILE}
		COMMAND ${GCOVR_ENV} ${GCOVR_BIN} --html-details -r ${PROJECT_SOURCE_DIR} ${TOBJECTS_DIR} --output ${COVERAGE_REPORT_FILE}
		DEPENDS ${EXE_TARGET_NAME}-gcovrun
		WORKING_DIRECTORY ${TBIN_DIR})
	add_custom_target(${EXE_TARGET_NAME}-gcovr DEPENDS ${COVERAGE_REPORT_FILE})

	# Add to global CodeCoverage target
	add_dependencies(CodeCoverage ${EXE_TARGET_NAME}-gcovr)
endfunction(add_coverage_for)

if(CMAKE_BUILD_TYPE STREQUAL "CodeCoverage")
	add_custom_target(CodeCoverage)
	add_coverage_for(CXXIterTests)
endif()
