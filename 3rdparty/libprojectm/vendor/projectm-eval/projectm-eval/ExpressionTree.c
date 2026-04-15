#include "ExpressionTree.h"

#include <stdlib.h>

void prjm_eval_destroy_exptreenode(prjm_eval_exptreenode_t* expr)
{
    if (!expr)
    {
        return;
    }

    /* Free arguments */
    if (expr->args)
    {
        prjm_eval_exptreenode_t** arg = expr->args;
        while (*arg)
        {
            prjm_eval_destroy_exptreenode(*arg);
            arg++;
        }

        free(expr->args);
    }

    /* Free execution list */
    if (expr->list)
    {
        prjm_eval_exptreenode_list_item_t* item = expr->list;
        while(item)
        {
            prjm_eval_exptreenode_list_item_t* free_item = item;
            item = item->next;
            prjm_eval_destroy_exptreenode(free_item->expr);
            free(free_item);
        }
    }

    free(expr);
}
