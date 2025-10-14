#pragma once

#include "sementic.h"
#include "irgen.h"

#define TEST_TKNZ 1U
#define TEST_PSR 2U
#define TEST_OPTM 4U
#define TEST_ALL 7U

void test_file(const char* filename, unsigned int flag);