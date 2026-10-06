#include "algoritmos.h"
#include "dados.h"
#include "tempo.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { ALGORITHM_COUNT = 5, MAX_SIZES = 64, MAX_REPEATS = 100 };
typedef struct {
    const char *name;
    SortFn plain;
    SortFn counted;
} Algorithm;

static const Algorithm algorithms[ALGORITHM_COUNT] = {
    {"insertion", insertion_sort, insertion_sort_counted},
    {"selection", selection_sort, selection_sort_counted},
    {"merge", merge_sort, merge_sort_counted},
    {"heap", heap_sort, heap_sort_counted},
    {"quick", quick_sort, quick_sort_counted}
};

typedef struct {
    size_t sizes[MAX_SIZES];
    size_t size_count;
    size_t samples;
    size_t repeats;
    size_t warmups;
    uint64_t seed;
    bool enabled_algorithms[ALGORITHM_COUNT];
    bool enabled_scenarios[SCENARIO_COUNT];
    const char *prefix;
    bool allow_slow;
} Options;

static void fail(const char *message)
{
    fprintf(stderr, "Erro: %s\n", message);
    exit(EXIT_FAILURE);
}

static void usage(const char *program)
{
    printf("Uso: %s [opcoes]\n"
           "  --sizes 1000,2000,4000,8000,16000\n"
           "  --samples 30           amostras/rodadas por tamanho e cenario\n"
           "  --repeats 3            cronometragens da MESMA entrada\n"
           "  --warmups 1            aquecimentos por algoritmo/tamanho/cenario\n"
           "  --seed 20260918         semente mestra decimal (uint64_t)\n"
           "  --algorithms insertion,selection,merge,heap,quick\n"
           "  --scenarios aleatorio,crescente,decrescente,quase_ordenado,poucos_valores,iguais\n"
           "  --out ensaio           cria ensaio_raw.csv e ensaio_meta.txt\n"
           "  --allow-slow           permite casos quadraticos com n > 32768\n"
           "  --help\n"
           "Padrao: cinco algoritmos, apenas cenario aleatorio.\n"
           "Nao sobrescreve resultados. O diretorio de --out deve existir.\n",
           program);
}

static uint64_t parse_u64(const char *text)
{
    if (*text == '\0') fail("valor numerico vazio");
    for (const char *p = text; *p != '\0'; ++p)
        if (*p < '0' || *p > '9') fail("use numeros inteiros decimais sem sinal");
    errno = 0;
    char *end = NULL;
    const unsigned long long value = strtoull(text, &end, 10);
    if (errno == ERANGE || *end != '\0' || value > UINT64_MAX)
        fail("inteiro fora do intervalo uint64_t");
    return (uint64_t)value;
}

static size_t bounded_size(const char *text, size_t minimum, size_t maximum)
{
    const uint64_t value = parse_u64(text);
    if (value < minimum || value > maximum) fail("parametro numerico fora do intervalo");
    return (size_t)value;
}

/* Faz uma copia mutavel da lista, preservando e rejeitando campos vazios. */
static char *copy_text(const char *text)
{
    char *copy = malloc(strlen(text) + 1);
    if (copy == NULL) fail("memoria insuficiente");
    strcpy(copy, text);
    return copy;
}

static void parse_list(Options *o, const char *text, int kind)
{
    char *copy = copy_text(text);
    if (kind == 0) o->size_count = 0;
    if (kind == 1) memset(o->enabled_algorithms, 0, sizeof o->enabled_algorithms);
    if (kind == 2) memset(o->enabled_scenarios, 0, sizeof o->enabled_scenarios);
    char *item = copy;
    for (;;) {
        char *comma = strchr(item, ',');
        if (comma != NULL) *comma = '\0';
        if (*item == '\0') fail("lista vazia ou virgulas consecutivas");
        if (kind == 0) {
            if (o->size_count == MAX_SIZES) fail("maximo de 64 tamanhos");
            const size_t n = bounded_size(item, 2, 10000000);
            for (size_t i = 0; i < o->size_count; ++i)
                if (o->sizes[i] == n) fail("tamanho repetido");
            o->sizes[o->size_count++] = n;
        } else {
            bool found = false;
            const int count = kind == 1 ? ALGORITHM_COUNT : SCENARIO_COUNT;
            for (int i = 0; i < count; ++i) {
                const char *name = kind == 1 ? algorithms[i].name : scenario_names[i];
                bool *enabled = kind == 1 ? o->enabled_algorithms : o->enabled_scenarios;
                if (strcmp(item, name) == 0) {
                    if (enabled[i]) fail("nome repetido na lista");
                    enabled[i] = true;
                    found = true;
                    break;
                }
            }
            if (!found) fail("algoritmo ou cenario desconhecido; consulte --help");
        }
        if (comma == NULL) break;
        item = comma + 1;
    }
    free(copy);
}

