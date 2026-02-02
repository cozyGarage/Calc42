#ifndef SET_OPS_H
#define SET_OPS_H

#include "common/error.h"
#include <stddef.h>

// Set union: returns combined unique elements
// Result array must be freed by caller
double *set_union(const double *a, size_t a_size, const double *b,
                  size_t b_size, size_t *result_size, error_t *error);

// Set intersection: returns common elements
double *set_intersection(const double *a, size_t a_size, const double *b,
                         size_t b_size, size_t *result_size, error_t *error);

// Set difference: elements in a but not in b
double *set_difference(const double *a, size_t a_size, const double *b,
                       size_t b_size, size_t *result_size, error_t *error);

// Check if element is in set
int set_contains(const double *set, size_t size, double element);

#endif // SET_OPS_H
