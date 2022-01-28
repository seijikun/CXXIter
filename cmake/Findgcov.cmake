if(NOT GCOV_${CMAKE_CXX_COMPILER_ID}_BIN)
	get_filename_component(COMPILER_PATH "${CMAKE_CXX_COMPILER}" PATH)

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		string(REGEX MATCH "^[0-9]+" GCC_VERSION
						"${CMAKE_CXX_COMPILER_VERSION}")
		find_program(GCOV_BIN NAMES gcov-${GCC_VERSION} gcov HINTS ${COMPILER_PATH})
	else()
		message(WARNING "Warning: Only GCC and Gcov supported, atm.")
	endif()

	if (NOT GCOV_BIN)
		# Fall back to gcov binary if llvm-cov was not found or is
		# incompatible. This is the default on OSX, but may crash on
		# recent Linux versions.
		find_program(GCOV_BIN gcov HINTS ${COMPILER_PATH})
	endif ()
endif()
