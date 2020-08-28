#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "bigint.h"

#define DEBUG 0
#define SHOW 0

#if BIGINT_LIMB_BITS == 32
#define BIGINT_LIMB_BASE (((bigint_larger_t)UINT32_MAX)+1)
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

#if 0
// set an arbitrary base, to prove it works with weird sizes
// NOTE: this has a HUGE negative impact in performance!!!
#ifdef BIGINT_LIMB_BASE
#undef BIGINT_LIMB_BASE
#endif
#define BIGINT_LIMB_BASE ((bigint_larger_t)17)
#endif

#define SET_DIGIT(b, p, value, base) \
    do { \
        if (b->cap <= p) { enlarge(b, p); } \
        b->lmb[p] = value % base; \
        value /= base; \
    } while (0)

static void enlarge(bigint* b, size_t p);
static void bigint_ass_base(bigint* b, unsigned long long value, bigint_larger_t base);
static void bigint_add_base(const bigint* b, const bigint* n, bigint* t, bigint_larger_t base);
static void bigint_mul_base(const bigint* b, const bigint* n, bigint* t, bigint_larger_t base);

#if defined(SHOW) && SHOW > 0
static void show(const char* msg, const bigint* b) {
    if (msg && msg[0] != '\0') {
        printf("%s | ", msg);
    }
    printf("size %u [", b->pos);
    for (int first = 1, j = b->pos - 1; j >= 0; first = 0, --j) {
        printf("%s%"BIGINT_LIMB_FMT, first ? "" : ":", b->lmb[j]);
    }
    printf("]\n");
    fflush(stdout);
    (void) msg;
    (void) b;
}
#endif

bigint* bigint_create(void) {
    bigint* b = (bigint*) malloc(sizeof(bigint));
    memset(b, 0, sizeof(bigint));
#if defined(DEBUG) && (DEBUG > 0)
    printf("CREATE %p\n", b);
#endif
    return b;
}

bigint* bigint_clone(bigint* o) {
    bigint* b = bigint_create();
    b->neg = o->neg;
    b->pos = o->pos;
    b->cap = o->cap;
    b->lmb = realloc(0, b->cap * sizeof(bigint_limb_t));
    for (uint64_t j = 0; j < o->pos; ++j) {
        b->lmb[j] = o->lmb[j];
    }
    memset(b->lmb + b->pos, 0, (b->cap - b->pos) * sizeof(bigint_limb_t));
    return b;
}

void bigint_destroy(bigint* b) {
#if defined(DEBUG) && (DEBUG > 0)
    printf("DESTROY %p\n", b);
#endif
    if (b->lmb) {
        free((void*) b->lmb);
        b->lmb = 0;
    }
    free((void*) b);
    b = 0;
}

bigint* bigint_clear(bigint* b) {
    memset(b->lmb, 0, b->pos * sizeof(bigint_limb_t));
    b->neg = 0;
    b->pos = 0;
#if defined(DEBUG) && (DEBUG > 0)
    printf("CLEAR %p\n", b);
#endif
    return b;
}

int bigint_is_zero(const bigint* b) {
    return b->pos == 0 || (b->pos == 1 && b->lmb[0] == 0);
}

int bigint_is_one(const bigint* b) {
    return b->pos == 1 && b->lmb[0] == 1 && !b->neg;
}

int bigint_compare(const bigint* a, const bigint* b) {
    if (!a->neg && b->neg) {
        // a > 0 and b < 0
        return +1;
    }
    if (a->neg && !b->neg) {
        // a < 0 and b > 0
        return -1;
    }

    // a and b have the same sign
    // cmp is -1 is both a and b are negative, +1 if both are positive
    int cmp = a->neg ? -1 : +1;
    if (a->pos > b->pos) {
        // a has more digits than b
        return +cmp;
    }
    if (a->pos < b->pos) {
        // a has fewer digits than b
        return -cmp;
    }

    // a and b have the same amount of digits
    for (int j = a->pos - 1; j >= 0; --j) {
        if (a->lmb[j] > b->lmb[j]) {
            // a's digit > b's digit
            return +cmp;
        }
        if (a->lmb[j] < b->lmb[j]) {
            // a's digit < b's digit
            return -cmp;
        }
    }

    // they are the same!
    return 0;
}

