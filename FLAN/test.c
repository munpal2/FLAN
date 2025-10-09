#include "test.h"

static void token_show(variable_arr* tokens)
{
	for (size_t i = 0; varr_get(tokens, token, i)->type != TK_END; i++)
	{
		token* elem = varr_get(tokens, token, i);
		printf("Line %u : <%s, %s>\n", elem->col, elem->attr, token_strty[elem->type]);
	}
}

static void node_show(AST_node* node, size_t depth)
{
	if (node->type == AST_CONJ && node->children[1] == NULL)
		return node_show(node->children[0], depth);

	for (size_t i = 0; i < depth; i++)
		printf("  ");

	printf("%s", AST_strty[node->type]);
	if (node->attr != NULL)
		printf(": %s", node->attr);
	puts(" {");

	for (size_t i = 0; i < 4; i++)
	{
		if (node->children[i] != NULL)
			node_show(node->children[i], depth + 1);
	}
	for (size_t i = 0; i < depth; i++)
		printf("  ");
	puts("}");
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
	//optimize_AST(result);

	if (flag & TEST_PSR)
	{
		node_show(result, 0);
		puts(color(0, 220, 0) "\n[[ parsing complete! ]]" color_clear "\n");
	}

	tknz_destroy(&tknz);
	psr_destroy(&psr);
}
