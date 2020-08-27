#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bigint.h"

#if BIGINT_LIMB_BITS == 32
// #define BIGINT_LIMB_BASE (((bigint_larger_t)UINT32_MAX)+1)
#define BIGINT_LIMB_BASE (11)
#define BIGINT_LIMB_FMT PRIu32
#define BIGINT_LARGER_FMT PRIu64
#endif

#if BIGINT_LIMB_BITS == 16
#define BIGINT_LIMB_BASE (((bigint_larger_t)UINT16_MAX)+1)
#define BIGINT_LIMB_FMT PRIu16
#define BIGINT_LARGER_FMT PRIu32
#endif

#if BIGINT_LIMB_BITS == 8
#define BIGINT_LIMB_BASE (((bigint_larger_t)UINT8_MAX)+1)
#define BIGINT_LIMB_FMT PRIu8
#define BIGINT_LARGER_FMT PRIu16
#endif

static void show(const char* msg,
                 const bigint* b,
                 bigint_larger_t base,
                 bigint_larger_t value)
{
#if defined(DEBUG) && (DEBUG > 0)
    printf("%s base %"BIGINT_LARGER_FMT" size %u %c [",
           msg, base, b->pos, b->negative ? '-' : '+');
    for (int first = 1, j = b->pos - 1; j >= 0; first = 0, --j) {
        printf("%s%"BIGINT_LIMB_FMT"", first ? "" : ":", b->limbs[j]);
    }
    printf("] -- %"BIGINT_LARGER_FMT"\n", value);
#endif
}

static void check_size(bigint* b, int p)
{
    if (b->size > p) {
        return;
    }

    size_t size = b->size ? b->size : 1;
    while (size <= p) {
        size <<= 1;
    }
#if defined(DEBUG) && (DEBUG > 0)
    printf("GROW %p %zu -> %zu\n", b, b->size, size);
#endif
    b->limbs = realloc(b->limbs, size * sizeof(bigint_limb_t));
    b->size = size;
}

#define SET_DIGIT(b, p, value, base) \
    do { \
        check_size(b, p); \
        b->limbs[p] = value % base; \
        value /= base; \
    } while (0)

static void incNumberByValue(bigint* b,
                             bigint_larger_t base,
                             bigint_larger_t value)
{
    show(">inc", b, base, value);
    bigint_larger_t overflow = value;
    int p = 0;
    while (p < b->pos) {
        if (!overflow) {
            break;
        }

        bigint_larger_t tmp = b->limbs[p] + overflow;
        SET_DIGIT(b, p, tmp, base);
        ++p;
        overflow = tmp;
    }
    while (overflow) {
        SET_DIGIT(b, p, overflow, base);
        ++p;
    }
    if (b->pos < p) {
        b->pos = p;
    }
    show("<inc", b, base, value);
}

static void mulNumberByValue(bigint* b,
                             bigint_larger_t base,
                             bigint_larger_t value)
{
    show(">mul", b, base, value);
    bigint_larger_t overflow = 0;
    int p = 0;
    while (p < b->pos) {
        bigint_larger_t tmp = b->limbs[p] * value + overflow;
        SET_DIGIT(b, p, tmp, base);
        ++p;
        overflow = tmp;
    }
    while (overflow) {
        SET_DIGIT(b, p, overflow, base);
        ++p;
    }
    if (b->pos < p) {
        b->pos = p;
    }
    show("<mul", b, base, value);
}

static void convertToBase(const bigint* b,
                          bigint_limb_t base,
                          bigint* num)
{
#if defined(DEBUG) && (DEBUG > 0)
    printf("Will convert to base %"BIGINT_LIMB_FMT": [", base);
    for (int j = b->pos - 1; j >= 0; --j) {
        printf("%s%"BIGINT_LIMB_FMT"", j == b->pos - 1 ? "" : ":", b->limbs[j]);
    }
    printf("]\n");
#endif

    for (int j = b->pos - 1; j >= 0; --j) {
        mulNumberByValue(num, base, BIGINT_LIMB_BASE);
        incNumberByValue(num, base, b->limbs[j]);
    }
}

static int magnitudeCmpBigint(const bigint* a, const bigint* b)
{
    if (a->pos > b->pos)
        return +1;
    if (a->pos < b->pos)
        return -1;
    for (int j = a->pos - 1; j >= 0; --j) {
        if (a->limbs[j] > b->limbs[j])
            return +1;
        if (a->limbs[j] < b->limbs[j])
            return -1;
    }
    return 0;
}

static int magnitudeCmpInteger(const bigint* a, long b)
{
    if (b < 0){
        b = -b;
    }
    bigint_larger_t val = 0;
    bigint_larger_t mul = 1;
    for (int j = 0; j < a->pos; ++j) {
        val += mul * a->limbs[j];
        if (val > b) {
            return 1;
        }
        mul *= BIGINT_LIMB_BASE;
    }
    if (val < b) {
        return -1;
    }
    return 0;
}

