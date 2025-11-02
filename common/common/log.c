#include "common/log.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static LogLevel g_loglevel = LOG_LEVEL_DEBUG;

#define ASCII_RESET "\e[0m"
#define ASCII_ERROR "\e[0;31m"
#define ASCII_DEBUG "\e[0;34m"
#define ASCII_FATAL "\e[0;31m"
#define ASCII_WARN "\e[0;33m"
#define ASCII_INFO "\e[0;32m"

void log_set(LogLevel lvl) { g_loglevel = lvl; }

void log_log(LogLevel lvl, const char *fmt, ...) {
    if (lvl > g_loglevel || g_loglevel == LOG_LEVEL_OFF) return;

    time_t rt;
    struct tm *info;
    char buffer[80];
    time(&rt);

    info = localtime(&rt);

    strftime(buffer, 80, "%Y-%m-%dT%XZ", info);
    printf("%s ", buffer);

    switch (lvl) {
    case LOG_LEVEL_ERROR: {
        printf("[" ASCII_ERROR "ERROR" ASCII_RESET "] ");
        break;
    }
    case LOG_LEVEL_DEBUG: {
        printf("[" ASCII_DEBUG "DEBUG" ASCII_RESET "] ");
        break;
    }
    case LOG_LEVEL_FATAL: {
        printf("[" ASCII_FATAL "FATAL" ASCII_RESET " ] ");
        break;
    }
    case LOG_LEVEL_INFO: {
        printf("[" ASCII_INFO "INFO" ASCII_RESET " ] ");
        break;
    }
    case LOG_LEVEL_WARN: {
        printf("[" ASCII_WARN "WARN" ASCII_RESET " ] ");
        break;
    }
    default:
        break;
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}
