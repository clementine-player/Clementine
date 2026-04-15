find_package(benchmark)

if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark not found, benchmark will not be built.")
    return()
endif()

add_executable(projectM_EvalLib-Benchmark
        BenchmarkFixture.hpp
        Functions.cpp
        Programs.cpp
        Stubs.cpp
        )

target_link_libraries(projectM_EvalLib-Benchmark
        PRIVATE
        projectM::Eval
        benchmark::benchmark_main
        )
