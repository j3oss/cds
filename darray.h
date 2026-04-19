#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define DARRAY_INIT_CAPACITY 10
#define DARRAY_DEFAULT_GROWTH 1.5f

#define DARRAY_TYPE(X, Y)																						\
struct X																										\
{																												\
	size_t size;																								\
	size_t capacity;																							\
	float growth_factor;																						\
	Y* data;																									\
};																												\
typedef struct X* X;

#define DARRAY_DEFINE(X, Y)																						\
static inline bool X##_internal_realloc(X da, size_t new_capacity)												\
{																												\
	if (new_capacity == 0)																						\
	{																											\
		free(da->data);																							\
		da->data = NULL;																						\
		da->size = 0;																							\
		da->capacity = 0;																						\
		return true;																							\
	}																											\
	Y* new_data = realloc(da->data, sizeof(Y) * new_capacity);													\
	if (new_data)																								\
	{																											\
		da->data = new_data;																					\
		da->capacity = new_capacity;																			\
	}																											\
	return new_data != NULL;																					\
}																												\
																												\
static inline bool X##_internal_grow(X da, size_t n)															\
{																												\
	size_t required = da->size + n;																				\
	if (da->capacity >= required)																				\
		return true;																							\
	size_t grown = (size_t)(da->capacity * da->growth_factor);													\
	size_t new_cap = grown > required ? grown : required;														\
	return X##_internal_realloc(da, new_cap);																	\
}																												\
																												\
static X X##_new(size_t capacity, float growth_factor)															\
{																												\
	size_t init_cap = capacity ? capacity : DARRAY_INIT_CAPACITY;												\
																												\
	X da = malloc(sizeof(struct X));																			\
	if (!da)																									\
		return NULL;																							\
																												\
	da->size = 0;																								\
	da->capacity = init_cap;																					\
	da->growth_factor = growth_factor <= 1.0f ? DARRAY_DEFAULT_GROWTH : growth_factor;							\
	da->data = malloc(sizeof(Y) * init_cap);																	\
	if (!da->data)																								\
	{																											\
		free(da);																								\
		return NULL;																							\
	}																											\
																												\
	return da;																									\
}																												\
																												\
static void X##_free(X da)																						\
{																												\
	if (!da)																									\
		return;																									\
	free(da->data);																								\
	free(da);																									\
}																												\
																												\
static bool X##_shrink_fit(X da)																				\
{																												\
	assert(da);																									\
	return X##_internal_realloc(da, da->size);																	\
}																												\
																												\
static bool X##_reserve(X da, size_t new_capacity)																\
{																												\
	assert(da);																									\
	if (da->capacity >= new_capacity)																			\
		return true;																							\
	return X##_internal_realloc(da, new_capacity);																\
}																												\
																												\
static bool X##_reserve_n(X da, size_t additional)																\
{																												\
	assert(da);																									\
	return X##_reserve(da, da->size + additional);																\
}																												\
																												\
static bool X##_resize(X da, size_t size)																		\
{																												\
	assert(da);																									\
	bool result = true;																							\
	if (da->size < size)																						\
	{																											\
		result = X##_reserve(da, size);																			\
		if (!result)																							\
			return false;																						\
	}																											\
	da->size = size;																							\
	return result;																								\
}																												\
																												\
static size_t X##_push(X da, Y* data)																			\
{																												\
	assert(da);																									\
	if (!X##_internal_grow(da, 1))																				\
		return ~0;																								\
	memcpy(da->data + da->size, data, sizeof(Y));																\
	return da->size++;																							\
}																												\
																												\
static bool X##_pop(X da)																						\
{																												\
	assert(da);																									\
	if (da->size == 0)																							\
		return false;																							\
	da->size--;																									\
	return true;																								\
}																												\
																												\
static size_t X##_remove(X da, size_t index)																	\
{																												\
	assert(da);																									\
	assert(index < da->size);																					\
	if (index != da->size - 1)																					\
		memcpy(da->data + index, da->data + da->size - 1, sizeof(Y));											\
	da->size--;																									\
	return da->size;																							\
}																												\
																												\
static size_t X##_remove_ordered(X da, size_t index)															\
{																												\
	assert(da);																									\
	assert(index < da->size);																					\
	memmove(da->data + index, da->data + index + 1,																\
	        (da->size - index - 1) * sizeof(Y));																\
	da->size--;																									\
	return da->size;																							\
}																												\
																												\
static size_t X##_push_indexed(X da, size_t index, Y* data)														\
{																												\
	assert(da);																									\
	if (index > da->size)																						\
		return ~0;																								\
	if (!X##_internal_grow(da, 1))																				\
		return ~0;																								\
	memmove(da->data + index + 1, da->data + index,																\
	        (da->size - index) * sizeof(Y));																	\
	memcpy(da->data + index, data, sizeof(Y));																	\
	da->size++;																									\
	return index;																								\
}																												\
																												\
static bool X##_is_empty(X da)																					\
{																												\
	assert(da);																									\
	return da->size == 0;																						\
}																												\
																												\
static Y* X##_get_data(X da)																					\
{																												\
	assert(da);																									\
	return da->data;																							\
}																												\
																												\
static size_t X##_get_size(X da)																				\
{																												\
	assert(da);																									\
	return da->size;																							\
}																												\
																												\
static size_t X##_get_capacity(X da)																			\
{																												\
	assert(da);																									\
	return da->capacity;																						\
}																												\
																												\
static Y* X##_get_at(X da, size_t index)																		\
{																												\
	assert(da && da->data);																						\
	assert(index < da->size);																					\
	return da->data + index;																					\
}
