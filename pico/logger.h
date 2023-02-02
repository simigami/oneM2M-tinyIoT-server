#ifndef __LOGGER_H__
#define __LOGGER_H__

typedef enum{
    LOG_LEVEL_DEBUG = 94,
    LOG_LEVEL_INFO = 32,
    LOG_LEVEL_WARN = 93,
    LOG_LEVEL_ERROR = 91,
    LOG_LEVEL_FATAL = 31
}LOGLEVEL;

typedef enum{
BLUE = 94,
GREEN = 32,
YELLOW = 93,
BR_RED = 91,
RED = 31
}COLOR;

int logger(const char* tag,  LOGLEVEL level, const char *msg, ...);

#endif