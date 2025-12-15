#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* 이거 쓰긴 함? */
#define acquire(type) acquire_impl(sizeof(type));
void* acquire_impl(size_t sz);

#define FILE_BUF_SIZE_LOG2 12
#define FILE_BUF_SIZE (1 << FILE_BUF_SIZE_LOG2)
typedef struct file_poller
{
	char buf[2][FILE_BUF_SIZE];
	size_t offset;
	size_t buffer_idx; //0번버퍼인지 1버퍼인지
	size_t page[2];
	size_t end[2];
	FILE* fp;
} file_poller;

bool fpl_open(file_poller* dest, const char* filename, const char* mode);
char fpl_lookahead(file_poller* dest, size_t n);
void fpl_close(file_poller* dest);

inline void fpl_dispose(file_poller* dest, size_t n)
{
	dest->offset += n;
}

inline char fpl_poll(file_poller* dest)
{
	char ch = fpl_lookahead(dest, 0);
	fpl_dispose(dest, 1);
	return ch;
}

typedef struct str_builder
{
	char* dest;
	size_t capacity;
	size_t len;
} str_builder;

#define STR_BUILDER_INITIAL_SIZE 32

bool str_builder_add(str_builder* strbd, char ch);
bool str_builder_add_str(str_builder* strbd, const char* str);
const char* str_builder_get(str_builder* strbd);
const char* str_builder_pop(str_builder* strbd);
void str_builder_create(str_builder* strbd);
void str_builder_destroy(str_builder* strbd);

typedef struct variable_arr
{
	void* data;
	size_t element_size;
	size_t size; //논리적 크기
	size_t capacity; //할당된 메모리 크기
} variable_arr; //넣어라, 생길지어니

#define varr_create(ptr, type, size) varr_create_impl((ptr), sizeof(type), (size))
void varr_destroy(variable_arr* varr);
bool varr_create_impl(variable_arr* varr, size_t elementsize, size_t initial_capacity);
void* varr_get(variable_arr* varr, size_t idx);
inline void* varr_push(variable_arr* varr) { return varr_get(varr, varr->size++); }
inline void* varr_back(variable_arr* varr) //비었으면 NULL
{
	return varr->size != 0 ? varr_get(varr, varr->size - 1) : NULL;
}
inline void varr_pop(variable_arr* varr) { if (varr->size != 0) varr->size--; }

#define color(R, G, B) "\033[38;2;" #R ";" #G ";" #B "m\b"
#define color_clear "\033[39m"

#define color_sys_err color(220, 0, 0)
#define color_err color(220, 130, 0)
#define color_warn color(220, 220, 0)

#define swap(a, b) \
	do { a = a ^ b; \
		 b = a ^ b; \
		 a = a ^ b; } while (0)
size_t hash(const char* cstr);