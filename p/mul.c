#include <stdio.h>
#include "timer.h"
#include "bigint.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static int test_mul_integer(void) {
    static struct {
        const char* l;
        long r;
        const char* e;
    } data[] = {
        // really basic shit
        {        "0",        0,                "0" },
        {        "1",        1,                "1" },
        {       "11",        1,               "11" },
        // small shit
        {        "9",        9,               "81" },
        {       "99",       99,             "9801" },
        {     "9999",     9999,         "99980001" },
        { "99999999", 99999999, "9999999800000001" },
        {       "45",       38,             "1710" },
        {       "11",       34,              "374" },
        {    "76324",     1234,         "94183816" },
    };

    bigint* l = bigint_create();
    bigint* r = bigint_create();
    bigint* g = bigint_create();
    bigint* e = bigint_create();
    int count = 0;
    for (int j = 0; j < ALEN(data); ++j) {
        bigint_assign_string(l, data[j].l);
        bigint_assign_integer(r, data[j].r);
        bigint_mul(l, r, g);

#if 0
        bigint_assign_string(e, data[j].e);
        int ok = bigint_compare(g, e) == 0;
        if (!ok) {
            continue;
        }
#endif
        ++count;
    }
    bigint_destroy(e);
    bigint_destroy(g);
    bigint_destroy(r);
    bigint_destroy(l);

    return count;
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    int count = 0;
    Timer t;
    timer_start(&t);
    for (int j = 0; j < 1000000; ++j) {
        count += test_mul_integer();
    }
    timer_stop(&t);
    fprintf(stderr, "Computed %d multiplications by integer in ", count);
    timer_format_elapsed(&t, stderr, 1);

    return 0;
}
