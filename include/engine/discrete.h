#ifndef DISCRETE_H
#define DISCRETE_H

#include "common/error.h"

/**
 * Calculate greatest common divisor (GCD) using Euclidean algorithm
 */
long long discrete_gcd(long long a, long long b, error_t *error);

/**
 * Calculate least common multiple (LCM)
 */
long long discrete_lcm(long long a, long long b, error_t *error);

/**
 * Modular arithmetic: a mod m
 */
long long discrete_mod(long long a, long long m, error_t *error);

/**
 * Modular exponentiation: (base^exp) mod m
 */
long long discrete_modpow(long long base, long long exp, long long m,
                          error_t *error);

/**
 * Check if number is prime
 */
int discrete_is_prime(long long n, error_t *error);

#endif // DISCRETE_H
