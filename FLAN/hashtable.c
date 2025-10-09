#include "hashtable.h"

size_t hash(const char* cstr)
{
	size_t ret = 5381;
	int c;
	while (c = *(cstr++))
		ret = (ret << 5) + ret + c; //ret * 33 + c
	return ret;
}

//key의 주소(값 아님) : char**
static inline char** key_ptr(void* node_ptr)
{
	return (char**)(node_ptr);
}

//next의 주소(값 아님) : void**
static inline void** next_ptr(void* node_ptr)
{
	return (void**)((char*)(node_ptr)+sizeof(char*));
}

//value의 주소 : void*
static inline void* value_ptr(void* node_ptr)
{
	return (void*)((char*)(node_ptr)+sizeof(char*) + sizeof(void*));
}

static inline void destruct_node(void* node_ptr)
{
	free(*key_ptr(node_ptr));
	free(node_ptr);
}

static bool htb_resize(hash_table* table)
{
	void** old_buckets = table->buckets;
	table->size = 0; //다빠짐
	table->capacity *= 2;
	table->buckets = (void**)calloc(table->capacity, sizeof(void*));
	if (table->buckets == NULL)
		return false;

	for (size_t i = 0; i < (table->capacity >> 1); i++)
	{
		void* node = old_buckets[i];
		while (node != NULL)
		{
			void* next = *next_ptr(node);
			void* dest = htb_insert(table, *key_ptr(node));
			memcpy(dest, value_ptr(node), table->value_size);
			if (dest == NULL)
				return false;
			destruct_node(node);
			node = next;
		}
	}
	free(old_buckets);
	return true;
}

bool htb_create_impl(hash_table* table, size_t value_size)
{
	table->size = 0;
	table->capacity = HTB_INITIAL_CAPACITY;
	table->node_size = sizeof(char*) + sizeof(void*) + value_size;
	table->value_size = value_size;
	table->buckets = (void**)calloc(table->capacity, sizeof(void*));
	return table->buckets != NULL;
}

void* htb_insert(hash_table* table, const char* key) //성공시 value의 주소, 실패시 NULL
{
	//                       resize 성공  ||  resize() 실패
	//  capacity 초과 o  ||      ok              bad
	//  capacity 초과 x  ||      ok               ok
	if (table->size == (table->capacity >> 1) + (table->capacity >> 2) && !htb_resize(table))
	{
		puts(color(220, 0, 0) "해시맵을 리사이징하는데 실패했습니다");
		abort();
	}

	size_t idx = hash(key) & (table->capacity - 1);
	void* new_node = (void*)malloc(table->node_size);
	if (new_node == NULL)
		return NULL;

	if (key == NULL)
		*key_ptr(new_node) = NULL;
	else
		*key_ptr(new_node) = _strdup(key);

	*next_ptr(new_node) = table->buckets[idx];
	table->buckets[idx] = new_node;
	table->size++;
	return value_ptr(new_node);
}

void* htb_find(hash_table* table, const char* key)
{
	size_t idx = hash(key) & (table->capacity - 1);
	void* cur = table->buckets[idx];
	while (cur != NULL)
	{
		if (strcmp(*key_ptr(cur), key) == 0)
			return value_ptr(cur);
		cur = *next_ptr(cur);
	}
	return NULL;
}

void htb_foreach(hash_table* table, void(*fp)(void*))
{
	for (size_t i = 0; i < table->capacity; i++)
	{
		void* cur = table->buckets[i];
		while (cur != NULL)
		{
			void* next = *next_ptr(cur);
			fp(value_ptr(cur));
			cur = next;
		}
	}
}

void htb_destroy(hash_table* table)
{
	for (size_t i = 0; i < table->capacity; i++)
	{
		while (table->buckets[i] != NULL)
		{
			void* next = *next_ptr(table->buckets[i]);
			destruct_node(table->buckets[i]);
			table->buckets[i] = next;
		}
	}
	free(table->buckets);
	table->buckets = NULL;
}

bool htb_delete(hash_table* table, const char* key)
{
	size_t idx = hash(key) & (table->capacity - 1);
	void* cur = table->buckets[idx];

	if (cur == NULL) // table->NULL
		return false;

	if (strcmp(*key_ptr(cur), key) == 0) // table->target->...->NULL
	{
		table->buckets[idx] = *next_ptr(cur);
		destruct_node(cur);
		table->size--;
		return true;
	}

	void* prev = cur;
	cur = *next_ptr(cur);
	while (cur != NULL) //table->...->target->...->NULL
	{
		if (strcmp(*key_ptr(cur), key) == 0)
		{
			*next_ptr(prev) = *next_ptr(cur);
			destruct_node(cur);
			table->size--;
			return true;
		}
		prev = cur;
		cur = *next_ptr(cur);
	}
	return false; //(fail)
}
