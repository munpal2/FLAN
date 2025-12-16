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
	for (size_t i = 0; i < syt->varr.size; i++)
	{
		htb_foreach(varr_get((&syt->varr), i), symbol_show);
	}
}

static void token_show(variable_arr* tokens)
{
	for (size_t i = 0; ((token*)varr_get(tokens, i))->type != TK_END; i++)
	{
		token* elem = varr_get(tokens, i);
		printf("%s(Line %u): <%s, %s>\n", elem->filename, elem->col, elem->attr, token_strty[elem->type]);
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

typedef enum ir_arg_type
{
	ADDR,
	RAW,
	ASIZE,
	NONE
} ir_arg_type;

ir_arg_type ir_arg_types[37][3] = {
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, NONE, ADDR},
	{ADDR, NONE, NONE},
	{ADDR, NONE, NONE},
	{ASIZE, ADDR, ADDR},
	{ASIZE, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, NONE, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{NONE, ADDR, NONE},
	{ADDR, ADDR, NONE},
	{ADDR, ADDR, NONE},
	{ADDR, NONE, NONE},
	{NONE, NONE, NONE},
	{RAW, NONE, ADDR},
	{ADDR, NONE, NONE},
	{ADDR, RAW, ADDR},
	{RAW, NONE, ADDR}, //load_const
	{ADDR, NONE, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, ADDR, ADDR},
	{ADDR, NONE, ADDR},
	{ADDR, NONE, ADDR},
	{ADDR, NONE, ADDR},
	{ADDR, ADDR, ADDR},
};

static void IR_arg_show(qword arg, ir_arg_type arg_type)
{
	switch (arg_type)
	{
		case ADDR:
		{
			printf("0x%llx(%lld)", arg.addr, arg.addr);
			break;
		}
		case RAW:
		{
			printf("%lld(%lff)", arg.dec, arg.flt); 
			break;
		}
		case ASIZE:
		{
			switch (arg.asize)
			{
			case SZ_BYTE:
				printf("BYTE");
				break;
			case SZ_WORD:
				printf("WORD");
				break;
			case SZ_DWORD:
				printf("DWORD");
				break;
			case SZ_QWORD:
				printf("QWORD");
				break;
			}
			break;
		}
	}
}

const char* arg_suffix[] = { ", ", " -> ", "\n" };
static void irg_show(irgen* irg)
{
	for (size_t i = 0; i < irg->irs.size; i++)
	{
		ir* elem = varr_get(&(irg->irs), i);
		printf("[%03llu] ", i);
		printf("%s: ", ir_strty[elem->type]);
		for (size_t j = 0; j < 3; j++)
		{
			if (ir_arg_types[elem->type][j] != NONE)
			{
				IR_arg_show(elem->args[j], ir_arg_types[elem->type][j]);
				printf("%s", arg_suffix[j]);
			}
		}
	}
}

void test_file(const char* filename, unsigned int flag) //토크나이저 테스트하깅
{
	pool_create(&global_strpool);
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

	irgen irg;
	irgen_create(&irg);
	irgen_gen(&irg, result);
	irg_show(&irg);
	irgen_destroy(&irg);

	tknz_destroy(&tknz);
	psr_destroy(&psr);
	pool_destroy(&global_strpool);
}
