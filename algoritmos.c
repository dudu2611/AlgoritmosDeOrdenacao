#include "algoritmos.h"

/* Este mesmo arquivo e compilado duas vezes. Sem -DINSTRUMENTED, as
 * macros sao operacoes normais: nao existem incrementos nem testes de
 * ativacao dos contadores no caminho cronometrado. Com -DINSTRUMENTED,
 * os nomes recebem _counted e as operacoes sao contabilizadas.
 * Nao passe expressoes com efeitos colaterais a essas macros.
 */
#ifdef INSTRUMENTED
#define SORT(name) name##_counted
#define LT(x, y) (++m->comparisons, (x) < (y))
#define LE(x, y) (++m->comparisons, (x) <= (y))
#define PUT_MAIN(dst, value) (++m->writes_main, (dst) = (value))
#define PUT_AUX(dst, value) (++m->writes_aux, (dst) = (value))
#define COUNT_SWAP() (++m->swaps)
#else
#define SORT(name) name
#define LT(x, y) ((x) < (y))
#define LE(x, y) ((x) <= (y))
#define PUT_MAIN(dst, value) ((dst) = (value))
#define PUT_AUX(dst, value) ((dst) = (value))
#define COUNT_SWAP() ((void)0)
#endif

static void swap_at(int32_t *a, size_t i, size_t j, Counters *m)
{
    (void)m;
    if (i == j) return;  /* Autotrocas nao contam. Indices nao sao chaves. */
    const int32_t temp = a[i];
    PUT_MAIN(a[i], a[j]);
    PUT_MAIN(a[j], temp);
    COUNT_SWAP();
}

/* 1. Insertion Sort: desloca os maiores e insere a chave no prefixo. */
void SORT(insertion_sort)(int32_t *a, size_t n, int32_t *aux, Counters *m)
{
    (void)aux;
    (void)m;
    for (size_t i = 1; i < n; ++i) {
        const int32_t key = a[i];
        size_t j = i;
        while (j > 0 && LT(key, a[j - 1])) {
            PUT_MAIN(a[j], a[j - 1]);
            --j;
        }
        PUT_MAIN(a[j], key);
    }
}

/* 2. Selection Sort: seleciona o minimo do sufixo ainda nao ordenado. */
void SORT(selection_sort)(int32_t *a, size_t n, int32_t *aux, Counters *m)
{
    (void)aux;
    (void)m;
    for (size_t i = 0; i + 1 < n; ++i) {
        size_t minimum = i;
        for (size_t j = i + 1; j < n; ++j) {
            if (LT(a[j], a[minimum])) minimum = j;
        }
        swap_at(a, i, minimum, m);
    }
}

/* Intervalos semiabertos: [lo, hi). Sem atalho para partes ja ordenadas. */
static void merge_range(int32_t *a, int32_t *aux,
                        size_t lo, size_t hi, Counters *m)
{
    (void)m;
    if (hi - lo < 2) return;
    const size_t mid = lo + (hi - lo) / 2;
    merge_range(a, aux, lo, mid, m);
    merge_range(a, aux, mid, hi, m);

    size_t left = lo;
    size_t right = mid;
    size_t out = lo;
    while (left < mid && right < hi) {
        if (LE(a[left], a[right])) {
            PUT_AUX(aux[out], a[left]);
            ++left;
        } else {
            PUT_AUX(aux[out], a[right]);
            ++right;
        }
        ++out;
    }
    while (left < mid) {
        PUT_AUX(aux[out], a[left]);
        ++out;
        ++left;
    }
    while (right < hi) {
        PUT_AUX(aux[out], a[right]);
        ++out;
        ++right;
    }
    for (size_t k = lo; k < hi; ++k) PUT_MAIN(a[k], aux[k]);
}

/* 3. Merge Sort: uma area auxiliar prealocada serve a toda a recursao. */
void SORT(merge_sort)(int32_t *a, size_t n, int32_t *aux, Counters *m)
{
    merge_range(a, aux, 0, n, m);
}

/* Restaura o max-heap em [0, n), descendo a chave da raiz indicada. */
static void sift_down(int32_t *a, size_t root, size_t n, Counters *m)
{
    (void)m;
    while (root < n / 2) {
        size_t child = 2 * root + 1;
        if (child + 1 < n && LT(a[child], a[child + 1])) ++child;
        if (!LT(a[root], a[child])) break;
        swap_at(a, root, child, m);
        root = child;
    }
}

/* 4. Heap Sort: constroi o heap de baixo para cima e extrai o maximo. */
void SORT(heap_sort)(int32_t *a, size_t n, int32_t *aux, Counters *m)
{
    (void)aux;
    for (size_t i = n / 2; i > 0; --i) sift_down(a, i - 1, n, m);
    for (size_t end = n; end > 1; --end) {
        swap_at(a, 0, end - 1, m);
        sift_down(a, 0, end - 1, m);
    }
}

/* Lomuto: ultimo elemento como pivo; iguais ficam a esquerda (<=). */
static size_t partition(int32_t *a, size_t lo, size_t hi, Counters *m)
{
    (void)m;
    const int32_t pivot = a[hi - 1];
    size_t boundary = lo;
    for (size_t j = lo; j + 1 < hi; ++j) {
        if (LE(a[j], pivot)) {
            swap_at(a, boundary, j, m);
            ++boundary;
        }
    }
    swap_at(a, boundary, hi - 1, m);
    return boundary;
}

static void quick_range(int32_t *a, size_t lo, size_t hi, Counters *m)
{
    while (hi - lo > 1) {
        const size_t pivot = partition(a, lo, hi, m);
        /* Recursao so na parte menor; parte maior por iteracao.
         * Pilha O(log n) mesmo quando o TEMPO degenera para Theta(n^2).
         */
        if (pivot - lo < hi - (pivot + 1)) {
            quick_range(a, lo, pivot, m);
            lo = pivot + 1;
        } else {
            quick_range(a, pivot + 1, hi, m);
            hi = pivot;
        }
    }
}

/* 5. Quick Sort: sem pivo aleatorio, mediana de tres ou troca de algoritmo. */
void SORT(quick_sort)(int32_t *a, size_t n, int32_t *aux, Counters *m)
{
    (void)aux;
    quick_range(a, 0, n, m);
}
