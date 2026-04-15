#include "TreeFunctionsTest.hpp"

extern "C" {
#include <projectm-eval/ExpressionTree.h>
}

#include <cmath>

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp _strnicmp
#endif

#if PRJM_F_SIZE == 4
#define ASSERT_PRJM_F_EQ ASSERT_FLOAT_EQ
#define EXPECT_PRJM_F_EQ EXPECT_FLOAT_EQ
#else
#define ASSERT_PRJM_F_EQ ASSERT_DOUBLE_EQ
#define EXPECT_PRJM_F_EQ EXPECT_DOUBLE_EQ
#endif

prjm_eval_variable_def_t* TreeFunctions::FindVariable(const char* name)
{
    for (const auto var : m_variables)
    {
        if (strcasecmp(name, var->name) == 0)
        {
            return var;
        }
    }

    return nullptr;
}

prjm_eval_variable_def_t* TreeFunctions::CreateVariable(const char* name, PRJM_EVAL_F initialValue)
{
    auto* var = FindVariable(name);
    if (!var)
    {
        var = new prjm_eval_variable_def_t;
    }

    var->name = strdup(name);
    var->value = initialValue;

    m_variables.push_back(var);

    return var;
}

prjm_eval_exptreenode_t* TreeFunctions::CreateEmptyNode(int argCount)
{
    auto* node = reinterpret_cast<prjm_eval_exptreenode_t*>(calloc(1, sizeof(prjm_eval_exptreenode_t)));
    if (argCount > 0)
    {
        node->args = reinterpret_cast<prjm_eval_exptreenode_t**>(calloc(argCount + 1,
                                                                        sizeof(prjm_eval_exptreenode_t*)));
    }
    return node;
}

prjm_eval_exptreenode_t* TreeFunctions::CreateConstantNode(PRJM_EVAL_F value)
{
    auto* varNode = CreateEmptyNode(0);

    varNode->func = prjm_eval_func_const;
    varNode->value = value;

    return varNode;
}

prjm_eval_exptreenode_t*
TreeFunctions::CreateVariableNode(const char* name, PRJM_EVAL_F initialValue, prjm_eval_variable_def_t** variable)
{
    *variable = CreateVariable(name, initialValue);

    auto* varNode = CreateEmptyNode(0);

    varNode->func = prjm_eval_func_var;
    varNode->var = &(*variable)->value;

    return varNode;
}

void TreeFunctions::SetUp()
{
    Test::SetUp();
}

void TreeFunctions::TearDown()
{
    for (auto node : m_treeNodes)
    {
        prjm_eval_destroy_exptreenode(node);
    }
    m_treeNodes.clear();

    for (const auto var : m_variables)
    {
        free(var->name); // alloc'd via C malloc/strdup!
        delete var;
    }
    m_variables.clear();

    if (m_memoryBuffer != nullptr)
    {
        prjm_eval_memory_destroy_buffer(m_memoryBuffer);
        m_memoryBuffer = nullptr;
    }

    Test::TearDown();
}


