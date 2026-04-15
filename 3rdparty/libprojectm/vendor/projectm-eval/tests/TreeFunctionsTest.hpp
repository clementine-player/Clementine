#pragma once

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

extern "C"
{
#include "projectm-eval/TreeFunctions.h"
#include <projectm-eval/MemoryBuffer.h>
};

#include <gtest/gtest.h>

#include <vector>

class TreeFunctions : public testing::Test
{
public:

protected:
    prjm_eval_variable_def_t* FindVariable(const char* name);

    prjm_eval_variable_def_t* CreateVariable(const char* name, PRJM_EVAL_F initialValue);

    prjm_eval_exptreenode_t* CreateEmptyNode(int argCount);

    prjm_eval_exptreenode_t* CreateConstantNode(PRJM_EVAL_F value);

    prjm_eval_exptreenode_t* CreateVariableNode(const char* name,
                                                PRJM_EVAL_F initialValue,
                                                prjm_eval_variable_def_t** variable);

    void SetUp() override;

    void TearDown() override;

    std::vector<prjm_eval_variable_def_t*> m_variables{};
    std::vector<prjm_eval_exptreenode_t*> m_treeNodes;
    projectm_eval_mem_buffer m_memoryBuffer{};
};