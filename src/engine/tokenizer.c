#include "engine/tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

tokenizer_t *tokenizer_create(const char *expression) {
    if (!expression) {
        return NULL;
    }
    
    tokenizer_t *tok = safe_malloc(sizeof(tokenizer_t));
    if (!tok) {
        return NULL;
    }
    
    tok->input = expression;
    tok->position = 0;
    tok->length = strlen(expression);
    tok->error = error_ok();
    
    return tok;
}

static void skip_whitespace(tokenizer_t *tok) {
    while (tok->position < tok->length && isspace(tok->input[tok->position])) {
        tok->position++;
    }
}

static int is_operator_char(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
           c == '&' || c == '|' || c == '^' || c == '~' ||
           c == '<' || c == '>';
}

static int parse_number(tokenizer_t *tok, token_t *token) {
    size_t start = tok->position;
    char buffer[MAX_TOKEN_LENGTH];
    int buf_pos = 0;
    int base = 10;
    
    // Check for hex/binary/octal prefix
    if (tok->input[tok->position] == '0' && tok->position + 1 < tok->length) {
        char next = tok->input[tok->position + 1];
        if (next == 'x' || next == 'X') {
            base = 16;
            tok->position += 2;
        } else if (next == 'b' || next == 'B') {
            base = 2;
            tok->position += 2;
        } else if (isdigit(next)) {
            base = 8;
        }
    }
    
    // Read digits
    while (tok->position < tok->length && buf_pos < MAX_TOKEN_LENGTH - 1) {
        char c = tok->input[tok->position];
        
        if (base == 16 && isxdigit(c)) {
            buffer[buf_pos++] = c;
            tok->position++;
        } else if (base == 2 && (c == '0' || c == '1')) {
            buffer[buf_pos++] = c;
            tok->position++;
        } else if (base == 8 && c >= '0' && c <= '7') {
            buffer[buf_pos++] = c;
            tok->position++;
        } else if (base == 10 && (isdigit(c) || c == '.' || c == 'e' || c == 'E')) {
            buffer[buf_pos++] = c;
            tok->position++;
            // Handle scientific notation sign
            if ((c == 'e' || c == 'E') && tok->position < tok->length) {
                char sign = tok->input[tok->position];
                if (sign == '+' || sign == '-') {
                    buffer[buf_pos++] = sign;
                    tok->position++;
                }
            }
        } else {
            break;
        }
    }
    
    buffer[buf_pos] = '\0';
    
    if (buf_pos == 0) {
        tok->error = error_create_at(ERR_SYNTAX, "Invalid number", start);
        return -1;
    }
    
    // Convert to double
    if (base == 10) {
        token->num_value = strtod(buffer, NULL);
    } else {
        // Convert integer bases to double
        long long val = strtoll(buffer, NULL, base);
        token->num_value = (double)val;
    }
    
    token->type = TOKEN_NUMBER;
    snprintf(token->value, MAX_TOKEN_LENGTH, "%s", buffer);
    token->position = start;
    
    return 1;
}

static int parse_identifier(tokenizer_t *tok, token_t *token) {
    size_t start = tok->position;
    char buffer[MAX_TOKEN_LENGTH];
    int buf_pos = 0;
    
    while (tok->position < tok->length && buf_pos < MAX_TOKEN_LENGTH - 1) {
        char c = tok->input[tok->position];
        if (isalnum(c) || c == '_') {
            buffer[buf_pos++] = c;
            tok->position++;
        } else {
            break;
        }
    }
    
    buffer[buf_pos] = '\0';
    
    token->type = TOKEN_FUNCTION;
    snprintf(token->value, MAX_TOKEN_LENGTH, "%s", buffer);
    token->position = start;
    
    return 1;
}