bigint* bigint_init(bigint* b)
{
    b->size = b->pos = 0;
    b->negative = 0;
    b->limbs = 0;
#if defined(DEBUG) && (DEBUG > 0)
    printf("INIT %p\n", b);
#endif
    return b;
}

bigint* bigint_fini(bigint* b)
{
#if defined(DEBUG) && (DEBUG > 0)
    printf("FINI %p\n", b);
#endif
    free(b->limbs);
    b->limbs = 0;
    b->negative = 0;
    b->size = b->pos = 0;
    return b;
}

bigint* bigint_clear(bigint* b)
{
    b->pos = 0;
    b->negative = 0;
#if defined(DEBUG) && (DEBUG > 0)
    printf("CLEAR %p\n", b);
#endif
    return b;
}

bigint* bigint_pack(bigint* b)
{
    int p = b->pos - 1;
    while (p >= 0) {
        if (b->limbs[p] > 0) {
            break;
        }
        --p;
    }
    b->pos = p + 1;
    if (b->pos == 0) {
        b->negative = 0;
    }
    return b;
}

bigint* bigint_negate(bigint* b)
{
    if (b->pos > 0) {
        b->negative = 1 - b->negative;
    }
    return b;
}

int bigint_is_zero(const bigint* b)
{
    return b->pos == 0;
}

int bigint_is_one(const bigint* b, int sign)
{
    return (b->pos == 1 &&
            b->limbs[0] == 1 &&
            ((sign < 0 &&  b->negative) ||
             (sign > 0 && !b->negative) ||
             (sign == 0)));
}

int bigint_is_positive(const bigint* b)
{
    return (b->pos > 0 &&
            !b->negative);
}

int bigint_is_negative(const bigint* b)
{
    return (b->pos > 0 &&
            b->negative);
}

bigint* bigint_assign_integer(bigint* b,
                              long value)
{
#if defined(DEBUG) && (DEBUG > 0)
    printf("Assigning integer [%ld]\n", value);
#endif
    bigint_clear(b);
    if (value < 0) {
        b->negative = 1;
        value = -value;
    }
    while (value > 0) {
        SET_DIGIT(b, b->pos, value, BIGINT_LIMB_BASE);
        ++b->pos;
    }
    return b;
}

bigint* bigint_assign_string(bigint* b,
                             const char* value,
                             int base)
{
    assert(base >= 2 && base <= 36);
#if defined(DEBUG) && (DEBUG > 0)
    printf("Assigning string [%s] (base %u)\n", value, base);
#endif
    bigint_clear(b);
    for (int j = 0; value[j] != '\0'; ++j) {
        uint8_t digit = value[j];
        if (digit == '+') {
            b->negative = 0;
            continue;
        }
        if (digit == '-') {
            b->negative = 1;
            continue;
        }
        if (digit >= '0' && digit <= '9') {
            digit = digit - '0';
        } else if (digit >= 'a' && digit <= 'z') {
            digit = digit - 'a' + 10;
        } else if (digit >= 'A' && digit <= 'Z') {
            digit = digit - 'A' + 10;
        } else {
            digit = UINT8_MAX;
        }
        assert(digit != UINT8_MAX);

        mulNumberByValue(b, BIGINT_LIMB_BASE, base);
        incNumberByValue(b, BIGINT_LIMB_BASE, digit);
    }

    if (b->pos == 0) {
        b->negative = 0;
    }

    return b;
}

bigint* bigint_assign_bigint(bigint* b,
                             const bigint* n)
{
#if defined(DEBUG) && (DEBUG > 0)
    printf("Assigning bigint ");
    bigint_print(n, stdout, 1);
#endif

    bigint_clear(b);
    b->negative = n->negative;
    while (b->pos < n->pos) {
        bigint_larger_t tmp = n->limbs[b->pos];
        SET_DIGIT(b, b->pos, tmp, BIGINT_LIMB_BASE);
        ++b->pos;
    }

    return b;
}

char* bigint_format(const bigint* b, char* buf)
{
    int p = 0;
    bigint num;
    bigint_init(&num);

    convertToBase(b, 10, &num);
    if (b->negative) {
        buf[p++] = '-';
    }
    for (int j = num.pos - 1; j >= 0; --j) {
        buf[p++] = num.limbs[j] + '0';
    }
    if (num.pos == 0) {
        buf[p++] = '0';
    }
    buf[p] = '\0';

    bigint_fini(&num);
    return buf;
}

void bigint_print(const char* msg,
                  const bigint* b,
                  FILE* stream,
                  int newline)
{
    char buf[1000];
    bigint_format(b, buf);

    if (msg && msg[0]) {
        fprintf(stream, "%s", msg);
    }

    fprintf(stream, "%s", buf);

    if (newline) {
        fputc('\n', stream);
    }
}

