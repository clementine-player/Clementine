#pragma once

#include <gtest/gtest.h>

#include <projectm-eval/api/projectm-eval.h>

class SyntaxTest : public testing::Test
{
public:

protected:

    void SetUp() override;

    void TearDown() override;

    struct projectm_eval_context* m_context{};
    projectm_eval_mem_buffer m_globalMemory{};
    PRJM_EVAL_F m_globalRegisters[100]{};
};
