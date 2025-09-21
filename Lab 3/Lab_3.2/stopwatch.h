#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <stdint.h>

typedef struct stopwatch {
    uint8_t hh;
    uint8_t mm;
    uint8_t ss;
    uint8_t init_hh;
    uint8_t init_mm;
    uint8_t init_ss;
} stopwatch;

extern stopwatch global_stopwatch;

void create_stopwatch(void);
void initialize_stopwatch(uint8_t hh, uint8_t mm, uint8_t ss);
void start_stopwatch(void);
void stop_stopwatch(void);
void update_stopwatch(uint8_t hh, uint8_t mm, uint8_t ss);
void reset_stopwatch(void);

#endif
