#include "engine/engine.h"
#include "engine/discrete.h"
#include "engine/linalg.h"
#include "engine/probability.h"
#include "engine/set_ops.h"
#include "engine/statistics.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

engine_context_t *engine_context_create(calc_mode_t mode) {
  engine_context_t *ctx = safe_malloc(sizeof(engine_context_t));
  if (ctx) {
    ctx->mode = mode;
    ctx->base = 10;
  }
  return ctx;
}

void engine_context_free(engine_context_t *ctx) { safe_free(ctx); }

// Forward declaration
static value_t eval_node(ast_node_t *node, engine_context_t *ctx,
                         error_t *error);

// Evaluate function calls
static value_t eval_function(ast_node_t *node, engine_context_t *ctx,
                             error_t *error) {
  const char *fname = node->op;

  // Discrete Math
  if (strcmp(fname, "gcd") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS, "gcd requires 2 arguments");
      return value_number(0);
    }
    value_t arg1 = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t arg2 = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&arg1);
      return value_number(0);
    }
    long long result = discrete_gcd((long long)arg1.as.number,
                                    (long long)arg2.as.number, error);
    value_free(&arg1);
    value_free(&arg2);
    return value_number((double)result);
  }

  if (strcmp(fname, "lcm") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS, "lcm requires 2 arguments");
      return value_number(0);
    }
    value_t arg1 = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t arg2 = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&arg1);
      return value_number(0);
    }
    long long result = discrete_lcm((long long)arg1.as.number,
                                    (long long)arg2.as.number, error);
    value_free(&arg1);
    value_free(&arg2);
    return value_number((double)result);
  }

  if (strcmp(fname, "mod") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS, "mod requires 2 arguments");
      return value_number(0);
    }
    value_t arg1 = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t arg2 = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&arg1);
      return value_number(0);
    }
    long long result = discrete_mod((long long)arg1.as.number,
                                    (long long)arg2.as.number, error);
    value_free(&arg1);
    value_free(&arg2);
    return value_number((double)result);
  }

  if (strcmp(fname, "modpow") == 0) {
    if (node->child_count != 3) {
      *error = error_create(ERR_INVALID_ARGS, "modpow requires 3 arguments");
      return value_number(0);
    }
    value_t arg1 = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t arg2 = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&arg1);
      return value_number(0);
    }
    value_t arg3 = eval_node(node->children[2], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&arg1);
      value_free(&arg2);
      return value_number(0);
    }
    long long result =
        discrete_modpow((long long)arg1.as.number, (long long)arg2.as.number,
                        (long long)arg3.as.number, error);
    value_free(&arg1);
    value_free(&arg2);
    value_free(&arg3);
    return value_number((double)result);
  }

  if (strcmp(fname, "is_prime") == 0) {
    if (node->child_count != 1) {
      *error = error_create(ERR_INVALID_ARGS, "is_prime requires 1 argument");
      return value_number(0);
    }
    value_t arg = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    int result = discrete_is_prime((long long)arg.as.number, error);
    value_free(&arg);
    return value_number((double)result);
  }

  // Probability
  if (strcmp(fname, "ncr") == 0 || strcmp(fname, "nCr") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS, "nCr requires 2 arguments");
      return value_number(0);
    }
    value_t arg1 = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t arg2 = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&arg1);
      return value_number(0);
    }
    double result = prob_ncr((int)arg1.as.number, (int)arg2.as.number, error);
    value_free(&arg1);
    value_free(&arg2);
    return value_number(result);
  }

  if (strcmp(fname, "npr") == 0 || strcmp(fname, "nPr") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS, "nPr requires 2 arguments");
      return value_number(0);
    }
    value_t arg1 = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t arg2 = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&arg1);
      return value_number(0);
    }
    double result = prob_npr((int)arg1.as.number, (int)arg2.as.number, error);
    value_free(&arg1);
    value_free(&arg2);
    return value_number(result);
  }

  if (strcmp(fname, "fact") == 0 || strcmp(fname, "factorial") == 0) {
    if (node->child_count != 1) {
      *error = error_create(ERR_INVALID_ARGS, "factorial requires 1 argument");
      return value_number(0);
    }
    value_t arg = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    double result = prob_factorial((int)arg.as.number, error);
    value_free(&arg);
    return value_number(result);
  }

  // Statistics (taking multiple arguments as a dataset)
  if (strcmp(fname, "mean") == 0 || strcmp(fname, "median") == 0 ||
      strcmp(fname, "mode") == 0 || strcmp(fname, "var") == 0 ||
      strcmp(fname, "stddev") == 0) {
    if (node->child_count == 0) {
      *error = error_create(ERR_INVALID_ARGS,
                            "Stats functions require at least 1 argument");
      return value_number(0);
    }
    double *data = safe_malloc(node->child_count * sizeof(double));
    for (size_t i = 0; i < node->child_count; i++) {
      value_t arg = eval_node(node->children[i], ctx, error);
      if (!error_is_ok(*error)) {
        safe_free(data);
        return value_number(0);
      }
      data[i] = arg.as.number;
      value_free(&arg);
    }
    double result = 0;
    if (strcmp(fname, "mean") == 0)
      result = stats_mean(data, node->child_count, error);
    else if (strcmp(fname, "median") == 0)
      result = stats_median(data, node->child_count, error);
    else if (strcmp(fname, "mode") == 0)
      result = stats_mode(data, node->child_count, error);
    else if (strcmp(fname, "var") == 0)
      result = stats_variance(data, node->child_count, error);
    else if (strcmp(fname, "stddev") == 0)
      result = stats_stddev(data, node->child_count, error);

    safe_free(data);
    return value_number(result);
  }

  // Z-score: zscore(value, data1, data2, ...)
  if (strcmp(fname, "zscore") == 0) {
    if (node->child_count < 2) {
      *error =
          error_create(ERR_INVALID_ARGS, "zscore requires value and dataset");
      return value_number(0);
    }
    value_t val_arg = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    double value = val_arg.as.number;
    value_free(&val_arg);

    size_t data_size = node->child_count - 1;
    double *data = safe_malloc(data_size * sizeof(double));
    for (size_t i = 0; i < data_size; i++) {
      value_t arg = eval_node(node->children[i + 1], ctx, error);
      if (!error_is_ok(*error)) {
        safe_free(data);
        return value_number(0);
      }
      data[i] = arg.as.number;
      value_free(&arg);
    }
    double result = stats_zscore(value, data, data_size, error);
    safe_free(data);
    return value_number(result);
  }

  // Correlation: correlation(x1, x2, ..., | y1, y2, ...) - split args in half
  if (strcmp(fname, "correlation") == 0) {
    if (node->child_count < 2 || node->child_count % 2 != 0) {
      *error = error_create(ERR_INVALID_ARGS,
                            "correlation requires even number of arguments");
      return value_number(0);
    }
    size_t half = node->child_count / 2;
    double *x = safe_malloc(half * sizeof(double));
    double *y = safe_malloc(half * sizeof(double));

    for (size_t i = 0; i < half; i++) {
      value_t arg_x = eval_node(node->children[i], ctx, error);
      if (!error_is_ok(*error)) {
        safe_free(x);
        safe_free(y);
        return value_number(0);
      }
      x[i] = arg_x.as.number;
      value_free(&arg_x);

      value_t arg_y = eval_node(node->children[half + i], ctx, error);
      if (!error_is_ok(*error)) {
        safe_free(x);
        safe_free(y);
        return value_number(0);
      }
      y[i] = arg_y.as.number;
      value_free(&arg_y);
    }

    double result = stats_correlation(x, half, y, half, error);
    safe_free(x);
    safe_free(y);
    return value_number(result);
  }

  // Binomial probability: binomial(n, p, k)
  if (strcmp(fname, "binomial") == 0) {
    if (node->child_count != 3) {
      *error = error_create(ERR_INVALID_ARGS,
                            "binomial requires 3 arguments (n, p, k)");
      return value_number(0);
    }
    value_t n_arg = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t p_arg = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&n_arg);
      return value_number(0);
    }
    value_t k_arg = eval_node(node->children[2], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&n_arg);
      value_free(&p_arg);
      return value_number(0);
    }

    double result = prob_binomial((int)n_arg.as.number, p_arg.as.number,
                                  (int)k_arg.as.number, error);
    value_free(&n_arg);
    value_free(&p_arg);
    value_free(&k_arg);
    return value_number(result);
  }

  // Geometric probability: geometric(p, k)
  if (strcmp(fname, "geometric") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS,
                            "geometric requires 2 arguments (p, k)");
      return value_number(0);
    }
    value_t p_arg = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t k_arg = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&p_arg);
      return value_number(0);
    }

    double result =
        prob_geometric(p_arg.as.number, (int)k_arg.as.number, error);
    value_free(&p_arg);
    value_free(&k_arg);
    return value_number(result);
  }

  // Linear Algebra - Vector operations
  // vector(1, 2, 3) -> [1, 2, 3]
  if (strcmp(fname, "vector") == 0) {
    if (node->child_count == 0) {
      *error = error_create(ERR_INVALID_ARGS, "vector requires elements");
      return value_number(0);
    }
    double *vec_data = safe_malloc(node->child_count * sizeof(double));
    for (size_t i = 0; i < node->child_count; i++) {
      value_t arg = eval_node(node->children[i], ctx, error);
      if (!error_is_ok(*error)) {
        safe_free(vec_data);
        return value_number(0);
      }
      vec_data[i] = arg.as.number;
      value_free(&arg);
    }
    return value_array(vec_data, node->child_count);
  }

  // matrix(rows, cols, e1, e2, ...)
  if (strcmp(fname, "matrix") == 0) {
    if (node->child_count < 3) {
      *error = error_create(ERR_INVALID_ARGS,
                            "matrix requires rows, cols and elements");
      return value_number(0);
    }
    value_t rows_v = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t cols_v = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&rows_v);
      return value_number(0);
    }

    size_t rows = (size_t)rows_v.as.number;
    size_t cols = (size_t)cols_v.as.number;
    value_free(&rows_v);
    value_free(&cols_v);

    if (node->child_count - 2 != rows * cols) {
      *error = error_create(ERR_INVALID_ARGS,
                            "Matrix element count does not match dimensions");
      return value_number(0);
    }

    double *mat_data = safe_malloc(rows * cols * sizeof(double));
    for (size_t i = 0; i < rows * cols; i++) {
      value_t arg = eval_node(node->children[i + 2], ctx, error);
      if (!error_is_ok(*error)) {
        safe_free(mat_data);
        return value_number(0);
      }
      mat_data[i] = arg.as.number;
      value_free(&arg);
    }
    return value_matrix(mat_data, rows, cols);
  }

  // vec_add(1, 2, 3, 4) treats first half as vector   a, second half as vector
  // b
  if (strcmp(fname, "vec_add") == 0 || strcmp(fname, "vec_sub") == 0 ||
      strcmp(fname, "vec_scale") == 0 || strcmp(fname, "vec_dot") == 0 ||
      strcmp(fname, "vec_mag") == 0) {

    if (strcmp(fname, "vec_scale") == 0) {
      // vec_scale(scalar, v1, v2, v3, ...)
      if (node->child_count < 2) {
        *error = error_create(ERR_INVALID_ARGS,
                              "vec_scale requires scalar and vector elements");
        return value_number(0);
      }
      value_t scalar_val = eval_node(node->children[0], ctx, error);
      if (!error_is_ok(*error))
        return value_number(0);
      double scalar = scalar_val.as.number;
      value_free(&scalar_val);

      size_t vec_size = node->child_count - 1;
      double *vec_data = safe_malloc(vec_size * sizeof(double));
      for (size_t i = 0; i < vec_size; i++) {
        value_t arg = eval_node(node->children[i + 1], ctx, error);
        if (!error_is_ok(*error)) {
          safe_free(vec_data);
          return value_number(0);
        }
        vec_data[i] = arg.as.number;
        value_free(&arg);
      }
      value_t v = value_array(vec_data, vec_size);
      value_t result = linalg_vec_scale(&v, scalar, error);
      value_free(&v);
      return result;

    } else if (strcmp(fname, "vec_mag") == 0) {
      // vec_mag(v1, v2, v3, ...)
      if (node->child_count == 0) {
        *error =
            error_create(ERR_INVALID_ARGS, "vec_mag requires vector elements");
        return value_number(0);
      }
      double *vec_data = safe_malloc(node->child_count * sizeof(double));
      for (size_t i = 0; i < node->child_count; i++) {
        value_t arg = eval_node(node->children[i], ctx, error);
        if (!error_is_ok(*error)) {
          safe_free(vec_data);
          return value_number(0);
        }
        vec_data[i] = arg.as.number;
        value_free(&arg);
      }
      value_t v = value_array(vec_data, node->child_count);
      double mag = linalg_vec_magnitude(&v, error);
      value_free(&v);
      return value_number(mag);

    } else {
      // vec_add, vec_sub, vec_dot: split args in half
      if (node->child_count < 2 || node->child_count % 2 != 0) {
        *error =
            error_create(ERR_INVALID_ARGS,
                         "Vector operations require even number of arguments");
        return value_number(0);
      }

      size_t vec_size = node->child_count / 2;
      double *vec_a = safe_malloc(vec_size * sizeof(double));
      double *vec_b = safe_malloc(vec_size * sizeof(double));

      for (size_t i = 0; i < vec_size; i++) {
        value_t arg_a = eval_node(node->children[i], ctx, error);
        if (!error_is_ok(*error)) {
          safe_free(vec_a);
          safe_free(vec_b);
          return value_number(0);
        }
        vec_a[i] = arg_a.as.number;
        value_free(&arg_a);

        value_t arg_b = eval_node(node->children[vec_size + i], ctx, error);
        if (!error_is_ok(*error)) {
          safe_free(vec_a);
          safe_free(vec_b);
          return value_number(0);
        }
        vec_b[i] = arg_b.as.number;
        value_free(&arg_b);
      }

      value_t a = value_array(vec_a, vec_size);
      value_t b = value_array(vec_b, vec_size);

      value_t result;
      if (strcmp(fname, "vec_add") == 0) {
        result = linalg_vec_add(&a, &b, error);
      } else if (strcmp(fname, "vec_sub") == 0) {
        result = linalg_vec_sub(&a, &b, error);
      } else { // vec_dot
        double dot = linalg_vec_dot(&a, &b, error);
        value_free(&a);
        value_free(&b);
        return value_number(dot);
      }

      value_free(&a);
      value_free(&b);
      return result;
    }
  }

  // Matrix operations
  if (strcmp(fname, "mat_add") == 0 || strcmp(fname, "mat_sub") == 0 ||
      strcmp(fname, "mat_mul") == 0 || strcmp(fname, "mat_vec_mul") == 0 ||
      strcmp(fname, "mat_det") == 0 || strcmp(fname, "mat_transpose") == 0) {

    if (strcmp(fname, "mat_det") == 0 || strcmp(fname, "mat_transpose") == 0) {
      if (node->child_count == 0) {
        *error = error_create(ERR_INVALID_ARGS, "Requires matrix arguments");
        return value_number(0);
      }
      // If first child is a matrix, use it. Otherwise error.
      // (For now we assume arguments are numbers that form a matrix, but that's
      // complex without constructor) Let's assume the user uses constructor:
      // mat_det(matrix(2, 2, 1, 0, 0, 1))
      value_t arg = eval_node(node->children[0], ctx, error);
      if (!error_is_ok(*error))
        return value_number(0);

      if (arg.type != VALUE_MATRIX) {
        *error = error_create(
            ERR_INVALID_ARGS,
            "Operand must be a matrix. Use matrix(r, c, ...) function.");
        value_free(&arg);
        return value_number(0);
      }

      value_t result;
      if (strcmp(fname, "mat_det") == 0) {
        double det = linalg_mat_det(&arg, error);
        result = value_number(det);
      } else {
        result = linalg_mat_transpose(&arg, error);
      }
      value_free(&arg);
      return result;
    } else {
      // Binary matrix ops: mat_add(m1, m2), mat_mul(m1, m2), mat_vec_mul(m, v)
      if (node->child_count != 2) {
        *error =
            error_create(ERR_INVALID_ARGS,
                         "Matrix binary ops require 2 matrix/vector arguments");
        return value_number(0);
      }

      value_t a = eval_node(node->children[0], ctx, error);
      if (!error_is_ok(*error))
        return value_number(0);

      value_t b = eval_node(node->children[1], ctx, error);
      if (!error_is_ok(*error)) {
        value_free(&a);
        return value_number(0);
      }

      value_t result;
      if (strcmp(fname, "mat_add") == 0) {
        result = linalg_mat_add(&a, &b, error);
      } else if (strcmp(fname, "mat_sub") == 0) {
        result = linalg_mat_sub(&a, &b, error);
      } else if (strcmp(fname, "mat_mul") == 0) {
        result = linalg_mat_mul(&a, &b, error);
      } else { // mat_vec_mul
        result = linalg_mat_vec_mul(&a, &b, error);
      }

      value_free(&a);
      value_free(&b);
      return result;
    }
  }

  // Unary operators as functions
  // neg(x) = -x
  if (strcmp(fname, "neg") == 0) {
    if (node->child_count != 1) {
      *error = error_create(ERR_INVALID_ARGS, "neg requires 1 argument");
      return value_number(0);
    }
    value_t arg = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    double result = -arg.as.number;
    value_free(&arg);
    return value_number(result);
  }

  // bnot(x) = bitwise NOT ~x
  if (strcmp(fname, "bnot") == 0) {
    if (node->child_count != 1) {
      *error = error_create(ERR_INVALID_ARGS, "bnot requires 1 argument");
      return value_number(0);
    }
    value_t arg = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    long long result = ~((long long)arg.as.number);
    value_free(&arg);
    return value_number((double)result);
  }

  // not(x) = logical NOT !x
  if (strcmp(fname, "not") == 0) {
    if (node->child_count != 1) {
      *error = error_create(ERR_INVALID_ARGS, "not requires 1 argument");
      return value_number(0);
    }
    value_t arg = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    double result = (arg.as.number == 0.0) ? 1.0 : 0.0;
    value_free(&arg);
    return value_number(result);
  }

  // Logic operators
  // and(a, b) = logical AND
  if (strcmp(fname, "and") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS, "and requires 2 arguments");
      return value_number(0);
    }
    value_t a = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t b = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&a);
      return value_number(0);
    }
    double result = (a.as.number != 0.0 && b.as.number != 0.0) ? 1.0 : 0.0;
    value_free(&a);
    value_free(&b);
    return value_number(result);
  }

  // or(a, b) = logical OR
  if (strcmp(fname, "or") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS, "or requires 2 arguments");
      return value_number(0);
    }
    value_t a = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t b = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&a);
      return value_number(0);
    }
    double result = (a.as.number != 0.0 || b.as.number != 0.0) ? 1.0 : 0.0;
    value_free(&a);
    value_free(&b);
    return value_number(result);
  }

  // xor(a, b) = logical XOR
  if (strcmp(fname, "xor") == 0) {
    if (node->child_count != 2) {
      *error = error_create(ERR_INVALID_ARGS, "xor requires 2 arguments");
      return value_number(0);
    }
    value_t a = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);
    value_t b = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&a);
      return value_number(0);
    }
    int a_bool = (a.as.number != 0.0);
    int b_bool = (b.as.number != 0.0);
    double result = (a_bool != b_bool) ? 1.0 : 0.0;
    value_free(&a);
    value_free(&b);
    return value_number(result);
  }

  // Set operations (treat arguments as two sets split in half)
  if (strcmp(fname, "set_union") == 0 || strcmp(fname, "set_intersect") == 0 ||
      strcmp(fname, "set_diff") == 0) {
    if (node->child_count < 2 || node->child_count % 2 != 0) {
      *error = error_create(ERR_INVALID_ARGS,
                            "Set operations require even number of arguments");
      return value_number(0);
    }

    size_t half = node->child_count / 2;
    double *set_a = safe_malloc(half * sizeof(double));
    double *set_b = safe_malloc(half * sizeof(double));

    for (size_t i = 0; i < half; i++) {
      value_t arg_a = eval_node(node->children[i], ctx, error);
      if (!error_is_ok(*error)) {
        safe_free(set_a);
        safe_free(set_b);
        return value_number(0);
      }
      set_a[i] = arg_a.as.number;
      value_free(&arg_a);

      value_t arg_b = eval_node(node->children[half + i], ctx, error);
      if (!error_is_ok(*error)) {
        safe_free(set_a);
        safe_free(set_b);
        return value_number(0);
      }
      set_b[i] = arg_b.as.number;
      value_free(&arg_b);
    }

    size_t result_size = 0;
    double *result_data = NULL;

    if (strcmp(fname, "set_union") == 0) {
      result_data = set_union(set_a, half, set_b, half, &result_size, error);
    } else if (strcmp(fname, "set_intersect") == 0) {
      result_data =
          set_intersection(set_a, half, set_b, half, &result_size, error);
    } else { // set_diff
      result_data =
          set_difference(set_a, half, set_b, half, &result_size, error);
    }

    safe_free(set_a);
    safe_free(set_b);

    if (!error_is_ok(*error) || !result_data) {
      return value_number(0);
    }

    return value_array(result_data, result_size);
  }

  *error = error_create(ERR_UNSUPPORTED, "Unknown function");
  return value_number(0);
}

