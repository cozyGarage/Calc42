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
// Helper to collect all arguments as a flattened array of doubles.
// Handles nesting: collect_args(1, [2, 3], matrix(2, 1, 4, 5)) -> [1, 2, 3, 4,
// 5]
static double *collect_args(ast_node_t *node, engine_context_t *ctx,
                            error_t *error, size_t *out_count) {
  *out_count = 0;
  if (node->child_count == 0)
    return NULL;

  value_t *results = safe_malloc(node->child_count * sizeof(value_t));
  size_t total = 0;

  for (size_t i = 0; i < node->child_count; i++) {
    results[i] = eval_node(node->children[i], ctx, error);
    if (!error_is_ok(*error)) {
      for (size_t j = 0; j <= i; j++)
        value_free(&results[j]);
      safe_free(results);
      return NULL;
    }
    if (results[i].type == VALUE_NUMBER) {
      total++;
    } else if (results[i].type == VALUE_ARRAY) {
      total += results[i].as.array.size;
    } else if (results[i].type == VALUE_MATRIX) {
      total += results[i].as.matrix.rows * results[i].as.matrix.cols;
    }
  }

  double *data = safe_malloc(total * sizeof(double));
  size_t pos = 0;
  for (size_t i = 0; i < node->child_count; i++) {
    if (results[i].type == VALUE_NUMBER) {
      data[pos++] = results[i].as.number;
    } else if (results[i].type == VALUE_ARRAY) {
      memcpy(data + pos, results[i].as.array.data,
             results[i].as.array.size * sizeof(double));
      pos += results[i].as.array.size;
    } else if (results[i].type == VALUE_MATRIX) {
      size_t size = results[i].as.matrix.rows * results[i].as.matrix.cols;
      memcpy(data + pos, results[i].as.matrix.data, size * sizeof(double));
      pos += size;
    }
    value_free(&results[i]);
  }
  safe_free(results);
  *out_count = total;
  return data;
}

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
    size_t data_size;
    double *data = collect_args(node, ctx, error, &data_size);
    if (!error_is_ok(*error))
      return value_number(0);

    double result = 0;
    if (strcmp(fname, "mean") == 0)
      result = stats_mean(data, data_size, error);
    else if (strcmp(fname, "median") == 0)
      result = stats_median(data, data_size, error);
    else if (strcmp(fname, "mode") == 0)
      result = stats_mode(data, data_size, error);
    else if (strcmp(fname, "var") == 0)
      result = stats_variance(data, data_size, error);
    else if (strcmp(fname, "stddev") == 0)
      result = stats_stddev(data, data_size, error);

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

    // Temp node for rest
    ast_node_t temp;
    temp.children = node->children + 1;
    temp.child_count = node->child_count - 1;
    size_t data_size;
    double *data = collect_args(&temp, ctx, error, &data_size);
    if (!error_is_ok(*error)) {
      value_free(&val_arg);
      return value_number(0);
    }

    double result = stats_zscore(val_arg.as.number, data, data_size, error);
    value_free(&val_arg);
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
    size_t data_size;
    double *data = collect_args(node, ctx, error, &data_size);
    if (!error_is_ok(*error))
      return value_number(0);
    if (data_size == 0) {
      safe_free(data);
      *error = error_create(ERR_INVALID_ARGS, "vector requires elements");
      return value_number(0);
    }
    return value_array(data, data_size);
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
      if (node->child_count < 2) {
        *error = error_create(ERR_INVALID_ARGS,
                              "vec_scale requires scalar and vector");
        return value_number(0);
      }
      value_t scalar_val = eval_node(node->children[0], ctx, error);
      if (!error_is_ok(*error))
        return value_number(0);
      double scalar = scalar_val.as.number;
      value_free(&scalar_val);

      ast_node_t temp;
      temp.children = node->children + 1;
      temp.child_count = node->child_count - 1;
      size_t vec_size;
      double *vec_data = collect_args(&temp, ctx, error, &vec_size);
      if (!error_is_ok(*error))
        return value_number(0);

      value_t v = value_array(vec_data, vec_size);
      value_t result = linalg_vec_scale(&v, scalar, error);
      value_free(&v);
      return result;

    } else if (strcmp(fname, "vec_mag") == 0) {
      size_t data_size;
      double *data = collect_args(node, ctx, error, &data_size);
      if (!error_is_ok(*error))
        return value_number(0);
      if (data_size == 0) {
        safe_free(data);
        *error = error_create(ERR_INVALID_ARGS, "vec_mag requires elements");
        return value_number(0);
      }
      value_t v = value_array(data, data_size);
      double mag = linalg_vec_magnitude(&v, error);
      value_free(&v);
      return value_number(mag);

    } else {
      // vec_add, vec_sub, vec_dot
      if (node->child_count == 2) {
        value_t a = eval_node(node->children[0], ctx, error);
        if (!error_is_ok(*error))
          return value_number(0);
        value_t b = eval_node(node->children[1], ctx, error);
        if (!error_is_ok(*error)) {
          value_free(&a);
          return value_number(0);
        }

        if (a.type == VALUE_ARRAY && b.type == VALUE_ARRAY) {
          value_t result;
          if (strcmp(fname, "vec_add") == 0)
            result = linalg_vec_add(&a, &b, error);
          else if (strcmp(fname, "vec_sub") == 0)
            result = linalg_vec_sub(&a, &b, error);
          else
            result = value_number(linalg_vec_dot(&a, &b, error));
          value_free(&a);
          value_free(&b);
          return result;
        }
        value_free(&a);
        value_free(&b);
      }

      size_t total;
      double *full = collect_args(node, ctx, error, &total);
      if (!error_is_ok(*error))
        return value_number(0);
      if (total < 2 || total % 2 != 0) {
        safe_free(full);
        *error =
            error_create(ERR_INVALID_ARGS, "Requires even number of elements");
        return value_number(0);
      }

      size_t half = total / 2;
      double *v_a_data = safe_malloc(half * sizeof(double));
      double *v_b_data = safe_malloc(half * sizeof(double));
      memcpy(v_a_data, full, half * sizeof(double));
      memcpy(v_b_data, full + half, half * sizeof(double));
      safe_free(full);

      value_t v_a = value_array(v_a_data, half);
      value_t v_b = value_array(v_b_data, half);

      value_t result;
      if (strcmp(fname, "vec_add") == 0)
        result = linalg_vec_add(&v_a, &v_b, error);
      else if (strcmp(fname, "vec_sub") == 0)
        result = linalg_vec_sub(&v_a, &v_b, error);
      else
        result = value_number(linalg_vec_dot(&v_a, &v_b, error));

      value_free(&v_a);
      value_free(&v_b);
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
  if (strcmp(fname, "and") == 0 || strcmp(fname, "or") == 0 ||
      strcmp(fname, "xor") == 0) {
    size_t data_size;
    double *data = collect_args(node, ctx, error, &data_size);
    if (!error_is_ok(*error))
      return value_number(0);
    if (data_size < 2) {
      safe_free(data);
      *error = error_create(ERR_INVALID_ARGS, "Logic ops require 2+ arguments");
      return value_number(0);
    }
    int res = (data[0] != 0.0);
    for (size_t i = 1; i < data_size; i++) {
      int val = (data[i] != 0.0);
      if (strcmp(fname, "and") == 0)
        res = res && val;
      else if (strcmp(fname, "or") == 0)
        res = res || val;
      else
        res = res ^ val;
    }
    safe_free(data);
    return value_number(res ? 1.0 : 0.0);
  }

  // Set operations (treat arguments as two sets split in half)
  if (strcmp(fname, "set_union") == 0 || strcmp(fname, "set_intersect") == 0 ||
      strcmp(fname, "set_diff") == 0) {
    if (node->child_count == 2) {
      value_t a = eval_node(node->children[0], ctx, error);
      if (!error_is_ok(*error))
        return value_number(0);
      value_t b = eval_node(node->children[1], ctx, error);
      if (!error_is_ok(*error)) {
        value_free(&a);
        return value_number(0);
      }

      if (a.type == VALUE_ARRAY && b.type == VALUE_ARRAY) {
        double *res_data;
        size_t res_size;
        if (strcmp(fname, "set_union") == 0)
          res_data =
              set_union(a.as.array.data, a.as.array.size, b.as.array.data,
                        b.as.array.size, &res_size, error);
        else if (strcmp(fname, "set_intersect") == 0)
          res_data = set_intersection(a.as.array.data, a.as.array.size,
                                      b.as.array.data, b.as.array.size,
                                      &res_size, error);
        else
          res_data =
              set_difference(a.as.array.data, a.as.array.size, b.as.array.data,
                             b.as.array.size, &res_size, error);
        value_free(&a);
        value_free(&b);
        if (!error_is_ok(*error) || !res_data)
          return value_number(0);
        return value_array(res_data, res_size);
      }
      value_free(&a);
      value_free(&b);
    }

    size_t total;
    double *full = collect_args(node, ctx, error, &total);
    if (!error_is_ok(*error))
      return value_number(0);
    if (total < 2 || total % 2 != 0) {
      safe_free(full);
      *error = error_create(ERR_INVALID_ARGS,
                            "Set ops require even number of elements");
      return value_number(0);
    }

    size_t half = total / 2;
    size_t res_size = 0;
    double *res_data = NULL;
    if (strcmp(fname, "set_union") == 0)
      res_data = set_union(full, half, full + half, half, &res_size, error);
    else if (strcmp(fname, "set_intersect") == 0)
      res_data =
          set_intersection(full, half, full + half, half, &res_size, error);
    else
      res_data =
          set_difference(full, half, full + half, half, &res_size, error);

    safe_free(full);
    if (!error_is_ok(*error) || !res_data)
      return value_number(0);
    return value_array(res_data, res_size);
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