static Options parse_options(int argc, char **argv)
{
    Options o = {
        .sizes = {1000, 2000, 4000, 8000, 16000}, .size_count = 5,
        .samples = 30, .repeats = 3, .warmups = 1,
        .seed = UINT64_C(20260918),
        .enabled_algorithms = {true, true, true, true, true},
        .enabled_scenarios = {true, false, false, false, false, false},
        .prefix = "ensaio", .allow_slow = false
    };
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (strcmp(arg, "--help") == 0) { usage(argv[0]); exit(EXIT_SUCCESS); }
        if (strcmp(arg, "--allow-slow") == 0) { o.allow_slow = true; continue; }
        if (i + 1 == argc) fail("opcao sem valor; consulte --help");
        const char *value = argv[++i];
        if (strcmp(arg, "--sizes") == 0) parse_list(&o, value, 0);
        else if (strcmp(arg, "--algorithms") == 0) parse_list(&o, value, 1);
        else if (strcmp(arg, "--scenarios") == 0) parse_list(&o, value, 2);
        else if (strcmp(arg, "--samples") == 0) o.samples = bounded_size(value, 2, 10000);
        else if (strcmp(arg, "--repeats") == 0) o.repeats = bounded_size(value, 1, MAX_REPEATS);
        else if (strcmp(arg, "--warmups") == 0) o.warmups = bounded_size(value, 0, 20);
        else if (strcmp(arg, "--seed") == 0) o.seed = parse_u64(value);
        else if (strcmp(arg, "--out") == 0) {
            if (*value == '\0') fail("prefixo vazio");
            o.prefix = value;
        } else fail("opcao desconhecida; consulte --help");
    }
    for (size_t k = 0; k < o.size_count; ++k) {
        if (o.sizes[k] > (size_t)INT32_MAX || o.sizes[k] > SIZE_MAX / sizeof(int32_t))
            fail("tamanho nao representavel nesta plataforma");
        bool quadratic = o.enabled_algorithms[0] || o.enabled_algorithms[1];
        for (int sc = 1; sc < SCENARIO_COUNT; ++sc)
            quadratic = quadratic || (o.enabled_algorithms[4] && o.enabled_scenarios[sc]);
        if (!o.allow_slow && quadratic && o.sizes[k] > 32768)
            fail("n > 32768 com casos potencialmente quadraticos; reduza n ou use --allow-slow");
    }
    return o;
}

static void check_sorted(const int32_t *work, const int32_t *reference,
                         size_t n, const char *algorithm)
{
    if (memcmp(work, reference, n * sizeof *work) != 0) {
        fprintf(stderr, "Ordenacao incorreta: %s, n=%zu\n", algorithm, n);
        exit(EXIT_FAILURE);
    }
}

static int compare_double(const void *a, const void *b)
{
    const double x = *(const double *)a;
    const double y = *(const double *)b;
    return (x > y) - (x < y);
}

static double timer_pair_median(void)
{
    double values[1001];
    for (size_t i = 0; i < 1001; ++i) {
        const uint64_t begin = timer_now();
        const uint64_t end = timer_now();
        values[i] = timer_elapsed_ns(begin, end);
    }
    qsort(values, 1001, sizeof *values, compare_double);
    return values[500];
}

static void shuffle_order(int *order, size_t length, Rng *rng)
{
    for (size_t i = length; i > 1; --i) {
        const size_t j = (size_t)rng_bounded(rng, (uint64_t)i);
        const int tmp = order[i - 1];
        order[i - 1] = order[j];
        order[j] = tmp;
    }
}

