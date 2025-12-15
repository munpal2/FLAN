#pragma once

#include "util.h"

typedef struct pool_node
{
	char* str;
	struct pool_node* next;
} pool_node;

typedef struct strpool
{
	size_t capacity;
	size_t size;
	pool_node** buckets;
} strpool;

#define POOL_INITIAL_CAPACITY 16
bool pool_create(strpool* pool);
bool pool_exists(strpool* pool, const char* str);
void pool_destroy(strpool* pool);
const char* pool_intern(strpool* pool, const char* str);

extern strpool global_strpool;