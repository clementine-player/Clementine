cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0127 NEW) # <depends> syntax in cmake_dependent_option() is properly formatted for both old and new behavior.

set(LANGUAGES C)
if(BUILD_TESTING)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    list(APPEND LANGUAGES CXX)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(PROJECTM_EVAL_FLOAT_SIZE "8" CACHE STRING "Byte size of floating-point numbers. 8 to use double, 4 to use float. Default is 8.")

include(CMakeDependentOption)

option(ENABLE_FAST_MATH "Enables aggressive math optimizations like -ffast-math to compile faster code. Applied to Release and RelWithDebInfo configurations only." ON)
option(BUILD_NS_EEL_SHIM "Build and install the ns-eel2 compatibility API shim." OFF)
option(BUILD_BENCHMARKS "Build benchmarks. Requires Google Benchmark." OFF)
if(NOT PROJECTM_EVAL_FLOAT_SIZE EQUAL 8 AND NOT PROJECTM_EVAL_FLOAT_SIZE EQUAL 4)
    message(FATAL_ERROR "PROJECTM_EVAL_FLOAT_SIZE must be set to either 4 (use floats) or 8 (use doubles).")
endif()

project(projectm-eval
        VERSION 1.0.5
        LANGUAGES ${LANGUAGES} # Using "enable_language(CXX)" in the test dir will NOT work properly!
        )

cmake_dependent_option(ENABLE_PROJECTM_EVAL_INSTALL "Enable installing projectm-eval libraries and headers." OFF "NOT projectm-eval_IS_TOP_LEVEL" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(FastMathOptimizations)
include(CheckCSourceCompiles)

check_c_source_compiles("#include <math.h>\nint main() { double x = sin(1.0);return 0;}" NO_MATH_LIB_REQUIRED)
if(NOT NO_MATH_LIB_REQUIRED)
    list(APPEND CMAKE_REQUIRED_LIBRARIES m)
endif()

find_package(BISON 3.8)
find_package(FLEX 2.6)

add_subdirectory(projectm-eval)

if(BUILD_NS_EEL_SHIM)
    add_subdirectory(ns-eel2-shim)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
