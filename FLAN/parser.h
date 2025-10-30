#pragma once

#include "tokenizer.h"

typedef enum AST_type 
{
	AST_CONJ,    AST_DECL,     AST_CONST,    AST_PTROF,   AST_ASSIGN,
	AST_ADD,     AST_SUB,      AST_MUL,      AST_DIV,     AST_NEG,
	AST_REF,     AST_GETADDR,  AST_ID,       AST_STR,     AST_INT,
	AST_FLOAT,   AST_UINT,     AST_TYINT,   AST_CAST,    AST_ARR,
	AST_FWINC,   AST_FWDEC,    AST_BWINC,    AST_BWDEC,   AST_DOT,
	AST_ARROW,   AST_TRUE,     AST_FALSE,    AST_NOT,     AST_BNOT,
	AST_MOD,     AST_LSHIFT,   AST_RSHIFT,   AST_LT,      AST_GT,
	AST_LTE,     AST_GTE,      AST_EQ,       AST_NEQ,     AST_BAND,
	AST_BOR,     AST_BXOR,     AST_OR,       AST_AND,     AST_ADDI,
	AST_SUBI,    AST_MULI,     AST_DIVI,     AST_MODI,    AST_ANDI,
	AST_ORI,     AST_XORI,     AST_LSHIFTI,  AST_RSHIFTI, AST_WHILE,
	AST_IF,      AST_FDECL,    AST_BLOCK,    AST_PARAM,   AST_RETURN,
	AST_IDX,     AST_FUNC,     AST_CALL,     AST_FOR,     AST_CHAR,
	AST_IDINIT,  AST_INITEXPR, AST_TYFLOAT,  AST_TYBOOL,  AST_TYCHAR,
	AST_TYUINT
} AST_type;

extern const char* AST_strty[];

typedef struct AST_node 
{
	unsigned int type;
	unsigned int col;
	char* attr;
	struct AST_node* next;
	struct AST_node* children[4];
} AST_node;

void AST_node_create(AST_node* dest, AST_type type, const char* str, unsigned int col);
void AST_node_destroy(AST_node* dest);

typedef struct parser 
{
	variable_arr* tokens;
	token* lookahead;
	size_t idx;
	AST_node* root;
} parser;

void psr_init(parser* psr, variable_arr* processed_tokens);
void psr_next(parser* psr);
void psr_expect(parser* psr, token_type type);
void psr_destroy(parser* psr);
AST_node* psr_parse(parser* psr);