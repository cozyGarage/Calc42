#ifndef PARSER_H
#define PARSER_H

#include "engine/tokenizer.h"
#include "common/error.h"
#include "common/memory.h"

/**
 * Value types
 */
typedef enum {
    VALUE_NUMBER,      // Double precision number
    VALUE_ARRAY,       // Array of numbers
    VALUE_MATRIX       // 2D matrix
} value_type_t;

/**
 * Value structure (discriminated union)
 */
typedef struct {
    value_type_t type;
    union {
        double number;
        struct {
            double *data;
            size_t size;
        } array;
        struct {
            double *data;
            size_t rows;
            size_t cols;
        } matrix;
    } as;
} value_t;

/**
 * Create a number value
 */
value_t value_number(double num);

/**
 * Create an array value
 */
value_t value_array(double *data, size_t size);

/**
 * Create a matrix value
 */
value_t value_matrix(double *data, size_t rows, size_t cols);

/**
 * Free value resources
 */
void value_free(value_t *val);

/**
 * Clone a value
 */
value_t value_clone(const value_t *val);

/**
 * AST node types
 */
typedef enum {
    NODE_NUMBER,
    NODE_OPERATOR,
    NODE_FUNCTION,
    NODE_ARRAY,
    NODE_MATRIX
} node_type_t;

/**
 * AST node
 */
typedef struct ast_node {
    node_type_t type;
    char op[MAX_TOKEN_LENGTH];  // Operator or function name
    double num_value;           // For numbers
    struct ast_node **children; // Child nodes
    size_t child_count;
} ast_node_t;

/**
 * Parser context
 */
typedef struct {
    darray_t *tokens;
    size_t position;
    error_t error;
} parser_t;

/**
 * Parse an expression into AST
 * Uses shunting-yard algorithm
 */
ast_node_t *parse(const char *expression, error_t *error);

/**
 * Free AST
 */
void ast_free(ast_node_t *node);

#endif // PARSER_H
