#include "common.h"

U64_MAP(HASHMAP_DEFINE)

#define N 10000000ULL

static double now_sec(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}

static void report(const char* label, size_t ops, double elapsed)
{
    double mops  = ops / elapsed / 1e6;
    double ns_op = elapsed / ops * 1e9;
    printf("  %-30s  %8.2f Mops/s  %6.1f ns/op\n", label, mops, ns_op);
}

static uint64_t* make_sequential(size_t n)
{
    uint64_t* a = malloc(n * sizeof(uint64_t));
    for (size_t i = 0; i < n; i++) a[i] = i;
    return a;
}

static uint64_t* make_random(size_t n, uint64_t seed)
{
    uint64_t* a = malloc(n * sizeof(uint64_t));
    uint64_t s = seed;
    for (size_t i = 0; i < n; i++) {
        s ^= s << 13; s ^= s >> 7; s ^= s << 17;
        a[i] = s;
    }
    return a;
}

static void shuffle(uint64_t* a, size_t n)
{
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = (size_t)rand() % (i + 1);
        uint64_t t = a[i]; a[i] = a[j]; a[j] = t;
    }
}

int main(void)
{
    srand(42);
    printf("Hashmap benchmark  (N = %llu)\n\n", (unsigned long long)N);

    uint64_t* seq  = make_sequential(N);
    uint64_t* rnd  = make_random(N, 0xdeadbeefcafe1234ULL);
    uint64_t* miss = make_random(N, 0x0123456789abcdefULL);

    double t0, t1;
    uint64_t* v;
    volatile uint64_t sink = 0;

    // sequential insert
    {
        uint64_map hm = uint64_map_new(16);
        uint64_t acc = 0;
        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            acc += uint64_map_add(hm, &seq[i], &seq[i]);
        t1 = now_sec();
        sink ^= acc;
        report("insert sequential", N, t1 - t0);
        uint64_map_free(hm);
    }

    // random insert
    {
        uint64_map hm = uint64_map_new(16);
        uint64_t acc = 0;
        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            acc += uint64_map_add(hm, &rnd[i], &rnd[i]);
        t1 = now_sec();
        sink ^= acc;
        report("insert random", N, t1 - t0);
        uint64_map_free(hm);
    }

    // lookup hit (sequential keys, sequential access)
    {
        uint64_map hm = uint64_map_new(16);
        for (size_t i = 0; i < N; i++) 
            uint64_map_add(hm, &seq[i], &seq[i]);
        t0 = now_sec();
        for (size_t i = 0; i < N; i++) 
        {
            v = uint64_map_get(hm, &seq[i]);
            sink ^= *v;
        }
        t1 = now_sec();
        report("lookup hit sequential", N, t1 - t0);
        uint64_map_free(hm);
    }

    // lookup hit (random key order)
    {
        uint64_map hm = uint64_map_new(16);
        for (size_t i = 0; i < N; i++) 
            uint64_map_add(hm, &seq[i], &seq[i]);

        uint64_t* shuffled = malloc(N * sizeof(uint64_t));
        memcpy(shuffled, seq, N * sizeof(uint64_t));
        shuffle(shuffled, N);

        t0 = now_sec();
        for (size_t i = 0; i < N; i++) 
        {
            v = uint64_map_get(hm, &shuffled[i]);
            sink ^= *v;
        }
        t1 = now_sec();
        report("lookup hit shuffled", N, t1 - t0);
        free(shuffled);
        uint64_map_free(hm);
    }

    // lookup miss
    {
        uint64_map hm = uint64_map_new(16);
        for (size_t i = 0; i < N; i++) 
            uint64_map_add(hm, &seq[i], &seq[i]);

        t0 = now_sec();
        for (size_t i = 0; i < N; i++) 
        {
            v = uint64_map_get(hm, &miss[i]);
            sink ^= (uint64_t)(uintptr_t)v;
        }
        t1 = now_sec();
        report("lookup miss", N, t1 - t0);
        uint64_map_free(hm);
    }

    //remove
    {
        uint64_map hm = uint64_map_new(16);
        for (size_t i = 0; i < N; i++) 
            uint64_map_add(hm, &seq[i], &seq[i]);

        uint64_t acc = 0;
        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            acc += uint64_map_remove(hm, &seq[i]);
        t1 = now_sec();
        sink ^= acc;
        report("remove sequential", N, t1 - t0);
        uint64_map_free(hm);
    }

    // mixed: 80% lookup / 20% insert (pre-warmed)
    {
        uint64_map hm = uint64_map_new(16);
        size_t warm = N / 2;
        for (size_t i = 0; i < warm; i++) 
            uint64_map_add(hm, &seq[i], &seq[i]);

        t0 = now_sec();
        for (size_t i = 0; i < N; i++) 
        {
            if (i % 5 == 0) 
            {
                uint64_t k = seq[warm + i % warm];
                sink ^= uint64_map_add(hm, &k, &k);
            } else 
            {
                v = uint64_map_get(hm, &seq[i % warm]);
                sink ^= (uint64_t)(uintptr_t)v;
            }
        }
        t1 = now_sec();
        report("mixed 80% rd / 20% wr", N, t1 - t0);
        uint64_map_free(hm);
    }

    printf("\n(sink=%llu)\n", (unsigned long long)sink);
    free(seq);
    free(rnd);
    free(miss);
    return 0;
}
