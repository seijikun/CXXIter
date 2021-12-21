cmake_minimum_required(VERSION 3.5)

project(CXXIter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CXXITER_WITH_TESTS "Compile the CIter tests" OFF)
option(CXXITER_WITH_DOCS "Run Doxygen to produce documentation" OFF)

FILE(GLOB_RECURSE CXXITER_SOURCES "src/*.h")
add_library(${PROJECT_NAME} INTERFACE ${CXXITER_SOURCES})
target_include_directories(${PROJECT_NAME} INTERFACE "src/")

# UNIT-TESTS
if(${CXXITER_WITH_TESTS})
	enable_testing() # has to be done in main CMakeLists file
	add_subdirectory(tests)
endif()

# DOXYGEN DOCS
if(${CXXITER_WITH_DOCS})
	find_package(Doxygen REQUIRED dot)
	# configure doxygen
	set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
	set(DOXYGEN_DISTRIBUTE_GROUP_DOC YES)
	set(DOXYGEN_EXTRACT_ALL YES)
	set(DOXYGEN_EXTRACT_STATIC YES)
	set(DOXYGEN_SOURCE_BROWSER YES)
	set(DOXYGEN_GENERATE_TREEVIEW YES)
	set(DOXYGEN_UML_LOOK YES)
	set(DOXYGEN_CALL_GRAPH YES)
	set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
	set(DOXYGEN_INTERACTIVE_SVG YES)
	set(DOXYGEN_DOT_TRANSPARENT YES)

	doxygen_add_docs("${PROJECT_NAME}Docs" "${CMAKE_CURRENT_SOURCE_DIR}/src" ALL
		COMMENT "Generate Doxygen documentation")
endif()
