#ifndef ENABLE_MQTT_TLS
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#include <string.h>

#include "wolfmqtt/mqtt_client.h"
#include "config.h"
#include "onem2m.h"
#include "cJSON.h"

int mqtt_ser(void);