#include <stdio.h>
#include "timer.h"
#include "bigint.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_assign_int(void) {
    static unsigned long long data[] = {
        0,
        1,
        9,
        10,
        11,
        12,
        19710426,
        4367125871342043,
    };

    bigint* b = bigint_create();
    bigint* e = bigint_create();
    Timer t;
    for (int j = 0; j < ALEN(data); ++j) {
        unsigned long long v = data[j];
        timer_start(&t);
        bigint_assign_integer(b, v);
        timer_stop(&t);

        char buf[1000];
        sprintf(buf, "%llu", data[j]);
        bigint_assign_string(e, buf);

        int ok = bigint_compare(b, e) == 0;
        printf("%-3s [%llu] -- ", ok ? "OK" : "XX", v);
        timer_format_elapsed(&t, stdout, 1);

        while (1) {
            timer_start(&t);
            unsigned m = bigint_mod_integer(b, 10);
            timer_stop(&t);
            unsigned d = v % 10;
            printf("%s MOD %u %u %llu -- ", m == d ? "OK" : "XX", m, d, v);
            timer_format_elapsed(&t, stdout, 1);
            if (!v) {
                break;
            }
            v /= 10;
            bigint_assign_integer(b, v);
        }
    }
    bigint_destroy(b);
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    test_assign_int();

    return 0;
}
