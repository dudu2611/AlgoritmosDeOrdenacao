#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "tempo.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static uint64_t frequency;

void timer_init(void)
{
    LARGE_INTEGER f;
    if (!QueryPerformanceFrequency(&f) || f.QuadPart <= 0) {
        fputs("QueryPerformanceFrequency falhou.\n", stderr);
        exit(EXIT_FAILURE);
    }
    frequency = (uint64_t)f.QuadPart;
}

uint64_t timer_now(void)
{
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter)) {
        fputs("QueryPerformanceCounter falhou.\n", stderr);
        exit(EXIT_FAILURE);
    }
    return (uint64_t)counter.QuadPart;
}

double timer_elapsed_ns(uint64_t begin, uint64_t end)
{
    return (double)(end - begin) * (1.0e9 / (double)frequency);
}

double timer_resolution_ns(void) { return 1.0e9 / (double)frequency; }
const char *timer_name(void) { return "QueryPerformanceCounter"; }

#else
#include <time.h>
static double resolution;

void timer_init(void)
{
    struct timespec value;
    if (clock_getres(CLOCK_MONOTONIC, &value) != 0) {
        perror("clock_getres");
        exit(EXIT_FAILURE);
    }
    resolution = (double)value.tv_sec * 1.0e9 + (double)value.tv_nsec;
}

uint64_t timer_now(void)
{
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (uint64_t)value.tv_sec * UINT64_C(1000000000)
           + (uint64_t)value.tv_nsec;
}

double timer_elapsed_ns(uint64_t begin, uint64_t end)
{
    return (double)(end - begin);
}

double timer_resolution_ns(void) { return resolution; }
const char *timer_name(void) { return "clock_gettime(CLOCK_MONOTONIC)"; }
#endif
