#include <stdio.h>
#include <string.h>
#include "bigint.h"

#define OK(value, expected) \
    do { \
        printf("%-3s [%s] == [%s]\n", \
               strcmp(value, expected) == 0 ? "OK" : "NOK", \
               value, expected); \
    } while (0)

static int test_sub(void)
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
        {    "+0",    "+0",      "0" },
        {    "+0",    "-0",      "0" },
        {    "-0",    "+0",      "0" },
        {    "-0",    "-0",      "0" },
        {     "0",     "1",     "-1" },
        {     "1",     "0",      "1" },
        {     "1",     "1",      "0" },
        {     "1",    "-1",      "2" },
        // small shit
        { "67890", "12345",  "55545" },
        { "12345", "67890", "-55545" },
        // generated with bc
/*
echo "r=$RANDOM ^ 3; n = r; for (j=0; j<10; ++j) { a = n; n *= 11; b = n; n *= 7; c = a - b; print \"\na \", a, \"\nb \", b, \"\nc \", c, \"\n\";}" | bc -q | sed 's+\n++g' | awk '$1 ~ /a/ {printf("        { @%s@,", $2);} $1 ~ /b/ {printf("@%s@,", $2);} $1 ~ /c/ {printf("@%s@ },\n", $2);}' | tr '@' '"'
*/
        {
            "1085106304576",
            "1177455692230582878539776",
            "-1177455692229497772235200",
        },
        {
          "1177455692231667984844352",
          "1277664594999481237515023573265354752",
          "-1277664594998303781822791905280510400",
        },
        {
          "6557053407813",
          "72127587485943",
          "-65570534078130",
        },
        {
          "504893112401601",
          "5553824236417611",
          "-5048931124016010",
        },
        {
          "38876769654923277",
          "427644466204156047",
          "-388767696549232770",
        },
        {
          "2993511263429092329",
          "32928623897720015619",
          "-29935112634290923290",
        },
        {
          "230500367284040109333",
          "2535504040124441202663",
          "-2305003672840401093330",
        },
        {
          "17748528280871088418641",
          "195233811089581972605051",
          "-177485282808710884186410",
        },
        {
          "1366636677627073808235357",
          "15033003453897811890588927",
          "-13666366776270738082353570",
        },
        {
          "105231024177284683234122489",
          "1157541265950131515575347379",
          "-1052310241772846832341224890",
        },
        {
          "8102788861650920609027431653",
          "89130677478160126699301748183",
          "-81027888616509206090274316530",
        },
        {
          "623914742347120886895112237281",
          "6863062165818329755846234610091",
          "-6239147423471208868951122372810",
        },
    };
    int count = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < count; ++j) {
        bigint_assign_string(&a, data[j].a, 10);
        bigint_assign_string(&b, data[j].b, 10);
        bigint_sub_bigint(&a, &b);
        OK(bigint_format(&a, buf), data[j].r);
    }

    bigint_fini(&b);
    bigint_fini(&a);
    return count;
}

static int test_sub_integer(void)
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
        OK(bigint_format(&a, buf), data[j].r);
    }

    bigint_fini(&a);
    return count;
}

int main(int argc, char* argv[])
{
    test_sub();
    test_sub_integer();

    return 0;
}