int bigint_compare(const bigint* a, const bigint* b)
{
    if (!a->negative && b->negative) {
        return +1;
    }
    if ( a->negative && !b->negative) {
        return -1;
    }

    int greater = +1;
    if (a->negative) {
        greater = -1;
    }

    if (a->pos > b->pos)
        return +greater;
    if (a->pos < b->pos)
        return -greater;
    for (int j = a->pos - 1; j >= 0; --j) {
        if (a->limbs[j] > b->limbs[j])
            return +greater;
        if (a->limbs[j] < b->limbs[j])
            return -greater;
    }

    return 0;
}

/*
 * ===================
 * ADDITION ALGORITHMS
 * ===================
 */

/*
 * Add |n| to |b|; don't even look at signs.
 */
static bigint* bigint_addbi_absolute(bigint* b, const bigint* n)
{
    bigint_larger_t overflow = 0;
    int p = 0;
    while (p < b->pos || p < n->pos) {
        if (p < b->pos) {
            overflow += b->limbs[p];
        }
        if (p < n->pos) {
            overflow += n->limbs[p];
        }
        SET_DIGIT(b, p, overflow, BIGINT_LIMB_BASE);
        ++p;
    }
    while (overflow) {
        SET_DIGIT(b, p, overflow, BIGINT_LIMB_BASE);
        ++p;
    }
    if (b->pos < p) {
        b->pos = p;
    }

    return b;
}

/*
 * Subtract |n| from |b|; don't even look at signs.
 * Assume |n| < |b|
 */
static bigint* bigint_subbi_absolute(bigint* b, const bigint* n)
{
    bigint_larger_t borrow = 0;
    int p = 0;
    while (p < b->pos) {
        if (borrow == 0 && p >= n->pos) {
            break;
        }

        bigint_larger_t c = 0;
        if (p < b->pos) {
            c = b->limbs[p];
        }

        bigint_larger_t m = 0;
        if (p < n->pos) {
            m = n->limbs[p];
        }

        if (c >= (m + borrow)) {
            c -= borrow;
            borrow = 0;
        } else {
            c += BIGINT_LIMB_BASE - borrow;
            borrow = 1;
        }
        c -= m;
        SET_DIGIT(b, p, c, BIGINT_LIMB_BASE);
        ++p;
    }
    if (b->pos < p) {
        b->pos = p;
    }

    bigint_pack(b);
    return b;
}

bigint* bigint_add_bigint(bigint* b, const bigint* n)
{
    if (b->negative == n->negative) {
        // b and n have same sign; simply add them
        return bigint_addbi_absolute(b, n);
    }

    // b and n have opposite signs
    int m = magnitudeCmpBigint(b, n);
    if (m == 0) {
        // |b| = |n|
        // therefore, b must be set to 0
        bigint_clear(b);
        return b;
    }

    if (m > 0) {
        // |b| > |n|
        // we subtract |n| from b
        return bigint_subbi_absolute(b, n);
    }

    // |b| < |n|
    // we swap b and n, and then
    // we subtract |n| from b
    bigint t;
    bigint_init(&t);
    bigint_assign_bigint(&t, b);
    bigint_assign_bigint( b, n);
    bigint_subbi_absolute(b, &t);
    bigint_fini(&t);
    return b;
}


/*
 * Add |n| to |b|; don't even look at signs.
 */
static bigint* bigint_addint_absolute(bigint* b, long n)
{
    if (n < 0) {
        n = -n;
    }
    bigint_larger_t overflow = 0;
    int p = 0;
    while (p < b->pos || n > 0) {
        if (p < b->pos) {
            overflow += b->limbs[p];
        }
        if (n > 0) {
            overflow += n % BIGINT_LIMB_BASE;
            n /= BIGINT_LIMB_BASE;
        }
        SET_DIGIT(b, p, overflow, BIGINT_LIMB_BASE);
        ++p;
    }
    while (overflow) {
        SET_DIGIT(b, p, overflow, BIGINT_LIMB_BASE);
        ++p;
    }
    if (b->pos < p) {
        b->pos = p;
    }

    return b;
}

/*
 * Subtract |n| from |b|; don't even look at signs.
 * Assume |n| < |b|
 */
static bigint* bigint_subint_absolute(bigint* b, long n)
{
    if (n < 0) {
        n = -n;
    }
    bigint_larger_t borrow = 0;
    int p = 0;
    while (p < b->pos) {
        if (borrow == 0 && n <= 0) {
            break;
        }

        bigint_larger_t c = 0;
        if (p < b->pos) {
            c = b->limbs[p];
        }

        bigint_larger_t m = 0;
        if (n > 0) {
            m = n % BIGINT_LIMB_BASE;
            n /= BIGINT_LIMB_BASE;
        }

        if (c >= (m + borrow)) {
            c -= borrow;
            borrow = 0;
        } else {
            c += BIGINT_LIMB_BASE - borrow;
            borrow = 1;
        }
        c -= m;
        SET_DIGIT(b, p, c, BIGINT_LIMB_BASE);
        ++p;
    }
    if (b->pos < p) {
        b->pos = p;
    }

    bigint_pack(b);
    return b;
}

