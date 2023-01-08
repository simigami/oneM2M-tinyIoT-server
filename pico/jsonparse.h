#ifndef __JSONPARSE_H__
#define __JSONPARSE_H__

#include "cJSON.h"

CSE* json_to_cse(char *json_payload);
AE* json_to_ae(char *json_payload);
CNT* json_to_cnt(char *json_payload);
CIN* json_to_cin(char *json_payload);
Sub* json_to_sub(char *json_payload);
ACP* json_to_acp(char *json_payload);

char* cse_to_json(CSE* cse_object);
char* ae_to_json(AE* ae_object);
char* cnt_to_json(CNT* cnt_object);
char* cin_to_json(CIN* cin_object);
char* sub_to_json(Sub *sub_object);
char* notification_to_json(char *sur, int net, char *rep);
char* node_to_json(Node *node);
char* acp_to_json(ACP *acp_object);
char* discovery_to_json(char **result, int size);

char* get_json_value_char(char *key, char *json);
int get_json_value_int(char *key, char *json);
int get_json_value_bool(char *key, char *json);
char *get_json_value_list(char *key, char *json);

bool json_key_exist(char *json, char *key);

#endif