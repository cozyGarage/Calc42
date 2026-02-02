#ifndef LINALG_H
#define LINALG_H

#include "common/error.h"
#include "engine/parser.h"

/**
 * Vector Operations
 */

// Vector addition: [a1, a2, ...] + [b1, b2, ...] = [a1+b1, a2+b2, ...]
value_t linalg_vec_add(const value_t *a, const value_t *b, error_t *error);

// Vector subtraction: [a1, a2, ...] - [b1, b2, ...] = [a1-b1, a2-b2, ...]
value_t linalg_vec_sub(const value_t *a, const value_t *b, error_t *error);

// Scalar multiplication: k * [a1, a2, ...] = [k*a1, k*a2, ...]
value_t linalg_vec_scale(const value_t *v, double scalar, error_t *error);

// Dot product: [a1, a2, ...] · [b1, b2, ...] = a1*b1 + a2*b2 + ...
double linalg_vec_dot(const value_t *a, const value_t *b, error_t *error);

// Vector magnitude: ||[a1, a2, ...]|| = sqrt(a1^2 + a2^2 + ...)
double linalg_vec_magnitude(const value_t *v, error_t *error);

/**
 * Matrix Operations
 */

// Matrix addition: A + B
value_t linalg_mat_add(const value_t *a, const value_t *b, error_t *error);

// Matrix subtraction: A - B
value_t linalg_mat_sub(const value_t *a, const value_t *b, error_t *error);

// Scalar multiplication: k * A
value_t linalg_mat_scale(const value_t *m, double scalar, error_t *error);

// Matrix multiplication: A × B
value_t linalg_mat_mul(const value_t *a, const value_t *b, error_t *error);

// Matrix-vector multiplication: A × v
value_t linalg_mat_vec_mul(const value_t *m, const value_t *v, error_t *error);

// Determinant (2x2 and 3x3 matrices)
double linalg_mat_det(const value_t *m, error_t *error);

// Matrix transpose: A^T
value_t linalg_mat_transpose(const value_t *m, error_t *error);

#endif // LINALG_H
