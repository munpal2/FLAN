#pragma once

#include "hashtable.h"

typedef enum token_type
{
	TK_END,         TK_INT,        TK_FLOAT,       TK_STR,          TK_ID, 
	TK_INVALID,     TK_CONST,      TK_PTR,         TK_FUNC,        TK_SEMICOLON,
	TK_UINT,        TK_DECL,       TK_FOR,         TK_WHILE,        TK_IF,
	TK_ELSE,        TK_TRUE,       TK_FALSE,       TK_PLUS,         TK_MINUS,
	TK_MUL,         TK_DIV,        TK_MOD,         TK_NOT,          TK_ASSIGN,
	TK_EQ,          TK_NEQ,        TK_AND,         TK_OR,           TK_BXOR,
	TK_BAND,        TK_BOR,        TK_BNOT,        TK_LSHIFT,       TK_RSHIFT,
	TK_INC,         TK_DEC,        TK_PLUSEQ,      TK_MINUSEQ,      TK_MULEQ,
	TK_DIVEQ,       TK_MODEQ,      TK_LTE,         TK_GTE,          TK_LT,
	TK_GT,          TK_DOT,        TK_ARROW,       TK_TYPE,         TK_SHLEQ,
	TK_SHREQ,       TK_OREQ,       TK_ANDEQ,       TK_XOREQ,        TK_OPEN_PAREN,
	TK_CLOSE_PAREN, TK_OPEN_BRACE, TK_CLOSE_BRACE, TK_OPEN_BRACKET, TK_CLOSE_BRACKET,
	TK_COMMA,       TK_AS,         TK_ARR,         TK_OF,           TK_COLON
} token_type;

extern const char* token_strty[];

typedef struct token
{
	token_type type;
	unsigned int col;
	char* attr;
} token;

typedef struct tokenizer 
{
	size_t token_cnt;
	unsigned int col;
	variable_arr result;
	hash_table token_map;
	file_poller fpl;
} tokenizer;

void token_create(token* dest, unsigned int type, unsigned int col, char* attr);
bool tknz_init(tokenizer* tknz, const char* filename);
void tknz_destroy(tokenizer* tknz);
const variable_arr* tokenize(tokenizer* tknz);