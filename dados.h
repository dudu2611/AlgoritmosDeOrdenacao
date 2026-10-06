#ifndef DADOS_H
#define DADOS_H

#include <stddef.h>
#include <stdint.h>

typedef struct { uint64_t state; } Rng;
enum { RANDOM, ASCENDING, DESCENDING, NEARLY_SORTED, FEW_UNIQUE, ALL_EQUAL,
       SCENARIO_COUNT };
extern const char *const scenario_names[SCENARIO_COUNT];
uint64_t rng_next(Rng *rng);
uint64_t rng_bounded(Rng *rng, uint64_t bound);
uint64_t sample_seed(uint64_t master, size_t n, int scenario, size_t sample);
void generate_input(int32_t *a, size_t n, int scenario, uint64_t seed);
uint64_t input_hash(const int32_t *a, size_t n);
int compare_i32(const void *left, const void *right);

#endif
