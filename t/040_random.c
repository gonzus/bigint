#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "bigint.h"

static long get_random_integer(void) {
    int r = rand();
    int m = r % 10;
    r /= 10;
    long ri = r * ((m&0x1) ? 1 : -1);
    return ri;
}

static void test_random(int count) {
    enum { OP_ADD, OP_SUB, OP_MUL, OP_LAST };
    static const char* sop[OP_LAST] = { "+", "-", "*" };

    bigint* l = bigint_create();
    bigint* r = bigint_create();
    bigint* g = bigint_create();
    bigint* e = bigint_create();

    for (int j = 0; j < count; ++j) {
        long li = get_random_integer();
        long ri = get_random_integer();
        bigint_assign_integer(l, li);
        bigint_assign_integer(r, ri);
        for (int op = OP_ADD; op < OP_LAST; ++op) {
            long long ei = 0;
            switch (op) {
                case OP_ADD:
                    bigint_add(l, r, g);
                    ei = li + ri;
                    break;
                case OP_SUB:
                    bigint_sub(l, r, g);
                    ei = li - ri;
                    break;
                case OP_MUL:
                    bigint_mul(l, r, g);
                    ei = li * ri;
                    break;
            }
            bigint_assign_integer(e, ei);
            int ok = bigint_compare(g, e) == 0;
            printf("%-3s %ld %s %ld = %lld\n", ok ? "OK" : "XX", li, sop[op], ri, ei);
        }
    }

    bigint_destroy(e);
    bigint_destroy(g);
    bigint_destroy(r);
    bigint_destroy(l);
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    srand(time(NULL));
    test_random(1000000);

    return 0;
}
