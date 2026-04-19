# cds - C Data Structures

A collection of generic, header-only C data structures.

- **[hashmap.h](hashmap.h)** Swiss Table inspired hash map
- **[hashset.h](hashset.h)** Swiss Table inspired hash set
- **[darray.h](darray.h)** Dynamic array

All three follow the same pattern: `TYPE` emits only the struct/typedef. `DEFINE` emits the function implementations as `static` and must appear in each translation unit that uses the API.

## hashmap

```c
#include "hashmap.h"

HASHMAP_TYPE(u64_map, uint64_t, uint64_t)
HASHMAP_DEFINE(u64_map, uint64_t, uint64_t)

u64_map m = u64_map_new(0);

uint64_t k = 1, v = 42;
u64_map_add(m, &k, &v);

uint64_t *val = u64_map_get(m, &k);  // NULL if absent
u64_map_remove(m, &k);
u64_map_free(m);
```

## hashset

```c
#include "hashset.h"

HASHSET_TYPE(u64_set, uint64_t)
HASHSET_DEFINE(u64_set, uint64_t)

u64_set s = u64_set_new(0);

uint64_t k = 1;
u64_set_add(s, &k);
u64_set_contains(s, &k);  // true
u64_set_remove(s, &k);
u64_set_free(s);
```

## darray

```c
#include "darray.h"

DARRAY_TYPE(int_da, int)
DARRAY_DEFINE(int_da, int)

int_da da = int_da_new(0, 1.5f);

int v = 42;
int_da_push(da, &v);

int *p = int_da_get_at(da, 0);
int_da_pop(da);
int_da_free(da);
```

## Sharing types across translation units

To avoid repeating template arguments in every `.c` file, define a macro alongside the type:

```c
// common.h
#define U64_MAP(f) f(u64_map, uint64_t, uint64_t)
U64_MAP(HASHMAP_TYPE)

#define INT_DA(f) f(int_da, int)
INT_DA(DARRAY_TYPE)
```

```c
// foo.c, bar.c, ...
#include "common.h"
U64_MAP(HASHMAP_DEFINE)
INT_DA(DARRAY_DEFINE)
```

`HASHMAP_TYPE_CUSTOM` / `HASHMAP_DEFINE_CUSTOM` and `HASHSET_TYPE_CUSTOM` / `HASHSET_DEFINE_CUSTOM` accept additional comparator and hash macro arguments.