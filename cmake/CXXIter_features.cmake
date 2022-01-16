# Probe for coroutine support

try_compile(CXXITER_HAS_COROUTINE "${CMAKE_CURRENT_BINARY_DIR}/coroutine_test" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/coroutine_test" FeatureTestCoroutine)
