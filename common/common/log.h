#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include <stdlib.h>

typedef enum {
    LOG_LEVEL_OFF = 0,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
} LogLevel;

void log_set(LogLevel lvl);
void log_log(LogLevel lvl, const char *fmt, ...);

#define log_assert(expr, fmt, ...)                                             \
    do {                                                                       \
        if (!(expr)) {                                                         \
            log_log(LOG_LEVEL_WARN, "Assert failed " fmt, ##__VA_ARGS__);      \
        }                                                                      \
    } while (0)

#define log_fatal(fmt, ...)                                                    \
    do {                                                                       \
        log_log(LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__);                          \
        exit(EXIT_FAILURE);                                                    \
    } while (0)

#define log_error(fmt, ...)                                                    \
    do {                                                                       \
        log_log(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__);                          \
    } while (0)
#define log_warn(fmt, ...)                                                     \
    do {                                                                       \
        log_log(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__);                           \
    } while (0)
#define log_info(fmt, ...)                                                     \
    do {                                                                       \
        log_log(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__);                           \
    } while (0)
#define log_debug(fmt, ...)                                                    \
    do {                                                                       \
        log_log(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__);                          \
    } while (0)

#endif // COMMON_LOG_H
