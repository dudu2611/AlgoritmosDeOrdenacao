#include "dados.h"

const char *const scenario_names[SCENARIO_COUNT] = {
    "aleatorio", "crescente", "decrescente", "quase_ordenado",
    "poucos_valores", "iguais"
};

/* SplitMix64: gerador deterministico, sem dependencia de rand()/libc.
 * A aritmetica unsigned usa modulo 2^64, como definido pela linguagem.
 */
uint64_t rng_next(Rng *rng)
{
    uint64_t z = (rng->state += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}

/* Rejeicao evita o vies de usar apenas rng_next() % bound. bound > 0. */
uint64_t rng_bounded(Rng *rng, uint64_t bound)
{
    const uint64_t threshold = (UINT64_C(0) - bound) % bound;
    uint64_t value;
    do { value = rng_next(rng); } while (value < threshold);
    return value % bound;
}

uint64_t sample_seed(uint64_t master, size_t n, int scenario, size_t sample)
{
    Rng rng = {master ^ UINT64_C(0xa0761d6478bd642f)};
    rng.state ^= (uint64_t)n * UINT64_C(0xe7037ed1a0b428db);
    rng.state ^= (uint64_t)(scenario + 1) * UINT64_C(0x8ebc6af09c88c6e3);
    rng.state ^= (uint64_t)sample * UINT64_C(0x589965cc75374cc3);
    return rng_next(&rng);
}

static void exchange(int32_t *a, size_t i, size_t j)
{
    const int32_t tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
}

void generate_input(int32_t *a, size_t n, int scenario, uint64_t seed)
{
    Rng rng = {seed};
    for (size_t i = 0; i < n; ++i) a[i] = (int32_t)i;
    switch (scenario) {
    case RANDOM:
        /* Fisher-Yates: permutacao dos valores distintos 0, ..., n-1. */
        for (size_t i = n; i > 1; --i)
            exchange(a, i - 1, (size_t)rng_bounded(&rng, (uint64_t)i));
        break;
    case DESCENDING:
        for (size_t i = 0; i < n; ++i) a[i] = (int32_t)(n - 1 - i);
        break;
    case NEARLY_SORTED:
        if (n > 1) {
            const size_t exchanges = n / 100 > 0 ? n / 100 : 1;
            for (size_t k = 0; k < exchanges; ++k) {
                const size_t i = (size_t)rng_bounded(&rng, (uint64_t)n);
                size_t j = (size_t)rng_bounded(&rng, (uint64_t)(n - 1));
                if (j >= i) ++j;
                exchange(a, i, j);
            }
        }
        break;
    case FEW_UNIQUE:
        for (size_t i = 0; i < n; ++i)
            a[i] = (int32_t)rng_bounded(&rng, UINT64_C(8));
        break;
    case ALL_EQUAL:
        for (size_t i = 0; i < n; ++i) a[i] = 7;
        break;
    case ASCENDING:
    default:
        break;
    }
}

/* FNV-1a com ordem de bytes explicita. Identificador, nao prova de igualdade. */
uint64_t input_hash(const int32_t *a, size_t n)
{
    uint64_t hash = UINT64_C(14695981039346656037);
    for (size_t i = 0; i < n; ++i) {
        const uint32_t value = (uint32_t)a[i];
        for (unsigned shift = 0; shift < 32; shift += 8) {
            hash ^= (value >> shift) & UINT32_C(255);
            hash *= UINT64_C(1099511628211);
        }
    }
    return hash;
}

int compare_i32(const void *left, const void *right)
{
    const int32_t a = *(const int32_t *)left;
    const int32_t b = *(const int32_t *)right;
    return (a > b) - (a < b); /* Evita overflow de a - b. */
}
