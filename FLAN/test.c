#include "test.h"

static token_show(variable_arr* tokens)
{
	for (size_t i = 0; varr_get(tokens, token, i)->type != TK_END; i++)
	{
		token* elem = varr_get(tokens, token, i);
		printf("Line %u : <%s, %s>\n", elem->col, elem->attr, token_strty[elem->type]);
	}
}

static node_show(AST_node* node, size_t depth)
{
	for (size_t i = 0; i < depth; i++)
		printf("  ");
	printf("%s: \"%s\" {\n", AST_strty[node->type], node->attr);
	for (size_t i = 0; i < 4; i++)
	{
		if (node->children[i] != NULL)
			node_show(node->children[i], depth + 1);
	}
	for (size_t i = 0; i < depth; i++)
		printf("  ");
	puts("}");
}

void test_file(const char* filename) //토크나이저 테스트하깅
{
	tokenizer tknz;
	tknz_init(&tknz, filename);
	variable_arr* tknzed = tokenize(&tknz);

	//token_show(tknzed);
	puts(color(0, 220, 0) "\n[[ tokenizing complete! ]]" color_clear);

	parser psr;
	psr_init(&psr, tknzed);
	AST_node* result = psr_parse(&psr);
	node_show(result, 0);
}
