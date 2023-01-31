#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "logger.h"
#include "config.h"

void logger(char *msg, LOGLEVEL level){

    if(LOG_LEVEL <= level){
        fprintf(stderr, msg);
    }

}