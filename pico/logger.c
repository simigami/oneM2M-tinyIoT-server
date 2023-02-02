#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "logger.h"
#include "config.h"


int logger(const char* tag,  LOGLEVEL level, const char *msg, ...){

    va_list ap;
    int charsCnt = 0;

    if(LOG_LEVEL <= level){
        time_t now;
        time(&now);
        fprintf(stderr, "%s [%s]: ", ctime(&now), tag);

        va_start(ap, msg);
        charsCnt = vprintf(msg, ap);
        va_end(ap);

    }

    return charsCnt;
}