#ifndef __LOGGER_H__
#define __LOGGER_H__

typedef enum{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_FATAL = 4
}LOGLEVEL;

int logger(const char* tag,  LOGLEVEL level, const char *msg, ...);

#endif