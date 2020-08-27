#ifndef BIGINT_H_
#define BIGINT_H_

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

// Choose the bits size for limbs here; hint: larger is better

// #define BIGINT_LIMB_BITS 8
// #define BIGINT_LIMB_BITS 16
#define BIGINT_LIMB_BITS 32

#if BIGINT_LIMB_BITS == 32
typedef uint32_t bigint_limb_t;
typedef uint64_t bigint_larger_t;
#endif

#if BIGINT_LIMB_BITS == 16
typedef uint16_t bigint_limb_t;
typedef uint32_t bigint_larger_t;
#endif

#if BIGINT_LIMB_BITS == 8
typedef uint8_t bigint_limb_t;
typedef uint16_t bigint_larger_t;
#endif

/*
 * A structure to represent an infinite precision integer.
 *
 * I = l[0] * b^0 + l[1] * b^1 + l[2] * b^2 + ... + l[n] * b^n
 *
 * The value of b depends on the number of bits chosen with
 * constant BIGINT_LIMB_BITS.
 */
typedef struct bigint
{
    bigint_limb_t* limbs;
    size_t size;
    int pos;
    uint8_t negative;
} bigint;

bigint* bigint_init(bigint* b);
bigint* bigint_fini(bigint* b);
bigint* bigint_clear(bigint* b);
bigint* bigint_pack(bigint* b);
bigint* bigint_negate(bigint* b);

int bigint_is_zero(const bigint* b);
int bigint_is_one(const bigint* b, int sign);
int bigint_is_neg_one(const bigint* b);
int bigint_is_positive(const bigint* b);
int bigint_is_negative(const bigint* b);

bigint* bigint_assign_integer(bigint* b,
                              long value);
bigint* bigint_assign_string(bigint* b,
                             const char* value,
                             int base);
bigint* bigint_assign_bigint(bigint* b,
                             const bigint* n);

char* bigint_format(const bigint* b, char* buf);
void bigint_print(const char* msg,
                  const bigint* b,
                  FILE* stream,
                  int newline);

int bigint_compare(const bigint* a, const bigint* b);

bigint* bigint_add_integer(bigint* b, long n);
bigint* bigint_add_bigint(bigint* b, const bigint* n);

bigint* bigint_sub_integer(bigint* b, long n);
bigint* bigint_sub_bigint(bigint* b, const bigint* n);

bigint* bigint_mul_bigint(bigint* b, const bigint* n);
bigint* bigint_mul_integer(bigint* b, long n);

#endif
