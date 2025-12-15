#include "pool.h"

strpool global_strpool;

static inline void destruct_node(pool_node* node_ptr)
{
	free(node_ptr->str);
	free(node_ptr);
}

bool pool_create(strpool* pool)
{
	pool->size = 0;
	pool->capacity = POOL_INITIAL_CAPACITY;
	pool->buckets = (const char**)calloc(pool->capacity, sizeof(const char*));
	return pool->buckets != NULL;
}

static const char* pool_find(strpool* pool, const char* str)
{
	size_t idx = hash(str) & (pool->capacity - 1);
	pool_node* cur = pool->buckets[idx];
	while (cur != NULL)
	{
		if (strcmp(cur->str, str) == 0)
			return cur->str;
		cur = cur->next;
	}
	return NULL;
}

bool pool_exists(strpool* pool, const char* str)
{
	return pool_find(pool, str) != NULL;
}

void pool_destroy(strpool* pool)
{
	for (size_t i = 0; i < pool->capacity; i++)
	{
		while (pool->buckets[i] != NULL)
		{
			pool_node* next = pool->buckets[i]->next;
			destruct_node(pool->buckets[i]);
			pool->buckets[i] = next;
		}
	}
	free(pool->buckets);
	pool->buckets = NULL;
}

static bool pool_resize(strpool* pool)
{
	pool_node** old_buckets = pool->buckets;
	pool->size = 0; //다빠짐
	pool->capacity *= 2;
	pool->buckets = (pool_node**)calloc(pool->capacity, sizeof(pool_node*));
	if (pool->buckets == NULL)
		return false;

	for (size_t i = 0; i < (pool->capacity >> 1); i++)
	{
		pool_node* node = old_buckets[i];
		while (node != NULL)
		{
			size_t new_idx = hash(node->str) & (pool->capacity - 1);
			node->next = pool->buckets[new_idx];
			pool->buckets[new_idx] = node;
			node = node->next;
		}
	}
	free(old_buckets);
	return true;
}

static const char* pool_insert(strpool* pool, const char* str)
{
	if (pool->size == (pool->capacity >> 1) + (pool->capacity >> 2) && !pool_resize(pool))
	{
		puts(color_sys_err "스트링 풀을 리사이징하는데 실패했습니다");
		abort();
	}

	size_t idx = hash(str) & (pool->capacity - 1);
	pool_node* new_node = (pool_node*)malloc(sizeof(pool_node));
	if (new_node == NULL)
		return NULL;

	if (str == NULL)
		new_node->str = NULL;
	else
		new_node->str = _strdup(str);

	new_node->next = pool->buckets[idx];
	pool->buckets[idx] = new_node;
	pool->size++;
	return new_node->str;
}

const char* pool_intern(strpool* pool, const char* str)
{
	if (str == NULL)
		return NULL;
	const char* findret = pool_find(pool, str);
	return findret != NULL ? findret : pool_insert(pool, str);
}