static FILE *create_output(const char *prefix, const char *suffix)
{
    char path[4096];
    const int length = snprintf(path, sizeof path, "%s%s", prefix, suffix);
    if (length < 0 || (size_t)length >= sizeof path) fail("caminho muito longo");
    FILE *file = fopen(path, "wx");
    if (file == NULL) {
        perror(path);
        fail("arquivo ja existe, diretorio inexistente ou acesso negado; use outro --out");
    }
    return file;
}

static void write_metadata(FILE *meta, int argc, char **argv, const Options *o,
                           double pair_ns, double short_ns)
{
    const time_t now = time(NULL);
    const struct tm *utc = gmtime(&now);
    char date[64] = "indisponivel";
    if (utc != NULL) strftime(date, sizeof date, "%Y-%m-%dT%H:%M:%SZ", utc);
    fprintf(meta, "protocol=ordenacao-c23-v1\nstarted_utc=%s\ncompiler=%s\n"
            "stdc_version=%ld\nbuild=%s %s\nsizeof_int32_t=%zu\nsizeof_size_t=%zu\n",
            date, __VERSION__, (long)__STDC_VERSION__, __DATE__, __TIME__,
            sizeof(int32_t), sizeof(size_t));
#ifdef _WIN32
    fputs("platform=Windows\n", meta);
#else
    fputs("platform=POSIX\n", meta);
#endif
#ifdef __OPTIMIZE__
    fputs("optimization_enabled=yes\n", meta);
#else
    fputs("optimization_enabled=no\n", meta);
    fputs("AVISO: compilacao sem otimizacao; confira as flags.\n", stderr);
#endif
    fprintf(meta, "timer=%s\ntimer_resolution_ns=%.3f\ntimer_pair_median_ns=%.3f\n"
            "short_measurement_threshold_ns=%.3f\nmaster_seed=%" PRIu64 "\n"
            "samples=%zu\nrepeats=%zu\nwarmups_per_cell=%zu\n",
            timer_name(), timer_resolution_ns(), pair_ns, short_ns,
            o->seed, o->samples, o->repeats, o->warmups);
    fputs("quick_variant=Lomuto,last_pivot,less_equal,smaller_side_recursion\n"
          "merge_variant=top_down,no_sorted_shortcut,preallocated_aux\n"
          "time_scope=sort_call_only,no_counters,no_allocation,no_copy,no_validation\n"
          "counts_scope=one_separate_sort_of_same_input\n"
          "input_protocol=copy_before_every_sort;warm_cache_not_controlled_cold_cache\n"
          "statistics_unit=mean_of_technical_repeats_per_sample\n"
          "exact_build_flags=see_build_command_and_registro_ambiente\n"
          "argv:", meta);
    for (int i = 0; i < argc; ++i) fprintf(meta, "\n  [%d] %s", i, argv[i]);
    fputs("\n", meta);
    if (fflush(meta) != 0) fail("falha ao gravar metadados");
}

