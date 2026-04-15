#include "PrecedenceTest.hpp"

void PrecedenceTest::SetUp()
{
    m_globalMemory = projectm_eval_memory_buffer_create();
    m_context = projectm_eval_context_create(m_globalMemory, &m_globalRegisters);
}

void PrecedenceTest::TearDown()
{
    projectm_eval_context_destroy(m_context);
    projectm_eval_memory_buffer_destroy(m_globalMemory);
    memset(&m_globalRegisters, 0, sizeof(m_globalRegisters));
}

TEST_F(PrecedenceTest, DivisionMultiplication)
{
    auto code1 = projectm_eval_code_compile(m_context, "3.0 * 2.0 / 5.0 * 7.0;");
    auto code2 = projectm_eval_code_compile(m_context, "2.0 / 5.0 * 3.0 * 7.0;");
    auto code3 = projectm_eval_code_compile(m_context, "3.0 * 7.0 * 2.0 / 5.0;");

    auto ret1 = projectm_eval_code_execute(code1);
    auto ret2 = projectm_eval_code_execute(code2);
    auto ret3 = projectm_eval_code_execute(code3);

    projectm_eval_code_destroy(code1);
    projectm_eval_code_destroy(code2);
    projectm_eval_code_destroy(code3);

    ASSERT_FLOAT_EQ(ret1, 8.4);
    ASSERT_FLOAT_EQ(ret1, ret2);
    ASSERT_FLOAT_EQ(ret2, ret3);
    ASSERT_FLOAT_EQ(ret1, ret3);

}

TEST_F(PrecedenceTest, MultiplicationModulo)
{
    auto code1 = projectm_eval_code_compile(m_context, "3.0 * 2.0 % 5.0 * 7.0;");
    auto code2 = projectm_eval_code_compile(m_context, "2.0 % 5.0 * 3.0 * 7.0;");
    auto code3 = projectm_eval_code_compile(m_context, "3.0 * 7.0 * 2.0 % 5.0;");

    auto ret1 = projectm_eval_code_execute(code1);
    auto ret2 = projectm_eval_code_execute(code2);
    auto ret3 = projectm_eval_code_execute(code3);

    projectm_eval_code_destroy(code1);
    projectm_eval_code_destroy(code2);
    projectm_eval_code_destroy(code3);

    ASSERT_FLOAT_EQ(ret1, 42.0);
    ASSERT_FLOAT_EQ(ret1, ret2);
    ASSERT_FLOAT_EQ(ret2, ret3);
    ASSERT_FLOAT_EQ(ret1, ret3);
}

