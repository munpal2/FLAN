#pragma once

#include "parser.h"

/*
TYTR_PARAM: [0]이 현재 파라미터 타입, [1]이 다음 TYTR (끝이면 NULL)
TYTR_FUNC: [0]이 파라미터, [1]이 리턴 타입
*/
typedef enum tytree_type
{
	TYTR_INT,   TYTR_UINT,  TYTR_CHAR,  TYTR_BOOL,  TYTR_FLOAT,
	TYTR_PTROF, TYTR_ARROF, TYTR_CONST, TYTR_PARAM, TYTR_FUNC
} tytree_type;

typedef struct tytree_node
{
	struct tytree_node* children[2];
	tytree_type type;
	size_t len; //TYTR_ARROF에만 사용
} tytree_node;

/*
ADDR_STACK = 절대주소
ADDR_PARAM = 상대주소(스택 꼭대기 기준)

함수 진입 시:
ebp = esp;

함수 헤더 :
[param1][param2][param3] ... [ret][old_ebp]
*/

typedef enum addr_type
{
	ADDR_REL, ADDR_ABS
} addr_type;

typedef struct symbol
{
	tytree_node* type;
	addr_type addrtype;
	long long int addr;//절대주소 of 상대주소
} symbol;

typedef struct symbol_table
{
	variable_arr varr;
	size_t depth;
	size_t stack_bottom;
	bool in_func_block;
} symbol_table;

void syt_create(symbol_table* syt);
symbol* syt_find(symbol_table* syt, const char* str);
void syt_insert(symbol_table* syt, AST_node* decltree);
//void syt_insertf(symbol_table* syt, AST_node* fdecltree);

inline void syt_enter(symbol_table* syt)
{
	htb_create(varr_get(&(syt->varr), hash_table, (syt->depth)++), symbol);
}

inline void syt_exit(symbol_table* syt)
{
	htb_destroy(varr_get(&(syt->varr), hash_table, (syt->depth)--));
}