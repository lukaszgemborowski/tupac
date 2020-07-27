cmake_minimum_required (VERSION 3.2)
project (tupac)

add_library(
    tupac
    INTERFACE
)

target_include_directories(
    tupac
    INTERFACE
    include
)

target_compile_features(
    tupac
    INTERFACE
    cxx_std_17
)

enable_testing()
add_subdirectory(tests)
