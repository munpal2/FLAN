#pragma once

#include "hashtable.h"
#include "pool.h"

typedef enum token_type
{
	TK_END,         TK_INT,        TK_FLOAT,       TK_STR,          TK_ID, 
	TK_INVALID,     TK_CONST,      TK_PTR,         TK_FUNC,         TK_SEMICOLON,
	TK_UINT,        TK_DECL,       TK_FOR,         TK_WHILE,        TK_IF,
	TK_ELSE,        TK_TRUE,       TK_FALSE,       TK_PLUS,         TK_MINUS,
	TK_MUL,         TK_DIV,        TK_MOD,         TK_NOT,          TK_ASSIGN,
	TK_EQ,          TK_NEQ,        TK_AND,         TK_OR,           TK_BXOR,
	TK_BAND,        TK_BOR,        TK_BNOT,        TK_LSHIFT,       TK_RSHIFT,
	TK_INC,         TK_DEC,        TK_PLUSEQ,      TK_MINUSEQ,      TK_MULEQ,
	TK_DIVEQ,       TK_MODEQ,      TK_LTE,         TK_GTE,          TK_LT,
	TK_GT,          TK_DOT,        TK_ARROW,       TK_TYINT,         TK_SHLEQ,
	TK_SHREQ,       TK_OREQ,       TK_ANDEQ,       TK_XOREQ,        TK_OPEN_PAREN,
	TK_CLOSE_PAREN, TK_OPEN_BRACE, TK_CLOSE_BRACE, TK_OPEN_BRACKET, TK_CLOSE_BRACKET,
	TK_COMMA,       TK_AS,         TK_ARR,         TK_OF,           TK_COLON,
	TK_RETURN,      TK_CHAR,       TK_TYFLOAT,     TK_TYBOOL,       TK_TYCHAR,
	TK_TYUINT
} token_type;

extern const char* token_strty[];

typedef struct token
{
	token_type type;
	unsigned int col;
	char* attr;
	char* filename;
} token;

typedef struct tokenizer 
{
	variable_arr result;
	hash_table token_map;
	hash_table mcmd_map; //include, define등 매크로 커맨드용 해시테이블
	hash_table macro_map;
	variable_arr macro_condit;
	variable_arr fileref_stack; //fileref_context 의 스택
	variable_arr strref_stack; //strref_context 의 스택
} tokenizer;

void token_create(token* dest, 
	              token_type type, 
	              unsigned int col,
	              char* attr,
	              char* filename);
bool tknz_init(tokenizer* tknz, const char* filename);
void tknz_destroy(tokenizer* tknz);
const variable_arr* tokenize(tokenizer* tknz);