int tokenizer_next(tokenizer_t *tok, token_t *token) {
    if (!tok || !token) {
        return -1;
    }
    
    skip_whitespace(tok);
    
    if (tok->position >= tok->length) {
        token->type = TOKEN_END;
        return 0;
    }
    
    char c = tok->input[tok->position];
    
    // Numbers
    if (isdigit(c) || (c == '.' && tok->position + 1 < tok->length && 
                       isdigit(tok->input[tok->position + 1]))) {
        return parse_number(tok, token);
    }
    
    // Hex numbers starting with 0x
    if (c == '0' && tok->position + 1 < tok->length) {
        char next = tok->input[tok->position + 1];
        if (next == 'x' || next == 'X' || next == 'b' || next == 'B') {
            return parse_number(tok, token);
        }
    }
    
    // Identifiers (functions)
    if (isalpha(c) || c == '_') {
        return parse_identifier(tok, token);
    }
    
    // Single character tokens
    token->position = tok->position;
    
    if (c == '(') {
        token->type = TOKEN_LPAREN;
        snprintf(token->value, MAX_TOKEN_LENGTH, "(");
        tok->position++;
        return 1;
    } else if (c == ')') {
        token->type = TOKEN_RPAREN;
        snprintf(token->value, MAX_TOKEN_LENGTH, ")");
        tok->position++;
        return 1;
    } else if (c == '[') {
        token->type = TOKEN_LBRACKET;
        snprintf(token->value, MAX_TOKEN_LENGTH, "[");
        tok->position++;
        return 1;
    } else if (c == ']') {
        token->type = TOKEN_RBRACKET;
        snprintf(token->value, MAX_TOKEN_LENGTH, "]");
        tok->position++;
        return 1;
    } else if (c == ',') {
        token->type = TOKEN_COMMA;
        snprintf(token->value, MAX_TOKEN_LENGTH, ",");
        tok->position++;
        return 1;
    }
    
    // Operators (including multi-char like <<, >>)
    if (is_operator_char(c)) {
        token->type = TOKEN_OPERATOR;
        char op[3] = {c, '\0', '\0'};
        tok->position++;
        
        // Check for two-character operators
        if (tok->position < tok->length) {
            char next = tok->input[tok->position];
            if ((c == '<' && next == '<') || (c == '>' && next == '>')) {
                op[1] = next;
                tok->position++;
            }
        }
        
        snprintf(token->value, MAX_TOKEN_LENGTH, "%s", op);
        return 1;
    }
    
    // Unknown character
    tok->error = error_create_at(ERR_SYNTAX, "Unexpected character", tok->position);
    return -1;
}

int tokenizer_peek(tokenizer_t *tok, token_t *token) {
    size_t saved_pos = tok->position;
    int result = tokenizer_next(tok, token);
    tok->position = saved_pos;
    return result;
}

error_t tokenizer_get_error(tokenizer_t *tok) {
    return tok ? tok->error : error_create(ERR_UNKNOWN, "Invalid tokenizer");
}

void tokenizer_free(tokenizer_t *tok) {
    safe_free(tok);
}

darray_t *tokenize(const char *expression, error_t *error) {
    tokenizer_t *tok = tokenizer_create(expression);
    if (!tok) {
        if (error) {
            *error = error_create(ERR_MEMORY, "Failed to create tokenizer");
        }
        return NULL;
    }
    
    darray_t *tokens = darray_create(sizeof(token_t), 16);
    if (!tokens) {
        if (error) {
            *error = error_create(ERR_MEMORY, "Failed to create token array");
        }
        tokenizer_free(tok);
        return NULL;
    }
    
    token_t token;
    int result;
    
    while ((result = tokenizer_next(tok, &token)) > 0) {
        if (darray_append(tokens, &token) != 0) {
            if (error) {
                *error = error_create(ERR_MEMORY, "Failed to append token");
            }
            darray_free(tokens);
            tokenizer_free(tok);
            return NULL;
        }
    }
    
    if (result < 0) {
        if (error) {
            *error = tokenizer_get_error(tok);
        }
        darray_free(tokens);
        tokenizer_free(tok);
        return NULL;
    }
    
    if (error) {
        *error = error_ok();
    }
    
    tokenizer_free(tok);
    return tokens;
}
