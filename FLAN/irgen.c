#include "irgen.h"

static tytree_node* tytreend_create(tytree_type type, size_t len)
{
	tytree_node* ret;
	if ((ret = calloc(sizeof(tytree_node), 1)) == NULL)
		return NULL;
	ret->type = type;
	ret->len = len;
	return ret;
}

static tytree_node* from_AST(AST_node* ast);
static tytree_node* func_from_AST(AST_node* funcnode, size_t pidx, size_t ridx)
{
	tytree_node* ret = tytreend_create(TYTR_FUNC, 0);
	if (funcnode->children[pidx] != NULL)
		ret->children[0] = from_AST(funcnode->children[pidx]);
	if (funcnode->children[ridx] != NULL)
		ret->children[1] = from_AST(funcnode->children[ridx]);
	return ret;
}

static tytree_node* from_AST(AST_node* ast)
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
		case AST_CONJ: //param이나 type_expr을 묶는 conj
		{
			if (ast->children[0] == NULL)
				return NULL;

			tytree_node* ret = tytreend_create(TYTR_PARAM, 0);
			AST_node* child_node = ast->children[0];
			if (child_node->type == AST_PARAM)
				ret->children[0] = from_AST(child_node->children[0]);
			else
				ret->children[0] = from_AST(child_node);

			if (ast->children[1] != NULL)
				ret->children[1] = from_AST(ast->children[1]);
			return ret;
		}
	}
}

static void tytree_destroy(tytree_node* root)
{
	for (size_t i = 0; i < 2; i++)
	{
		if (root->children[i] != NULL)
			tytree_destroy(root->children[i]);
	};
	free(root);
}

static bool tytree_eq(tytree_node* t1, tytree_node* t2)
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

static inline void sym_destroy(symbol* sym)
{
	tytree_destroy(sym->type);
}

static void sym_create(symbol* sym, addr_type atype, long long int addr)
{
	sym->addrtype = atype;
	sym->type = NULL;
	sym->addr = 0;
}

void syt_create(symbol_table* syt)
{
	syt->depth = 0;
	syt->stack_bottom = 0;
	syt->in_func_block = false;
	varr_create(&(syt->varr), hash_table, 16);
	syt_enter(syt);
}

symbol* syt_find(symbol_table* syt, const char* str)
{
	for (int i = syt->depth - 1; i > 0; i--)
	{
		hash_table* cur = varr_get(&(syt->varr), hash_table, i);
		symbol* find_value = htb_find(cur, str);
		if (find_value != NULL)
			return find_value;
	}
	return NULL;
}

void syt_insert(symbol_table* syt, AST_node* decltree)
{
	hash_table* cur = varr_get(&(syt->varr), hash_table, syt->depth - 1);
	AST_node* type = decltree->children[0];
	AST_node* id_list = decltree->children[1];
	long long int offset = 0; //ebp 기준 오프셋

	while (id_list->children[0] != NULL)
	{
		AST_node* id_node = id_list->children[0]->children[0];
		offset -= 8;
		sym_create(htb_insert(syt, id_node->attr), ADDR_REL, offset);
		id_list = id_list->children[1];
	}
}
