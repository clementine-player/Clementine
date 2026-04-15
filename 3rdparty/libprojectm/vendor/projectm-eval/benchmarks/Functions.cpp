#include "BenchmarkFixture.hpp"

class FunctionBenchmarks : public BenchmarkFixture
{};

BENCHMARK_F(FunctionBenchmarks, Constant)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "1");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, Variable)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "x");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, AssignConstToVar)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "x = 1");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, AssignVarToVar)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "x = y");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, AddConstants)(benchmark::State& st)
{
    // Gets optimized to a single "3.5" expression
    auto code = projectm_eval_code_compile(m_context, "1 + 2.5");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, AddVars)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "x + y");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, SubtractConstants)(benchmark::State& st)
{
    // Gets optimized to a single "-1.5" expression
    auto code = projectm_eval_code_compile(m_context, "1 - 2.5");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, SubtractVars)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "x - y");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, MultiplyConstants)(benchmark::State& st)
{
    // Gets optimized to a single "5.0" expression
    auto code = projectm_eval_code_compile(m_context, "2 * 2.5");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, MultiplyVars)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "x * y");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, DivideConstants)(benchmark::State& st)
{
    // Gets optimized to a single "3.0" expression
    auto code = projectm_eval_code_compile(m_context, "4.5 / 1.5");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, DivideVars)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "reg00 = 4.5; reg01 = 1.5;");
    projectm_eval_code_execute(code);
    projectm_eval_code_destroy(code);

    code = projectm_eval_code_compile(m_context, "reg00 / reg01");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, ModuloConstants)(benchmark::State& st)
{
    // Gets optimized to a single "1.0" expression
    auto code = projectm_eval_code_compile(m_context, "4 % 3");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, ModuloVars)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "reg00 = 4; reg01 = 3;");
    projectm_eval_code_execute(code);
    projectm_eval_code_destroy(code);

    code = projectm_eval_code_compile(m_context, "reg00 % reg01");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, PowConstants)(benchmark::State& st)
{
    // Gets optimized to a single "16.0" expression
    auto code = projectm_eval_code_compile(m_context, "4 ^ 2");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, PowVars)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "reg00 = 4; reg01 = 2;");
    projectm_eval_code_execute(code);
    projectm_eval_code_destroy(code);

    code = projectm_eval_code_compile(m_context, "reg00 ^ reg01");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, SineConstant)(benchmark::State& st)
{
    // Gets optimized to a single (ca) "-0.7568" expression
    auto code = projectm_eval_code_compile(m_context, "sin(4)");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, SineVar)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "reg00 = 4");
    projectm_eval_code_execute(code);
    projectm_eval_code_destroy(code);

    code = projectm_eval_code_compile(m_context, "sin(reg00)");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}


BENCHMARK_F(FunctionBenchmarks, CosineConstant)(benchmark::State& st)
{
    // Gets optimized to a single (ca) "-0.6536" expression
    auto code = projectm_eval_code_compile(m_context, "cos(4)");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}

BENCHMARK_F(FunctionBenchmarks, CosineVar)(benchmark::State& st)
{
    auto code = projectm_eval_code_compile(m_context, "reg00 = 4");
    projectm_eval_code_execute(code);
    projectm_eval_code_destroy(code);

    code = projectm_eval_code_compile(m_context, "cos(reg00)");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}
