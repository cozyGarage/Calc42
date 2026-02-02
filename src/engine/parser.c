#include "engine/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Value constructors
value_t value_number(double num) {
  value_t val;
  val.type = VALUE_NUMBER;
  val.as.number = num;
  return val;
}

value_t value_array(double *data, size_t size) {
  value_t val;
  val.type = VALUE_ARRAY;
  val.as.array.data = data;
  val.as.array.size = size;
  return val;
}

value_t value_matrix(double *data, size_t rows, size_t cols) {
  value_t val;
  val.type = VALUE_MATRIX;
  val.as.matrix.data = data;
  val.as.matrix.rows = rows;
  val.as.matrix.cols = cols;
  return val;
}

void value_free(value_t *val) {
  if (!val)
    return;

  if (val->type == VALUE_ARRAY && val->as.array.data) {
    safe_free(val->as.array.data);
    val->as.array.data = NULL;
  } else if (val->type == VALUE_MATRIX && val->as.matrix.data) {
    safe_free(val->as.matrix.data);
    val->as.matrix.data = NULL;
  }
}

value_t value_clone(const value_t *val) {
  if (!val) {
    return value_number(0);
  }

  if (val->type == VALUE_NUMBER) {
    return value_number(val->as.number);
  } else if (val->type == VALUE_ARRAY) {
    size_t size = val->as.array.size;
    double *data = safe_malloc(size * sizeof(double));
    if (data) {
      memcpy(data, val->as.array.data, size * sizeof(double));
    }
    return value_array(data, size);
  } else if (val->type == VALUE_MATRIX) {
    size_t total = val->as.matrix.rows * val->as.matrix.cols;
    double *data = safe_malloc(total * sizeof(double));
    if (data) {
      memcpy(data, val->as.matrix.data, total * sizeof(double));
    }
    return value_matrix(data, val->as.matrix.rows, val->as.matrix.cols);
  }

  return value_number(0);
}

// AST node functions
static ast_node_t *ast_node_create(node_type_t type) {
  ast_node_t *node = safe_calloc(1, sizeof(ast_node_t));
  if (node) {
    node->type = type;
    node->children = NULL;
    node->child_count = 0;
  }
  return node;
}

/* Unused for now - will be needed for function calls with arguments
static int ast_node_add_child(ast_node_t *parent, ast_node_t *child) {
    if (!parent || !child) return -1;

    ast_node_t **new_children = safe_realloc(parent->children,
                                              (parent->child_count + 1) *
sizeof(ast_node_t *)); if (!new_children) return -1;

    parent->children = new_children;
    parent->children[parent->child_count] = child;
    parent->child_count++;

    return 0;
}
*/
void ast_free(ast_node_t *node) {
  if (!node)
    return;

  for (size_t i = 0; i < node->child_count; i++) {
    ast_free(node->children[i]);
  }

  safe_free(node->children);
  free(node);
}

// Operator precedence
static int get_precedence(const char *op) {
  if (strcmp(op, "<<") == 0 || strcmp(op, ">>") == 0)
    return 1;
  if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0)
    return 2;
  if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0)
    return 3;
  if (strcmp(op, "&") == 0)
    return 4;
  if (strcmp(op, "^") == 0)
    return 5;
  if (strcmp(op, "|") == 0)
    return 6;
  return 0;
}

static int is_right_associative(const char *op) {
  (void)op; // All our operators are left-associative
  return 0;
}

