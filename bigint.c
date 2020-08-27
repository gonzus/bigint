#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "bigint.h"

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

#if 1
// set an arbitrary base, to prove it works with weird sizes
#ifdef BIGINT_LIMB_BASE
#undef BIGINT_LIMB_BASE
#endif
// TODO: I broke this, only base 10 works now
#define BIGINT_LIMB_BASE (10)
#endif

#define SET_DIGIT(b, p, value, base) \
    do { \
        check_size(b, p); \
        b->lmb[p] = value % base; \
        value /= base; \
    } while (0)

static void check_size(bigint* b, size_t p);

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
    b->pos = o->pos;
    b->cap = o->cap;
    b->lmb = realloc(0, b->cap * sizeof(bigint_limb_t));
    for (uint64_t j = 0; j < o->pos; ++j) {
        b->lmb[j] = o->lmb[j];
    }
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
    return b->pos == 1 && b->lmb[0] == 1;
}

int bigint_compare(const bigint* a, const bigint* b) {
    if (a->pos > b->pos) {
        return +1;
    }
    if (a->pos < b->pos) {
        return -1;
    }
    for (int j = a->pos - 1; j >= 0; --j) {
        if (a->lmb[j] > b->lmb[j]) {
            return +1;
        }
        if (a->lmb[j] < b->lmb[j]) {
            return -1;
        }
    }
    return 0;
}

bigint* bigint_assign_integer(bigint* b, unsigned long long value) {
#if defined(DEBUG) && (DEBUG > 0)
    printf("Assigning integer [%llu]\n", value);
#endif
    bigint_clear(b);
    while (value > 0) {
        SET_DIGIT(b, b->pos, value, BIGINT_LIMB_BASE);
        ++b->pos;
    }
    return b;
}

bigint* bigint_assign_string(bigint* b, const char* value) {
#if defined(DEBUG) && (DEBUG > 0)
    printf("Assigning string [%s]\n", value);
#endif
    bigint_clear(b);
    int l = strlen(value);
    for (int j = l-1; j >= 0; --j) {
        uint8_t digit = value[j];
        if (digit >= '0' && digit <= '9') {
            digit = digit - '0';
        } else {
            digit = UINT8_MAX;
        }
        assert(digit != UINT8_MAX);
        SET_DIGIT(b, b->pos, digit, BIGINT_LIMB_BASE);
        ++b->pos;
    }
    return b;
}

bigint* bigint_assign_bigint(bigint* b, const bigint* n) {
#if defined(DEBUG) && (DEBUG > 0)
    bigint_print("Assigning bigint ", n, stdout, 1);
#endif
    bigint_clear(b);
    while (b->pos < n->pos) {
        bigint_limb_t t = n->lmb[b->pos];
        SET_DIGIT(b, b->pos, t, BIGINT_LIMB_BASE);
        ++b->pos;
    }
    return b;
}

char* bigint_format(const bigint* b, char* buf) {
    int p = 0;
    for (int j = b->pos - 1; j >= 0; --j) {
        buf[p++] = b->lmb[j] + '0';
    }
    if (b->pos == 0) {
        buf[p++] = '0';
    }
    buf[p] = '\0';
    return buf;
}

void bigint_print(const char* msg, const bigint* b, FILE* stream, int newline) {
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

bigint* bigint_addeq(bigint* b, const bigint* n) {
    bigint_larger_t overflow = 0;
    uint64_t p = 0;
    while (p < b->pos || p < n->pos) {
        if (p < b->pos) {
            overflow += b->lmb[p];
        }
        if (p < n->pos) {
            overflow += n->lmb[p];
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

bigint* bigint_muleq(bigint* b, const bigint* n) {
    bigint* t = bigint_clone(b);
    bigint_clear(b);
    for (uint64_t p = 0; p < t->pos; ++p) {
        bigint_larger_t total = 0;
        uint64_t q;
        for (q = 0; q < n->pos; ++q) {
            if (p+q < b->pos) {
                total += b->lmb[p+q];
            }
            total += (bigint_larger_t) t->lmb[p] * (bigint_larger_t) n->lmb[q];
            SET_DIGIT(b, p+q, total, BIGINT_LIMB_BASE);
        }
        for (; total; ++q) {
            SET_DIGIT(b, p+q, total, BIGINT_LIMB_BASE);
        }
        if (b->pos < p+q) {
            b->pos = p+q;
        }
    }
    bigint_destroy(t);
    return b;
}

static void check_size(bigint* b, size_t p) {
    if (b->cap > p) {
        return;
    }

    size_t size = b->cap ? 2*b->cap : 1;
    while (size <= p) {
        size *= 2;
    }
#if defined(DEBUG) && (DEBUG > 0)
    printf("GROW %p %zu -> %zu\n", b, b->cap, size);
#endif
    bigint_limb_t* limb = realloc(b->lmb, size * sizeof(bigint_limb_t));
    b->lmb = limb;
    b->cap = size;
}
