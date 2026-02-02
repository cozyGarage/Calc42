#include "engine/set_ops.h"
#include "common/memory.h"
#include <math.h>

// Helper: check if element is in set (with floating point tolerance)
int set_contains(const double *set, size_t size, double element) {
  const double EPSILON = 1e-9;
  for (size_t i = 0; i < size; i++) {
    if (fabs(set[i] - element) < EPSILON) {
      return 1;
    }
  }
  return 0;
}

// Set union
double *set_union(const double *a, size_t a_size, const double *b,
                  size_t b_size, size_t *result_size, error_t *error) {
  // Worst case: all elements are unique
  double *result = safe_malloc((a_size + b_size) * sizeof(double));
  if (!result) {
    *error = error_create(ERR_MEMORY, "Failed to allocate set union");
    *result_size = 0;
    return NULL;
  }

  size_t count = 0;

  // Add all elements from a
  for (size_t i = 0; i < a_size; i++) {
    if (!set_contains(result, count, a[i])) {
      result[count++] = a[i];
    }
  }

  // Add unique elements from b
  for (size_t i = 0; i < b_size; i++) {
    if (!set_contains(result, count, b[i])) {
      result[count++] = b[i];
    }
  }

  *result_size = count;
  *error = error_ok();
  return result;
}

// Set intersection
double *set_intersection(const double *a, size_t a_size, const double *b,
                         size_t b_size, size_t *result_size, error_t *error) {
  double *result =
      safe_malloc(a_size * sizeof(double)); // At most a_size elements
  if (!result) {
    *error = error_create(ERR_MEMORY, "Failed to allocate set intersection");
    *result_size = 0;
    return NULL;
  }

  size_t count = 0;

  for (size_t i = 0; i < a_size; i++) {
    if (set_contains(b, b_size, a[i]) && !set_contains(result, count, a[i])) {
      result[count++] = a[i];
    }
  }

  *result_size = count;
  *error = error_ok();
  return result;
}

// Set difference
double *set_difference(const double *a, size_t a_size, const double *b,
                       size_t b_size, size_t *result_size, error_t *error) {
  double *result = safe_malloc(a_size * sizeof(double));
  if (!result) {
    *error = error_create(ERR_MEMORY, "Failed to allocate set difference");
    *result_size = 0;
    return NULL;
  }

  size_t count = 0;

  for (size_t i = 0; i < a_size; i++) {
    if (!set_contains(b, b_size, a[i]) && !set_contains(result, count, a[i])) {
      result[count++] = a[i];
    }
  }

  *result_size = count;
  *error = error_ok();
  return result;
}
