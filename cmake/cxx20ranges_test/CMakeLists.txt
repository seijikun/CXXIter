cmake_minimum_required(VERSION 3.9)

project(FeatureTestCXX20Ranges LANGUAGES CXX)

add_executable(FeatureTestCXX20Ranges "main.cpp")
target_compile_features(FeatureTestCXX20Ranges PRIVATE cxx_std_20)
