#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <emmintrin.h>

#define HASHMAP_MAX_LOAD_FACTOR 0.85
#define HASHMAP_MAX_CAP_DEL_PERCENTAGE 0.25

#define HASHMAP_GROUP_SIZE 16

#define HASHMAP_KEY_EMPTY -128
#define HASHMAP_KEY_DELETED -1

#define HASHMAP_LIKELY(cond_) (__builtin_expect(false || (cond_), true))
#define HASHMAP_UNLIKELY(cond_) (__builtin_expect(false || (cond_), false))

#define HASHMAP_MASK_MATCH(X, Y) (_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(X), _mm_loadu_si128((const __m128i*)Y))))

#define HASHMAP_PROBE_RESULT_SET_FOUND(x) ((x) | 0x8000000000000000)
#define HASHMAP_PROBE_RESULT_SET_NOT_FOUND(x) ((x) & 0x7fffffffffffffff)

#define HASHMAP_PROBE_RESULT_GET_INDEX(x) ((x) & 0x7fffffffffffffff)
#define HASHMAP_PROBE_RESULT_GET_FOUND(x) (((x) & 0x8000000000000000) != 0)

#define HASHMAP_DEFAULT_CMP(a, b) (memcmp((a), (b), sizeof(*(a))) == 0)
#define HASHMAP_DEFAULT_HASH(key, len, seed) ({																\
    const uint8_t* _p = (const uint8_t*)(key);																\
    uint64_t _h = (seed);																					\
    size_t _len = (len);																					\
    while (_len >= 8)																						\
	{																										\
        uint64_t _v;																						\
        memcpy(&_v, _p, 8);																					\
        _h ^= _v;																							\
        _h *= 0xff51afd7ed558ccdULL;																		\
        _p += 8; 																							\
		_len -= 8;																							\
    }																										\
    if (_len > 0)																							\
	{																										\
        uint64_t _tail = 0;																					\
        memcpy(&_tail, _p, _len);																			\
        _h ^= _tail;																						\
        _h *= 0xc4ceb9fe1a85ec53ULL;																		\
    }																										\
    _h ^= _h >> 33;																							\
    _h *= 0xff51afd7ed558ccdULL;																			\
    _h ^= _h >> 33;																							\
    _h *= 0xc4ceb9fe1a85ec53ULL;																			\
    _h;																										\
})

#define HASHSET_TYPE(X, Y) HASHSET_TYPE_CUSTOM(X, Y, HASHMAP_DEFAULT_CMP, HASHMAP_DEFAULT_HASH)
#define HASHSET_TYPE_CUSTOM(X, Y, CMP, HASH)																\
typedef struct X* X;																						\
struct X																									\
{																											\
	size_t seed;																							\
	size_t size;																							\
	size_t deleted_size;																					\
	size_t capacity;																						\
																											\
	int8_t *ctrl;																							\
	Y *keys;																								\
};

#define HASHSET_DEFINE(X, Y) HASHSET_DEFINE_CUSTOM(X, Y, HASHMAP_DEFAULT_CMP, HASHMAP_DEFAULT_HASH)
#define HASHSET_DEFINE_CUSTOM(X, Y, CMP, HASH)																\
static X X##_new(size_t capacity);																			\
static void X##_free(X hs);																					\
static bool X##_contains(const X hs, const Y* key);															\
static bool X##_add(X hs, const Y* key);																	\
static bool X##_remove(X hs, const Y* key);																	\
																											\
static X X##_new(size_t capacity)																			\
{																											\
	X hs = malloc(sizeof(struct X));																		\
	if (!hs)																								\
		return 0;																							\
																											\
	hs->seed = (size_t)(uintptr_t)hs ^ (size_t)__builtin_ia32_rdtsc();										\
	hs->size = 0;																							\
	hs->deleted_size = 0;																					\
	hs->capacity = 16;																						\
																											\
	while (hs->capacity < capacity)																			\
		hs->capacity <<= 1;																					\
																											\
	hs->ctrl = malloc(hs->capacity + HASHMAP_GROUP_SIZE);													\
	hs->keys = malloc(hs->capacity * sizeof(Y));															\
																											\
	if (!(hs->ctrl && hs->keys))																			\
	{																										\
		X##_free(hs);																						\
		return 0;																							\
	}																										\
																											\
	memset(hs->ctrl, HASHMAP_KEY_EMPTY, hs->capacity + HASHMAP_GROUP_SIZE);									\
	return hs;																								\
}																											\
																											\
static void X##_free(X hs)																					\
{																											\
	free(hs->ctrl);																							\
	free(hs->keys);																							\
	free(hs);																								\
}																											\
																											\
static inline bool X##_compare(const Y* a, const Y* b)														\
{																											\
	return CMP(a, b);																						\
}																											\
																											\
static inline uint64_t X##_hash(const Y* key, size_t len, uint64_t seed)									\
{																											\
	return HASH(key, len, seed);																			\
}																											\
																											\
