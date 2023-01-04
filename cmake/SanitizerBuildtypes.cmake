#############
# ASAN[RelWithDebinfo] builds (add ASAN (address sanitizer) build configuration)
#############
list(APPEND CMAKE_CONFIGURATION_TYPES Asan)
set(CMAKE_CXX_FLAGS_ASAN
	"${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address -fsanitize-address-use-after-scope"
	CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds." FORCE)
set(CMAKE_EXE_LINKER_FLAGS_ASAN
	"${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -fsanitize=address"
	CACHE STRING "Flags used by the C++ linker during AddressSanitizer builds." FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_ASAN
	"${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -fsanitize=address"
	CACHE STRING "Flags used by the C++ linker during AddressSanitizer builds." FORCE)
mark_as_advanced(CMAKE_CXX_FLAGS_ASAN CMAKE_EXE_LINKER_FLAGS_ASAN CMAKE_SHARED_LINKER_FLAGS_ASAN)


#############
# ASAN[Debug] builds (add ASAN (address sanitizer) build configuration)
#############
list(APPEND CMAKE_CONFIGURATION_TYPES AsanDebug)
set(CMAKE_CXX_FLAGS_ASANDEBUG
	"${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address -fsanitize-address-use-after-scope"
	CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds." FORCE)
set(CMAKE_EXE_LINKER_FLAGS_ASANDEBUG
	"${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address"
	CACHE STRING "Flags used by the C++ linker during AddressSanitizer builds." FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_ASANDEBUG
	"${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address"
	CACHE STRING "Flags used by the C++ linker during AddressSanitizer builds." FORCE)
mark_as_advanced(CMAKE_CXX_FLAGS_ASANDEBUG CMAKE_EXE_LINKER_FLAGS_ASANDEBUG CMAKE_SHARED_LINKER_FLAGS_ASANDEBUG)


#############
# TSAN builds (add TSAN (thread sanitizer) build configuration)
#############
list(APPEND CMAKE_CONFIGURATION_TYPES Tsan)
set(CMAKE_CXX_FLAGS_TSAN
	"${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=thread"
	CACHE STRING "Flags used by the C++ compiler during ThreadSanitizer builds." FORCE)
set(CMAKE_EXE_LINKER_FLAGS_TSAN
	"${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -fsanitize=thread"
	CACHE STRING "Flags used by the C++ linker during ThreadSanitizer builds." FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_TSAN
	"${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -fsanitize=thread"
	CACHE STRING "Flags used by the C++ linker during ThreadSanitizer builds." FORCE)
mark_as_advanced(CMAKE_CXX_FLAGS_TSAN CMAKE_EXE_LINKER_FLAGS_TSAN CMAKE_SHARED_LINKER_FLAGS_TSAN)
