#include <stdio.h>
#include "timer.h"
#include "bigint.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_sub(void) {
    static struct {
        const char* l;
        const char* r;
        const char* e;
    } data[] = {
        {     "+2",     "+1",     "+1" },
        {     "+2",     "-1",     "+3" },
        {     "-2",     "+1",     "-3" },
        {     "-2",     "-1",     "-1" },
        {     "+1",     "+2",     "-1" },
        {     "+1",     "-2",     "+3" },
        {     "-1",     "+2",     "-3" },
        {     "-1",     "-2",     "+1" },
        {    "100",      "0",    "100" },
        {    "100",      "1",     "99" },
        {    "100",     "99",      "1" },
        {    "100",    "100",      "0" },
        {    "100",    "101",     "-1" },
    };

    bigint* l = bigint_create();
    bigint* r = bigint_create();
    bigint* g = bigint_create();
    bigint* e = bigint_create();
    Timer t;
    for (int j = 0; j < ALEN(data); ++j) {
        bigint_assign_string(l, data[j].l);
        bigint_assign_string(r, data[j].r);
        timer_start(&t);
        bigint_sub(l, r, g);
        timer_stop(&t);

        bigint_assign_string(e, data[j].e);
        int ok = bigint_compare(g, e) == 0;
        printf("%-3s [%s] -- ", ok ? "OK" : "XX", data[j].e);
        timer_format_elapsed(&t, stdout, 1);
    }
    bigint_destroy(g);
    bigint_destroy(e);
    bigint_destroy(r);
    bigint_destroy(l);
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    test_sub();

    return 0;
}