static inline size_t X##_probe(const X hs, const Y* key, const uint64_t h)									\
{																											\
	size_t pos = h >> 7 & (hs->capacity - 1);																\
	int8_t h2 = h & 0x7f;																					\
																											\
	while (true)																							\
	{																										\
		const int8_t* ctrl = hs->ctrl + pos;																\
		uint32_t match = HASHMAP_MASK_MATCH(h2, ctrl);														\
																											\
		while (match)																						\
		{																									\
			uint32_t i = __builtin_ctz(match);																\
			size_t slot = (pos + i) & (hs->capacity - 1);													\
																											\
			if (HASHMAP_LIKELY(X##_compare((hs->keys + slot), key)))										\
				return HASHMAP_PROBE_RESULT_SET_FOUND(slot);												\
																											\
			match &= match - 1;																				\
		}																									\
																											\
		uint32_t empty = HASHMAP_MASK_MATCH(HASHMAP_KEY_EMPTY, ctrl);										\
		if (HASHMAP_LIKELY(empty))																			\
			return HASHMAP_PROBE_RESULT_SET_NOT_FOUND((pos + __builtin_ctz(empty)) & (hs->capacity - 1)); 	\
																											\
		pos = (pos + HASHMAP_GROUP_SIZE) & (hs->capacity - 1);												\
	}																										\
}																											\
																											\
static size_t X##_probe_insert(const X hs, const Y* key, const uint64_t h)									\
{																											\
	size_t pos = h >> 7 & (hs->capacity - 1);																\
	int8_t h2 = h & 0x7f;																					\
	size_t first_deleted = ~(size_t)0;																		\
																											\
	while (true)																							\
	{																										\
		const int8_t* ctrl = hs->ctrl + pos;																\
		uint32_t match = HASHMAP_MASK_MATCH(h2, ctrl);														\
																											\
		while (match)																						\
		{																									\
			uint32_t i = __builtin_ctz(match);																\
			size_t slot = (pos + i) & (hs->capacity - 1);													\
																											\
			if (HASHMAP_LIKELY(X##_compare((hs->keys + slot), key)))										\
				return HASHMAP_PROBE_RESULT_SET_FOUND(slot);												\
																											\
			match &= match - 1;																				\
		}																									\
																											\
		if (hs->deleted_size && first_deleted == ~(size_t)0)												\
		{																									\
			uint32_t deleted = HASHMAP_MASK_MATCH(HASHMAP_KEY_DELETED, ctrl);								\
			if (deleted)																					\
				first_deleted = (pos + __builtin_ctz(deleted)) & (hs->capacity - 1);						\
		}																									\
																											\
		uint32_t empty = HASHMAP_MASK_MATCH(HASHMAP_KEY_EMPTY, ctrl);										\
		if (HASHMAP_LIKELY(empty))																			\
		{																									\
			if (first_deleted != ~(size_t)0)																\
				return HASHMAP_PROBE_RESULT_SET_NOT_FOUND(first_deleted);									\
																											\
			return HASHMAP_PROBE_RESULT_SET_NOT_FOUND((pos + __builtin_ctz(empty)) & (hs->capacity - 1)); 	\
		}																									\
																											\
		pos = (pos + HASHMAP_GROUP_SIZE) & (hs->capacity - 1);												\
	}																										\
}																											\
																											\
static inline bool X##_rehash_grow(X hs, size_t new_capacity)												\
{																											\
	X tmp = X##_new(new_capacity);																			\
	if (!tmp)																								\
		return false;																						\
	tmp->seed = hs->seed;																					\
																											\
	for (size_t i = 0; i < hs->capacity; i++)																\
	{																										\
		if (hs->ctrl[i] >= 0)																				\
			X##_add(tmp, hs->keys + i);																		\
	}																										\
																											\
	free(hs->ctrl);																							\
	free(hs->keys);																							\
	*hs = *tmp;																								\
	free(tmp);																								\
	return true;																							\
}																											\
																											\
static bool X##_contains(const X hs, const Y* key)															\
{																											\
	uint64_t h = X##_hash(key, sizeof(Y), hs->seed);														\
	return HASHMAP_PROBE_RESULT_GET_FOUND(X##_probe(hs, key, h));											\
}																											\
																											\
static bool X##_add(X hs, const Y* key)																		\
{																											\
	if (HASHMAP_UNLIKELY((hs->size + hs->deleted_size) >= hs->capacity * HASHMAP_MAX_LOAD_FACTOR))			\
	{																										\
		size_t new_cap = (hs->deleted_size >= hs->capacity * HASHMAP_MAX_CAP_DEL_PERCENTAGE)				\
		                 ? hs->capacity : hs->capacity * 2;													\
																											\
		if (!X##_rehash_grow(hs, new_cap))																	\
			return false;																					\
	}																										\
																											\
	uint64_t h = X##_hash(key, sizeof(Y), hs->seed);														\
																											\
	size_t result = X##_probe_insert(hs, key, h);															\
	if (HASHMAP_PROBE_RESULT_GET_FOUND(result))																\
		return true;																						\
																											\
	size_t index = HASHMAP_PROBE_RESULT_GET_INDEX(result);													\
	hs->size++;																								\
	if (hs->ctrl[index] == HASHMAP_KEY_DELETED)																\
		hs->deleted_size--;																					\
																											\
	int8_t h2 = h & 0x7f;																					\
	hs->ctrl[index] = h2;																					\
	if (HASHMAP_UNLIKELY(index < HASHMAP_GROUP_SIZE))														\
		hs->ctrl[index + hs->capacity] = h2;																\
																											\
	memcpy(hs->keys + index, key, sizeof(Y));																\
	return true;																							\
}																											\
																											\
static bool X##_remove(X hs, const Y* key)																	\
{																											\
	uint64_t h = X##_hash(key, sizeof(Y), hs->seed);														\
																											\
	size_t result = X##_probe(hs, key, h);																	\
	if (HASHMAP_PROBE_RESULT_GET_FOUND(result))																\
	{																										\
		hs->size--;																							\
		hs->deleted_size++;																					\
																											\
		size_t index = HASHMAP_PROBE_RESULT_GET_INDEX(result);												\
		hs->ctrl[index] = HASHMAP_KEY_DELETED;																\
		if (HASHMAP_UNLIKELY(index < HASHMAP_GROUP_SIZE))													\
			hs->ctrl[index + hs->capacity] = HASHMAP_KEY_DELETED;											\
																											\
		return true;																						\
	}																										\
																											\
	return false;																							\
}
