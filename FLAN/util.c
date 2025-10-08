#include "util.h"

bool open_file(file_poller* dest, const char* filename, const char* mode)
{
	dest->fp = fopen(filename, mode);
	if (dest->fp == NULL)
		return false;

	/*대충 이 상태로 lookahead들어가면 읽혀서 나옴*/
	dest->end[1] = FILE_BUF_SIZE;
	dest->page[0] = -1;
	dest->page[1] = -1;
	dest->offset = 0;
	dest->buffer_idx = 1;

	return true;
}

char lookahead(file_poller* dest, size_t n)
{
	if (n >= FILE_BUF_SIZE)
		return EOF;

	n += dest->offset;
	size_t page = (n >> FILE_BUF_SIZE_LOG2);
	size_t idx = (n & (FILE_BUF_SIZE - 1));

	if (page != dest->page[dest->buffer_idx]) //같은 버퍼 안에 없음
	{
		if (dest->end[dest->buffer_idx] != FILE_BUF_SIZE) //또 읽어봤자 안나옴
			return EOF;
		dest->buffer_idx ^= 1;
		dest->page[dest->buffer_idx] = page;
		size_t read_cnt = fread(dest->buf[dest->buffer_idx], sizeof(char), FILE_BUF_SIZE, dest->fp);
		if (read_cnt < FILE_BUF_SIZE)
			dest->buf[dest->buffer_idx][read_cnt++] = EOF;
		dest->end[dest->buffer_idx] = read_cnt;
		return dest->buf[dest->buffer_idx][idx];
	}

	// 같은 버퍼 안에는 있음
	if (idx >= dest->end[dest->buffer_idx])
		return EOF;
	return dest->buf[dest->buffer_idx][idx];
}

void close_file(file_poller* dest)
{
	fclose(dest->fp);
}

void str_builder_create(str_builder* strbd)
{
	strbd->dest = (char*)malloc(STR_BUILDER_INITIAL_SIZE);
	strbd->capacity = STR_BUILDER_INITIAL_SIZE;
	strbd->len = 0;
}

bool str_builder_add(str_builder* strbd, char ch)
{
	if (strbd->len >= strbd->capacity - 1)
	{
		strbd->dest = realloc(strbd->dest, strbd->capacity * 2);
		if (strbd->dest == NULL)
		{
			puts(color(220, 0, 0) "스트링빌더를 리사이징하는데 실패했습니다");
			abort();
		}
		strbd->capacity *= 2;
	}
	strbd->dest[(strbd->len)++] = ch;
	return true;
}

bool str_builder_add_str(str_builder* strbd, const char* str)
{
	while (*str)
	{
		if (!str_builder_add(strbd, *str++))
			return false;
	}
	return true;
}

const char* str_builder_get(str_builder* strbd)
{
	strbd->dest[strbd->len] = '\0';
	return strbd->dest;
}

const char* str_builder_pop(str_builder* strbd)
{
	strbd->dest[strbd->len] = '\0';
	strbd->len = 0;
	return strbd->dest;
}

void str_builder_destroy(str_builder* strbd)
{
	free(strbd->dest);
	strbd->dest = NULL;
	strbd->capacity = 0;
	strbd->len = 0;
}

static bool expand(variable_arr* varr)
{
	void* new_data = realloc(varr->data, varr->element_size * varr->size * 2);
	if (new_data == NULL)
		return false;
	varr->data = new_data;
	varr->size *= 2;
	return true;
}

void* varr_get_impl(variable_arr* varr, size_t idx)
{
	while (idx >= varr->size)
	{
		if (!expand(varr))
			return NULL;
	}
	return ((char*)(varr->data) + varr->element_size * idx);
}

void varr_destroy(variable_arr* varr) //걍 clear용으로 써도 됨
{
	free(varr->data);
	varr->data = NULL;
	varr->size = 0;
}

bool varr_create_impl(variable_arr* varr, size_t elementsize, size_t size)
{
	varr->element_size = elementsize;
	varr->size = size;
	return (varr->data = malloc(elementsize * size)) != NULL;
}