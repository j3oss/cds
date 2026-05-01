# cds - C Data Structures

A collection of generic, header-only C data structures.

- **[hashmap.h](hashmap.h)** Swiss Table inspired hash map
- **[hashset.h](hashset.h)** Swiss Table inspired hash set
- **[darray.h](darray.h)** Dynamic array

All three follow the same two-macro pattern:

- `DECLARE` — emits the struct/typedef **and** forward declarations for all public functions.
- `DEFINE` — emits the function implementations. Call this exactly once per type, in one `.c` file.

## hashmap

```c
#include "hashmap.h"

HASHMAP_DECLARE(u64_map, uint64_t, uint64_t)
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

HASHSET_DECLARE(u64_set, uint64_t)
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

DARRAY_DECLARE(int_da, int)
DARRAY_DEFINE(int_da, int)

int_da da = int_da_new(0, 1.5f);

int v = 42;
int_da_push(da, &v);

int *p = int_da_get_at(da, 0);
int_da_pop(da);
int_da_free(da);
```

## Alias macros

To avoid repeating type arguments across `DECLARE` and `DEFINE` calls, define an alias macro:

```c
#define INT_DA(f) f(int_da, int)

INT_DA(DARRAY_DECLARE)
INT_DA(DARRAY_DEFINE)
```

`HASHMAP_DECLARE_CUSTOM` / `HASHMAP_DEFINE_CUSTOM` and `HASHSET_DECLARE_CUSTOM` / `HASHSET_DEFINE_CUSTOM` accept additional comparator and hash macro arguments.
