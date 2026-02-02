#ifndef ENGINE_H
#define ENGINE_H

#include "engine/parser.h"
#include "common/error.h"

/**
 * Calculator modes
 */
typedef enum {
    MODE_STANDARD,
    MODE_PROGRAMMER,
    MODE_STATISTICS,
    MODE_PROBABILITY,
    MODE_DISCRETE,
    MODE_LINEAR_ALGEBRA
} calc_mode_t;

/**
 * Engine context
 */
typedef struct {
    calc_mode_t mode;
    int base;  // For programmer mode (2, 8, 10, 16)
} engine_context_t;

/**
 * Create engine context
 */
engine_context_t *engine_context_create(calc_mode_t mode);

/**
 * Evaluate an expression
 */
value_t engine_eval(const char *expression, engine_context_t *ctx, error_t *error);

/**
 * Free engine context
 */
void engine_context_free(engine_context_t *ctx);

/**
 * Format value as string
 */
char *value_to_string(const value_t *val, int base);

#endif // ENGINE_H
