#include "test.h"

const char* idx_str[] = { "[0] ", "[1] ", "[2] ", "[3] " };
static void tytree_show(tytree_node* node, size_t depth, const char* idx)
{
	for (size_t i = 0; i < depth; i++)
		printf("  ");

	printf("%s%s", idx, tytree_strty[node->type]);
	if (node->len != 0)
		printf("(%llu)", node->len);
	puts(" {");

	for (size_t i = 0; i < 2; i++)
	{
		if (node->children[i] != NULL)
			tytree_show(node->children[i], depth + 1, idx_str[i]);
	}

	for (size_t i = 0; i < depth; i++)
		printf("  ");
	puts("}");

	if (node->next != NULL)
		tytree_show(node->next, depth, "[+] ");
}

static void symbol_show(symbol* sym)
{
	if (sym->addrtype == ADDR_REL)
		printf("\nREL(");
	else
		printf("\nABS(");
	printf("%lld)\n", sym->addr);

	printf("type: \n");
	tytree_show(sym->type, 1, "[T] ");
}

static void syt_show(symbol_table* syt)
{
	for (size_t i = 0; i < syt->depth; i++)
	{
		htb_foreach(varr_get((&syt->varr), hash_table, i), symbol_show);
	}
}

static void token_show(variable_arr* tokens)
{
	for (size_t i = 0; varr_get(tokens, token, i)->type != TK_END; i++)
	{
		token* elem = varr_get(tokens, token, i);
		printf("Line %u : <%s, %s>\n", elem->col, elem->attr, token_strty[elem->type]);
	}
}

static void node_show(AST_node* node, size_t depth, const char* idx)
{
	for (size_t i = 0; i < depth; i++)
		printf("  ");

	printf("%s%s", idx, AST_strty[node->type]);
	if (node->attr != NULL)
		printf(": %s", node->attr);
	puts(" {");

	for (size_t i = 0; i < 4; i++)
	{
		if (node->children[i] != NULL)
			node_show(node->children[i], depth + 1, idx_str[i]);
	}

	for (size_t i = 0; i < depth; i++)
		printf("  ");
	puts("}");

	if (node->next != NULL)
		node_show(node->next, depth, "[+] ");
}

void test_file(const char* filename, unsigned int flag) //토크나이저 테스트하깅
{
	tokenizer tknz;
	tknz_init(&tknz, filename);
	variable_arr* tknzed = tokenize(&tknz);

	if (flag & TEST_TKNZ)
	{
		token_show(tknzed);
		puts(color(0, 220, 0) "\n[[ tokenizing complete! ]]" color_clear "\n");
	}

	parser psr;
	psr_init(&psr, tknzed);
	AST_node* result = psr_parse(&psr);

	if (flag & TEST_OPTM)
	{
		optimize_AST(result);
		puts(color(0, 220, 0) "[[ optimizing complete! ]]" color_clear "\n");
	}

	if (flag & TEST_PSR)
	{
		node_show(result, 0, "[R] ");
		puts(color(0, 220, 0) "\n[[ parsing complete! ]]" color_clear "\n");
	}

	//symbol_table syt;
	//syt_create(&syt);
	//syt_insertf(&syt, result);
	//syt_show(&syt);
	//syt_destroy(&syt);

	tknz_destroy(&tknz);
	psr_destroy(&psr);
}
