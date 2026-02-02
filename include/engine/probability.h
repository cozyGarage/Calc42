#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "common/error.h"

/**
 * Calculate factorial (n!)
 * Returns 0 on overflow
 */
double prob_factorial(int n, error_t *error);

/**
 * Calculate combinations nCr (n choose r)
 */
double prob_ncr(int n, int r, error_t *error);

/**
 * Calculate permutations nPr
 */
double prob_npr(int n, int r, error_t *error);

/**
 * Binomial probability: P(X = k) in n trials with probability p
 */
double prob_binomial(int n, double p, int k, error_t *error);

/**
 * Geometric probability: P(X = k) with success probability p
 */
double prob_geometric(double p, int k, error_t *error);

#endif // PROBABILITY_H
