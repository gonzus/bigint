#include <stdio.h>
#include <stdlib.h>

#define IS_EVEN(x) ((x & 1) == 0)

// Utility function to do modular exponentiation.  It returns (a^b) % m
static unsigned long power_mod(unsigned long a, unsigned long b, unsigned long m) {
    unsigned long res = 1; // Initialize result
    a = a % m; // Update a if it is more than or equal to m
    while (b > 0) {
        if (!IS_EVEN(b)) {
            res = (res*a) % m; // if b is odd, multiply a with result
        }

        // treat b as even now
        b >>= 1; // b /= 2
        a = (a*a) % m;
    }
    return res;
}

// This function is called for all k trials.
// It returns 0 if n is composite and 1 if n is probably prime.
// d is an odd number such that d*2^q = n-1 for some q >= 1
static int miller_rabin_trial(unsigned long d, unsigned long n) {
    // Pick a random number in [2..n-2]; corner cases make sure that n > 4
    unsigned long r = 2 + rand() % (n - 4);

    // Compute m = r^d % n
    unsigned long m = power_mod(r, d, n);
    if (m == 1 || m == n-1) {
        return 1; // likely prime
    }

    while (d != n-1) { // d has not reached n-1
        m = power_mod(m, 2, n); // keep squaring m
        if (m == 1) {
            return 0; // (m^2) % n is 1 => composite
        }
        if (m == n-1) {
            return 1; // (m^2) % n is n-1 => likely prime
        }
        d <<= 1; // d *= 2
    }
    return 0; // d has reached n-1 => composite
}

// Return 0 if n is composite and 1 if n is probably prime.
// k is an input parameter that determines accuracy level.
// Higher value of k indicates more accuracy.
static int is_prime(unsigned long n, unsigned int k) {
    // Corner cases: 1-digit numbers
    if (n <= 1) {
        return 0; // composite
    }
    if (n == 2 || n == 3 || n == 5 || n == 7) {
        return 1; // prime -- yes, for sure :-)
    }
    if (n <= 9) {
        return 0; // composite
    }

    // Find odd d such that n = d * 2^q + 1 for some q >= 1
    unsigned long d = n - 1;
    while (IS_EVEN(d)) {
        d >>= 1; // d /= 2
    }

    // Test given number k times
    for (unsigned int j = 0; j < k; ++j) {
        if (!miller_rabin_trial(d, n)) { // miller (-rabin) time!
            return 0; // composite
        }
    }

    return 1; // likely prime
}

int main(int argc, char* argv[]) {
    int k = 4; // Number of iterations
    for (int j = 1; j < argc; ++j) {
        int t = atoi(argv[j]); // Top number tested
        int c = 0;
        printf("All primes smaller than %d:", t);
        for (int n = 1; n < t; ++n) {
            if (is_prime(n, k)) {
                ++c;
                // printf(" %d", n);
            }
        }
        printf(" -- total %d\n", c);
    }
    return 0;
}
