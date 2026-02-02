#include "engine/statistics.h"
#include "common/memory.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

double stats_mean(const double *data, size_t size, error_t *error) {
  if (!data || size == 0) {
    *error = error_create(ERR_INVALID_ARGS, "Empty dataset for mean");
    return 0.0;
  }

  double sum = 0.0;
  for (size_t i = 0; i < size; i++) {
    sum += data[i];
  }

  *error = error_ok();
  return sum / (double)size;
}

static int compare_double(const void *a, const void *b) {
  double da = *(const double *)a;
  double db = *(const double *)b;
  if (da < db)
    return -1;
  if (da > db)
    return 1;
  return 0;
}

double stats_median(const double *data, size_t size, error_t *error) {
  if (!data || size == 0) {
    *error = error_create(ERR_INVALID_ARGS, "Empty dataset for median");
    return 0.0;
  }

  // Create a copy to sort
  double *sorted = safe_malloc(size * sizeof(double));
  if (!sorted) {
    *error = error_create(ERR_MEMORY, "Failed to allocate for median");
    return 0.0;
  }

  memcpy(sorted, data, size * sizeof(double));
  qsort(sorted, size, sizeof(double), compare_double);

  double result;
  if (size % 2 == 0) {
    // Even number of elements - average middle two
    result = (sorted[size / 2 - 1] + sorted[size / 2]) / 2.0;
  } else {
    // Odd number - take middle element
    result = sorted[size / 2];
  }

  safe_free(sorted);
  *error = error_ok();
  return result;
}

double stats_mode(const double *data, size_t size, error_t *error) {
  if (!data || size == 0) {
    *error = error_create(ERR_INVALID_ARGS, "Empty dataset for mode");
    return 0.0;
  }

  // Create a copy to sort
  double *sorted = safe_malloc(size * sizeof(double));
  if (!sorted) {
    *error = error_create(ERR_MEMORY, "Failed to allocate for mode");
    return 0.0;
  }

  memcpy(sorted, data, size * sizeof(double));
  qsort(sorted, size, sizeof(double), compare_double);

  // Find most frequent value
  double mode = sorted[0];
  size_t max_count = 1;
  size_t current_count = 1;

  for (size_t i = 1; i < size; i++) {
    if (fabs(sorted[i] - sorted[i - 1]) < 1e-9) { // Equal within tolerance
      current_count++;
      if (current_count > max_count) {
        max_count = current_count;
        mode = sorted[i];
      }
    } else {
      current_count = 1;
    }
  }

  safe_free(sorted);
  *error = error_ok();
  return mode;
}

double stats_variance(const double *data, size_t size, error_t *error) {
  if (!data || size == 0) {
    *error = error_create(ERR_INVALID_ARGS, "Empty dataset for variance");
    return 0.0;
  }

  double mean = stats_mean(data, size, error);
  if (!error_is_ok(*error)) {
    return 0.0;
  }

  double sum_sq_diff = 0.0;
  for (size_t i = 0; i < size; i++) {
    double diff = data[i] - mean;
    sum_sq_diff += diff * diff;
  }

  *error = error_ok();
  return sum_sq_diff / (double)size;
}

double stats_stddev(const double *data, size_t size, error_t *error) {
  double var = stats_variance(data, size, error);
  if (!error_is_ok(*error)) {
    return 0.0;
  }

  *error = error_ok();
  return sqrt(var);
}

double stats_zscore(double value, const double *data, size_t size,
                    error_t *error) {
  if (!data || size == 0) {
    *error = error_create(ERR_INVALID_ARGS, "Empty dataset for z-score");
    return 0.0;
  }

  double mean = stats_mean(data, size, error);
  if (!error_is_ok(*error)) {
    return 0.0;
  }

  double stddev = stats_stddev(data, size, error);
  if (!error_is_ok(*error)) {
    return 0.0;
  }

  if (stddev == 0.0) {
    *error = error_create(ERR_DIV_ZERO, "Zero standard deviation for z-score");
    return 0.0;
  }

  *error = error_ok();
  return (value - mean) / stddev;
}

double stats_correlation(const double *x, size_t x_size, const double *y,
                         size_t y_size, error_t *error) {
  if (!x || !y || x_size == 0 || y_size == 0) {
    *error = error_create(ERR_INVALID_ARGS, "Empty dataset for correlation");
    return 0.0;
  }

  if (x_size != y_size) {
    *error =
        error_create(ERR_DIMENSION, "Dataset sizes must match for correlation");
    return 0.0;
  }

  size_t n = x_size;
  double mean_x = stats_mean(x, n, error);
  if (!error_is_ok(*error))
    return 0.0;

  double mean_y = stats_mean(y, n, error);
  if (!error_is_ok(*error))
    return 0.0;

  double sum_xy = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;

  for (size_t i = 0; i < n; i++) {
    double dx = x[i] - mean_x;
    double dy = y[i] - mean_y;
    sum_xy += dx * dy;
    sum_x2 += dx * dx;
    sum_y2 += dy * dy;
  }

  double denom = sqrt(sum_x2 * sum_y2);
  if (denom == 0.0) {
    *error = error_create(ERR_DIV_ZERO, "Zero denominator in correlation");
    return 0.0;
  }

  *error = error_ok();
  return sum_xy / denom;
}
