#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "bigint.h"

static int test_sub_integer(void)
{
    bigint a;

    bigint_init(&a);

    static struct {
        const char* a;
        long b;
        const char* r;
    } data[] = {
        // really basic shit
        {    "+0",    +0,      "0" },
        {    "+0",    -0,      "0" },
        {    "-0",    +0,      "0" },
        {    "-0",    -0,      "0" },
        {     "0",     1,     "-1" },
        {     "1",     0,      "1" },
        {     "1",     1,      "0" },
        {     "1",    -1,      "2" },
        // small shit
        { "67890", 12345,  "55545" },
        { "12345", 67890, "-55545" },
    };
    int count = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < count; ++j) {
        bigint_assign_string(&a, data[j].a, 10);
        long b = data[j].b;
        bigint_sub_integer(&a, b);
    }

    bigint_fini(&a);
    return count;
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;
    int count = 0;
    Timer t;
    timer_start(&t);
    for (int j = 0; j < 3000000; ++j) {
        count += test_sub_integer();
    }
    timer_stop(&t);
    fprintf(stderr, "Computed %d subtractions by integer in ", count);
    timer_format_elapsed(&t, stderr, 1);

    return 0;
}
