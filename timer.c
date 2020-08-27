#include <time.h>
#include "timer.h"

void timer_start(Timer* t) {
    gettimeofday(&t->t0, NULL);
}

void timer_stop(Timer* t) {
    gettimeofday(&t->t1, NULL);
}

unsigned long timer_elapsed(Timer* t)
{
    return ((t->t1.tv_sec  - t->t0.tv_sec ) * 1000000 +
            (t->t1.tv_usec - t->t0.tv_usec));
}

void timer_format_elapsed(Timer* t, FILE* fp, int newline)
{
    static struct {
        unsigned long min;
        const char* name;
    } units[] = {
        { 24 * 60 * 60 * 1000000UL, "day" },
        {      60 * 60 * 1000000UL, "h"   },
        {           60 * 1000000UL, "min" },
        {                1000000UL, "s"   },
        {                   1000UL, "ms"  },
        {                      0UL, "us"  },  // default
    };
    unsigned long us = timer_elapsed(t);
    for (int j = 0; 1; ++j) {
        if (us > units[j].min) {
            float value = us / (float) units[j].min;
            fprintf(fp, "%.2f %s", value, units[j].name);
            break;
        }
    }

    if (newline) {
        fputc('\n', fp);
    }
}
