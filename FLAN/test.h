#pragma once

#include "sementic.h"

#define TEST_TKNZ 1U
#define TEST_PSR 2U
#define TEST_ALL TEST_TKNZ | TEST_PSR

void test_file(const char* filename, unsigned int flag);