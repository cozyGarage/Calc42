#include "engine/linalg.h"
#include "common/memory.h"
#include <math.h>
#include <string.h>

// Vector addition
value_t linalg_vec_add(const value_t *a, const value_t *b, error_t *error) {
  if (!a || !b) {
    *error = error_create(ERR_INVALID_ARGS, "Null vector in addition");
    return value_number(0);
  }

  if (a->type != VALUE_ARRAY || b->type != VALUE_ARRAY) {
    *error =
        error_create(ERR_INVALID_ARGS, "Vector addition requires array values");
    return value_number(0);
  }

  if (a->as.array.size != b->as.array.size) {
    *error = error_create(ERR_DIMENSION, "Vector dimensions must match");
    return value_number(0);
  }

  size_t size = a->as.array.size;
  double *result_data = safe_malloc(size * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result vector");
    return value_number(0);
  }

  for (size_t i = 0; i < size; i++) {
    result_data[i] = a->as.array.data[i] + b->as.array.data[i];
  }

  *error = error_ok();
  return value_array(result_data, size);
}

// Vector subtraction
value_t linalg_vec_sub(const value_t *a, const value_t *b, error_t *error) {
  if (!a || !b) {
    *error = error_create(ERR_INVALID_ARGS, "Null vector in subtraction");
    return value_number(0);
  }

  if (a->type != VALUE_ARRAY || b->type != VALUE_ARRAY) {
    *error = error_create(ERR_INVALID_ARGS,
                          "Vector subtraction requires array values");
    return value_number(0);
  }

  if (a->as.array.size != b->as.array.size) {
    *error = error_create(ERR_DIMENSION, "Vector dimensions must match");
    return value_number(0);
  }

  size_t size = a->as.array.size;
  double *result_data = safe_malloc(size * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result vector");
    return value_number(0);
  }

  for (size_t i = 0; i < size; i++) {
    result_data[i] = a->as.array.data[i] - b->as.array.data[i];
  }

  *error = error_ok();
  return value_array(result_data, size);
}

// Scalar multiplication
value_t linalg_vec_scale(const value_t *v, double scalar, error_t *error) {
  if (!v) {
    *error = error_create(ERR_INVALID_ARGS, "Null vector in scaling");
    return value_number(0);
  }

  if (v->type != VALUE_ARRAY) {
    *error = error_create(ERR_INVALID_ARGS, "Scaling requires array value");
    return value_number(0);
  }

  size_t size = v->as.array.size;
  double *result_data = safe_malloc(size * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result vector");
    return value_number(0);
  }

  for (size_t i = 0; i < size; i++) {
    result_data[i] = v->as.array.data[i] * scalar;
  }

  *error = error_ok();
  return value_array(result_data, size);
}

// Dot product
double linalg_vec_dot(const value_t *a, const value_t *b, error_t *error) {
  if (!a || !b) {
    *error = error_create(ERR_INVALID_ARGS, "Null vector in dot product");
    return 0.0;
  }

  if (a->type != VALUE_ARRAY || b->type != VALUE_ARRAY) {
    *error =
        error_create(ERR_INVALID_ARGS, "Dot product requires array values");
    return 0.0;
  }

  if (a->as.array.size != b->as.array.size) {
    *error = error_create(ERR_DIMENSION, "Vector dimensions must match");
    return 0.0;
  }

  double result = 0.0;
  for (size_t i = 0; i < a->as.array.size; i++) {
    result += a->as.array.data[i] * b->as.array.data[i];
  }

  *error = error_ok();
  return result;
}

// Vector magnitude
double linalg_vec_magnitude(const value_t *v, error_t *error) {
  if (!v) {
    *error = error_create(ERR_INVALID_ARGS, "Null vector in magnitude");
    return 0.0;
  }

  if (v->type != VALUE_ARRAY) {
    *error = error_create(ERR_INVALID_ARGS, "Magnitude requires array value");
    return 0.0;
  }

  double sum_sq = 0.0;
  for (size_t i = 0; i < v->as.array.size; i++) {
    sum_sq += v->as.array.data[i] * v->as.array.data[i];
  }

  *error = error_ok();
  return sqrt(sum_sq);
}

