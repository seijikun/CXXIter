cmake_minimum_required(VERSION 3.5)

project(CXXIter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CXXITER_WITH_TESTS "Compile the CIter tests" OFF)


FILE(GLOB_RECURSE CXXITER_SOURCES "*.h")
add_library(${PROJECT_NAME} INTERFACE ${CITER_SOURCES})
target_include_directories(${PROJECT_NAME} INTERFACE "src/")

if(${CXXITER_WITH_TESTS})
	add_subdirectory(tests)
endif()
