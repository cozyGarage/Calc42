#ifndef STATISTICS_H
#define STATISTICS_H

#include "common/error.h"
#include <stddef.h>

/**
 * Calculate mean (average) of array
 */
double stats_mean(const double *data, size_t size, error_t *error);

/**
 * Calculate median (middle value) of array
 */
double stats_median(const double *data, size_t size, error_t *error);

/**
 * Calculate mode (most frequent value) of array
 * Returns first mode if multiple
 */
double stats_mode(const double *data, size_t size, error_t *error);

/**
 * Calculate variance of array
 */
double stats_variance(const double *data, size_t size, error_t *error);

/**
 * Calculate standard deviation of array
 */
double stats_stddev(const double *data, size_t size, error_t *error);

/**
 * Calculate z-score of value in dataset
 */
double stats_zscore(double value, const double *data, size_t size,
                    error_t *error);

/**
 * Calculate correlation coefficient between two datasets
 */
double stats_correlation(const double *x, size_t x_size, const double *y,
                         size_t y_size, error_t *error);

#endif // STATISTICS_H
