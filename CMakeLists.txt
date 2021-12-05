cmake_minimum_required(VERSION 3.5)

project(CIter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CIter main.cpp CIter.h)

target_compile_options(CIter PUBLIC -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-variable)
