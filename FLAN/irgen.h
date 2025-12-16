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
	htb_create(varr_push(&(syt->varr)), symbol);
}

inline void syt_exit(symbol_table* syt)
{
	hash_table* cur = varr_back(&(syt->varr));
	htb_foreach(cur, sym_destroy);
	htb_destroy(cur);
	varr_pop(&(syt->varr));
}

typedef enum ir_type
{
	IR_ADD,   IR_SUB,     IR_MUL,       IR_DIV,      IR_MOD,
	IR_NEG,   IR_INC,     IR_DEC,       IR_LOAD,     IR_STORE, 
	IR_AND,   IR_OR,      IR_XOR,       IR_LSH,      IR_RSH,
	IR_NOT,   IR_CMP,     IR_GT,        IR_LT,       IR_JMP,
	IR_JZ,    IR_JNZ,     IR_CALL,      IR_RET,      IR_ALLOC,
	IR_FREE,  IR_SYSCALL, IR_LOADCONST, IR_MOVE,     IR_ADDF,
	IR_SUBF,  IR_MULF,    IR_DIVF,      IR_ITOF,     IR_FTOI,
	IR_NEGF,  IR_GTF,     IR_LTF,       IR_MEMCPY,   IR_ERR
} ir_type;
extern const char* ir_strty[];

typedef enum ir_access_size
{
	SZ_BYTE = 1,
	SZ_WORD = 2,
	SZ_DWORD = 4,
	SZ_QWORD = 8,
	SZ_UNABLE = 0
} ir_access_size;
ir_access_size tytree_to_irsize(tytree_node* type);

typedef union qword
{
	double flt;
	long long int dec;
	addr_t addr; 
	ir_access_size asize;
} qword;

typedef struct ir
{
	ir_type type;
	qword args[3]; //op1, op2, dest 
} ir;

//(stack start)
//    ...
//(stack end)
//[GLOBAL]
//[DATA]
//[REG]
//[CODE]
typedef enum section_type
{
	DATA_SECTION, GLOBAL_SECTION, REG_SECTION, CODE_SECTION
} section_type;
#define ADDR_ON_DATA 0x0000000000000000LL
#define ADDR_ON_GLOBAL 0x0100000000000000LL
#define ADDR_ON_REG 0x0200000000000000LL
#define ADDR_ON_CODE 0x0300000000000000LL

//이 두 주소는 모두 REG_SECTION에 존재함...
#define EBP_ADDR (0 | ADDR_ON_REG)
#define ESP_ADDR (8 | ADDR_ON_REG)

typedef struct irgen
{
	variable_arr type_storage; //변수나 함수에 종속되지 않은 타입들이 저장되는 장소
	variable_arr section[2];
	variable_arr addr_on[3];
	variable_arr irs;
	symbol_table syt;
	hash_table str_addr; //key: str literal, value: long long int addr
	size_t reg_count;
} irgen;

void irgen_create(irgen* irg);
void irgen_destroy(irgen* irg);
void irgen_gen(irgen* irg, AST_node* root);
ir* irgen_push(irgen* irg, ir_type type, addr_t addr);
