#include "engine/discrete.h"
#include <stdlib.h>

long long discrete_gcd(long long a, long long b, error_t *error) {
  // Handle negative numbers
  a = llabs(a);
  b = llabs(b);

  // Euclidean algorithm
  while (b != 0) {
    long long temp = b;
    b = a % b;
    a = temp;
  }

  *error = error_ok();
  return a;
}

long long discrete_lcm(long long a, long long b, error_t *error) {
  if (a == 0 || b == 0) {
    *error = error_ok();
    return 0;
  }

  long long gcd = discrete_gcd(a, b, error);
  if (!error_is_ok(*error)) {
    return 0;
  }

  // LCM = |a * b| / GCD
  // Use division before multiplication to avoid overflow
  a = llabs(a);
  b = llabs(b);

  *error = error_ok();
  return (a / gcd) * b;
}

long long discrete_mod(long long a, long long m, error_t *error) {
  if (m == 0) {
    *error = error_create(ERR_DIV_ZERO, "Modulo by zero");
    return 0;
  }

  long long result = a % m;

  // Ensure positive result for negative dividends
  if (result < 0) {
    result += llabs(m);
  }

  *error = error_ok();
  return result;
}

long long discrete_modpow(long long base, long long exp, long long m,
                          error_t *error) {
  if (m == 0) {
    *error = error_create(ERR_DIV_ZERO, "Modulo by zero in modpow");
    return 0;
  }

  if (m == 1) {
    *error = error_ok();
    return 0;
  }

  if (exp < 0) {
    *error = error_create(ERR_INVALID_ARGS,
                          "Negative exponent not supported in modpow");
    return 0;
  }

  long long result = 1;
  base = base % m;

  if (base < 0) {
    base += m;
  }

  // Fast exponentiation by squaring
  while (exp > 0) {
    if (exp % 2 == 1) {
      result = (result * base) % m;
    }
    exp = exp >> 1;
    base = (base * base) % m;
  }

  *error = error_ok();
  return result;
}

int discrete_is_prime(long long n, error_t *error) {
  if (n < 2) {
    *error = error_ok();
    return 0;
  }

  if (n == 2 || n == 3) {
    *error = error_ok();
    return 1;
  }

  if (n % 2 == 0 || n % 3 == 0) {
    *error = error_ok();
    return 0;
  }

  // Check divisibility up to sqrt(n) using 6k±1 optimization
  for (long long i = 5; i * i <= n; i += 6) {
    if (n % i == 0 || n % (i + 2) == 0) {
      *error = error_ok();
      return 0;
    }
  }

  *error = error_ok();
  return 1;
}
