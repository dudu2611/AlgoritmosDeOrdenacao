#ifndef ALGORITMOS_H
#define ALGORITMOS_H

#include <stddef.h>
#include <stdint.h>

/* Operacoes logicas do codigo-fonte, nao instrucoes de CPU. */
typedef struct {
    uint64_t comparisons;
    uint64_t swaps;
    uint64_t writes_main;
    uint64_t writes_aux;
} Counters;

/* aux: vetor de n elementos, usado somente pelo Merge Sort.
 * Versao normal: m pode ser NULL, pois nao ha instrumentacao.
 * Versao _counted: m deve apontar para contadores inicialmente zerados.
 * Para n == 0, a e aux podem ser NULL. Nao ha alocacao nos algoritmos.
 */
typedef void (*SortFn)(int32_t *a, size_t n, int32_t *aux, Counters *m);

void insertion_sort(int32_t *a, size_t n, int32_t *aux, Counters *m);
void selection_sort(int32_t *a, size_t n, int32_t *aux, Counters *m);
void merge_sort(int32_t *a, size_t n, int32_t *aux, Counters *m);
void heap_sort(int32_t *a, size_t n, int32_t *aux, Counters *m);
void quick_sort(int32_t *a, size_t n, int32_t *aux, Counters *m);

void insertion_sort_counted(int32_t *a, size_t n, int32_t *aux, Counters *m);
void selection_sort_counted(int32_t *a, size_t n, int32_t *aux, Counters *m);
void merge_sort_counted(int32_t *a, size_t n, int32_t *aux, Counters *m);
void heap_sort_counted(int32_t *a, size_t n, int32_t *aux, Counters *m);
void quick_sort_counted(int32_t *a, size_t n, int32_t *aux, Counters *m);

#endif
