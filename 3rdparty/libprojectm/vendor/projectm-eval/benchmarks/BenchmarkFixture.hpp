#pragma once

#include <projectm-eval/api/projectm-eval.h>

#include <benchmark/benchmark.h>

/**
 * @brief Fixture that creates a context and an empty gmegabuf to run the benchmark.
 */
class BenchmarkFixture : public benchmark::Fixture
{
public:
    void SetUp(const benchmark::State& state) override
    {
        m_gmegabuf = projectm_eval_memory_buffer_create();
        m_context = projectm_eval_context_create(m_gmegabuf, m_globals);
    }

    void TearDown(const benchmark::State& state) override
    {
        projectm_eval_context_destroy(m_context);
        projectm_eval_memory_buffer_destroy(m_gmegabuf);
    }

protected:
    PRJM_EVAL_F (*m_globals)[100]{};
    projectm_eval_context* m_context{nullptr};
    projectm_eval_mem_buffer m_gmegabuf{nullptr};
};