TEST_F(TreeFunctions, Constant)
{
    auto* constNode = CreateConstantNode(5.0f);

    m_treeNodes.push_back(constNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    constNode->func(constNode, &valuePointer);

    ASSERT_PRJM_F_EQ(*valuePointer, 5.);
}

TEST_F(TreeFunctions, Variable)
{
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 5.f, &var);

    m_treeNodes.push_back(varNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    varNode->func(varNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 5.);
    ASSERT_EQ(valuePointer, &var->value);
}

TEST_F(TreeFunctions, ExecuteList)
{
    // Expression list ("x = -50; y = 50;")
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 5.f, &var1);
    auto* constNode1 = CreateConstantNode(-50.0f);

    auto* setNode1 = CreateEmptyNode(2);
    setNode1->func = prjm_eval_func_set;
    setNode1->args[0] = varNode1;
    setNode1->args[1] = constNode1;


    prjm_eval_variable_def_t* var2;
    auto* varNode2 = CreateVariableNode("y", 123.f, &var2);
    auto* constNode2 = CreateConstantNode(50.0f);

    auto* setNode2 = CreateEmptyNode(2);
    setNode2->func = prjm_eval_func_set;
    setNode2->args[0] = varNode2;
    setNode2->args[1] = constNode2;

    auto* listItem = new prjm_eval_exptreenode_list_item_t{};
    listItem->expr = setNode1;
    listItem->next = new prjm_eval_exptreenode_list_item_t{};
    listItem->next->expr = setNode2;

    // Executor
    auto* listNode = CreateEmptyNode(1);
    listNode->func = prjm_eval_func_execute_list;
    listNode->list = listItem;

    m_treeNodes.push_back(listNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    listNode->func(listNode, &valuePointer);

    // Last executed node (y = 50) is return value
    EXPECT_PRJM_F_EQ(*valuePointer, 50.);

    // Both constants should now be assigned to each respective variable
    EXPECT_PRJM_F_EQ(var1->value, -50.);
    EXPECT_PRJM_F_EQ(var2->value, 50.);
}

TEST_F(TreeFunctions, ExecuteLoop)
{
    // Test expression: "loop(42, x += 1)" with x starting at 0.
    prjm_eval_variable_def_t* varX;
    auto* varNodeX = CreateVariableNode("x", 0.f, &varX);
    auto* constNode1 = CreateConstantNode(1.0f);
    auto* constNode42 = CreateConstantNode(42.0f);

    auto* incrementNode1 = CreateEmptyNode(2);
    incrementNode1->func = prjm_eval_func_add_op;
    incrementNode1->args[0] = varNodeX;
    incrementNode1->args[1] = constNode1;

    // Executor
    auto* loopNode = CreateEmptyNode(2);
    loopNode->func = prjm_eval_func_execute_loop;
    loopNode->args[0] = constNode42;
    loopNode->args[1] = incrementNode1;

    m_treeNodes.push_back(loopNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    loopNode->func(loopNode, &valuePointer);

    // Last executed node ("41 += 1" resulting in 42) is return value
    EXPECT_PRJM_F_EQ(*valuePointer, 42.);
    EXPECT_PRJM_F_EQ(varX->value, 42.);
}

TEST_F(TreeFunctions, ExecuteWhile)
{
    // Test expression: "while(x -= 1)" with x starting at 42.
    prjm_eval_variable_def_t* varX;
    auto* varNodeX = CreateVariableNode("x", 42.f, &varX);
    auto* constNode1 = CreateConstantNode(1.0f);

    auto* decrementNode1 = CreateEmptyNode(2);
    decrementNode1->func = prjm_eval_func_sub_op;
    decrementNode1->args[0] = varNodeX;
    decrementNode1->args[1] = constNode1;

    // Executor
    auto* whileNode = CreateEmptyNode(1);
    whileNode->func = prjm_eval_func_execute_while;
    whileNode->args[0] = decrementNode1;

    m_treeNodes.push_back(whileNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    whileNode->func(whileNode, &valuePointer);

    // Last executed node ("1 -= 1" resulting in 0) is return value
    EXPECT_PRJM_F_EQ(*valuePointer, 0.);
    EXPECT_PRJM_F_EQ(varX->value, 0.);
}

TEST_F(TreeFunctions, If)
{
    // Test expression: "if(x < 10, 42, 666)"
    prjm_eval_variable_def_t* varX;
    auto* varNodeX = CreateVariableNode("x", 0.f, &varX);
    auto* constNode10 = CreateConstantNode(10.0f);
    auto* constNode42 = CreateConstantNode(42.0f);
    auto* constNode666 = CreateConstantNode(999.0f);

    auto* comparisonNode = CreateEmptyNode(2);
    comparisonNode->func = prjm_eval_func_below;
    comparisonNode->args[0] = varNodeX;
    comparisonNode->args[1] = constNode10;

    auto* ifNode = CreateEmptyNode(3);
    ifNode->func = prjm_eval_func_if;
    ifNode->args[0] = comparisonNode;
    ifNode->args[1] = constNode42;
    ifNode->args[2] = constNode666;

    m_treeNodes.push_back(ifNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    ifNode->func(ifNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, constNode42->value);

    varX->value = 1000.;

    ifNode->func(ifNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, constNode666->value);
}

TEST_F(TreeFunctions, IfWithReferenceReturn)
{
    // Test expression: "if(x < 10, 42, y) = 666"
    // Assigns 666 to variable y if x >= 10
    prjm_eval_variable_def_t* varX;
    auto* varNodeX = CreateVariableNode("x", 0., &varX);
    prjm_eval_variable_def_t* varY;
    auto* varNodeY = CreateVariableNode("y", 999., &varY);
    auto* constNode10 = CreateConstantNode(10.);
    auto* constNode42 = CreateConstantNode(42.);
    auto* constNode666 = CreateConstantNode(666.);

    auto* comparisonNode = CreateEmptyNode(2);
    comparisonNode->func = prjm_eval_func_below;
    comparisonNode->args[0] = varNodeX;
    comparisonNode->args[1] = constNode10;

    auto* ifNode = CreateEmptyNode(3);
    ifNode->func = prjm_eval_func_if;
    ifNode->args[0] = comparisonNode;
    ifNode->args[1] = constNode42;
    ifNode->args[2] = varNodeY;

    auto* setNode = CreateEmptyNode(2);
    setNode->func = prjm_eval_func_set;
    setNode->args[0] = ifNode;
    setNode->args[1] = constNode666;

    m_treeNodes.push_back(setNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    setNode->func(setNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, constNode666->value);
    EXPECT_PRJM_F_EQ(varY->value, 999.);

    varX->value = 1000.;

    setNode->func(setNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, constNode666->value);
    EXPECT_PRJM_F_EQ(varY->value, constNode666->value);
}

TEST_F(TreeFunctions, Execute2)
{
    // Expression: "exec2(x = -50, y = 50)"
    // Syntactically identical to just writing "x = -50; y = 50"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 5.f, &var1);
    auto* constNode1 = CreateConstantNode(-50.0f);

    auto* setNode1 = CreateEmptyNode(2);
    setNode1->func = prjm_eval_func_set;
    setNode1->args[0] = varNode1;
    setNode1->args[1] = constNode1;

    prjm_eval_variable_def_t* var2;
    auto* varNode2 = CreateVariableNode("y", 123.f, &var2);
    auto* constNode2 = CreateConstantNode(50.0f);

    auto* setNode2 = CreateEmptyNode(2);
    setNode2->func = prjm_eval_func_set;
    setNode2->args[0] = varNode2;
    setNode2->args[1] = constNode2;

    auto* exec2Node = CreateEmptyNode(2);
    exec2Node->func = prjm_eval_func_exec2;
    exec2Node->args[0] = setNode1;
    exec2Node->args[1] = setNode2;

    m_treeNodes.push_back(exec2Node);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    exec2Node->func(exec2Node, &valuePointer);

    // Last executed node (y = 50) is return value
    EXPECT_PRJM_F_EQ(*valuePointer, 50.);

    // Both constants should now be assigned to each respective variable
    EXPECT_PRJM_F_EQ(var1->value, -50.);
    EXPECT_PRJM_F_EQ(var2->value, 50.);
}

TEST_F(TreeFunctions, Execute3)
{
    // Expression: "exec2(x = -50, y = 50)"
    // Syntactically identical to just writing "x = -50; y = 50"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 5.f, &var1);
    auto* constNode1 = CreateConstantNode(-50.0f);

    auto* setNode1 = CreateEmptyNode(2);
    setNode1->func = prjm_eval_func_set;
    setNode1->args[0] = varNode1;
    setNode1->args[1] = constNode1;

    prjm_eval_variable_def_t* var2;
    auto* varNode2 = CreateVariableNode("y", 123.f, &var2);
    auto* constNode2 = CreateConstantNode(50.0f);

    auto* setNode2 = CreateEmptyNode(2);
    setNode2->func = prjm_eval_func_set;
    setNode2->args[0] = varNode2;
    setNode2->args[1] = constNode2;

    prjm_eval_variable_def_t* var3;
    auto* varNode3 = CreateVariableNode("z", 456.f, &var3);
    auto* constNode3 = CreateConstantNode(200.0f);

    auto* setNode3 = CreateEmptyNode(2);
    setNode3->func = prjm_eval_func_set;
    setNode3->args[0] = varNode3;
    setNode3->args[1] = constNode3;

    auto* exec3Node = CreateEmptyNode(3);
    exec3Node->func = prjm_eval_func_exec3;
    exec3Node->args[0] = setNode1;
    exec3Node->args[1] = setNode2;
    exec3Node->args[2] = setNode3;

    m_treeNodes.push_back(exec3Node);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    exec3Node->func(exec3Node, &valuePointer);

    // Last executed node (z = 200) is return value
    EXPECT_PRJM_F_EQ(*valuePointer, 200.);

    // All three constants should now be assigned to each respective variable
    EXPECT_PRJM_F_EQ(var1->value, -50.);
    EXPECT_PRJM_F_EQ(var2->value, 50.);
    EXPECT_PRJM_F_EQ(var3->value, 200.);
}

TEST_F(TreeFunctions, Assignment)
{
    // Expression: "x = y" or "_set(x, y)" or "assign(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5.f, &var1);
    auto* varNode2 = CreateVariableNode("y", 45.f, &var2);

    auto* setNode = CreateEmptyNode(2);
    setNode->func = prjm_eval_func_set;
    setNode->args[0] = varNode1;
    setNode->args[1] = varNode2;

    m_treeNodes.push_back(setNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    setNode->func(setNode, &valuePointer);

    EXPECT_EQ(valuePointer, &var1->value);
    EXPECT_NE(valuePointer, &var2->value);
    EXPECT_PRJM_F_EQ(*valuePointer, 45.0);
    EXPECT_PRJM_F_EQ(var1->value, 45.0);
}

TEST_F(TreeFunctions, MemoryAccess)
{
    // Expression: "mem[42] = 59"
    // megabuf(), gmem[] and gmegabuf() are equivalent, they only get different memory buffer pointers during compilation.
    m_memoryBuffer = prjm_eval_memory_create_buffer();

    auto* constNode42 = CreateConstantNode(42.);
    auto* constNode50 = CreateConstantNode(50.0f);

    auto* memNode = CreateEmptyNode(1);
    memNode->func = prjm_eval_func_mem;
    memNode->memory_buffer = m_memoryBuffer;
    memNode->args[0] = constNode42;

    auto* setNode = CreateEmptyNode(2);
    setNode->func = prjm_eval_func_set;
    setNode->args[0] = memNode;
    setNode->args[1] = constNode50;

    m_treeNodes.push_back(setNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    setNode->func(setNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 50.0);

    PRJM_EVAL_F* memoryPointer = prjm_eval_memory_allocate(m_memoryBuffer, static_cast<int>(constNode42->value));
    ASSERT_NE(memoryPointer, nullptr);
    EXPECT_EQ(*memoryPointer, 50.0);
}

TEST_F(TreeFunctions, FreeMemoryBuffer)
{
    // Expression: "freembuf(10)"
    // No memory should be freed, as this function doesn't do anything in Milkdrop.
    m_memoryBuffer = prjm_eval_memory_create_buffer();

    auto* constNode10 = CreateConstantNode(10.);

    auto* freembufNode = CreateEmptyNode(1);
    freembufNode->func = prjm_eval_func_freembuf;
    freembufNode->memory_buffer = m_memoryBuffer;
    freembufNode->args[0] = constNode10;

    m_treeNodes.push_back(freembufNode);

    // 10th value in block 10
    PRJM_EVAL_F* memoryPointer = prjm_eval_memory_allocate(m_memoryBuffer, 65536 * 10 + 10);
    *memoryPointer = 123.;

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    freembufNode->func(freembufNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 10.);
    EXPECT_PRJM_F_EQ(*memoryPointer, 123.);
}

TEST_F(TreeFunctions, MemoryCopyWithOverlap)
{
    // Expression: "memcpy(65536, 65636, 200)"
    m_memoryBuffer = prjm_eval_memory_create_buffer();

    auto* constNode65536 = CreateConstantNode(65536.);
    auto* constNode65636 = CreateConstantNode(65636.);
    auto* constNode200 = CreateConstantNode(200.);

    auto* memcpyNode = CreateEmptyNode(3);
    memcpyNode->func = prjm_eval_func_memcpy;
    memcpyNode->memory_buffer = m_memoryBuffer;
    memcpyNode->args[0] = constNode65536;
    memcpyNode->args[1] = constNode65636;
    memcpyNode->args[2] = constNode200;

    m_treeNodes.push_back(memcpyNode);

    // Populate the whole range (65536 to 65835) with increasing numbers
    for (int index = 0; index < 300; ++index)
    {
        PRJM_EVAL_F* memoryPointer = prjm_eval_memory_allocate(m_memoryBuffer, 65536 + index);
        ASSERT_NE(memoryPointer, nullptr);
        *memoryPointer = static_cast<PRJM_EVAL_F>(index + 1);
    }

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    memcpyNode->func(memcpyNode, &valuePointer);

    // Destination index is returned
    EXPECT_PRJM_F_EQ(*valuePointer, constNode65536->value);

    // Check memory, should now contain indices 101-300, followed by 201-300.
    // Using assertions here to not spam the output with 300 errors if something failed.
    for (int index = 0; index < 300; ++index)
    {
        PRJM_EVAL_F* memoryPointer = prjm_eval_memory_allocate(m_memoryBuffer, 65536 + index);
        ASSERT_NE(memoryPointer, nullptr);
        if (index < 200)
        {
            ASSERT_EQ(static_cast<int>(*memoryPointer), index + 101);
        }
        else
        {
            ASSERT_EQ(static_cast<int>(*memoryPointer), index + 1);
        }
    }
}

TEST_F(TreeFunctions, LogicalNotOperator)
{
    // Expression: "!x" or "_not(x)" or "bnot(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);

    auto* bnotNode = CreateEmptyNode(1);
    bnotNode->func = prjm_eval_func_bnot;
    bnotNode->args[0] = varNode1;

    m_treeNodes.push_back(bnotNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    bnotNode->func(bnotNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "!5";

    var1->value = 0.0;
    bnotNode->func(bnotNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "!0";

    var1->value = 0.00000000001;
    bnotNode->func(bnotNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "!0.00000000001";

    var1->value = -1.0;
    bnotNode->func(bnotNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "!-1";
}

TEST_F(TreeFunctions, EqualOperator)
{
    // Expression: "x == y" or "_equal(x, y)" or "equal(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* equalNode = CreateEmptyNode(2);
    equalNode->func = prjm_eval_func_equal;
    equalNode->args[0] = varNode1;
    equalNode->args[1] = varNode2;

    m_treeNodes.push_back(equalNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    equalNode->func(equalNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 == 2";

    var1->value = 0.0;
    var2->value = 0.0;
    equalNode->func(equalNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0 == 0";

    var1->value = 0.000000000000001;
    var2->value = 0.000000000000001;
    equalNode->func(equalNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0.000000000000001 == 0.000000000000001";

    var1->value = -0.0;
    var2->value = +0.0;
    equalNode->func(equalNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-0 == 0";
}

TEST_F(TreeFunctions, NotEqualOperator)
{
    // Expression: "x != y" or "_noteq(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* notEqualNode = CreateEmptyNode(2);
    notEqualNode->func = prjm_eval_func_notequal;
    notEqualNode->args[0] = varNode1;
    notEqualNode->args[1] = varNode2;

    m_treeNodes.push_back(notEqualNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    notEqualNode->func(notEqualNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 != 2";

    var1->value = 0.0;
    var2->value = 0.0;
    notEqualNode->func(notEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 != 0";

    var1->value = 0.000000000000001;
    var2->value = 0.000000000000001;
    notEqualNode->func(notEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0.000000000000001 != 0.000000000000001";

    var1->value = -0.0;
    var2->value = +0.0;
    notEqualNode->func(notEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "-0 != 0";
}

TEST_F(TreeFunctions, BelowOperator)
{
    // Expression: "x < y" or "_below(x, y)" or "below(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* belowNode = CreateEmptyNode(2);
    belowNode->func = prjm_eval_func_below;
    belowNode->args[0] = varNode1;
    belowNode->args[1] = varNode2;

    m_treeNodes.push_back(belowNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    belowNode->func(belowNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 < 2";

    var1->value = 2.0;
    var2->value = 5.0;
    belowNode->func(belowNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "2 < 5";

    var1->value = 0.000000000000001;
    var2->value = 0.000000000000001;
    belowNode->func(belowNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0.000000000000001 < 0.000000000000001";

    var1->value = 0.0000000000000001;
    var2->value = 0.000000000000001;
    belowNode->func(belowNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0.0000000000000001 < 0.000000000000001";

    var1->value = -0.0;
    var2->value = +0.0;
    belowNode->func(belowNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "-0 < 0";

    var1->value = -1.0;
    var2->value = 1.0;
    belowNode->func(belowNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-1 < 1";
}

TEST_F(TreeFunctions, AboveOperator)
{
    // Expression: "x > y" or "_above(x, y)" or "above(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* aboveNode = CreateEmptyNode(2);
    aboveNode->func = prjm_eval_func_above;
    aboveNode->args[0] = varNode1;
    aboveNode->args[1] = varNode2;

    m_treeNodes.push_back(aboveNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    aboveNode->func(aboveNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 > 2";

    var1->value = 2.0;
    var2->value = 5.0;
    aboveNode->func(aboveNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "2 > 5";

    var1->value = 0.000000000000001;
    var2->value = 0.000000000000001;
    aboveNode->func(aboveNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0.000000000000001 > 0.000000000000001";

    var1->value = 0.000000000000001;
    var2->value = 0.0000000000000001;
    aboveNode->func(aboveNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0.000000000000001 > 0.0000000000000001";

    var1->value = -0.0;
    var2->value = +0.0;
    aboveNode->func(aboveNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "-0 > 0";

    var1->value = 1.0;
    var2->value = -1.0;
    aboveNode->func(aboveNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "1 > -1";
}

TEST_F(TreeFunctions, BelowOrEqualOperator)
{
    // Expression: "x <= y" or "_beleq(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* belowOrEqualNode = CreateEmptyNode(2);
    belowOrEqualNode->func = prjm_eval_func_beloweq;
    belowOrEqualNode->args[0] = varNode1;
    belowOrEqualNode->args[1] = varNode2;

    m_treeNodes.push_back(belowOrEqualNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    belowOrEqualNode->func(belowOrEqualNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 <= 2";

    var1->value = 2.0;
    var2->value = 5.0;
    belowOrEqualNode->func(belowOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "2 <= 5";

    var1->value = 0.000000000000001;
    var2->value = 0.000000000000001;
    belowOrEqualNode->func(belowOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0.000000000000001 <= 0.000000000000001";

    var1->value = 0.0000000000000001;
    var2->value = 0.000000000000001;
    belowOrEqualNode->func(belowOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0.0000000000000001 <= 0.000000000000001";

    var1->value = -0.0;
    var2->value = +0.0;
    belowOrEqualNode->func(belowOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-0 <= 0";

    var1->value = -1.0;
    var2->value = 1.0;
    belowOrEqualNode->func(belowOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-1 <= 1";
}

TEST_F(TreeFunctions, AboveOrEqualOperator)
{
    // Expression: "x >= y" or "_aboeq(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* aboveOrEqualNode = CreateEmptyNode(2);
    aboveOrEqualNode->func = prjm_eval_func_aboveeq;
    aboveOrEqualNode->args[0] = varNode1;
    aboveOrEqualNode->args[1] = varNode2;

    m_treeNodes.push_back(aboveOrEqualNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    aboveOrEqualNode->func(aboveOrEqualNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 >= 2";

    var1->value = 2.0;
    var2->value = 5.0;
    aboveOrEqualNode->func(aboveOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "2 >= 5";

    var1->value = 0.000000000000001;
    var2->value = 0.000000000000001;
    aboveOrEqualNode->func(aboveOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0.000000000000001 >= 0.000000000000001";

    var1->value = 0.000000000000001;
    var2->value = 0.0000000000000001;
    aboveOrEqualNode->func(aboveOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0.000000000000001 >= 0.0000000000000001";

    var1->value = -0.0;
    var2->value = +0.0;
    aboveOrEqualNode->func(aboveOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-0 >= 0";

    var1->value = 1.0;
    var2->value = -1.0;
    aboveOrEqualNode->func(aboveOrEqualNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "1 >= -1";
}

TEST_F(TreeFunctions, AdditionOperator)
{
    // Expression: "x + y" or "_add(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* addNode = CreateEmptyNode(2);
    addNode->func = prjm_eval_func_add;
    addNode->args[0] = varNode1;
    addNode->args[1] = varNode2;

    m_treeNodes.push_back(addNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    addNode->func(addNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 7.0) << "5 + 2";

    var1->value = -1.0;
    var2->value = 1.0;
    addNode->func(addNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "-1 + 1";

    var1->value = 5.0;
    var2->value = -5.0;
    addNode->func(addNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 + -5";
}

TEST_F(TreeFunctions, SubtractionOperator)
{
    // Expression: "x - y" or "_sub(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* subtractNode = CreateEmptyNode(2);
    subtractNode->func = prjm_eval_func_sub;
    subtractNode->args[0] = varNode1;
    subtractNode->args[1] = varNode2;

    m_treeNodes.push_back(subtractNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    subtractNode->func(subtractNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 3.0) << "5 - 2";

    var1->value = -1.0;
    var2->value = 1.0;
    subtractNode->func(subtractNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -2.0) << "-1 - 1";

    var1->value = 5.0;
    var2->value = -5.0;
    subtractNode->func(subtractNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 10.0) << "5 - -5";
}

TEST_F(TreeFunctions, MultiplicationOperator)
{
    // Expression: "x * y" or "_mul(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* multiplyNode = CreateEmptyNode(2);
    multiplyNode->func = prjm_eval_func_mul;
    multiplyNode->args[0] = varNode1;
    multiplyNode->args[1] = varNode2;

    m_treeNodes.push_back(multiplyNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    multiplyNode->func(multiplyNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 10.0) << "5 * 2";

    var1->value = -1.0;
    var2->value = 1.0;
    multiplyNode->func(multiplyNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "-1 * 1";

    var1->value = 5.0;
    var2->value = -5.0;
    multiplyNode->func(multiplyNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -25.0) << "5 * -5";
}

TEST_F(TreeFunctions, DivisionOperator)
{
    // Expression: "x / y" or "_div(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* divideNode = CreateEmptyNode(2);
    divideNode->func = prjm_eval_func_div;
    divideNode->args[0] = varNode1;
    divideNode->args[1] = varNode2;

    m_treeNodes.push_back(divideNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    divideNode->func(divideNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 2.5) << "5 / 2";

    var1->value = 0.0;
    var2->value = 5.0;
    divideNode->func(divideNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 / 5";

    // Division by 0 should return 0, not NaN
    var1->value = 5.0;
    var2->value = 0.0;
    divideNode->func(divideNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 / 0 - expected 0.0, not NaN";
}

TEST_F(TreeFunctions, ModuloOperator)
{
    // Expression: "x % y" or "_mod(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* moduloNode = CreateEmptyNode(2);
    moduloNode->func = prjm_eval_func_mod;
    moduloNode->args[0] = varNode1;
    moduloNode->args[1] = varNode2;

    m_treeNodes.push_back(moduloNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    moduloNode->func(moduloNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 % 2";

    var1->value = 0.0;
    var2->value = 5.0;
    moduloNode->func(moduloNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 % 5";

    var1->value = 5.0;
    var2->value = 5.0;
    moduloNode->func(moduloNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 % 5";

    var1->value = 5.0;
    var2->value = 1.9;
    moduloNode->func(moduloNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 % 1.9 (is 5 % 1)";
}

TEST_F(TreeFunctions, LogicalAndOperator)
{
    // Expression: "x && y" or "_and(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* logicalAndNode = CreateEmptyNode(2);
    logicalAndNode->func = prjm_eval_func_boolean_and_op;
    logicalAndNode->args[0] = varNode1;
    logicalAndNode->args[1] = varNode2;

    m_treeNodes.push_back(logicalAndNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    logicalAndNode->func(logicalAndNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 && 2";

    var1->value = -5.0;
    var2->value = 5.0;
    logicalAndNode->func(logicalAndNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-5 && 5";

    var1->value = 0.0;
    var2->value = 5.0;
    logicalAndNode->func(logicalAndNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 && 5";

    var1->value = 5.0;
    var2->value = 0.0;
    logicalAndNode->func(logicalAndNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 && 0";
}

TEST_F(TreeFunctions, LogicalAndOperatorSideEffect)
{
    // The && operator only executes the right operand if the left evaluates to a non-zero value.
    // Expression: "x && (y = 2)" or "_and(x, y = 2)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 5., &var2);

    auto* constNode1 = CreateConstantNode(2.0);

    auto* setNode = CreateEmptyNode(2);
    setNode->func = prjm_eval_func_set;
    setNode->args[0] = varNode2;
    setNode->args[1] = constNode1;

    auto* logicalAndNode = CreateEmptyNode(2);
    logicalAndNode->func = prjm_eval_func_boolean_and_op;
    logicalAndNode->args[0] = varNode1;
    logicalAndNode->args[1] = setNode;

    m_treeNodes.push_back(logicalAndNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    logicalAndNode->func(logicalAndNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 && 2";
    EXPECT_PRJM_F_EQ(var2->value, 2.0);

    var1->value = 0.0;
    var2->value = 5.0;
    logicalAndNode->func(logicalAndNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 && 5";
    EXPECT_PRJM_F_EQ(var2->value, 5.0);
}

TEST_F(TreeFunctions, LogicalOrOperator)
{
    // Expression: "x || y" or "_or(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* logicalOrNode = CreateEmptyNode(2);
    logicalOrNode->func = prjm_eval_func_boolean_or_op;
    logicalOrNode->args[0] = varNode1;
    logicalOrNode->args[1] = varNode2;

    m_treeNodes.push_back(logicalOrNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    logicalOrNode->func(logicalOrNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 || 2";

    var1->value = -5.0;
    var2->value = 5.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-5 || 5";

    var1->value = 0.0;
    var2->value = 5.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0 || 5";

    var1->value = 5.0;
    var2->value = 0.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 || 0";

    var1->value = 0.0;
    var2->value = 0.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 || 0";
}

TEST_F(TreeFunctions, LogicalOrOperatorSideEffect)
{
    // The || operator only executes the right operand if the left evaluates to zero.
    // Expression: "x || (y = 2)" or "_or(x, y = 2)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 5., &var2);

    auto* constNode1 = CreateConstantNode(2.0);

    auto* setNode = CreateEmptyNode(2);
    setNode->func = prjm_eval_func_set;
    setNode->args[0] = varNode2;
    setNode->args[1] = constNode1;

    auto* logicalOrNode = CreateEmptyNode(2);
    logicalOrNode->func = prjm_eval_func_boolean_or_op;
    logicalOrNode->args[0] = varNode1;
    logicalOrNode->args[1] = setNode;

    m_treeNodes.push_back(logicalOrNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    logicalOrNode->func(logicalOrNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 || 2";
    EXPECT_PRJM_F_EQ(var2->value, 5.0);

    var1->value = 0.0;
    var2->value = 5.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0 || 5";
    EXPECT_PRJM_F_EQ(var2->value, 2.0);
}

TEST_F(TreeFunctions, LogicalAndFunction)
{
    // Expression: "band(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* logicalAndNode = CreateEmptyNode(2);
    logicalAndNode->func = prjm_eval_func_boolean_and_func;
    logicalAndNode->args[0] = varNode1;
    logicalAndNode->args[1] = varNode2;

    m_treeNodes.push_back(logicalAndNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    logicalAndNode->func(logicalAndNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 && 2";

    var1->value = -5.0;
    var2->value = 5.0;
    logicalAndNode->func(logicalAndNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-5 && 5";

    var1->value = 0.0;
    var2->value = 5.0;
    logicalAndNode->func(logicalAndNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 && 5";

    var1->value = 5.0;
    var2->value = 0.0;
    logicalAndNode->func(logicalAndNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "5 && 0";
}

TEST_F(TreeFunctions, LogicalOrFunction)
{
    // Expression: "bor(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* logicalOrNode = CreateEmptyNode(2);
    logicalOrNode->func = prjm_eval_func_boolean_or_func;
    logicalOrNode->args[0] = varNode1;
    logicalOrNode->args[1] = varNode2;

    m_treeNodes.push_back(logicalOrNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    logicalOrNode->func(logicalOrNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 || 2";

    var1->value = -5.0;
    var2->value = 5.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "-5 || 5";

    var1->value = 0.0;
    var2->value = 5.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "0 || 5";

    var1->value = 5.0;
    var2->value = 0.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 || 0";

    var1->value = 0.0;
    var2->value = 0.0;
    logicalOrNode->func(logicalOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 || 0";
}

TEST_F(TreeFunctions, NegationOperator)
{
    // Expression: "-x" or "_neg(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);

    auto* negNode = CreateEmptyNode(1);
    negNode->func = prjm_eval_func_neg;
    negNode->args[0] = varNode1;

    m_treeNodes.push_back(negNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    negNode->func(negNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, -5.0) << "-(5)";

    var1->value = -5.0;
    negNode->func(negNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 5.0) << "-(-5)";

    var1->value = 0.0;
    negNode->func(negNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "-(0)";
}

TEST_F(TreeFunctions, AdditionAssignmentOperator)
{
    // Expression: "x += y" or "_addop(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* addopNode = CreateEmptyNode(2);
    addopNode->func = prjm_eval_func_add_op;
    addopNode->args[0] = varNode1;
    addopNode->args[1] = varNode2;

    m_treeNodes.push_back(addopNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    addopNode->func(addopNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 7.0) << "5 += 2";
    EXPECT_PRJM_F_EQ(var1->value, 7.0);

    var1->value = 0.0;
    var2->value = -5.0;
    addopNode->func(addopNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -5.0) << "0 += -5";
    EXPECT_PRJM_F_EQ(var1->value, -5.0);
}

TEST_F(TreeFunctions, SubtractionAssignmentOperator)
{
    // Expression: "x -= y" or "_subop(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* subopNode = CreateEmptyNode(2);
    subopNode->func = prjm_eval_func_sub_op;
    subopNode->args[0] = varNode1;
    subopNode->args[1] = varNode2;

    m_treeNodes.push_back(subopNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    subopNode->func(subopNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 3.0) << "5 -= 2";
    EXPECT_PRJM_F_EQ(var1->value, 3.0);

    var1->value = 0.0;
    var2->value = -5.0;
    subopNode->func(subopNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 5.0) << "0 -= -5";
    EXPECT_PRJM_F_EQ(var1->value, 5.0);
}

TEST_F(TreeFunctions, MultiplicationAssignmentOperator)
{
    // Expression: "x *= y" or "_mulop(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* mulopNode = CreateEmptyNode(2);
    mulopNode->func = prjm_eval_func_mul_op;
    mulopNode->args[0] = varNode1;
    mulopNode->args[1] = varNode2;

    m_treeNodes.push_back(mulopNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    mulopNode->func(mulopNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 10.0) << "5 *= 2";
    EXPECT_PRJM_F_EQ(var1->value, 10.0);

    var1->value = 2.0;
    var2->value = -5.0;
    mulopNode->func(mulopNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -10.0) << "2 *= -5";
    EXPECT_PRJM_F_EQ(var1->value, -10.0);
}

TEST_F(TreeFunctions, DivisionAssignmentOperator)
{
    // Expression: "x /= y" or "_divop(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* divopNode = CreateEmptyNode(2);
    divopNode->func = prjm_eval_func_div_op;
    divopNode->args[0] = varNode1;
    divopNode->args[1] = varNode2;

    m_treeNodes.push_back(divopNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    divopNode->func(divopNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 2.5) << "5 /= 2";
    EXPECT_PRJM_F_EQ(var1->value, 2.5);

    var1->value = 2.0;
    var2->value = -5.0;
    divopNode->func(divopNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -0.4) << "2 /= -5";
    EXPECT_PRJM_F_EQ(var1->value, -0.4);

    var1->value = 2.0;
    var2->value = 0.0;
    divopNode->func(divopNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "2 /= 0 (not expecting NaN)";
    EXPECT_PRJM_F_EQ(var1->value, 0.0);
}

TEST_F(TreeFunctions, BitewiseOrAssignmentOperator)
{
    // Expression: "x |= y" or "_orop(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 2., &var1);
    auto* varNode2 = CreateVariableNode("y", 7., &var2);

    auto* bitwiseOrOpNode = CreateEmptyNode(2);
    bitwiseOrOpNode->func = prjm_eval_func_bitwise_or_op;
    bitwiseOrOpNode->args[0] = varNode1;
    bitwiseOrOpNode->args[1] = varNode2;

    m_treeNodes.push_back(bitwiseOrOpNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    bitwiseOrOpNode->func(bitwiseOrOpNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 7.0) << "2 |= 7";
    EXPECT_PRJM_F_EQ(var1->value, 7.0);

    var1->value = -1.0; // Sets all bits to 1
    var2->value = 2.0;
    bitwiseOrOpNode->func(bitwiseOrOpNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "-1 |= 2";
    EXPECT_PRJM_F_EQ(var1->value, -1.0);
}

TEST_F(TreeFunctions, BitewiseOrOperator)
{
    // Expression: "x | y"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 2., &var1);
    auto* varNode2 = CreateVariableNode("y", 7., &var2);

    auto* bitwiseOrNode = CreateEmptyNode(2);
    bitwiseOrNode->func = prjm_eval_func_bitwise_or;
    bitwiseOrNode->args[0] = varNode1;
    bitwiseOrNode->args[1] = varNode2;

    m_treeNodes.push_back(bitwiseOrNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    bitwiseOrNode->func(bitwiseOrNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 7.0) << "2 | 7";

    var1->value = -1.0; // Sets all bits to 1
    var2->value = 2.0;
    bitwiseOrNode->func(bitwiseOrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "-1 | 2";
}

TEST_F(TreeFunctions, BitewiseAndAssignmentOperator)
{
    // Expression: "x &= y" or "_andop(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 2., &var1);
    auto* varNode2 = CreateVariableNode("y", 7., &var2);

    auto* bitwiseAndOpNode = CreateEmptyNode(2);
    bitwiseAndOpNode->func = prjm_eval_func_bitwise_and_op;
    bitwiseAndOpNode->args[0] = varNode1;
    bitwiseAndOpNode->args[1] = varNode2;

    m_treeNodes.push_back(bitwiseAndOpNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    bitwiseAndOpNode->func(bitwiseAndOpNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "2 &= 7";
    EXPECT_PRJM_F_EQ(var1->value, 2.0);

    var1->value = -1.0; // Sets all bits to 1
    var2->value = 2.0;
    bitwiseAndOpNode->func(bitwiseAndOpNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "-1 &= 2";
    EXPECT_PRJM_F_EQ(var1->value, 2.0);
}

TEST_F(TreeFunctions, BitewiseAndOperator)
{
    // Expression: "x & y"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 2., &var1);
    auto* varNode2 = CreateVariableNode("y", 7., &var2);

    auto* bitwiseAndNode = CreateEmptyNode(2);
    bitwiseAndNode->func = prjm_eval_func_bitwise_and;
    bitwiseAndNode->args[0] = varNode1;
    bitwiseAndNode->args[1] = varNode2;

    m_treeNodes.push_back(bitwiseAndNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    bitwiseAndNode->func(bitwiseAndNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "2 & 7";

    var1->value = -1.0; // Sets all bits to 1
    var2->value = 2.0;
    bitwiseAndNode->func(bitwiseAndNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "-1 & 2";
}

TEST_F(TreeFunctions, ModuloAssignmentOperator)
{
    // Expression: "x %= y" or "_modop(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* modopNode = CreateEmptyNode(2);
    modopNode->func = prjm_eval_func_mod_op;
    modopNode->args[0] = varNode1;
    modopNode->args[1] = varNode2;

    m_treeNodes.push_back(modopNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    modopNode->func(modopNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "5 %= 2";
    EXPECT_PRJM_F_EQ(var1->value, 1.0);

    var1->value = 2.0;
    var2->value = -5.0;
    modopNode->func(modopNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "2 %= -5";
    EXPECT_PRJM_F_EQ(var1->value, 2.0);
}

TEST_F(TreeFunctions, PowerAssignmentOperator)
{
    // Expression: "x ^= y" or "_powop(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* powopNode = CreateEmptyNode(2);
    powopNode->func = prjm_eval_func_pow_op;
    powopNode->args[0] = varNode1;
    powopNode->args[1] = varNode2;

    m_treeNodes.push_back(powopNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    powopNode->func(powopNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 25.0) << "5 ^= 2";
    EXPECT_PRJM_F_EQ(var1->value, 25.0);

    var1->value = 2.0;
    var2->value = -5.0;
    powopNode->func(powopNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.03125) << "2 ^= -5";
    EXPECT_PRJM_F_EQ(var1->value, 0.03125);

    // Special case: not expecting NaN
    var1->value = 0.0;
    var2->value = -5.0;
    powopNode->func(powopNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 ^= -5 (not expecting NaN)";
    EXPECT_PRJM_F_EQ(var1->value, 0.0);
}

TEST_F(TreeFunctions, SineFunction)
{
    // Expression: "sin(x)"
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* sinNode = CreateEmptyNode(1);
    sinNode->func = prjm_eval_func_sin;
    sinNode->args[0] = varNode;

    m_treeNodes.push_back(sinNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    sinNode->func(sinNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "sin(0.0)";

    var->value = M_PI_2;
    sinNode->func(sinNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "sin($PI/2)";

    var->value = -M_PI_2;
    sinNode->func(sinNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "sin(-$PI/2)";
}

TEST_F(TreeFunctions, CosineFunction)
{
    // Expression: "cos(x)"
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* cosNode = CreateEmptyNode(1);
    cosNode->func = prjm_eval_func_cos;
    cosNode->args[0] = varNode;

    m_treeNodes.push_back(cosNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    cosNode->func(cosNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "cos(0.0)";

    // Results for cos(PI/2) are close to 0, but too far for GTest to see as equal (~6e-17)
    var->value = M_PI;
    cosNode->func(cosNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "cos($PI/2)";

    var->value = -M_PI;
    cosNode->func(cosNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "cos(-$PI/2)";
}

TEST_F(TreeFunctions, TangentFunction)
{
    // Expression: "tan(x)"
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* tanNode = CreateEmptyNode(1);
    tanNode->func = prjm_eval_func_tan;
    tanNode->args[0] = varNode;

    m_treeNodes.push_back(tanNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    tanNode->func(tanNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "tan(0.0)";

    var->value = M_PI_4;
    tanNode->func(tanNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "tan($PI/4)";

    var->value = -M_PI_4;
    tanNode->func(tanNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "tan(-$PI/4)";
}

TEST_F(TreeFunctions, ArcSineFunction)
{
    // Expression: "asin(x)"
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* asinNode = CreateEmptyNode(1);
    asinNode->func = prjm_eval_func_asin;
    asinNode->args[0] = varNode;

    m_treeNodes.push_back(asinNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    asinNode->func(asinNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "asin(0.0)";

    var->value = 1.0;
    asinNode->func(asinNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, M_PI_2) << "asin(1.0)";

    var->value = -1.0;
    asinNode->func(asinNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -M_PI_2) << "asin(-1.0)";

    var->value = 2.0;
    asinNode->func(asinNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "asin(2.0) - expected 0.0, not NaN";

    var->value = -2.0;
    asinNode->func(asinNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "asin(-2.0) - expected 0.0, not NaN";
}

TEST_F(TreeFunctions, ArcCosineFunction)
{
    // Expression: "acos(x)"
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* acosNode = CreateEmptyNode(1);
    acosNode->func = prjm_eval_func_acos;
    acosNode->args[0] = varNode;

    m_treeNodes.push_back(acosNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    acosNode->func(acosNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, M_PI_2) << "acos(0.0)";

    var->value = 1.0;
    acosNode->func(acosNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "acos(1.0)";

    var->value = -1.0;
    acosNode->func(acosNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, M_PI) << "acos(-1.0)";

    var->value = 2.0;
    acosNode->func(acosNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0f) << "acos(2.0) - expected 0.0, not NaN";

    var->value = -2.0;
    acosNode->func(acosNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0f) << "acos(-2.0) - expected 0.0, not NaN";
}

TEST_F(TreeFunctions, ArcTangentFunction)
{
    // Expression: "atan(x)"
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* atanNode = CreateEmptyNode(1);
    atanNode->func = prjm_eval_func_atan;
    atanNode->args[0] = varNode;

    m_treeNodes.push_back(atanNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    atanNode->func(atanNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "atan(0)";

    var->value = 1.0;
    atanNode->func(atanNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, M_PI_4) << "atan(1)";

    var->value = -1.0;
    atanNode->func(atanNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -M_PI_4) << "atan(-1)";

    var->value = 100000000000000000000000000.0;
    atanNode->func(atanNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, M_PI_2) << "atan(100000000000000000000000000)";

    var->value = -100000000000000000000000000.0;
    atanNode->func(atanNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -M_PI_2) << "atan(-100000000000000000000000000)";
}

TEST_F(TreeFunctions, ArcTangent2Function)
{
    // Expression: "atan2(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 0.0, &var1);
    auto* varNode2 = CreateVariableNode("y", 1.0, &var2);

    auto* atan2Node = CreateEmptyNode(2);
    atan2Node->func = prjm_eval_func_atan2;
    atan2Node->args[0] = varNode1;
    atan2Node->args[1] = varNode2;

    m_treeNodes.push_back(atan2Node);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    atan2Node->func(atan2Node, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "atan2(0, 1)";

    var1->value = 1.0;
    atan2Node->func(atan2Node, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, M_PI_4) << "atan(1, 1)";

    var1->value = -1.0;
    atan2Node->func(atan2Node, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -M_PI_4) << "atan(-1, 1)";

    var1->value = 100000000000000000000000000.0;
    atan2Node->func(atan2Node, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, M_PI_2) << "atan(100000000000000000000000000, 1)";

    var1->value = -100000000000000000000000000.0;
    atan2Node->func(atan2Node, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -M_PI_2) << "atan(-100000000000000000000000000, 1)";
}

TEST_F(TreeFunctions, SquareRootFunction)
{
    // Expression: "sqrt(x)"
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* sqrtNode = CreateEmptyNode(1);
    sqrtNode->func = prjm_eval_func_sqrt;
    sqrtNode->args[0] = varNode;

    m_treeNodes.push_back(sqrtNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    sqrtNode->func(sqrtNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "sqrt(0)";

    var->value = 1.0;
    sqrtNode->func(sqrtNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "sqrt(1)";

    // This function uses the absolute value of the argument!
    var->value = -1.0;
    sqrtNode->func(sqrtNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "sqrt(-1)";

    var->value = 25.0;
    sqrtNode->func(sqrtNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 5) << "sqrt(25)";

    var->value = -25.0;
    sqrtNode->func(sqrtNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 5) << "sqrt(-25)";
}

TEST_F(TreeFunctions, PowerFunction)
{
    // Expression: "pow(x, y)" or "x ^ y"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5., &var1);
    auto* varNode2 = CreateVariableNode("y", 2., &var2);

    auto* powNode = CreateEmptyNode(2);
    powNode->func = prjm_eval_func_pow;
    powNode->args[0] = varNode1;
    powNode->args[1] = varNode2;

    m_treeNodes.push_back(powNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    powNode->func(powNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 25.0) << "5 ^ 2";

    var1->value = 2.0;
    var2->value = -5.0;
    powNode->func(powNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.03125) << "2 ^ -5";

    // Special case: not expecting NaN
    var1->value = 0.0;
    var2->value = -5.0;
    powNode->func(powNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "0 ^ -5 (not expecting NaN)";
}

TEST_F(TreeFunctions, ExponentialFunction)
{
    // Expression: "exp(x)" (equivalent to "pow($E, x)")
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 1., &var1);

    auto* expNode = CreateEmptyNode(1);
    expNode->func = prjm_eval_func_exp;
    expNode->args[0] = varNode1;

    m_treeNodes.push_back(expNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    expNode->func(expNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, M_E) << "exp(1)";

    var1->value = 2.0;
    expNode->func(expNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, M_E * M_E) << "exp(2)";
}

TEST_F(TreeFunctions, NaturalLogarithmFunction)
{
    // Expression: "log(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 0., &var1);

    auto* logNode = CreateEmptyNode(1);
    logNode->func = prjm_eval_func_log;
    logNode->args[0] = varNode1;

    m_treeNodes.push_back(logNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    logNode->func(logNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "log(0) (not expecting Inf)";

    var1->value = -1.0;
    logNode->func(logNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "log(-1) (not expecting NaN)";

    var1->value = 1.0;
    logNode->func(logNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "log(1)";

    var1->value = M_E;
    logNode->func(logNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "log($E)";

    var1->value = M_E * M_E;
    logNode->func(logNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "log($E * $E)";
}

TEST_F(TreeFunctions, Logarithm10Function)
{
    // Expression: "log10(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 0., &var1);

    auto* log10Node = CreateEmptyNode(1);
    log10Node->func = prjm_eval_func_log10;
    log10Node->args[0] = varNode1;

    m_treeNodes.push_back(log10Node);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    log10Node->func(log10Node, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "log10(0) (not expecting Inf)";

    var1->value = -1.0;
    log10Node->func(log10Node, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "log10(-1) (not expecting NaN)";

    var1->value = 1.0;
    log10Node->func(log10Node, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "log10(1)";

    var1->value = 10;
    log10Node->func(log10Node, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "log10(10)";

    var1->value = 100;
    log10Node->func(log10Node, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "log(100)";
}

TEST_F(TreeFunctions, FloorFunction)
{
    // Expression: "floor(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 0.5, &var1);

    auto* floorNode = CreateEmptyNode(1);
    floorNode->func = prjm_eval_func_floor;
    floorNode->args[0] = varNode1;

    m_treeNodes.push_back(floorNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    floorNode->func(floorNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "floor(0.5)";

    var1->value = -1.5;
    floorNode->func(floorNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -2.0) << "floor(-1.5)";

    var1->value = 0.0;
    floorNode->func(floorNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "floor(0)";

    var1->value = 1.9999999999;
    floorNode->func(floorNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "floor(1.9999999999)";
}

TEST_F(TreeFunctions, CeilingFunction)
{
    // Expression: "ceil(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 0.5, &var1);

    auto* ceilNode = CreateEmptyNode(1);
    ceilNode->func = prjm_eval_func_ceil;
    ceilNode->args[0] = varNode1;

    m_treeNodes.push_back(ceilNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    ceilNode->func(ceilNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "ceil(0.5)";

    var1->value = -1.5;
    ceilNode->func(ceilNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "ceil(-1.5)";

    var1->value = 0.0;
    ceilNode->func(ceilNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "ceil(0)";

    var1->value = 1.000000001;
    ceilNode->func(ceilNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "ceil(1.000000001)";
}

TEST_F(TreeFunctions, SigmoidalFunction)
{
    // Expression: "sigmoid(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 0.0, &var1);
    auto* varNode2 = CreateVariableNode("y", 1.0, &var2);

    auto* sigmoidalNode = CreateEmptyNode(2);
    sigmoidalNode->func = prjm_eval_func_sigmoid;
    sigmoidalNode->args[0] = varNode1;
    sigmoidalNode->args[1] = varNode2;

    m_treeNodes.push_back(sigmoidalNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    sigmoidalNode->func(sigmoidalNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.5) << "sigmoid(0, 1)";

    var1->value = 100000.0;
    var2->value = 1.0;
    sigmoidalNode->func(sigmoidalNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "sigmoid(100000, 1)";

    var1->value = -100000.0;
    var2->value = 1.0;
    sigmoidalNode->func(sigmoidalNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "sigmoid(-100000, 1)";
}

TEST_F(TreeFunctions, SquareFunction)
{
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* sqrNode = CreateEmptyNode(1);
    sqrNode->func = prjm_eval_func_sqr;
    sqrNode->args[0] = varNode;

    m_treeNodes.push_back(sqrNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    sqrNode->func(sqrNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "sqr(0.0)";

    var->value = 1.0;
    sqrNode->func(sqrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "sqr(1.0)";

    var->value = -1.0;
    sqrNode->func(sqrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "sqr(-1.0)";

    var->value = 2.0;
    sqrNode->func(sqrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 4.0) << "sqr(2.0)";

    var->value = -2.0;
    sqrNode->func(sqrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 4.0) << "sqr(-2.0)";

    var->value = 1000000.0;
    sqrNode->func(sqrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1000000000000.0) << "sqr(1000000.0)";

    var->value = -1000000.0;
    sqrNode->func(sqrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1000000000000.0) << "sqr(-1000000.0)";

    var->value = 9999999999999.0;
    sqrNode->func(sqrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 9.9999999999980007e+25) << "sqr(9999999999999.0)";

    // Overflow: should return Inf
#if PRJM_F_SIZE == 4
    // 10^4 below max value for a float
    var->value = 3.402823466E+34f;
#else
    // 10^8 below max value for a double
    var->value = 1.7976931348623157E+300;
#endif
    sqrNode->func(sqrNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, INFINITY) << "sqr(" << var->value << ")";
}

TEST_F(TreeFunctions, AbsoluteFunction)
{
    // Expression: "abs(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 0.0, &var1);

    auto* absNode = CreateEmptyNode(1);
    absNode->func = prjm_eval_func_abs;
    absNode->args[0] = varNode1;

    m_treeNodes.push_back(absNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    absNode->func(absNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "abs(0)";

    var1->value = -1.5;
    absNode->func(absNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.5) << "abs(-1.5)";

    var1->value = 1.5;
    absNode->func(absNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.5) << "abs(1.5)";
}

TEST_F(TreeFunctions, MinimumFunction)
{
    // Expression: "min(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5.0, &var1);
    auto* varNode2 = CreateVariableNode("y", 2.0, &var2);

    auto* minNode = CreateEmptyNode(2);
    minNode->func = prjm_eval_func_min;
    minNode->args[0] = varNode1;
    minNode->args[1] = varNode2;

    m_treeNodes.push_back(minNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    minNode->func(minNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "min(5, 2)";

    var1->value = 2.0;
    var2->value = 5.0;
    minNode->func(minNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 2.0) << "min(2, 5)";

    var1->value = -100000.0;
    var2->value = 100000.0;
    minNode->func(minNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -100000.0) << "min(-100000, 100000)";
}

TEST_F(TreeFunctions, MaximumFunction)
{
    // Expression: "max(x, y)"
    prjm_eval_variable_def_t* var1;
    prjm_eval_variable_def_t* var2;
    auto* varNode1 = CreateVariableNode("x", 5.0, &var1);
    auto* varNode2 = CreateVariableNode("y", 2.0, &var2);

    auto* maxNode = CreateEmptyNode(2);
    maxNode->func = prjm_eval_func_max;
    maxNode->args[0] = varNode1;
    maxNode->args[1] = varNode2;

    m_treeNodes.push_back(maxNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    maxNode->func(maxNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 5.0) << "max(5, 2)";

    var1->value = 2.0;
    var2->value = 5.0;
    maxNode->func(maxNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 5.0) << "max(2, 5)";

    var1->value = -100000.0;
    var2->value = 100000.0;
    maxNode->func(maxNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 100000.0) << "max(-100000, 100000)";
}

TEST_F(TreeFunctions, SignFunction)
{
    // Expression: "sign(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 0.0, &var1);

    auto* signNode = CreateEmptyNode(1);
    signNode->func = prjm_eval_func_sign;
    signNode->args[0] = varNode1;

    m_treeNodes.push_back(signNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    signNode->func(signNode, &valuePointer);

    EXPECT_PRJM_F_EQ(*valuePointer, 0.0) << "sign(0)";

    var1->value = -1.5;
    signNode->func(signNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -1.0) << "sign(-1.5)";

    var1->value = 1.5;
    signNode->func(signNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, 1.0) << "sign(1.5)";
}

TEST_F(TreeFunctions, RandFunction)
{
    prjm_eval_variable_def_t* var;
    auto* varNode = CreateVariableNode("x", 0.f, &var);

    auto* randNode = CreateEmptyNode(1);
    randNode->func = prjm_eval_func_rand;
    randNode->args[0] = varNode;

    m_treeNodes.push_back(randNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;

    for (int i = 0; i < 100; i++)
    {
        randNode->func(randNode, &valuePointer);
        EXPECT_LT(*valuePointer, 1.0) << "rand(0.0)";
    }

    auto testValue = [&var, &randNode, &valuePointer](PRJM_EVAL_F value)
    {
        var->value = value;
        bool oneValueOverThreshold = false;
        PRJM_EVAL_F threshold = value / 2.0;
        for (int i = 0; i < 100; i++)
        {
            randNode->func(randNode, &valuePointer);
            EXPECT_LT(*valuePointer, value) << "rand(" << value << ")";
            if (*valuePointer > threshold)
            {
                oneValueOverThreshold = true;
            }
        }
        EXPECT_TRUE(oneValueOverThreshold) << "One value must be over " << threshold;
    };

    testValue(100.0);
    testValue(10000.0);
    testValue(1000000.0);
    testValue(100000000.0);
    testValue(INT32_MAX);
    testValue(INT32_MAX * 1000000000000000.0);
}

TEST_F(TreeFunctions, InverseSquareRootFunction)
{
    // Expression: "invsqrt(x)"
    prjm_eval_variable_def_t* var1;
    auto* varNode1 = CreateVariableNode("x", 1.0, &var1);

    auto* invsqrtNode = CreateEmptyNode(1);
    invsqrtNode->func = prjm_eval_func_invsqrt;
    invsqrtNode->args[0] = varNode1;

    m_treeNodes.push_back(invsqrtNode);

    PRJM_EVAL_F value{};
    PRJM_EVAL_F* valuePointer = &value;
    invsqrtNode->func(invsqrtNode, &valuePointer);

    EXPECT_LE(abs(*valuePointer - 1.0 / sqrt(1.0)), 0.01) << "invsqrt(1)";

    var1->value = 25.0;
    invsqrtNode->func(invsqrtNode, &valuePointer);
    EXPECT_LE(abs(*valuePointer - 1.0 / sqrt(25.0)), 0.01) << "invsqrt(25)";

    var1->value = -1.0;
    invsqrtNode->func(invsqrtNode, &valuePointer);
    EXPECT_PRJM_F_EQ(*valuePointer, -INFINITY) << "invsqrt(-1.0)";
}
