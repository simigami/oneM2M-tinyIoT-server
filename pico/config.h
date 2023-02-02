#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "logger.h"

#define SERVER_PORT "3000"
#define CSE_BASE_NAME "TinyIoT"
#define CSE_BASE_RI "tinyiot"

#define MONO_THREAD 0 // 0 → multithread, 1 → monothread

#define MAX_PAYLOAD_SIZE 16384
#define MAX_URI_SIZE 1024
#define MAX_PROPERTY_SIZE 16384

// MQTT Settings
#define ENABLE_MQTT

#ifdef ENABLE_MQTT
#define MQTT_HOST            "127.0.0.1"
#define MQTT_QOS             MQTT_QOS_0
#define MQTT_KEEP_ALIVE_SEC  60
#define MQTT_CMD_TIMEOUT_MS  30000
#define MQTT_CON_TIMEOUT_MS  5000
#define MQTT_CLIENT_ID       "TinyIoT"
#define MQTT_USERNAME        "test"
#define MQTT_PASSWORD        "mqtt"

#define topicPrefix ""

// MQTT TLS Setting
#ifdef ENABLE_MQTT_TLS
    #define MQTT_USE_TLS     1
    #define MQTT_PORT        8883
#else
    #define MQTT_USE_TLS     0
    #define MQTT_PORT        1883
#endif

#define MQTT_MAX_PACKET_SZ   16384
#define INVALID_SOCKET_FD    -1
#define PRINT_BUFFER_SIZE    16384

#endif

#include "logger.h"
#define LOG_LEVEL LOG_LEVEL_DEBUG
#define LOG_BUFFER_SIZE 1024

#endif