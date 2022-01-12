cmake_minimum_required(VERSION 3.9)

project(CXXIter LANGUAGES CXX)

option(CXXITER_WITH_TESTS "Compile the CIter tests" OFF)
option(CXXITER_WITH_DOCS "Run Doxygen to produce documentation" OFF)

FILE(GLOB_RECURSE CXXITER_SOURCES "include/*.h")
add_library(CXXIter INTERFACE ${CXXITER_SOURCES})
target_include_directories(CXXIter INTERFACE "include/")
target_compile_features(CXXIter INTERFACE cxx_std_20)

# INSTALL
install(DIRECTORY include/CXXIter TYPE INCLUDE)

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
	set(DOXYGEN_EXTRACT_ALL NO)
	set(DOXYGEN_SOURCE_BROWSER YES)
	set(DOXYGEN_GENERATE_TREEVIEW YES)
	set(DOXYGEN_UML_LOOK YES)
	set(DOXYGEN_CALL_GRAPH YES)
	set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
	set(DOXYGEN_INTERACTIVE_SVG YES)
	set(DOXYGEN_DOT_TRANSPARENT YES)

	doxygen_add_docs("CXXIterDocs"
		"${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/doc"
		ALL
		COMMENT "Generate Doxygen documentation"
	)
endif()