// Shunting-yard algorithm implementation with function call support
ast_node_t *parse(const char *expression, error_t *error) {
  if (!expression) {
    if (error)
      *error = error_create(ERR_PARSE, "Null expression");
    return NULL;
  }

  error_t tok_error;
  darray_t *tokens = tokenize(expression, &tok_error);
  if (!tokens) {
    if (error)
      *error = tok_error;
    return NULL;
  }

  // Operator stack and output queue for shunting-yard
  darray_t *operator_stack = darray_create(sizeof(token_t), 16);
  darray_t *output_queue = darray_create(sizeof(ast_node_t *), 16);
  darray_t *arg_count_stack =
      darray_create(sizeof(int), 16); // Track argument counts

  if (!operator_stack || !output_queue || !arg_count_stack) {
    if (error)
      *error = error_create(ERR_MEMORY, "Failed to create parser stacks");
    darray_free(tokens);
    darray_free(operator_stack);
    darray_free(output_queue);
    darray_free(arg_count_stack);
    return NULL;
  }

  // Process tokens
  for (size_t i = 0; i < tokens->size; i++) {
    token_t *tok = (token_t *)darray_get(tokens, i);

    if (tok->type == TOKEN_NUMBER) {
      ast_node_t *node = ast_node_create(NODE_NUMBER);
      node->num_value = tok->num_value;
      darray_append(output_queue, &node);

    } else if (tok->type == TOKEN_FUNCTION) {
      // Push function onto operator stack
      darray_append(operator_stack, tok);

      // Check if next token is '('
      if (i + 1 < tokens->size) {
        token_t *next = (token_t *)darray_get(tokens, i + 1);
        if (next->type == TOKEN_LPAREN) {
          int arg_count = 0;
          darray_append(arg_count_stack, &arg_count);
        }
      }

    } else if (tok->type == TOKEN_COMMA) {
      // Comma separates function arguments
      // Pop operators until we find '('
      int found = 0;
      while (operator_stack->size > 0) {
        token_t *top =
            (token_t *)darray_get(operator_stack, operator_stack->size - 1);

        if (top->type == TOKEN_LPAREN) {
          found = 1;
          // Increment argument count
          if (arg_count_stack->size > 0) {
            int *count =
                (int *)darray_get(arg_count_stack, arg_count_stack->size - 1);
            (*count)++;
          }
          break;
        }

        operator_stack->size--;

        if (top->type == TOKEN_OPERATOR) {
          ast_node_t *node = ast_node_create(NODE_OPERATOR);
          snprintf(node->op, MAX_TOKEN_LENGTH, "%s", top->value);
          darray_append(output_queue, &node);
        }
      }

      if (!found) {
        if (error)
          *error = error_create(ERR_PARSE, "Misplaced comma");
        // Cleanup
        darray_free(tokens);
        darray_free(operator_stack);
        darray_free(arg_count_stack);
        for (size_t j = 0; j < output_queue->size; j++) {
          ast_node_t **node_ptr = (ast_node_t **)darray_get(output_queue, j);
          ast_free(*node_ptr);
        }
        darray_free(output_queue);
        return NULL;
      }

    } else if (tok->type == TOKEN_OPERATOR) {
      while (operator_stack->size > 0) {
        token_t *top =
            (token_t *)darray_get(operator_stack, operator_stack->size - 1);

        if (top->type != TOKEN_OPERATOR)
          break;

        int prec = get_precedence(tok->value);
        int top_prec = get_precedence(top->value);

        if ((top_prec > prec) ||
            (top_prec == prec && !is_right_associative(tok->value))) {
          operator_stack->size--;

          ast_node_t *node = ast_node_create(NODE_OPERATOR);
          snprintf(node->op, MAX_TOKEN_LENGTH, "%s", top->value);
          darray_append(output_queue, &node);
        } else {
          break;
        }
      }

      darray_append(operator_stack, tok);

    } else if (tok->type == TOKEN_LPAREN) {
      darray_append(operator_stack, tok);

    } else if (tok->type == TOKEN_RPAREN) {
      // Pop until matching (
      int found_lparen = 0;
      while (operator_stack->size > 0) {
        token_t *top =
            (token_t *)darray_get(operator_stack, operator_stack->size - 1);
        operator_stack->size--;

        if (top->type == TOKEN_LPAREN) {
          found_lparen = 1;

          // Check if there's a function before the '('
          if (operator_stack->size > 0) {
            token_t *maybe_func =
                (token_t *)darray_get(operator_stack, operator_stack->size - 1);
            if (maybe_func->type == TOKEN_FUNCTION) {
              operator_stack->size--;

              ast_node_t *node = ast_node_create(NODE_FUNCTION);
              snprintf(node->op, MAX_TOKEN_LENGTH, "%s", maybe_func->value);

              // Get argument count
              int arg_count = 1; // Default to 1 if we have any content
              if (arg_count_stack->size > 0) {
                int *count = (int *)darray_get(arg_count_stack,
                                               arg_count_stack->size - 1);
                arg_count = *count + 1; // +1 for the last argument
                arg_count_stack->size--;
              }

              // Store arg count in a marker node (will be processed during AST
              // building)
              node->child_count = arg_count;

              darray_append(output_queue, &node);
            }
          }
          break;
        }

        if (top->type == TOKEN_OPERATOR) {
          ast_node_t *node = ast_node_create(NODE_OPERATOR);
          snprintf(node->op, MAX_TOKEN_LENGTH, "%s", top->value);
          darray_append(output_queue, &node);
        } else if (top->type == TOKEN_FUNCTION) {
          ast_node_t *node = ast_node_create(NODE_FUNCTION);
          snprintf(node->op, MAX_TOKEN_LENGTH, "%s", top->value);
          darray_append(output_queue, &node);
        }
      }

      if (!found_lparen) {
        if (error)
          *error = error_create(ERR_PARSE, "Mismatched parentheses");
        darray_free(tokens);
        darray_free(operator_stack);
        darray_free(arg_count_stack);
        for (size_t j = 0; j < output_queue->size; j++) {
          ast_node_t **node_ptr = (ast_node_t **)darray_get(output_queue, j);
          ast_free(*node_ptr);
        }
        darray_free(output_queue);
        return NULL;
      }
    }
  }

  // Pop remaining operators
  while (operator_stack->size > 0) {
    token_t *top =
        (token_t *)darray_get(operator_stack, operator_stack->size - 1);
    operator_stack->size--;

    if (top->type == TOKEN_LPAREN || top->type == TOKEN_RPAREN) {
      if (error)
        *error = error_create(ERR_PARSE, "Mismatched parentheses");
      darray_free(tokens);
      darray_free(operator_stack);
      darray_free(arg_count_stack);
      for (size_t j = 0; j < output_queue->size; j++) {
        ast_node_t **node_ptr = (ast_node_t **)darray_get(output_queue, j);
        ast_free(*node_ptr);
      }
      darray_free(output_queue);
      return NULL;
    }

    if (top->type == TOKEN_OPERATOR) {
      ast_node_t *node = ast_node_create(NODE_OPERATOR);
      snprintf(node->op, MAX_TOKEN_LENGTH, "%s", top->value);
      darray_append(output_queue, &node);
    } else if (top->type == TOKEN_FUNCTION) {
      ast_node_t *node = ast_node_create(NODE_FUNCTION);
      snprintf(node->op, MAX_TOKEN_LENGTH, "%s", top->value);
      darray_append(output_queue, &node);
    }
  }

  // Build AST from postfix notation using a stack
  darray_t *build_stack = darray_create(sizeof(ast_node_t *), 16);
  if (!build_stack) {
    if (error)
      *error = error_create(ERR_MEMORY, "Failed to create build stack");
    for (size_t j = 0; j < output_queue->size; j++) {
      ast_node_t **node_ptr = (ast_node_t **)darray_get(output_queue, j);
      ast_free(*node_ptr);
    }
    darray_free(output_queue);
    darray_free(tokens);
    darray_free(operator_stack);
    darray_free(arg_count_stack);
    return NULL;
  }

  for (size_t i = 0; i < output_queue->size; i++) {
    ast_node_t **node_ptr = (ast_node_t **)darray_get(output_queue, i);
    ast_node_t *node = *node_ptr;

    if (node->type == NODE_NUMBER) {
      // Push number onto stack
      darray_append(build_stack, &node);

    } else if (node->type == NODE_FUNCTION) {
      // Pop arguments for function
      int arg_count = node->child_count;

      if ((size_t)arg_count > build_stack->size) {
        if (error)
          *error = error_create(ERR_PARSE, "Not enough arguments for function");
        ast_free(node);
        for (size_t j = 0; j < build_stack->size; j++) {
          ast_node_t **n = (ast_node_t **)darray_get(build_stack, j);
          ast_free(*n);
        }
        darray_free(build_stack);
        darray_free(output_queue);
        darray_free(tokens);
        darray_free(operator_stack);
        darray_free(arg_count_stack);
        return NULL;
      }

      // Allocate children array
      if (arg_count > 0) {
        node->children = safe_malloc(arg_count * sizeof(ast_node_t *));
        if (!node->children) {
          if (error)
            *error = error_create(ERR_MEMORY,
                                  "Failed to allocate function arguments");
          ast_free(node);
          for (size_t j = 0; j < build_stack->size; j++) {
            ast_node_t **n = (ast_node_t **)darray_get(build_stack, j);
            ast_free(*n);
          }
          darray_free(build_stack);
          darray_free(output_queue);
          darray_free(tokens);
          darray_free(operator_stack);
          darray_free(arg_count_stack);
          return NULL;
        }

        // Pop arguments in reverse order (they're on stack)
        for (int j = arg_count - 1; j >= 0; j--) {
          ast_node_t **arg_ptr =
              (ast_node_t **)darray_get(build_stack, build_stack->size - 1);
          node->children[j] = *arg_ptr;
          build_stack->size--;
        }
      }

      // Push function node onto stack
      darray_append(build_stack, &node);

    } else if (node->type == NODE_OPERATOR) {
      // Pop 2 operands, make them children
      if (build_stack->size < 2) {
        if (error)
          *error = error_create(ERR_PARSE, "Not enough operands");
        ast_free(node);
        for (size_t j = 0; j < build_stack->size; j++) {
          ast_node_t **n = (ast_node_t **)darray_get(build_stack, j);
          ast_free(*n);
        }
        darray_free(build_stack);
        darray_free(output_queue);
        darray_free(tokens);
        darray_free(operator_stack);
        darray_free(arg_count_stack);
        return NULL;
      }

      // Pop right operand
      ast_node_t **right_ptr =
          (ast_node_t **)darray_get(build_stack, build_stack->size - 1);
      ast_node_t *right = *right_ptr;
      build_stack->size--;

      // Pop left operand
      ast_node_t **left_ptr =
          (ast_node_t **)darray_get(build_stack, build_stack->size - 1);
      ast_node_t *left = *left_ptr;
      build_stack->size--;

      // Make them children of the operator node
      node->children = safe_malloc(2 * sizeof(ast_node_t *));
      if (!node->children) {
        if (error)
          *error = error_create(ERR_MEMORY, "Failed to allocate children");
        ast_free(node);
        ast_free(left);
        ast_free(right);
        for (size_t j = 0; j < build_stack->size; j++) {
          ast_node_t **n = (ast_node_t **)darray_get(build_stack, j);
          ast_free(*n);
        }
        darray_free(build_stack);
        darray_free(output_queue);
        darray_free(tokens);
        darray_free(operator_stack);
        darray_free(arg_count_stack);
        return NULL;
      }
      node->children[0] = left;
      node->children[1] = right;
      node->child_count = 2;

      // Push operator node onto stack
      darray_append(build_stack, &node);
    }
  }

  // The final result should be the only item on the stack
  ast_node_t *result = NULL;
  if (build_stack->size == 1) {
    ast_node_t **result_ptr = (ast_node_t **)darray_get(build_stack, 0);
    result = *result_ptr;
  } else if (build_stack->size == 0) {
    if (error)
      *error = error_create(ERR_PARSE, "Empty expression");
  } else {
    if (error)
      *error = error_create(ERR_PARSE, "Invalid expression");
    // Clean up extra nodes
    for (size_t j = 0; j < build_stack->size; j++) {
      ast_node_t **n = (ast_node_t **)darray_get(build_stack, j);
      ast_free(*n);
    }
  }

  if (error && result)
    *error = error_ok();

  darray_free(build_stack);
  darray_free(tokens);
  darray_free(operator_stack);
  darray_free(arg_count_stack);
  darray_free(output_queue);

  return result;
}
