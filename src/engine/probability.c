#include "engine/probability.h"
#include <math.h>

double prob_factorial(int n, error_t *error) {
  if (n < 0) {
    *error = error_create(ERR_INVALID_ARGS, "Factorial of negative number");
    return 0.0;
  }

  if (n > 170) {
    *error = error_create(ERR_OVERFLOW, "Factorial too large (overflow)");
    return 0.0;
  }

  double result = 1.0;
  for (int i = 2; i <= n; i++) {
    result *= i;
  }

  *error = error_ok();
  return result;
}

double prob_ncr(int n, int r, error_t *error) {
  if (n < 0 || r < 0) {
    *error =
        error_create(ERR_INVALID_ARGS, "nCr requires non-negative n and r");
    return 0.0;
  }

  if (r > n) {
    *error = error_create(ERR_INVALID_ARGS, "nCr requires r <= n");
    return 0.0;
  }

  // Optimize: nCr(n, r) = nCr(n, n-r)
  if (r > n - r) {
    r = n - r;
  }

  // Calculate using iterative method to avoid overflow
  double result = 1.0;
  for (int i = 0; i < r; i++) {
    result *= (double)(n - i) / (double)(i + 1);
  }

  *error = error_ok();
  return result;
}

double prob_npr(int n, int r, error_t *error) {
  if (n < 0 || r < 0) {
    *error =
        error_create(ERR_INVALID_ARGS, "nPr requires non-negative n and r");
    return 0.0;
  }

  if (r > n) {
    *error = error_create(ERR_INVALID_ARGS, "nPr requires r <= n");
    return 0.0;
  }

  // nPr = n! / (n-r)!
  double result = 1.0;
  for (int i = 0; i < r; i++) {
    result *= (n - i);
  }

  *error = error_ok();
  return result;
}

double prob_binomial(int n, double p, int k, error_t *error) {
  if (n < 0 || k < 0) {
    *error = error_create(ERR_INVALID_ARGS,
                          "Binomial requires non-negative n and k");
    return 0.0;
  }

  if (k > n) {
    *error = error_create(ERR_INVALID_ARGS, "Binomial requires k <= n");
    return 0.0;
  }

  if (p < 0.0 || p > 1.0) {
    *error = error_create(ERR_INVALID_ARGS, "Probability p must be in [0, 1]");
    return 0.0;
  }

  // P(X = k) = nCk * p^k * (1-p)^(n-k)
  double ncr = prob_ncr(n, k, error);
  if (!error_is_ok(*error)) {
    return 0.0;
  }

  double result = ncr * pow(p, k) * pow(1.0 - p, n - k);

  *error = error_ok();
  return result;
}

double prob_geometric(double p, int k, error_t *error) {
  if (k < 1) {
    *error = error_create(ERR_INVALID_ARGS, "Geometric requires k >= 1");
    return 0.0;
  }

  if (p <= 0.0 || p > 1.0) {
    *error = error_create(ERR_INVALID_ARGS, "Probability p must be in (0, 1]");
    return 0.0;
  }

  // P(X = k) = (1-p)^(k-1) * p
  double result = pow(1.0 - p, k - 1) * p;

  *error = error_ok();
  return result;
}
