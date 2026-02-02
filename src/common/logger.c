#include "common/logger.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

static FILE *log_file = NULL;

int logger_init(const char *filename) {
    if (filename) {
        log_file = fopen(filename, "a");
        if (!log_file) {
            return -1;
        }
    }
    return 0;
}

static const char *level_to_string(log_level_t level) {
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR:   return "ERROR";
        default:          return "UNKNOWN";
    }
}

static void log_json(const char *json) {
    // Get timestamp
    time_t now = time(NULL);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", gmtime(&now));
    
    // Log to stdout
    printf("{\"timestamp\":\"%s\",%s}\n", timestamp, json);
    
    // Log to file if available
    if (log_file) {
        fprintf(log_file, "{\"timestamp\":\"%s\",%s}\n", timestamp, json);
        fflush(log_file);
    }
}

void logger_log(log_level_t level, const char *category, const char *message) {
    char json[512];
    snprintf(json, sizeof(json),
             "\"level\":\"%s\",\"category\":\"%s\",\"message\":\"%s\"",
             level_to_string(level), category, message);
    log_json(json);
}

void logger_log_expression(const char *expression, const char *result) {
    char json[512];
    snprintf(json, sizeof(json),
             "\"type\":\"expression\",\"expression\":\"%s\",\"result\":\"%s\"",
             expression, result);
    log_json(json);
}

void logger_log_mode_switch(const char *from_mode, const char *to_mode) {
    char json[256];
    snprintf(json, sizeof(json),
             "\"type\":\"mode_switch\",\"from\":\"%s\",\"to\":\"%s\"",
             from_mode, to_mode);
    log_json(json);
}

void logger_log_error(const char *error_message, const char *expression) {
    char json[512];
    snprintf(json, sizeof(json),
             "\"type\":\"error\",\"error\":\"%s\",\"expression\":\"%s\"",
             error_message, expression ? expression : "");
    log_json(json);
}

void logger_shutdown(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