// Matrix addition
value_t linalg_mat_add(const value_t *a, const value_t *b, error_t *error) {
  if (!a || !b) {
    *error = error_create(ERR_INVALID_ARGS, "Null matrix in addition");
    return value_number(0);
  }

  if (a->type != VALUE_MATRIX || b->type != VALUE_MATRIX) {
    *error = error_create(ERR_INVALID_ARGS,
                          "Matrix addition requires matrix values");
    return value_number(0);
  }

  if (a->as.matrix.rows != b->as.matrix.rows ||
      a->as.matrix.cols != b->as.matrix.cols) {
    *error = error_create(ERR_DIMENSION, "Matrix dimensions must match");
    return value_number(0);
  }

  size_t rows = a->as.matrix.rows;
  size_t cols = a->as.matrix.cols;
  size_t total = rows * cols;

  double *result_data = safe_malloc(total * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result matrix");
    return value_number(0);
  }

  for (size_t i = 0; i < total; i++) {
    result_data[i] = a->as.matrix.data[i] + b->as.matrix.data[i];
  }

  *error = error_ok();
  return value_matrix(result_data, rows, cols);
}

// Matrix subtraction
value_t linalg_mat_sub(const value_t *a, const value_t *b, error_t *error) {
  if (!a || !b) {
    *error = error_create(ERR_INVALID_ARGS, "Null matrix in subtraction");
    return value_number(0);
  }

  if (a->type != VALUE_MATRIX || b->type != VALUE_MATRIX) {
    *error = error_create(ERR_INVALID_ARGS,
                          "Matrix subtraction requires matrix values");
    return value_number(0);
  }

  if (a->as.matrix.rows != b->as.matrix.rows ||
      a->as.matrix.cols != b->as.matrix.cols) {
    *error = error_create(ERR_DIMENSION, "Matrix dimensions must match");
    return value_number(0);
  }

  size_t rows = a->as.matrix.rows;
  size_t cols = a->as.matrix.cols;
  size_t total = rows * cols;

  double *result_data = safe_malloc(total * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result matrix");
    return value_number(0);
  }

  for (size_t i = 0; i < total; i++) {
    result_data[i] = a->as.matrix.data[i] - b->as.matrix.data[i];
  }

  *error = error_ok();
  return value_matrix(result_data, rows, cols);
}

// Matrix scalar multiplication
value_t linalg_mat_scale(const value_t *m, double scalar, error_t *error) {
  if (!m) {
    *error = error_create(ERR_INVALID_ARGS, "Null matrix in scaling");
    return value_number(0);
  }

  if (m->type != VALUE_MATRIX) {
    *error = error_create(ERR_INVALID_ARGS, "Scaling requires matrix value");
    return value_number(0);
  }

  size_t rows = m->as.matrix.rows;
  size_t cols = m->as.matrix.cols;
  size_t total = rows * cols;

  double *result_data = safe_malloc(total * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result matrix");
    return value_number(0);
  }

  for (size_t i = 0; i < total; i++) {
    result_data[i] = m->as.matrix.data[i] * scalar;
  }

  *error = error_ok();
  return value_matrix(result_data, rows, cols);
}

// Matrix multiplication
value_t linalg_mat_mul(const value_t *a, const value_t *b, error_t *error) {
  if (!a || !b) {
    *error = error_create(ERR_INVALID_ARGS, "Null matrix in multiplication");
    return value_number(0);
  }

  if (a->type != VALUE_MATRIX || b->type != VALUE_MATRIX) {
    *error = error_create(ERR_INVALID_ARGS,
                          "Matrix multiplication requires matrix values");
    return value_number(0);
  }

  if (a->as.matrix.cols != b->as.matrix.rows) {
    *error = error_create(ERR_DIMENSION,
                          "Matrix dimensions incompatible for multiplication");
    return value_number(0);
  }

  size_t m = a->as.matrix.rows;
  size_t n = a->as.matrix.cols;
  size_t p = b->as.matrix.cols;

  double *result_data = safe_malloc(m * p * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result matrix");
    return value_number(0);
  }

  // C[i][j] = sum(A[i][k] * B[k][j]) for k = 0 to n-1
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < p; j++) {
      double sum = 0.0;
      for (size_t k = 0; k < n; k++) {
        sum += a->as.matrix.data[i * n + k] * b->as.matrix.data[k * p + j];
      }
      result_data[i * p + j] = sum;
    }
  }

  *error = error_ok();
  return value_matrix(result_data, m, p);
}

