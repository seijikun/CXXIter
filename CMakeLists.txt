cmake_minimum_required(VERSION 3.13)

project(CXXIter LANGUAGES CXX VERSION 0.2)

# add cmake to module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CXXIter_features)

option(CXXITER_WITH_TESTS "Compile the CIter tests" OFF)
option(CXXITER_WITH_DOCS "Run Doxygen to produce documentation" OFF)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0")
	# cmake pre 3.19 does not support the specifying of source files in INTERFACE libraries
	file(GLOB_RECURSE CXXITER_SOURCES "include/*.h")
endif()
add_library(CXXIter INTERFACE ${CXXITER_SOURCES})
target_include_directories(CXXIter INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(CXXIter INTERFACE cxx_std_20)
target_compile_definitions(CXXIter INTERFACE ${CXXITER_FEATUREFLAG_COMPILE_DEFINITIONS})

# INSTALL
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

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
	# enable all conditional feature-flags for documentation
	set(DOXYGEN_PREDEFINED ${CXXITER_FEATUREFLAG_NAMES})

	doxygen_add_docs("CXXIterDocs"
		"${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/doc"
		ALL
		COMMENT "Generate Doxygen documentation"
	)
endif()
