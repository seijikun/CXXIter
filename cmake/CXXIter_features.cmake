# Probe for coroutine support

try_compile(CXXITER_HAS_COROUTINE "${CMAKE_CURRENT_BINARY_DIR}/coroutine_test" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/coroutine_test" FeatureTestCoroutine)
try_compile(CXXITER_HAS_CXX20RANGES "${CMAKE_CURRENT_BINARY_DIR}/cxx20ranges_test" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx20ranges_test" FeatureTestCXX20Ranges)

set(CXXITER_FEATUREFLAG_NAMES "")
list(APPEND CXXITER_FEATUREFLAG_NAMES "CXXITER_HAS_COROUTINE")
list(APPEND CXXITER_FEATUREFLAG_NAMES "CXXITER_HAS_CXX20RANGES")

set(CXXITER_FEATUREFLAG_COMPILE_DEFINITIONS
	$<$<BOOL:${CXXITER_HAS_COROUTINE}>:CXXITER_HAS_COROUTINE> $<$<BOOL:${CXXITER_HAS_CXX20RANGES}>:CXXITER_HAS_CXX20RANGES>
)