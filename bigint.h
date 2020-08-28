#ifndef BIGINT_H_
#define BIGINT_H_

/*
 * A bare-bones implementation of bigints -- integers without a size limit.
 *
 *
 * TODO
 *
 * Add support for negative numbers.
 *
 * Add support for subtraction.
 *
 * Implement Karatsuba multiplication algorithm -- https://en.wikipedia.org/wiki/Karatsuba_algorithm
 *
 * Add support for integer division.
 */
#include <stdint.h>
#include <stdio.h>

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
 * A structure to represent an infinite precision positive integer.
 *
 * I = l[0] * b^0 + l[1] * b^1 + l[2] * b^2 + ... + l[n] * b^n
 *
 * The value of b depends on the number of bits chosen with
 * constant BIGINT_LIMB_BITS.
 */
typedef struct bigint {
    uint32_t cap;
    uint32_t pos;
    bigint_limb_t* lmb;
} bigint;

// Create an empty (zero) bigint
bigint* bigint_create(void);

// Clone an existing bigint into a new one
bigint* bigint_clone(bigint* b);

// Destroy a bigint
void bigint_destroy(bigint* b);

// Clear a bigint, leaving its value as zero.
// It does not get rid of any allocated memory, so it is cheap to reuse the
// bigint.
bigint* bigint_clear(bigint* b);

// Return true if the value of b is zero.
int bigint_is_zero(const bigint* b);

// Return true if the value of b is one.
int bigint_is_one(const bigint* b);

// Compare two bigints a and b, returning:
// a < b: -1   a == b: 0   a > b: +1
int bigint_compare(const bigint* a, const bigint* b);

// Assign a numeric value to bigint b
bigint* bigint_assign_integer(bigint* b, unsigned long long value);

// Assign the base-10 value stated in a string to bigint b
bigint* bigint_assign_string(bigint* b, const char* value);

// Assign the value of bigint n to bigint b
bigint* bigint_assign_bigint(bigint* b, const bigint* n);

// Format bigint b as a base-10 number into a string buffer
char* bigint_format(const bigint* b, char* buf);

// Print bigint b as a base-10 number into a FILE stream.
// Optional prepending message and newline.
void bigint_print(const char* msg, const bigint* b, FILE* stream, int newline);

// Compute b += n
// FIXME: maybe change this interface? Unwieldy?
bigint* bigint_addeq(bigint* b, const bigint* n);

// Compute b *= n
// FIXME: maybe change this interface? Unwieldy?
bigint* bigint_muleq(bigint* b, const bigint* n);

// Compute b % value
bigint_limb_t bigint_mod_integer(bigint* b, bigint_limb_t value);

// Compute n! and store the value in bigint b.
void bigint_factorial(bigint_limb_t n, bigint* b);

#endif
