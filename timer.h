#ifndef TIMER_H_
#define TIMER_H_

#include <stdio.h>
#include <sys/time.h>

typedef struct Timer {
    struct timeval t0;
    struct timeval t1;
} Timer;

void timer_start(Timer* t);
void timer_stop(Timer* t);

// Return elapsed time in us
unsigned long timer_elapsed(Timer* t);

// Format elapsed time in the most readable units
void timer_format_elapsed(Timer* t, FILE* fp, int newline);

#endif
