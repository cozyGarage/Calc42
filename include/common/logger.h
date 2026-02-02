#ifndef LOGGER_H
#define LOGGER_H

/**
 * Log levels
 */
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} log_level_t;

/**
 * Initialize logger
 * If filename is NULL, logs to stdout only
 */
int logger_init(const char *filename);

/**
 * Log a message with level
 */
void logger_log(log_level_t level, const char *category, const char *message);

/**
 * Log an expression evaluation
 */
void logger_log_expression(const char *expression, const char *result);

/**
 * Log a mode switch
 */
void logger_log_mode_switch(const char *from_mode, const char *to_mode);

/**
 * Log an error
 */
void logger_log_error(const char *error_message, const char *expression);

/**
 * Shutdown logger
 */
void logger_shutdown(void);

#endif // LOGGER_H