// Simple recursive evaluator for AST
static value_t eval_node(ast_node_t *node, engine_context_t *ctx,
                         error_t *error) {
  if (!node) {
    *error = error_create(ERR_EVAL, "Null node");
    return value_number(0);
  }

  if (node->type == NODE_NUMBER) {
    *error = error_ok();
    return value_number(node->num_value);
  }

  if (node->type == NODE_FUNCTION) {
    return eval_function(node, ctx, error);
  }

  if (node->type == NODE_OPERATOR) {
    // Binary operators need 2 children
    if (node->child_count < 2) {
      *error = error_create(ERR_EVAL, "Operator requires 2 operands");
      return value_number(0);
    }

    value_t left = eval_node(node->children[0], ctx, error);
    if (!error_is_ok(*error))
      return value_number(0);

    value_t right = eval_node(node->children[1], ctx, error);
    if (!error_is_ok(*error)) {
      value_free(&left);
      return value_number(0);
    }

    // Only handle number operations for now
    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
      *error = error_create(ERR_EVAL, "Operator requires numeric operands");
      value_free(&left);
      value_free(&right);
      return value_number(0);
    }

    double result = 0;
    *error = error_ok();

    if (strcmp(node->op, "+") == 0) {
      result = left.as.number + right.as.number;
    } else if (strcmp(node->op, "-") == 0) {
      result = left.as.number - right.as.number;
    } else if (strcmp(node->op, "*") == 0) {
      result = left.as.number * right.as.number;
    } else if (strcmp(node->op, "/") == 0) {
      if (right.as.number == 0) {
        *error = error_create(ERR_DIV_ZERO, "Division by zero");
        value_free(&left);
        value_free(&right);
        return value_number(0);
      }
      result = left.as.number / right.as.number;
    } else if (strcmp(node->op, "%") == 0) {
      result = fmod(left.as.number, right.as.number);
    } else if (strcmp(node->op, "&") == 0) {
      result = (double)((long long)left.as.number & (long long)right.as.number);
    } else if (strcmp(node->op, "|") == 0) {
      result = (double)((long long)left.as.number | (long long)right.as.number);
    } else if (strcmp(node->op, "^") == 0) {
      result = (double)((long long)left.as.number ^ (long long)right.as.number);
    } else if (strcmp(node->op, "<<") == 0) {
      long long shift = (long long)right.as.number;
      if (shift < 0 || shift > 63) {
        *error = error_create(ERR_INVALID_ARGS, "Invalid shift count");
        value_free(&left);
        value_free(&right);
        return value_number(0);
      }
      result = (double)((long long)left.as.number << shift);
    } else if (strcmp(node->op, ">>") == 0) {
      long long shift = (long long)right.as.number;
      if (shift < 0 || shift > 63) {
        *error = error_create(ERR_INVALID_ARGS, "Invalid shift count");
        value_free(&left);
        value_free(&right);
        return value_number(0);
      }
      result = (double)((long long)left.as.number >> shift);
    } else {
      *error = error_create(ERR_UNSUPPORTED, "Unsupported operator");
      value_free(&left);
      value_free(&right);
      return value_number(0);
    }

    value_free(&left);
    value_free(&right);

    // Check for NaN or Inf
    if (isnan(result) || isinf(result)) {
      *error = error_create(ERR_DOMAIN, "Result is not a finite number");
      return value_number(0);
    }

    return value_number(result);
  }

  *error = error_create(ERR_UNSUPPORTED, "Unsupported node type");
  return value_number(0);
}

