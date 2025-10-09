#include "sementic.h"

static void move_child(AST_node* to, AST_node* from)
{
	for (size_t i = 0; i < 4; i++)
	{
		to->children[i] = from->children[i];
		from->children[i] = NULL;
	}
}

/*
LTE -> NOT + GT
GTE -> NOT + LT
NEQ -> NOT + EQ
*/
static AST_node* unroll_to_not_node(AST_node* target)
{
	AST_node* not_node = AST_node_create(AST_NOT, NULL, target->col);
	AST_type outcome;
	switch (target->type)
	{
	case AST_LTE:
		outcome = AST_GT;
		break;
	case AST_GTE:
		outcome = AST_LT;
		break;
	case AST_NEQ:
		outcome = AST_EQ;
		break;
	}
	AST_node* outcome_node = AST_node_create(outcome, NULL, target->col);
	move_child(outcome_node, target);
	not_node->children[0] = outcome_node;
	AST_node_destroy(target);
	return not_node;
}

void optimize_AST(AST_node* root)
{
	for (size_t i = 0; i < 4; i++)
	{
		AST_node* node = root->children[i];
		if (node == NULL)
			continue;

		switch (node->type)
		{
			case AST_LTE: case AST_GTE: case AST_NEQ:
				root->children[i] = (node = unroll_to_not_node(node));
				break;
		}
		optimize_AST(node);
	}
}