int main(int argc, char **argv)
{
    const Options o = parse_options(argc, argv);
    timer_init();
    const double pair_ns = timer_pair_median();
    const double base_ns = pair_ns > timer_resolution_ns() ? pair_ns : timer_resolution_ns();
    const double short_ns = 100.0 * base_ns;
    FILE *raw = create_output(o.prefix, "_raw.csv");
    FILE *meta = create_output(o.prefix, "_meta.txt");
    write_metadata(meta, argc, argv, &o, pair_ns, short_ns);
    fputs("algorithm,scenario,n,sample,repeat,seed_hex,input_hash,order,time_ns,"
          "comparisons,swaps,writes_main,writes_aux,validated,short_measurement\n", raw);

    int selected[ALGORITHM_COUNT];
    size_t algorithm_count = 0;
    for (int a = 0; a < ALGORITHM_COUNT; ++a)
        if (o.enabled_algorithms[a]) selected[algorithm_count++] = a;
    uint64_t rows = 0;
    uint64_t short_rows = 0;

    for (int sc = 0; sc < SCENARIO_COUNT; ++sc) {
        if (!o.enabled_scenarios[sc]) continue;
        for (size_t ni = 0; ni < o.size_count; ++ni) {
            const size_t n = o.sizes[ni];
            const size_t bytes = n * sizeof(int32_t);
            int32_t *base = malloc(bytes);
            int32_t *work = malloc(bytes);
            int32_t *reference = malloc(bytes);
            int32_t *aux = calloc(n, sizeof *aux);
            if (base == NULL || work == NULL || reference == NULL || aux == NULL)
                fail("memoria insuficiente para os quatro vetores");
            for (size_t sample = 1; sample <= o.samples; ++sample) {
                const uint64_t seed = sample_seed(o.seed, n, sc, sample);
                generate_input(base, n, sc, seed);
                const uint64_t hash = input_hash(base, n);
                memcpy(reference, base, bytes);
                qsort(reference, n, sizeof *reference, compare_i32);

                if (sample == 1) {
                    for (size_t k = 0; k < algorithm_count; ++k) {
                        const int a = selected[k];
                        for (size_t warm = 0; warm < o.warmups; ++warm) {
                            memcpy(work, base, bytes);
                            algorithms[a].plain(work, n, aux, NULL);
                            check_sorted(work, reference, n, algorithms[a].name);
                        }
                    }
                }

                double elapsed[ALGORITHM_COUNT][MAX_REPEATS];
                size_t positions[ALGORITHM_COUNT][MAX_REPEATS];
                Rng order_rng = {seed ^ UINT64_C(0xd1b54a32d192ed03)};
                for (size_t rep = 0; rep < o.repeats; ++rep) {
                    int order[ALGORITHM_COUNT];
                    memcpy(order, selected, algorithm_count * sizeof *order);
                    shuffle_order(order, algorithm_count, &order_rng);
                    for (size_t position = 0; position < algorithm_count; ++position) {
                        const int a = order[position];
                        memcpy(work, base, bytes); /* Fora do intervalo medido. */
                        const uint64_t begin = timer_now();
                        algorithms[a].plain(work, n, aux, NULL);
                        const uint64_t end = timer_now();
                        if (end < begin) fail("relogio nao monotono");
                        elapsed[a][rep] = timer_elapsed_ns(begin, end);
                        positions[a][rep] = position + 1;
                        /* Usa a saida de toda ordenacao: impede descarte do trabalho
                         * e verifica ordem E preservacao exata dos elementos.
                         */
                        check_sorted(work, reference, n, algorithms[a].name);
                    }
                }

                /* Contagens depois de TODAS as cronometragens desta amostra. */
                for (size_t k = 0; k < algorithm_count; ++k) {
                    const int a = selected[k];
                    Counters counters = {0};
                    memcpy(work, base, bytes);
                    algorithms[a].counted(work, n, aux, &counters);
                    check_sorted(work, reference, n, algorithms[a].name);
                    for (size_t rep = 0; rep < o.repeats; ++rep) {
                        const bool is_short = elapsed[a][rep] < short_ns;
                        fprintf(raw, "%s,%s,%zu,%zu,%zu,%016" PRIx64 ",%016" PRIx64
                                ",%zu,%.3f,%" PRIu64 ",%" PRIu64 ",%" PRIu64
                                ",%" PRIu64 ",1,%d\n",
                                algorithms[a].name, scenario_names[sc], n, sample, rep + 1,
                                seed, hash, positions[a][rep], elapsed[a][rep],
                                counters.comparisons, counters.swaps,
                                counters.writes_main, counters.writes_aux, (int)is_short);
                        ++rows;
                        if (is_short) ++short_rows;
                    }
                }
                if (fflush(raw) != 0) fail("falha ao gravar CSV; confira espaco em disco");
            }
            free(aux);
            free(reference);
            free(work);
            free(base);
            fprintf(stderr, "%s n=%zu concluido (%zu amostras x %zu tempos).\n",
                    scenario_names[sc], n, o.samples, o.repeats);
        }
    }
    fprintf(meta, "completed=yes\nrows=%" PRIu64 "\nshort_measurements=%" PRIu64 "\n",
            rows, short_rows);
    if (fclose(raw) != 0) fail("falha ao fechar CSV");
    if (fclose(meta) != 0) fail("falha ao fechar metadados");
    fprintf(stderr, "Salvos %s_raw.csv e %s_meta.txt (%" PRIu64 " linhas).\n",
            o.prefix, o.prefix, rows);
    if (short_rows > 0)
        fprintf(stderr, "AVISO: %" PRIu64 " tempos abaixo de %.0f ns; consulte o guia.\n",
                short_rows, short_ns);
    return EXIT_SUCCESS;
}
