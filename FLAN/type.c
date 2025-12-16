#include "type.h"

const char* tytree_strty[] = { "TYTR_INT",   "TYTR_UINT",  "TYTR_CHAR",  "TYTR_BOOL",  "TYTR_FLOAT",
							   "TYTR_PTROF", "TYTR_ARROF", "TYTR_CONST", "TYTR_FUNC" };

tytree_node* tytreend_create(tytree_type type, size_t len)
{
	tytree_node* ret = calloc(sizeof(tytree_node), 1);
	if (ret == NULL)
		return NULL;
	ret->type = type;
	ret->len = len;
	return ret;
}

tytree_node* func_from_AST(AST_node* funcnode, size_t pidx, size_t ridx)
{
	tytree_node* ret = tytreend_create(TYTR_FUNC, 0);
	tytree_node** param_push_pos = &(ret->children[0]);
	AST_node* param_head = funcnode->children[pidx];

	while (param_head != NULL)
	{
		if (param_head->type == AST_PARAM)
			*param_push_pos = from_AST(param_head->children[0]);
		else
			*param_push_pos = from_AST(param_head);

		param_push_pos = &((*param_push_pos)->next);
		param_head = param_head->next;
	}

	if (funcnode->children[ridx] != NULL)
		ret->children[1] = from_AST(funcnode->children[ridx]);
	return ret;
}

tytree_node* from_AST(AST_node* ast)
{
	switch (ast->type)
	{
	case AST_TYINT: return tytreend_create(TYTR_INT, 0);
	case AST_TYUINT: return tytreend_create(TYTR_UINT, 0);
	case AST_TYBOOL: return tytreend_create(TYTR_BOOL, 0);
	case AST_TYCHAR: return tytreend_create(TYTR_CHAR, 0);
	case AST_TYFLOAT: return tytreend_create(TYTR_FLOAT, 0);
	case AST_PTROF:
	{
		tytree_node* ret = tytreend_create(TYTR_PTROF, 0);
		ret->children[0] = from_AST(ast->children[0]);
		return ret;
	}
	case AST_CONST:
	{
		tytree_node* ret = tytreend_create(TYTR_CONST, 0);
		ret->children[0] = from_AST(ast->children[0]);
		return ret;
	}
	case AST_ARR:
	{
		tytree_node* ret = tytreend_create(TYTR_ARROF, atoi(ast->children[1]->attr));
		ret->children[0] = from_AST(ast->children[0]);
		return ret;
	}
	case AST_FDECL: return func_from_AST(ast, 1, 2);
	case AST_FUNC: return func_from_AST(ast, 0, 1);
	}
}

tytree_node* tytree_copy(tytree_node* root)
{
	tytree_node* ret = tytreend_create(root->type, root->len);
	for (size_t i = 0; i < 2; i++)
	{
		if (root->children[i] != NULL)
			ret->children[i] = tytree_copy(root->children[i]);
	}
	if (root->next != NULL)
		ret->next = tytree_copy(root->next);
	return ret;
}

void tytree_destroy(tytree_node* root)
{
	for (size_t i = 0; i < 2; i++)
	{
		if (root->children[i] != NULL)
			tytree_destroy(root->children[i]);
	};
	if (root->next != NULL)
		tytree_destroy(root->next);
	free(root);
}

bool tytree_eq(tytree_node* t1, tytree_node* t2)
{
	if ((t1->type != t2->type) || (t1->len != t2->len))
		return false;

	for (size_t i = 0; i < 2; i++)
	{
		bool eq1 = (t1->children[i] == NULL);
		bool eq2 = (t2->children[i] == NULL);
		if (eq1 != eq2)
			return false;
		if (eq1 && !tytree_eq(t1->children[i], t2->children[i]))
			return false;
	}
	return true;
}

addr_t size[] = { 8, 8, 1, 1, 8, 8 };
addr_t tytree_sizeof(tytree_node* root)
{
	switch (root->type)
	{
	case TYTR_ARROF:
		return tytree_sizeof(root->children[0]) * root->len;
	case TYTR_CONST:
		return tytree_sizeof(root->children[0]);
	case TYTR_FUNC:
		return -1;
	default:
		return size[root->type];
	}
}

tytree_node* literal_type[6];
void lit_types_init()
{
	literal_type[LTT_INT] = tytreend_create(TYTR_INT, 0);
	literal_type[LTT_UINT] = tytreend_create(TYTR_UINT, 0);
	literal_type[LTT_CHAR] = tytreend_create(TYTR_CHAR, 0);
	literal_type[LTT_BOOL] = tytreend_create(TYTR_BOOL, 0);
	literal_type[LTT_FLOAT] = tytreend_create(TYTR_FLOAT, 0);
	literal_type[LTT_STR] = tytreend_create(TYTR_PTROF, 0);
	literal_type[LTT_STR]->children[0] = tytreend_create(TYTR_CONST, 0);
	literal_type[LTT_STR]->children[0]->children[0] = tytreend_create(TYTR_CHAR, 0);
}

void lit_types_destroy()
{
	for (size_t i = 0; i < 6; i++)
		tytree_destroy(literal_type[i]);
}

bool tytree_is_nearint(tytree_node* type)
{
	type = tytree_get_base_type(type);
	switch (type->type)
	{
	case TYTR_INT:
	case TYTR_UINT:
	case TYTR_CHAR:
	case TYTR_BOOL:
	case TYTR_PTROF:
	case TYTR_ARROF:
		return true;
	case TYTR_FLOAT:
		return false;
	}
}