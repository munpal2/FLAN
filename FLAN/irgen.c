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

static void tytree_destroy(tytree_node* root)
{
	if (root->next != NULL)
		tytree_destroy(root->next);
	free(root->next);
}

static bool tytree_eq(tytree_node* t1, tytree_node* t2)
{
	if (t1->type != t2->type)
		return false;
	
	for (size_t i = 0; i < 2; i++)
	{
		bool eq1 = (t1->child[i] == NULL);
		bool eq2 = (t2->child[i] == NULL);
		if (eq1 != eq2)
			return false;
		if (eq1 && !tytree_eq(t1->child[i], t2->child[i]))
			return false;
	}
	return true;
}

static inline void sym_destroy(symbol* sym)
{
	tytree_destroy(sym->type);
}

static inline void syt_push(symbol_table* syt)
{
	htb_create(varr_get(&(syt->varr), hash_table, (syt->depth)++), symbol);
}

static inline void syt_pop(symbol_table* syt)
{
	htb_destroy(varr_get(&(syt->varr), hash_table, (syt->depth)--));
}

void syt_create(symbol_table* syt)
{
	syt->depth = 0;
	syt->stack_bottom = 0;
	varr_create(&(syt->varr), hash_table, 16);
	syt_push(syt);
}


