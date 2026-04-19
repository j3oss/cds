#include "common.h"

U64_SET(HASHSET_DEFINE)

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
    printf("Hashset benchmark  (N = %llu)\n\n", (unsigned long long)N);

    uint64_t* seq  = make_sequential(N);
    uint64_t* rnd  = make_random(N, 0xdeadbeefcafe1234ULL);
    uint64_t* miss = make_random(N, 0x0123456789abcdefULL);

    double t0, t1;
    volatile uint64_t sink = 0;

    // sequential insert
    {
        uint64_set hs = uint64_set_new(16);
        uint64_t acc = 0;
        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            acc += uint64_set_add(hs, &seq[i]);
        t1 = now_sec();
        sink ^= acc;
        report("insert sequential", N, t1 - t0);
        uint64_set_free(hs);
    }

    // random insert
    {
        uint64_set hs = uint64_set_new(16);
        uint64_t acc = 0;
        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            acc += uint64_set_add(hs, &rnd[i]);
        t1 = now_sec();
        sink ^= acc;
        report("insert random", N, t1 - t0);
        uint64_set_free(hs);
    }

    // contains hit (sequential keys, sequential access)
    {
        uint64_set hs = uint64_set_new(16);
        for (size_t i = 0; i < N; i++)
            uint64_set_add(hs, &seq[i]);
        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            sink ^= uint64_set_contains(hs, &seq[i]);
        t1 = now_sec();
        report("contains hit sequential", N, t1 - t0);
        uint64_set_free(hs);
    }

    // contains hit (random key order)
    {
        uint64_set hs = uint64_set_new(16);
        for (size_t i = 0; i < N; i++)
            uint64_set_add(hs, &seq[i]);

        uint64_t* shuffled = malloc(N * sizeof(uint64_t));
        memcpy(shuffled, seq, N * sizeof(uint64_t));
        shuffle(shuffled, N);

        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            sink ^= uint64_set_contains(hs, &shuffled[i]);
        t1 = now_sec();
        report("contains hit shuffled", N, t1 - t0);
        free(shuffled);
        uint64_set_free(hs);
    }

    // contains miss
    {
        uint64_set hs = uint64_set_new(16);
        for (size_t i = 0; i < N; i++)
            uint64_set_add(hs, &seq[i]);

        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            sink ^= uint64_set_contains(hs, &miss[i]);
        t1 = now_sec();
        report("contains miss", N, t1 - t0);
        uint64_set_free(hs);
    }

    // remove
    {
        uint64_set hs = uint64_set_new(16);
        for (size_t i = 0; i < N; i++)
            uint64_set_add(hs, &seq[i]);

        uint64_t acc = 0;
        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
            acc += uint64_set_remove(hs, &seq[i]);
        t1 = now_sec();
        sink ^= acc;
        report("remove sequential", N, t1 - t0);
        uint64_set_free(hs);
    }

    // mixed: 80% contains / 20% insert (pre-warmed)
    {
        uint64_set hs = uint64_set_new(16);
        size_t warm = N / 2;
        for (size_t i = 0; i < warm; i++)
            uint64_set_add(hs, &seq[i]);

        t0 = now_sec();
        for (size_t i = 0; i < N; i++)
        {
            if (i % 5 == 0)
            {
                uint64_t k = seq[warm + i % warm];
                sink ^= uint64_set_add(hs, &k);
            } else
            {
                sink ^= uint64_set_contains(hs, &seq[i % warm]);
            }
        }
        t1 = now_sec();
        report("mixed 80% rd / 20% wr", N, t1 - t0);
        uint64_set_free(hs);
    }

    printf("\n(sink=%llu)\n", (unsigned long long)sink);
    free(seq);
    free(rnd);
    free(miss);
    return 0;
}
