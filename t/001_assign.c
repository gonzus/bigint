#include <stdio.h>
#include <string.h>
#include "bigint.h"

#define OK(value, expected) \
    do { \
        printf("%-3s [%s] == [%s]\n", \
               strcmp(value, expected) == 0 ? "OK" : "NOK", \
               value, expected); \
    } while (0)

static int test_assign_int(void)
{
    char bufb[1000];
    char buft[1000];
    bigint b;

    bigint_init(&b);

    static int data[] = {
        0,
        1,
        11,
        19710426,
    };
    int count = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < count; ++j) {
        bigint_assign_integer(&b, + data[j]);
        sprintf(buft, "%d", data[j]);
        OK(bigint_format(&b, bufb), buft);

        bigint_assign_integer(&b, - data[j]);
        sprintf(buft, "%d", - data[j]);
        OK(bigint_format(&b, bufb), buft);
    }

    bigint_fini(&b);
    return count;
}

static int test_assign_string(void)
{
    char bufa[1000];
    char bufb[1000];
    bigint a;
    bigint b;

    bigint_init(&a);
    bigint_init(&b);

    static const char* data[] = {
        "0",
        "1",
        "11",
        "19710426",
        "19961111197104262002101920050730",
    };
    int count = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < count; ++j) {
        bigint_assign_string(&b, data[j], 10);
        OK(bigint_format(&b, bufb), data[j]);

        bigint_assign_bigint(&a, &b);
        OK(bigint_format(&a, bufa), bufb);
    }

    bigint_fini(&b);
    bigint_fini(&a);
    return count;
}

int main(int argc, char* argv[])
{
    test_assign_int();
    test_assign_string();

    return 0;
}