bigint* bigint_add_integer(bigint* b, long n)
{
    // We reimplement the algorithm for the special case
    // of a long; we get almost 50% increase in speed

    if (b->negative == (n < 0 ? 1 : 0)) {
        // b and n have same sign; simply add them
        return bigint_addint_absolute(b, n);
    }

    // b and n have opposite signs
    int m = magnitudeCmpInteger(b, n);
    if (m == 0) {
        // |b| = |n|
        // therefore, b must be set to 0
        bigint_clear(b);
        return b;
    }

    if (m > 0) {
        // |b| > |n|
        // we subtract |n| from b
        return bigint_subint_absolute(b, n);
    }

    // |b| < |n|
    // we swap b and n, and then
    // we subtract |n| from b
    bigint t;
    bigint_init(&t);
    bigint_assign_bigint(&t, b);
    bigint_assign_integer(b, n);
    bigint_subbi_absolute(b, &t);
    bigint_fini(&t);
    return b;
}

/*
 * ======================
 * SUBTRACTION ALGORITHMS
 * ======================
 */

bigint* bigint_sub_bigint(bigint* b, const bigint* n)
{
    // Yes, this is ugly and breaks const-correctness
    // It is also faster (around 5%) than creating a
    // temporary to hold -n for a while...
    bigint_negate((bigint*) n);
    bigint_add_bigint(b, n);
    bigint_negate((bigint*) n);
    return b;
}

bigint* bigint_sub_integer(bigint* b, long n)
{
    return bigint_add_integer(b, -n);
}

/*
 * =========================
 * MULTIPLICATION ALGORITHMS
 * =========================
 */

bigint* bigint_mul_bigint(bigint* b, const bigint* n)
{
#if 0
    /*
     * These optimizations are not necessary because the
     * loop below works in all these cases.
     * Should we keep them or remove them?
     * MEASURE THIS!!!
     */
    if (bigint_is_zero(b) ||
        bigint_is_zero(n)) {
        bigint_clear(b);
        return b;
    }

    if (bigint_is_one(n, +1)) {
        return b;
    }

    if (bigint_is_one(n, -1)) {
        bigint_negate(b);
        return b;
    }
#endif

    bigint t;
    bigint_init(&t);

    bigint_assign_bigint(&t, b);
    bigint_clear(b);
    b->negative = (t.negative != n->negative);
    for (int p = 0; p < t.pos; ++p) {
        bigint_larger_t total = 0;
        int q;
        for (q = 0; q < n->pos; ++q) {
            if (p+q < b->pos) {
                total += b->limbs[p+q];
            }
            total += (bigint_larger_t) t.limbs[p] * (bigint_larger_t) n->limbs[q];
            SET_DIGIT(b, p+q, total, BIGINT_LIMB_BASE);
        }
        for (; total; ++q) {
            SET_DIGIT(b, p+q, total, BIGINT_LIMB_BASE);
        }
        if (b->pos < p+q) {
            b->pos = p+q;
        }
    }
    bigint_pack(b);

    bigint_fini(&t);
    return b;
}

bigint* bigint_mul_integer(bigint* b, long n)
{
    // We reimplement the algorithm for the special case
    // of a long; we get almost 50% increase in speed

    bigint t;
    bigint_init(&t);

    bigint_assign_bigint(&t, b);
    bigint_clear(b);
    b->negative = (t.negative != (n < 0 ? 1 : 0));
    if (n < 0) {
        n = -n;
    }
    for (int p = 0; p < t.pos; ++p) {
        bigint_larger_t total = 0;
        int q;
        int m = n;
        for (q = 0; m > 0; ++q) {
            if (p+q < b->pos) {
                total += b->limbs[p+q];
            }
            total += (bigint_larger_t) t.limbs[p] * (bigint_larger_t) (m % BIGINT_LIMB_BASE);
            m /= BIGINT_LIMB_BASE;
            SET_DIGIT(b, p+q, total, BIGINT_LIMB_BASE);
        }
        for (; total; ++q) {
            SET_DIGIT(b, p+q, total, BIGINT_LIMB_BASE);
        }
        if (b->pos < p+q) {
            b->pos = p+q;
        }
    }
    bigint_pack(b);

    bigint_fini(&t);
    return b;
}

/*
 * ===================
 * DIVISION ALGORITHMS
 * ===================
 */

// TODO
