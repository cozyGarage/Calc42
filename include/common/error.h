#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>

/**
 * Error codes for CALC42
 */
typedef enum {
    ERR_NONE = 0,
    ERR_SYNTAX,           // Syntax error in expression
    ERR_DOMAIN,           // Domain error (e.g., sqrt(-1))
    ERR_OVERFLOW,         // Numeric overflow
    ERR_UNDERFLOW,        // Numeric underflow
    ERR_DIV_ZERO,         // Division by zero
    ERR_INVALID_ARGS,     // Invalid function arguments
    ERR_DIMENSION,        // Matrix/vector dimension mismatch
    ERR_MEMORY,           // Memory allocation failure
    ERR_UNSUPPORTED,      // Unsupported operation
    ERR_PARSE,            // Parse error
    ERR_EVAL,             // Evaluation error
    ERR_UNKNOWN           // Unknown error
} error_code_t;

/**
 * Error context with position and message
 */
typedef struct {
    error_code_t code;
    char message[256];
    size_t position;      // Position in expression where error occurred
    int has_position;
} error_t;

/**
 * Create an error with code and message
 */
error_t error_create(error_code_t code, const char *message);

/**
 * Create an error with position information
 */
error_t error_create_at(error_code_t code, const char *message, size_t position);

/**
 * Get human-readable error message
 */
const char *error_to_string(error_code_t code);

/**
 * Check if error represents success
 */
int error_is_ok(error_t err);

/**
 * Clear error state
 */
error_t error_ok(void);

#endif // ERROR_H
