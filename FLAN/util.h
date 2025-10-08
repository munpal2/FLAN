#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

bool open_file(file_poller* dest, const char* filename, const char* mode);
char lookahead(file_poller* dest, size_t n);
void close_file(file_poller* dest);

inline void dispose(file_poller* dest, size_t n)
{
	dest->offset += n;
}

inline char poll(file_poller* dest)
{
	char ch = lookahead(dest, 0);
	dispose(dest, 1);
	return ch;
}

typedef struct str_builder
{
	char* dest;
	size_t capacity;
	size_t len;
} str_builder;

#define STR_BUILDER_INITIAL_SIZE 32

void str_builder_create(str_builder* strbd);
bool str_builder_add(str_builder* strbd, char ch);
bool str_builder_add_str(str_builder* strbd, const char* str);
const char* str_builder_get(str_builder* strbd);
const char* str_builder_pop(str_builder* strbd);
void str_builder_destroy(str_builder* strbd);

typedef struct variable_arr
{
	void* data;
	size_t element_size;
	size_t size;
} variable_arr; //넣어라, 생길지어니

#define varr_create(ptr, type, size) varr_create_impl((ptr), sizeof(type), (size))
#define varr_get(ptr, type, idx) ((type*)varr_get_impl((ptr), (idx)))
void varr_destroy(variable_arr* varr);
bool varr_create_impl(variable_arr* varr, size_t elementsize, size_t size);
void* varr_get_impl(variable_arr* varr, size_t idx);

#define color(R, G, B) "\033[38;2;" #R ";" #G ";" #B "m\b"
#define color_clear "\033[39m"
