#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "bigint.h"

static int test_mul_integer(void) {
    bigint* a = bigint_create();

    static struct {
        const char* a;
        long b;
        const char* r;
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
    int count = sizeof(data) / sizeof(data[0]);
    bigint* b = bigint_create();
    for (int j = 0; j < count; ++j) {
        bigint_assign_string(a, data[j].a);
        bigint_assign_integer(b, data[j].b);
        bigint_muleq(a, b);
    }
    bigint_destroy(b);

    bigint_destroy(a);
    return count;
}

int main(int argc, char* argv[])
{
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
