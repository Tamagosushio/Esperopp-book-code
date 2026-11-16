cmake_minimum_required(VERSION 3.20)
project(epp LANGUAGES CXX)

add_library(epp_compiler STATIC
  src/unicode/x_system.cc
)
target_compile_features(epp_compiler PUBLIC cxx_std_20)
target_include_directories(epp_compiler
  PUBLIC "${PROJECT_SOURCE_DIR}/include"
)

add_executable(epp src/main.cc)
target_include_directories(epp PRIVATE
  "${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(epp PRIVATE epp_compiler)
