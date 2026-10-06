#ifndef TEMPO_H
#define TEMPO_H

#include <stdint.h>

void timer_init(void);
uint64_t timer_now(void);
double timer_elapsed_ns(uint64_t begin, uint64_t end);
double timer_resolution_ns(void);
const char *timer_name(void);

#endif
