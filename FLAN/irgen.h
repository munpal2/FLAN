#pragma once

#include "type.h"

/*
ADDR_STACK = 절대주소
ADDR_PARAM = 상대주소(스택 꼭대기 기준)

함수 진입 시:
ebp = esp;

함수 헤더 :
                                 ebp
                                  v
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
	addr_t addr;//절대주소 of 상대주소
} symbol;

inline void sym_destroy(symbol* sym)
{
	tytree_destroy(sym->type);
}

void sym_create(symbol* sym, addr_type atype, addr_t addr, tytree_node* type);

typedef struct symbol_table
{
	variable_arr varr;
	size_t depth;
	size_t stack_bottom;
	bool in_func_block;
} symbol_table;

void syt_create(symbol_table* syt);
void syt_destroy(symbol_table* syt);
symbol* syt_find(symbol_table* syt, const char* str);
void syt_insert(symbol_table* syt, AST_node* decltree);
void syt_insertf(symbol_table* syt, AST_node* fdecltree);

inline void syt_enter(symbol_table* syt)
{
	htb_create(varr_get(&(syt->varr), hash_table, (syt->depth)++), symbol);
}

inline void syt_exit(symbol_table* syt)
{
	hash_table* cur = varr_get(&(syt->varr), hash_table, syt->depth - 1);
	htb_foreach(cur, sym_destroy);
	htb_destroy(cur);
	syt->depth--;
}

typedef enum ir_type
{
	IR_ADD,   IR_SUB,     IR_MUL,       IR_DIV,      IR_MOD,
	IR_NEG,   IR_INC,     IR_DEC,       IR_LOAD,     IR_STORE, 
	IR_AND,   IR_OR,      IR_XOR,       IR_LSHF,     IR_RSHF,
	IR_NOT,   IR_CMP,     IR_GT,        IR_LT,       IR_JMP,
	IR_JZ,    IR_JNZ,     IR_CALL,      IR_RET,      IR_ALLOC,
	IR_FREE,  IR_SYSCALL, IR_LOADCONST, IR_MOVE,     IR_ADDF,
	IR_SUBF,  IR_MULF,    IR_DIVF
} ir_type;

typedef enum ir_access_size
{
	SZ_BYTE = 1,
	SZ_WORD = 2,
	SZ_DWORD = 4,
	SZ_QWORD = 8,
	SZ_UNABLE = 0
} ir_access_size;
ir_access_size tytree_to_irsize(tytree_node* type);

typedef unsigned long long int mvar_code;
#define EBP_MVCODE 0
#define ESP_MVCODE 1

typedef union qword
{
	double flt;
	long long int dec;
	addr_t addr;
	bool boolean;
	ir_access_size asize;
	mvar_code mvcode; //0:EBP, 1:ESP, 2:T0, 3:T1, ...
	char ch;
} qword;

typedef struct ir
{
	ir_type type;
	qword op1;
	qword op2;
	mvar_code dest;
} ir;

typedef enum literal_type_idx
{
	LTT_INT,
	LTT_UINT,
	LTT_CHAR,
	LTT_BOOL,
	LTT_FLOAT,
	LTT_STR
} literal_type_idx;

typedef struct irgen
{
	variable_arr irs;
	symbol_table syt;
	hash_table str_addr; //key: str literal, value: long long int addr
	addr_t data_end;
	size_t irs_len;
	tytree_node* literal_type[6]; //0:int, 1:uint, 2:char, 3:bool, 4:float, 5:str(const ptr of char)
	size_t mvar_count;
} irgen;

void irgen_create(irgen* irg);
void irgen_destroy(irgen* irg);
void irgen_gen(irgen* irg, AST_node* root);
ir* irgen_push(irgen* irg, ir_type type, mvar_code mvcode);
