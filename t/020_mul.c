#include <stdio.h>
#include "timer.h"
#include "bigint.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_mul(void) {
    static struct {
        const char* l;
        const char* r;
        const char* e;
    } data[] = {
        // really basic shit
        {        "0",        "0",                "0" },
        {        "1",        "1",                "1" },
        {       "+1",       "+1",               "+1" },
        {       "+1",       "-1",               "-1" },
        {       "-1",       "+1",               "-1" },
        {       "-1",       "-1",               "+1" },
        {       "11",        "1",               "11" },
        // small shit
        {        "9",        "9",               "81" },
        {       "99",       "99",             "9801" },
        {     "9999",     "9999",         "99980001" },
        { "99999999", "99999999", "9999999800000001" },
        {       "45",       "38",             "1710" },
        {       "11",       "34",              "374" },
        {    "76324",     "1234",         "94183816" },
        { "54760608",        "0",                "0" },
        // large shit
        {
           "348972942347234239482734",
           "12347236423498237492384723",
           "4308851424565121118759122422335541763779643872682",
        },
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
        bigint_mul(l, r, g);
        timer_stop(&t);

        bigint_assign_string(e, data[j].e);
        int ok = bigint_compare(g, e) == 0;
        printf("%-3s [%s] -- ", ok ? "OK" : "XX", data[j].e);
        timer_format_elapsed(&t, stdout, 1);
    }
    bigint_destroy(e);
    bigint_destroy(g);
    bigint_destroy(r);
    bigint_destroy(l);
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    test_mul();

    return 0;
}
