#include "SyntaxTest.hpp"


void SyntaxTest::SetUp()
{
    m_globalMemory = projectm_eval_memory_buffer_create();
    m_context = projectm_eval_context_create(m_globalMemory, &m_globalRegisters);
}

void SyntaxTest::TearDown()
{
    projectm_eval_context_destroy(m_context);
    projectm_eval_memory_buffer_destroy(m_globalMemory);
    memset(&m_globalRegisters, 0, sizeof(m_globalRegisters));
}

TEST_F(SyntaxTest, AdditionalLineBreak)
{
    auto code = projectm_eval_code_compile(m_context, "k1 =  is_\nbeat*equal(index%2,0);");

    ASSERT_EQ(code, nullptr);
    ASSERT_STREQ(projectm_eval_get_error(m_context, nullptr, nullptr), "syntax error, unexpected VAR");
}
