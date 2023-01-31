#ifndef __LOGGER__
#define __LOGGER__
#endif


typedef enum{
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
}LOGLEVEL;

void logger(char *msg, LOGLEVEL level);
