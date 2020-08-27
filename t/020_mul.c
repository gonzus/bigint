#include <stdio.h>
#include <string.h>
#include "bigint.h"

#define OK(value, expected) \
    do { \
        printf("%-3s [%s] == [%s]\n", \
               strcmp(value, expected) == 0 ? "OK" : "NOK", \
               value, expected); \
    } while (0)

static int test_mul(void)
{
    char buf[1000];
    bigint a;
    bigint b;

    bigint_init(&a);
    bigint_init(&b);

    static struct {
        const char* a;
        const char* b;
        const char* r;
    } data[] = {
        // really basic shit
        {       "+0",       "+0",                "0" },
        {       "+0",       "-0",                "0" },
        {       "-0",       "+0",                "0" },
        {       "-0",       "-0",                "0" },
        {        "1",        "1",                "1" },
        {        "1",       "-1",               "-1" },
        {       "-1",        "1",               "-1" },
        {       "-1",       "-1",                "1" },
        {       "11",        "1",               "11" },
        {       "11",       "-1",              "-11" },
        // small shit
        {        "9",        "9",               "81" },
        {       "99",       "99",             "9801" },
        {     "9999",     "9999",         "99980001" },
        { "99999999", "99999999", "9999999800000001" },
        {       "45",       "38",             "1710" },
        {       "11",       "34",              "374" },
        {    "76324",     "1234",         "94183816" },
        // large shit
        {
           "348972942347234239482734",
           "12347236423498237492384723",
           "4308851424565121118759122422335541763779643872682",
        },
    };
    int count = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < count; ++j) {
        bigint_assign_string(&a, data[j].a, 10);
        bigint_assign_string(&b, data[j].b, 10);
        bigint_mul_bigint(&a, &b);
        OK(bigint_format(&a, buf), data[j].r);
    }

    bigint_fini(&b);
    bigint_fini(&a);
    return count;
}

static int test_mul_integer(void)
{
    char buf[1000];
    bigint a;

    bigint_init(&a);

    static struct {
        const char* a;
        long b;
        const char* r;
    } data[] = {
        // really basic shit
        {       "+0",       +0,                "0" },
        {       "+0",       -0,                "0" },
        {       "-0",       +0,                "0" },
        {       "-0",       -0,                "0" },
        {        "1",        1,                "1" },
        {        "1",       -1,               "-1" },
        {       "-1",        1,               "-1" },
        {       "-1",       -1,                "1" },
        {       "11",        1,               "11" },
        {       "11",       -1,              "-11" },
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
    for (int j = 0; j < count; ++j) {
        bigint_assign_string(&a, data[j].a, 10);
        long b = data[j].b;
        bigint_mul_integer(&a, b);
        OK(bigint_format(&a, buf), data[j].r);
    }

    bigint_fini(&a);
    return count;
}

int main(int argc, char* argv[])
{
    test_mul();
    test_mul_integer();

    return 0;
}
