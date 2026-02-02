#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "common/error.h"
#include "common/memory.h"
#include <stddef.h>

#define MAX_TOKEN_LENGTH 64

/**
 * Token types
 */
typedef enum {
    TOKEN_NUMBER,      // 123, 3.14, 0xFF, 0b1010
    TOKEN_OPERATOR,    // +, -, *, /, %
    TOKEN_FUNCTION,    // sin, mean, dot
    TOKEN_LPAREN,      // (
    TOKEN_RPAREN,      // )
    TOKEN_LBRACKET,    // [
    TOKEN_RBRACKET,    // ]
    TOKEN_COMMA,       // ,
    TOKEN_END          // End of input
} token_type_t;

/**
 * Token structure
 */
typedef struct {
    token_type_t type;
    char value[MAX_TOKEN_LENGTH];
    double num_value;    // For numbers
    size_t position;     // Position in original expression
} token_t;

/**
 * Tokenizer state
 */
typedef struct {
    const char *input;
    size_t position;
    size_t length;
    error_t error;
} tokenizer_t;

/**
 * Create a tokenizer for an expression
 */
tokenizer_t *tokenizer_create(const char *expression);

/**
 * Get the next token
 * Returns 1 if a token was retrieved, 0 if at end, -1 on error
 */
int tokenizer_next(tokenizer_t *tok, token_t *token);

/**
 * Peek at the next token without consuming it
 */
int tokenizer_peek(tokenizer_t *tok, token_t *token);

/**
 * Get tokenizer error
 */
error_t tokenizer_get_error(tokenizer_t *tok);

/**
 * Free tokenizer
 */
void tokenizer_free(tokenizer_t *tok);

/**
 * Tokenize entire expression into array
 * Returns dynamic array of tokens, or NULL on error
 */
darray_t *tokenize(const char *expression, error_t *error);

#endif // TOKENIZER_H
