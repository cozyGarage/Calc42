#include "common/error.h"
#include <string.h>
#include <stdio.h>

error_t error_create(error_code_t code, const char *message) {
    error_t err;
    err.code = code;
    err.has_position = 0;
    err.position = 0;
    
    if (message) {
        snprintf(err.message, sizeof(err.message), "%s", message);
    } else {
        snprintf(err.message, sizeof(err.message), "%s", error_to_string(code));
    }
    
    return err;
}

error_t error_create_at(error_code_t code, const char *message, size_t position) {
    error_t err = error_create(code, message);
    err.has_position = 1;
    err.position = position;
    return err;
}

const char *error_to_string(error_code_t code) {
    switch (code) {
        case ERR_NONE:          return "No error";
        case ERR_SYNTAX:        return "Syntax error";
        case ERR_DOMAIN:        return "Domain error";
        case ERR_OVERFLOW:      return "Numeric overflow";
        case ERR_UNDERFLOW:     return "Numeric underflow";
        case ERR_DIV_ZERO:      return "Division by zero";
        case ERR_INVALID_ARGS:  return "Invalid arguments";
        case ERR_DIMENSION:     return "Dimension mismatch";
        case ERR_MEMORY:        return "Memory allocation failed";
        case ERR_UNSUPPORTED:   return "Unsupported operation";
        case ERR_PARSE:         return "Parse error";
        case ERR_EVAL:          return "Evaluation error";
        case ERR_UNKNOWN:       return "Unknown error";
        default:                return "Unrecognized error";
    }
}

int error_is_ok(error_t err) {
    return err.code == ERR_NONE;
}

error_t error_ok(void) {
    error_t err;
    err.code = ERR_NONE;
    err.has_position = 0;
    err.position = 0;
    err.message[0] = '\0';
    return err;
}
