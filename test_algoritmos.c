#include "algoritmos.h"
#include "dados.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const SortFn plain[] = {
    insertion_sort, selection_sort, merge_sort, heap_sort, quick_sort
};
static const SortFn counted[] = {
    insertion_sort_counted, selection_sort_counted, merge_sort_counted,
    heap_sort_counted, quick_sort_counted
};
static unsigned long cases_checked;

static void check_case(const int32_t *input, size_t n)
{
    int32_t reference[256], work[256], aux[256];
    assert(n <= 256);
    if (n > 0) memcpy(reference, input, n * sizeof *input);
    qsort(reference, n, sizeof *reference, compare_i32);
    for (size_t alg = 0; alg < 5; ++alg) {
        Counters untouched = {11, 12, 13, 14};
        if (n > 0) memcpy(work, input, n * sizeof *input);
        plain[alg](work, n, aux, &untouched);
        assert(memcmp(work, reference, n * sizeof *work) == 0);
        assert(untouched.comparisons == 11 && untouched.swaps == 12);
        assert(untouched.writes_main == 13 && untouched.writes_aux == 14);
        if (n > 0) memcpy(work, input, n * sizeof *input);
        Counters metrics = {0};
        counted[alg](work, n, aux, &metrics);
        assert(memcmp(work, reference, n * sizeof *work) == 0);
        if (alg == 1)
            assert(metrics.comparisons == (uint64_t)n * (n > 0 ? n - 1 : 0) / 2);
    }
    ++cases_checked;
}

static void permute(int32_t *a, size_t start, size_t n)
{
    if (start == n) { check_case(a, n); return; }
    for (size_t i = start; i < n; ++i) {
        int32_t tmp = a[start]; a[start] = a[i]; a[i] = tmp;
        permute(a, start + 1, n);
        tmp = a[start]; a[start] = a[i]; a[i] = tmp;
    }
}

static void check_known_counts(void)
{
    /* Contagens obtidas manualmente para [3, 1, 2]. */
    const Counters expected[] = {
        {3, 0, 4, 0}, {3, 2, 4, 0}, {3, 0, 5, 5},
        {3, 2, 4, 0}, {2, 2, 4, 0}
    };
    for (size_t alg = 0; alg < 5; ++alg) {
        int32_t a[] = {3, 1, 2}, aux[3];
        Counters m = {0};
        counted[alg](a, 3, aux, &m);
        assert(m.comparisons == expected[alg].comparisons);
        assert(m.swaps == expected[alg].swaps);
        assert(m.writes_main == expected[alg].writes_main);
        assert(m.writes_aux == expected[alg].writes_aux);
    }
    for (size_t n = 2; n <= 128; ++n) {
        int32_t a[128], aux[128];
        Counters m = {0};
        generate_input(a, n, ASCENDING, 0);
        insertion_sort_counted(a, n, aux, &m);
        assert(m.comparisons == n - 1 && m.writes_main == n - 1);
        generate_input(a, n, DESCENDING, 0);
        m = (Counters){0};
        insertion_sort_counted(a, n, aux, &m);
        const uint64_t pairs = (uint64_t)n * (n - 1) / 2;
        assert(m.comparisons == pairs && m.writes_main == pairs + n - 1);
        for (int sc = ASCENDING; sc <= DESCENDING; ++sc) {
            generate_input(a, n, sc, 0);
            m = (Counters){0};
            quick_sort_counted(a, n, aux, &m);
            assert(m.comparisons == pairs);
        }
        generate_input(a, n, ALL_EQUAL, 0);
        m = (Counters){0};
        quick_sort_counted(a, n, aux, &m);
        assert(m.comparisons == pairs && m.swaps == 0);
    }
    for (size_t n = 2, levels = 1; n <= 128; n *= 2, ++levels) {
        int32_t a[128], aux[128];
        generate_input(a, n, ASCENDING, 0);
        Counters m = {0};
        merge_sort_counted(a, n, aux, &m);
        assert(m.writes_main == n * levels && m.writes_aux == n * levels);
        assert(m.comparisons == (n / 2) * levels);
    }
}

int main(void)
{
    for (size_t alg = 0; alg < 5; ++alg) {
        Counters m = {0};
        plain[alg](NULL, 0, NULL, NULL);
        counted[alg](NULL, 0, NULL, &m);
        assert(m.comparisons == 0 && m.swaps == 0);
    }
    const int32_t edge[] = {INT32_MAX, INT32_MIN, 0, -1, 1, INT32_MIN, INT32_MAX};
    check_case(edge, sizeof edge / sizeof *edge);
    int32_t values[256];
    for (size_t n = 0; n <= 8; ++n) {
        for (size_t i = 0; i < n; ++i) values[i] = (int32_t)i;
        permute(values, 0, n);
    }
    const size_t lengths[] = {9, 10, 15, 16, 17, 31, 32, 33, 127, 128, 129, 255, 256};
    for (size_t k = 0; k < sizeof lengths / sizeof *lengths; ++k)
        for (int sc = 0; sc < SCENARIO_COUNT; ++sc)
            for (uint64_t seed = 0; seed < 20; ++seed) {
                generate_input(values, lengths[k], sc, seed);
                check_case(values, lengths[k]);
            }
    check_known_counts();
    printf("OK: %lu entradas, cinco algoritmos nas duas versoes; contagens conferidas.\n",
           cases_checked);
    return EXIT_SUCCESS;
}