value_t engine_eval(const char *expression, engine_context_t *ctx,
                    error_t *error) {
  if (!expression || !ctx) {
    if (error)
      *error = error_create(ERR_EVAL, "Invalid input");
    return value_number(0);
  }

  ast_node_t *ast = parse(expression, error);
  if (!ast) {
    return value_number(0);
  }

  value_t result = eval_node(ast, ctx, error);
  ast_free(ast);

  return result;
}

char *value_to_string(const value_t *val, int base) {
  if (!val)
    return NULL;

  char *buffer = safe_malloc(256);
  if (!buffer)
    return NULL;

  if (val->type == VALUE_NUMBER) {
    // Check if it's an integer value
    double num = val->as.number;
    if (base != 10 && floor(num) == num) {
      long long int_val = (long long)num;
      if (base == 16) {
        snprintf(buffer, 256, "0x%llX", int_val);
      } else if (base == 8) {
        snprintf(buffer, 256, "0%llo", int_val);
      } else if (base == 2) {
        // Binary representation
        int pos = 0;
        buffer[pos++] = '0';
        buffer[pos++] = 'b';
        int started = 0;
        for (int i = 63; i >= 0; i--) {
          if (int_val & (1LL << i)) {
            started = 1;
          }
          if (started) {
            buffer[pos++] = (int_val & (1LL << i)) ? '1' : '0';
          }
        }
        if (!started) {
          buffer[pos++] = '0';
        }
        buffer[pos] = '\0';
      } else {
        snprintf(buffer, 256, "%.10g", num);
      }
    } else {
      snprintf(buffer, 256, "%.10g", num);
    }
  } else if (val->type == VALUE_ARRAY) {
    int pos = 0;
    pos += snprintf(buffer + pos, 256 - pos, "[");
    for (size_t i = 0; i < val->as.array.size && pos < 250; i++) {
      if (i > 0)
        pos += snprintf(buffer + pos, 256 - pos, ", ");
      pos += snprintf(buffer + pos, 256 - pos, "%.6g", val->as.array.data[i]);
    }
    snprintf(buffer + pos, 256 - pos, "]");
  } else if (val->type == VALUE_MATRIX) {
    int pos = 0;
    size_t total = val->as.matrix.rows * val->as.matrix.cols;
    pos += snprintf(buffer + pos, 256 - pos, "[");
    for (size_t i = 0; i < total && pos < 250; i++) {
      if (i > 0)
        pos += snprintf(buffer + pos, 256 - pos, ", ");
      pos += snprintf(buffer + pos, 256 - pos, "%.6g", val->as.matrix.data[i]);
    }
    snprintf(buffer + pos, 256 - pos, "]");
  }

  return buffer;
}
