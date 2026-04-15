#pragma once

#include "CompilerTypes.h"

/**
 * @brief Recursively frees the memory of the given node.
 * @param expr The node to free.
 */
void prjm_eval_destroy_exptreenode(prjm_eval_exptreenode_t* expr);