// Matrix-vector multiplication
value_t linalg_mat_vec_mul(const value_t *m, const value_t *v, error_t *error) {
  if (!m || !v) {
    *error = error_create(ERR_INVALID_ARGS,
                          "Null value in matrix-vector multiplication");
    return value_number(0);
  }

  if (m->type != VALUE_MATRIX || v->type != VALUE_ARRAY) {
    *error =
        error_create(ERR_INVALID_ARGS, "Requires matrix and vector values");
    return value_number(0);
  }

  if (m->as.matrix.cols != v->as.array.size) {
    *error =
        error_create(ERR_DIMENSION, "Matrix columns must match vector size");
    return value_number(0);
  }

  size_t rows = m->as.matrix.rows;
  size_t cols = m->as.matrix.cols;

  double *result_data = safe_malloc(rows * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result vector");
    return value_number(0);
  }

  for (size_t i = 0; i < rows; i++) {
    double sum = 0.0;
    for (size_t j = 0; j < cols; j++) {
      sum += m->as.matrix.data[i * cols + j] * v->as.array.data[j];
    }
    result_data[i] = sum;
  }

  *error = error_ok();
  return value_array(result_data, rows);
}

// Determinant (2x2 and 3x3)
double linalg_mat_det(const value_t *m, error_t *error) {
  if (!m) {
    *error = error_create(ERR_INVALID_ARGS, "Null matrix in determinant");
    return 0.0;
  }

  if (m->type != VALUE_MATRIX) {
    *error =
        error_create(ERR_INVALID_ARGS, "Determinant requires matrix value");
    return 0.0;
  }

  if (m->as.matrix.rows != m->as.matrix.cols) {
    *error = error_create(ERR_DIMENSION, "Determinant requires square matrix");
    return 0.0;
  }

  size_t n = m->as.matrix.rows;
  double *d = m->as.matrix.data;

  if (n == 2) {
    // det([[a, b], [c, d]]) = ad - bc
    *error = error_ok();
    return d[0] * d[3] - d[1] * d[2];
  } else if (n == 3) {
    // det([[a,b,c],[d,e,f],[g,h,i]]) = aei + bfg + cdh - ceg - bdi - afh
    *error = error_ok();
    return d[0] * d[4] * d[8] + d[1] * d[5] * d[6] + d[2] * d[3] * d[7] -
           d[2] * d[4] * d[6] - d[1] * d[3] * d[8] - d[0] * d[5] * d[7];
  } else {
    *error = error_create(ERR_UNSUPPORTED,
                          "Determinant only supports 2x2 and 3x3 matrices");
    return 0.0;
  }
}

// Matrix transpose
value_t linalg_mat_transpose(const value_t *m, error_t *error) {
  if (!m) {
    *error = error_create(ERR_INVALID_ARGS, "Null matrix in transpose");
    return value_number(0);
  }

  if (m->type != VALUE_MATRIX) {
    *error = error_create(ERR_INVALID_ARGS, "Transpose requires matrix value");
    return value_number(0);
  }

  size_t rows = m->as.matrix.rows;
  size_t cols = m->as.matrix.cols;

  double *result_data = safe_malloc(rows * cols * sizeof(double));
  if (!result_data) {
    *error = error_create(ERR_MEMORY, "Failed to allocate result matrix");
    return value_number(0);
  }

  // Transpose: result[j][i] = m[i][j]
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      result_data[j * rows + i] = m->as.matrix.data[i * cols + j];
    }
  }

  *error = error_ok();
  return value_matrix(result_data, cols, rows); // Note: dimensions swapped
}
