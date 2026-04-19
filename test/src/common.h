#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../../darray.h"
#include "../../hashmap.h"
#include "../../hashset.h"

#define STRING_SIZE 32

typedef struct
{
    int id;
    float value;
    double weight;
} test_struct;

typedef struct { char s[STRING_SIZE]; } str32;

#define INT_DA(f)         f(int_da, int)
#define TEST_STRUCT_DA(f) f(test_struct_da, test_struct)
#define STR32_DA(f)       f(str32_da, str32)
#define CHARPTR_DA(f)     f(charptr_da, char*)

#define U64_MAP(f) f(uint64_map, uint64_t, uint64_t)
#define U64_SET(f) f(uint64_set, uint64_t)

INT_DA(DARRAY_TYPE)
TEST_STRUCT_DA(DARRAY_TYPE)
STR32_DA(DARRAY_TYPE)
CHARPTR_DA(DARRAY_TYPE)

U64_MAP(HASHMAP_TYPE)
U64_SET(HASHSET_TYPE)
