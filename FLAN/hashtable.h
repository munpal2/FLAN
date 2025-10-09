#pragma once

#include "util.h"

/*typedef struct hash_node
{
	char* key;
	struct hash_node* next;
	[value]...
} hash_node;*/

typedef struct hash_table
{
	size_t capacity;
	size_t size;
	size_t node_size;
	size_t value_size;
	void** buckets;
} hash_table;

#define HTB_INITIAL_CAPACITY 16

#define htb_create(ptr, type) htb_create_impl(ptr, sizeof(type))

size_t hash(const char* cstr);
bool htb_create_impl(hash_table* table, size_t value_size);
void* htb_insert(hash_table* table, const char* key); //insert된 노드에서 value의 주소를 리턴, 실패시 NULL
bool htb_delete(hash_table* table, const char* key);
void* htb_find(hash_table* table, const char* key); //find된 노드에서 value의 주소를 리턴, 실패시 NULL
void htb_foreach(hash_table* table, void (*fp)(void*));
void htb_destroy(hash_table* table);