bigint* bigint_assign_integer(bigint* b, long long value) {
#if defined(DEBUG) && (DEBUG > 0)
    printf("Assigning integer [%lld]\n", value);
#endif
    uint8_t n = 0;
    unsigned long long v = 0;
    if (value >= 0) {
        v = value;
    } else {
        v = -value;
        n = 1;
    }
    bigint_ass_base(b, v, BIGINT_LIMB_BASE);
    b->neg = n;
    return b;
}

bigint* bigint_assign_string(bigint* b, const char* value) {
    enum {
        STATE_INIT,
        STATE_DIGIT,
    };
#if defined(DEBUG) && (DEBUG > 0)
    printf("Assigning string [%s]\n", value);
#endif
    int state = STATE_INIT;
    int neg = 0;
    bigint_clear(b);
    bigint* m = bigint_create();
    bigint_assign_integer(m, 10);
    bigint* d = bigint_create();
    bigint* t = bigint_create();
    uint32_t l = strlen(value);
    for (uint32_t j = 0; j < l; ++j) {
        uint8_t digit = UINT8_MAX;
        if (isspace(value[j])) {
            if (state == STATE_INIT) {
                continue;
            } else {
                break;
            }
        } else if (value[j] == '+') {
            if (state == STATE_INIT) {
                state = STATE_DIGIT;
                continue;
            } else {
                break;
            }
        } else if (value[j] == '-') {
            if (state == STATE_INIT) {
                state = STATE_DIGIT;
                neg = 1;
                continue;
            } else {
                break;
            }
        } else if (isdigit(value[j])) {
            if (state == STATE_INIT || state == STATE_DIGIT) {
                state = STATE_DIGIT;
                digit = value[j] - '0';
            } else {
                break;
            }
        }
        assert(digit != UINT8_MAX); // could not read a valid digit
        bigint_assign_integer(d, digit);
        // show("DIGIT", d);
        bigint_mul_base(b, m, t, BIGINT_LIMB_BASE);
        // show("AFTER MUL", t);
        bigint_add_base(t, d, b, BIGINT_LIMB_BASE);
        // show("AFTER ADD", b);
    }
    assert(state == STATE_DIGIT); // could not read any digit
    b->neg = neg;
    bigint_destroy(t);
    bigint_destroy(d);
    bigint_destroy(m);
    return b;
}

bigint* bigint_assign_bigint(bigint* b, const bigint* n) {
#if defined(DEBUG) && (DEBUG > 0)
    bigint_print("Assigning bigint ", n, stdout, 1);
#endif
    bigint_clear(b);
    b->neg = n->neg;
    while (b->pos < n->pos) {
        bigint_limb_t t = n->lmb[b->pos];
        SET_DIGIT(b, b->pos, t, BIGINT_LIMB_BASE);
        ++b->pos;
    }
    return b;
}

char* bigint_format(const bigint* b, char* buf) {
    // show("Formatting", b);
    bigint* c = bigint_create();

    bigint* m = bigint_create();
    bigint_ass_base(m, BIGINT_LIMB_BASE, 10);
    // show("USING BASE", m);
    bigint* d = bigint_create();
    bigint* t = bigint_create();
    for (int j = b->pos - 1; j >= 0; --j) {
        bigint_assign_integer(d, b->lmb[j]);
        // show("DIGIT", d);
        bigint_mul_base(c, m, t, 10);
        // show("AFTER MUL", c);
        bigint_add_base(t, d, c, 10);
        // show("AFTER ADD", c);
    }
    bigint_destroy(t);
    bigint_destroy(d);
    bigint_destroy(m);

    int p = 0;
    int e = 1;
    if (b->neg) {
        buf[p++] = '-';
    }
    for (int j = c->pos - 1; j >= 0; --j) {
        buf[p++] = c->lmb[j] + '0';
        e = 0;
    }
    if (e) {
        buf[p++] = '0';
        e = 0;
    }
    buf[p] = '\0';

    bigint_destroy(c);
    // printf("FORMATTED [%s]\n", buf);
    return buf;
}

