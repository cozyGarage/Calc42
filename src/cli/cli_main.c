#include "common/logger.h"
#include "engine/engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Check if readline is available
#ifdef __APPLE__
#include <readline/history.h>
#include <readline/readline.h>
#define HAVE_READLINE 1
#elif __linux__
#include <readline/history.h>
#include <readline/readline.h>
#define HAVE_READLINE 1
#else
#define HAVE_READLINE 0
#endif

static const char *mode_to_string(calc_mode_t mode) {
  switch (mode) {
  case MODE_STANDARD:
    return "standard";
  case MODE_PROGRAMMER:
    return "programmer";
  case MODE_STATISTICS:
    return "statistics";
  case MODE_PROBABILITY:
    return "probability";
  case MODE_DISCRETE:
    return "discrete";
  case MODE_LINEAR_ALGEBRA:
    return "linalg";
  default:
    return "unknown";
  }
}

static void print_help(void) {
  printf("CALC42 - Multi-Mode Calculator\n");
  printf("Commands:\n");
  printf("  :mode <mode>  - Switch mode (standard, programmer, statistics, "
         "etc.)\n");
  printf("  :base <n>     - Set base for programmer mode (2, 8, 10, 16)\n");
  printf("  :help         - Show this help\n");
  printf("  :quit         - Exit calculator\n");
  printf("\nExamples:\n");
  printf("  3 + 4 * 2     = 11\n");
  printf("  0xFF & 0x0F   = 0x0F (programmer mode)\n");
  printf("  1 << 4        = 16 (programmer mode)\n");
  printf("  gcd(12, 18)   = 6\n");
  printf("  vec_dot(1,2,3,4,5,6) = 32\n");
#if HAVE_READLINE
  printf("\nReadline shortcuts:\n");
  printf("  Up/Down       - Navigate history\n");
  printf("  Ctrl-R        - Reverse search history\n");
  printf("  Tab           - Command completion (if available)\n");
#endif
}

int main(int argc, char **argv) {
  logger_init("calc42.log");

  // Check for one-shot mode
  if (argc > 1) {
    // Concatenate all arguments as expression
    size_t total_len = 0;
    for (int i = 1; i < argc; i++) {
      total_len += strlen(argv[i]) + 1;
    }

    char *expression = safe_malloc(total_len + 1);
    if (!expression) {
      fprintf(stderr, "Memory allocation failed\n");
      return 1;
    }

    expression[0] = '\0';
    for (int i = 1; i < argc; i++) {
      strcat(expression, argv[i]);
      if (i < argc - 1)
        strcat(expression, " ");
    }

    engine_context_t *ctx = engine_context_create(MODE_STANDARD);
    error_t error;
    value_t result = engine_eval(expression, ctx, &error);

    if (error_is_ok(error)) {
      char *result_str = value_to_string(&result, ctx->base);
      printf("%s\n", result_str);
      safe_free(result_str);
      value_free(&result);
    } else {
      fprintf(stderr, "Error: %s\n", error.message);
      logger_log_error(error.message, expression);
    }

    safe_free(expression);
    engine_context_free(ctx);
    logger_shutdown();
    return error_is_ok(error) ? 0 : 1;
  }

  // Interactive REPL mode
  printf("CALC42 Interactive Calculator\n");
  printf("Type :help for help, :quit to exit\n");
#if HAVE_READLINE
  printf("(readline enabled - use arrow keys for history)\n");
#endif
  printf("\n");

  engine_context_t *ctx = engine_context_create(MODE_STANDARD);

#if HAVE_READLINE
  // Set up history
  using_history();
  char history_file[256];
  snprintf(history_file, sizeof(history_file), "%s/.calc42_history",
           getenv("HOME") ? getenv("HOME") : ".");
  read_history(history_file);
#endif

  while (1) {
    char prompt[64];
    snprintf(prompt, sizeof(prompt), "[%s] > ", mode_to_string(ctx->mode));

#if HAVE_READLINE
    char *line = readline(prompt);

    // Check for EOF (Ctrl-D)
    if (!line) {
      printf("\n");
      break;
    }

    // Skip empty lines
    if (strlen(line) == 0) {
      free(line);
      continue;
    }

    // Add to history (will be saved on exit)
    add_history(line);
#else
    printf("%s", prompt);
    fflush(stdout);

    char line_buf[1024];
    if (!fgets(line_buf, sizeof(line_buf), stdin)) {
      break;
    }

    // Remove newline
    size_t len = strlen(line_buf);
    if (len > 0 && line_buf[len - 1] == '\n') {
      line_buf[len - 1] = '\0';
    }

    // Skip empty lines
    if (strlen(line_buf) == 0) {
      continue;
    }

    char *line = line_buf;
#endif

    // Handle commands
    if (line[0] == ':') {
      if (strcmp(line, ":quit") == 0 || strcmp(line, ":q") == 0) {
#if HAVE_READLINE
        free(line);
#endif
        break;
      } else if (strcmp(line, ":help") == 0 || strcmp(line, ":h") == 0) {
        print_help();
      } else if (strncmp(line, ":mode ", 6) == 0) {
        const char *mode_name = line + 6;
        calc_mode_t old_mode = ctx->mode;

        if (strcmp(mode_name, "standard") == 0) {
          ctx->mode = MODE_STANDARD;
        } else if (strcmp(mode_name, "programmer") == 0) {
          ctx->mode = MODE_PROGRAMMER;
        } else if (strcmp(mode_name, "statistics") == 0) {
          ctx->mode = MODE_STATISTICS;
        } else if (strcmp(mode_name, "probability") == 0) {
          ctx->mode = MODE_PROBABILITY;
        } else if (strcmp(mode_name, "discrete") == 0) {
          ctx->mode = MODE_DISCRETE;
        } else if (strcmp(mode_name, "linalg") == 0) {
          ctx->mode = MODE_LINEAR_ALGEBRA;
        } else {
          printf("Unknown mode: %s\n", mode_name);
#if HAVE_READLINE
          free(line);
#endif
          continue;
        }

        printf("Switched to %s mode\n", mode_name);
        logger_log_mode_switch(mode_to_string(old_mode),
                               mode_to_string(ctx->mode));

      } else if (strncmp(line, ":base ", 6) == 0) {
        int base = atoi(line + 6);
        if (base == 2 || base == 8 || base == 10 || base == 16) {
          ctx->base = base;
          printf("Base set to %d\n", base);
        } else {
          printf("Invalid base (must be 2, 8, 10, or 16)\n");
        }
      } else {
        printf("Unknown command: %s\n", line);
      }
#if HAVE_READLINE
      free(line);
#endif
      continue;
    }

    // Evaluate expression
    error_t error;
    value_t result = engine_eval(line, ctx, &error);

    if (error_is_ok(error)) {
      char *result_str = value_to_string(&result, ctx->base);
      printf("= %s\n", result_str);
      logger_log_expression(line, result_str);
      safe_free(result_str);
      value_free(&result);
    } else {
      printf("Error: %s", error.message);
      if (error.has_position) {
        printf(" at position %zu", error.position);
      }
      printf("\n");
      logger_log_error(error.message, line);
    }

#if HAVE_READLINE
    free(line);
#endif
  }

#if HAVE_READLINE
  // Save history on exit
  write_history(history_file);
#endif

  printf("\nGoodbye!\n");
  engine_context_free(ctx);
  logger_shutdown();

  return 0;
}
