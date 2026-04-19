#include <stdio.h>
#include <assert.h>

#include "common.h"

INT_DA(DARRAY_DEFINE)
TEST_STRUCT_DA(DARRAY_DEFINE)
STR32_DA(DARRAY_DEFINE)
CHARPTR_DA(DARRAY_DEFINE)

#define TEST_COUNT 10000000

/* ------------------------------------------------------------ */

void test_int_basic()
{
	printf("Running int basic test...\n");

	int_da da = int_da_new(0, 1.5f);
	assert(da);
	assert(int_da_is_empty(da));

	for (int i = 0; i < 1000; i++)
	{
		size_t idx = int_da_push(da, &i);
		assert(idx == (size_t)i);
	}

	assert(int_da_get_size(da) == 1000);

	for (int i = 0; i < 1000; i++)
	{
		int* v = int_da_get_at(da, i);
		assert(*v == i);
	}

	for (int i = 999; i >= 0; i--)
		assert(int_da_pop(da));

	assert(int_da_is_empty(da));
	int_da_free(da);
}

/* ------------------------------------------------------------ */

void test_structs()
{
	printf("Running struct test...\n");

	test_struct_da da = test_struct_da_new(0, 2.0f);

	for (int i = 0; i < 10000; i++)
	{
		test_struct s = { i, i * 0.5f, i * 2.0 };
		test_struct_da_push(da, &s);
	}
	assert(test_struct_da_get_size(da) == 10000);

	for (int i = 0; i < 10000; i++)
	{
		test_struct* s = test_struct_da_get_at(da, i);
		assert(s->id == i);
		assert(s->value == i * 0.5f);
		assert(s->weight == i * 2.0);
	}

	test_struct_da_remove(da, 10);
	assert(test_struct_da_get_size(da) == 9999);

	test_struct_da_remove_ordered(da, 20);
	assert(test_struct_da_get_size(da) == 9998);

	test_struct_da_free(da);
}

/* ------------------------------------------------------------ */

void test_strings()
{
	printf("Running string test...\n");

	str32_da da = str32_da_new(0, 1.5f);

	for (int i = 0; i < 1000; i++)
	{
		str32 buf;
		snprintf(buf.s, STRING_SIZE, "str_%d", i);
		str32_da_push(da, &buf);
	}

	for (int i = 0; i < 1000; i++)
	{
		str32* s = str32_da_get_at(da, i);
		char expected[STRING_SIZE];
		snprintf(expected, STRING_SIZE, "str_%d", i);
		assert(strcmp(s->s, expected) == 0);
	}

	str32_da_free(da);
}

/* ------------------------------------------------------------ */

void test_string_pointers()
{
	printf("Running string pointer test...\n");

	charptr_da da = charptr_da_new(0, 1.5f);

	for (int i = 0; i < 1000; i++)
	{
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "str_%d", i);

		char* heap_str = strdup(buffer);
		charptr_da_push(da, &heap_str);
	}

	for (int i = 0; i < 1000; i++)
	{
		char** s = charptr_da_get_at(da, i);
		char expected[64];
		snprintf(expected, sizeof(expected), "str_%d", i);

		assert(strcmp(*s, expected) == 0);
	}

	for (size_t i = 0; i < charptr_da_get_size(da); i++)
	{
		char** s = charptr_da_get_at(da, i);
		free(*s);
	}

	charptr_da_free(da);
}

/* ------------------------------------------------------------ */

void test_indexed_insert()
{
	printf("Running indexed insert test...\n");

	int_da da = int_da_new(0, 1.5f);

	for (int i = 0; i < 10; i++)
		int_da_push(da, &i);

	int x = 999;
	int_da_push_indexed(da, 5, &x);

	assert(*(int*)int_da_get_at(da, 5) == 999);
	assert(int_da_get_size(da) == 11);

	int_da_free(da);
}

/* ------------------------------------------------------------ */

void test_resize_reserve_shrink()
{
	printf("Running resize/reserve test...\n");

	int_da da = int_da_new(0, 1.5f);

	assert(int_da_reserve(da, 1000));
	assert(int_da_get_capacity(da) >= 1000);

	assert(int_da_resize(da, 500));
	assert(int_da_get_size(da) == 500);

	assert(int_da_shrink_fit(da));
	assert(int_da_get_capacity(da) == 500);

	int_da_free(da);
}

/* ------------------------------------------------------------ */

void test_get_data()
{
	printf("Running get_data test...\n");

	int_da da = int_da_new(0, 1.5f);

	for (int i = 0; i < 5; i++)
		int_da_push(da, &i);

	int* raw = int_da_get_data(da);
	assert(raw != NULL);

	for (int i = 0; i < 5; i++)
		assert(raw[i] == i);

	int_da_free(da);
}

/* ------------------------------------------------------------ */

void test_reserve_n()
{
	printf("Running reserve_n test...\n");

	int_da da = int_da_new(0, 1.5f);

	for (int i = 0; i < 10; i++)
		int_da_push(da, &i);

	assert(int_da_reserve_n(da, 100));
	assert(int_da_get_capacity(da) >= 110);
	assert(int_da_get_size(da) == 10);

	int_da_free(da);
}

/* ------------------------------------------------------------ */

void test_pop_empty()
{
	printf("Running pop empty test...\n");

	int_da da = int_da_new(0, 1.5f);
	assert(!int_da_pop(da));

	int x = 1;
	int_da_push(da, &x);
	assert(int_da_pop(da));
	assert(!int_da_pop(da));

	int_da_free(da);
}

/* ------------------------------------------------------------ */

void test_stress()
{
	printf("Running stress test...\n");

	int_da da = int_da_new(0, 1.5f);
	assert(da);

	for (size_t i = 0; i < TEST_COUNT; i++)
	{
		int v = (int)i;
		assert(int_da_push(da, &v) != (size_t)-1);
	}

	assert(int_da_get_size(da) == TEST_COUNT);

	for (size_t i = 0; i < TEST_COUNT; i++)
	{
		int* v = int_da_get_at(da, i);
		assert(*v == (int)i);
	}

	for (size_t i = 0; i < TEST_COUNT; i++)
		assert(int_da_pop(da));

	assert(int_da_is_empty(da));

	int_da_free(da);
}

/* ------------------------------------------------------------ */

int main()
{
	test_int_basic();
	test_structs();
	test_strings();
	test_indexed_insert();
	test_resize_reserve_shrink();
	test_get_data();
	test_reserve_n();
	test_pop_empty();
	test_stress();
	test_string_pointers();

	printf("All tests passed successfully.\n");
	return 0;
}
