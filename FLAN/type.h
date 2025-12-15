#pragma once

#include "parser.h"

typedef long long int addr_t;
/*
TYTR_FUNC: [0]이 파라미터, [1]이 리턴 타입
*/
typedef enum tytree_type
{
	TYTR_INT, TYTR_UINT, TYTR_CHAR, TYTR_BOOL, TYTR_FLOAT,
	TYTR_PTROF, TYTR_ARROF, TYTR_CONST, TYTR_FUNC
} tytree_type;

extern const char* tytree_strty[];

typedef struct tytree_node
{
	struct tytree_node* children[2];
	struct tytree_node* next;
	tytree_type type;
	size_t len; //TYTR_ARROF에만 사용
} tytree_node;

typedef enum literal_type_idx
{
	LTT_INT,
	LTT_UINT,
	LTT_CHAR,
	LTT_BOOL,
	LTT_FLOAT,
	LTT_STR
} literal_type_idx;
extern tytree_node* literal_type[6];
void lit_types_init();
void lit_types_destroy();

tytree_node* tytreend_create(tytree_type type, size_t len);
tytree_node* func_from_AST(AST_node* funcnode, size_t pidx, size_t ridx);
tytree_node* from_AST(AST_node* ast);
void tytree_destroy(tytree_node* root);
bool tytree_eq(tytree_node* t1, tytree_node* t2);
long long tytree_sizeof(tytree_node* root);

inline tytree_node* tytree_get_element_type(tytree_node* type)
{
	if (type->type == TYTR_PTROF || type->type == TYTR_ARROF)
		return type->children[0];
	else
		return NULL;
}// 포인터나 배열의 요소 타입 반환

inline tytree_node* tytree_get_base_type(tytree_node* type)
{
	return (type->type == TYTR_CONST) ? type->children[0] : type;
}// const를 벗긴 기본 타입 반환

inline bool tytree_is_const(tytree_node* type)
{
	return (type->type == TYTR_CONST);
}// 타입이 const인지 확인

inline bool tytree_is_ptr(tytree_node* type)
{
	type = tytree_get_base_type(type);
	return (type->type == TYTR_PTROF);
}// 타입이 포인터인지 확인

bool tytree_is_nearint(tytree_node* type);
