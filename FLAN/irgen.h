#pragma once

#include "sementic.h"

typedef enum tytree_type
{
	TYTR_INT,   TYTR_UINT,  TYTR_CHAR,  TYTR_BOOL,  TYTR_FLOAT,
	TYTR_PTROF, TYTR_ARROF, TYTR_CONST, TYTR_PARAM, TYTR_FUNC
} tytree_type;

typedef struct tytree_node
{
	struct tytree_node* child[2];
	tytree_type type;
	size_t len; //TYTR_ARROF에만 사용
} tytree_node;

typedef struct symbol
{
	tytree_node* type;
	size_t address;
} symbol;

typedef struct symbol_table
{
	variable_arr varr;
	size_t depth;
	size_t stack_bottom;
} symbol_table;

void syt_create(symbol_table* syt);