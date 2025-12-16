#include "hashtable.h"

static inline void destruct_node(hash_node* node_ptr)
{
	free(node_ptr->key);
	free(node_ptr->value);
	free(node_ptr);
}

static bool htb_resize(hash_table* table)
{
	hash_node** old_buckets = table->buckets;
	table->size = 0; //다빠짐
	table->capacity *= 2;
	table->buckets = (hash_node**)calloc(table->capacity, sizeof(hash_node*));
	if (table->buckets == NULL)
		return false;

	for (size_t i = 0; i < (table->capacity >> 1); i++)
	{
		hash_node* node = old_buckets[i];
		while (node != NULL)
		{
			size_t new_idx = hash(node->key) & (table->capacity - 1);
			hash_node* old_next = node->next;
			node->next = table->buckets[new_idx];
			table->buckets[new_idx] = node;
			node = old_next;
		}
	}
	free(old_buckets);
	return true;
}

bool htb_create_impl(hash_table* table, size_t value_size)
{
	table->size = 0;
	table->capacity = HTB_INITIAL_CAPACITY;
	table->buckets = (hash_node**)calloc(table->capacity, sizeof(hash_node*));
	table->value_size = value_size;
	return table->buckets != NULL;
}

void* htb_insert(hash_table* table, const char* key) //성공시 value의 주소, 실패시 NULL
{
	//                       resize 성공  ||  resize() 실패
	//  capacity 초과 o  ||      ok              bad
	//  capacity 초과 x  ||      ok               ok
	if (table->size == (table->capacity >> 1) + (table->capacity >> 2) && !htb_resize(table))
	{
		puts(color_sys_err "해시맵을 리사이징하는데 실패했습니다");
		abort();
	}

	size_t idx = hash(key) & (table->capacity - 1);
	hash_node* new_node = (hash_node*)malloc(sizeof(hash_node));
	if (new_node == NULL)
		return NULL;
	new_node->value = malloc(table->value_size);
	if (new_node->value == NULL)
		return NULL;

	if (key == NULL)
		new_node->key = NULL;
	else
		new_node->key = _strdup(key);

	new_node->next = table->buckets[idx];
	table->buckets[idx] = new_node;
	table->size++;
	return new_node->value;
}

void* htb_find(hash_table* table, const char* key)
{
	size_t idx = hash(key) & (table->capacity - 1);
	hash_node* cur = table->buckets[idx];
	while (cur != NULL)
	{
		if (strcmp(cur->key, key) == 0)
			return cur->value;
		cur = cur->next;
	}
	return NULL;
}

void htb_foreach(hash_table* table, void(*fp)(void*))
{
	for (size_t i = 0; i < table->capacity; i++)
	{
		hash_node* cur = table->buckets[i];
		while (cur != NULL)
		{
			hash_node* next = cur->next;
			fp(cur->value);
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
			hash_node* next = table->buckets[i]->next;
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
	hash_node* cur = table->buckets[idx];

	if (cur == NULL) // table->NULL
		return false;

	if (strcmp(cur->key, key) == 0) // table->target->...->NULL
	{
		table->buckets[idx] = cur->next;
		destruct_node(cur);
		table->size--;
		return true;
	}

	hash_node* prev = cur;
	cur = cur->next;
	while (cur != NULL) //table->...->target->...->NULL
	{
		if (strcmp(cur->key, key) == 0)
		{
			prev->next = cur->next;
			destruct_node(cur);
			table->size--;
			return true;
		}
		prev = cur;
		cur = cur->next;
	}
	return false; //(fail)
}