void bigint_print(const char* msg, const bigint* b, FILE* stream, int newline) {
    // TODO: change this to use a buffer
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

bigint* bigint_add(const bigint* l, const bigint* r, bigint* a) {
    uint8_t s = l->neg == r->neg ? 0 : 1;
    // TODO: implement subtraction
    assert(!s); // must have equal sign for now
    bigint_add_base(l, r, a, BIGINT_LIMB_BASE);
    a->neg = l->neg;
    return a;
}

bigint* bigint_mul(const bigint* l, const bigint* r, bigint* a) {
    uint8_t s = l->neg == r->neg ? 0 : 1;
    bigint_mul_base(l, r, a, BIGINT_LIMB_BASE);
    a->neg = s;
    return a;
}

bigint_limb_t bigint_mod_integer(bigint* b, bigint_limb_t value) {
    assert(!b->neg); // must be positive for now
    bigint_larger_t mod = 0;
    for (int j = b->pos - 1; j >= 0; --j) {
        mod = (mod * BIGINT_LIMB_BASE + b->lmb[j]) % value;
    }
    return mod;
}

void bigint_factorial(bigint_limb_t n, bigint* b) {
    bigint_ass_base(b, 1, BIGINT_LIMB_BASE);
    if (n <= 1) {
        return;
    }

    bigint* d = bigint_create();
    bigint* t = bigint_clone(b);
    bigint* l = b;
    bigint* r = t;
    for (bigint_limb_t x = 2; x <= n; ++x) {
        bigint_ass_base(d, x, BIGINT_LIMB_BASE);
        bigint_mul_base(l, d, r, BIGINT_LIMB_BASE);
        bigint* z = l;
        l = r;
        r = z;
    }
    if (l != b) {
        bigint_assign_bigint(b, t);
    }
    bigint_destroy(t);
    bigint_destroy(d);
}

static void enlarge(bigint* b, size_t p) {
    if (b->cap > p) {
        return;
    }
    size_t size = b->cap ? 2*b->cap : 1;
    while (size <= p) {
        size *= 2;
    }
#if defined(DEBUG) && (DEBUG > 0)
    printf("GROW %p %u -> %zu\n", b, b->cap, size);
#endif
    bigint_limb_t* limb = realloc(b->lmb, size * sizeof(bigint_limb_t));
    memset(limb + b->cap, 0, (size - b->cap) * sizeof(bigint_limb_t));
    b->lmb = limb;
    b->cap = size;
}

static void bigint_ass_base(bigint* b, unsigned long long value, bigint_larger_t base) {
    bigint_clear(b);
    while (value > 0) {
        SET_DIGIT(b, b->pos, value, base);
        ++b->pos;
    }
}

static void bigint_add_base(const bigint* l, const bigint* r, bigint* a, bigint_larger_t base) {
    bigint_clear(a);
    bigint_larger_t overflow = 0;
    uint64_t p = 0;
    while (p < l->pos || p < r->pos) {
        if (p < l->pos) {
            overflow += l->lmb[p];
        }
        if (p < r->pos) {
            overflow += r->lmb[p];
        }
        SET_DIGIT(a, p, overflow, base);
        ++p;
    }
    while (overflow) {
        SET_DIGIT(a, p, overflow, base);
        ++p;
    }
    if (a->pos < p) {
        a->pos = p;
    }
}

static void bigint_mul_base(const bigint* l, const bigint* r, bigint* a, bigint_larger_t base) {
    // show("MUL l", l);
    // show("MUL r", r);
    bigint_clear(a);
    enlarge(a, l->pos + r->pos);
    for (uint64_t p = 0; p < l->pos; ++p) {
        bigint_larger_t total = 0;
        uint64_t q;
        for (q = 0; q < r->pos; ++q) {
            total += (bigint_larger_t) a->lmb[p+q] + (bigint_larger_t) l->lmb[p] * (bigint_larger_t) r->lmb[q];
            SET_DIGIT(a, p+q, total, base);
        }
        for (; total; ++q) {
            SET_DIGIT(a, p+q, total, base);
        }
        if (a->pos < p+q) {
            a->pos = p+q;
        }
    }
}
