cmake_minimum_required(VERSION 3.9)

project(FeatureTestCoroutine LANGUAGES CXX)

add_executable(FeatureTestCoroutine "main.cpp")
target_compile_features(FeatureTestCoroutine PRIVATE cxx_std_20)
