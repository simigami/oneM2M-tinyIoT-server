#include "onem2m.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

AE* JSON_to_AE(char *json_payload) {
	AE *ae = (AE *)malloc(sizeof(AE));

	cJSON *root = NULL;
	cJSON *api = NULL;
	cJSON *rr = NULL;
	cJSON *rn = NULL;

	cJSON* json = cJSON_Parse(json_payload);
	if (json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	root = cJSON_GetObjectItem(json, "m2m:ae");

	// api
	api = cJSON_GetObjectItem(root, "api");
	if (!cJSON_IsString(api) && (api->valuestring == NULL))
	{
		goto end;
	}
	ae->api = cJSON_Print(api);
	ae->api = strtok(ae->api, "\"");

	// rr
	rr = cJSON_GetObjectItemCaseSensitive(root, "rr");
	if (!cJSON_IsTrue(rr) && !cJSON_IsFalse(rr))
	{
		goto end;
	}
	else if (cJSON_IsTrue(rr))
	{
		ae->rr = true;
	}
	else if (cJSON_IsFalse(rr))
	{
		ae->rr = false;
	}
	
	// rn
	rn = cJSON_GetObjectItem(root, "rn");
	if (!cJSON_IsString(rn) && (rn->valuestring == NULL))
	{
		goto end;
	}
	ae->rn = cJSON_Print(rn);
	ae->rn = strtok(ae->rn, "\"");

end:
	cJSON_Delete(json);

	return ae;
}

CNT* JSON_to_CNT(char *json_payload) {
	CNT *cnt = (CNT *)malloc(sizeof(CNT));

	cJSON *root = NULL;
	cJSON *rn = NULL;

	cJSON* json = cJSON_Parse(json_payload);
	if (json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	root = cJSON_GetObjectItem(json, "m2m:cnt");

	// rn
	rn = cJSON_GetObjectItem(root, "rn");
	if (!cJSON_IsString(rn) && (rn->valuestring == NULL))
	{
		goto end;
	}
	cnt->rn = cJSON_Print(rn);
	cnt->rn = strtok(cnt->rn, "\"");

end:
	cJSON_Delete(json);

	return cnt;
}

char* CNT_to_json(CNT* cnt_object) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *cnt = NULL;

	/* Our "cnt" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:cnt", cnt = cJSON_CreateObject());
	cJSON_AddStringToObject(cnt, "rn", cnt_object->rn);
	cJSON_AddNumberToObject(cnt, "ty", cnt_object->ty);
	cJSON_AddStringToObject(cnt, "pi", cnt_object->pi);
	cJSON_AddStringToObject(cnt, "ri", cnt_object->ri);
	cJSON_AddStringToObject(cnt, "ct", cnt_object->ct);
	cJSON_AddStringToObject(cnt, "lt", cnt_object->lt);
	cJSON_AddNumberToObject(cnt, "st", cnt_object->st);
	cJSON_AddStringToObject(cnt, "et", cnt_object->et);
	cJSON_AddNumberToObject(cnt, "cni", cnt_object->cni);
	cJSON_AddNumberToObject(cnt, "cbs", cnt_object->cbs);

	json = cJSON_Print(root);

	cJSON_Delete(root);

	return json;
}

char* AE_to_json(AE *ae_object) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *ae = NULL;

	/* Our "ae" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:ae", ae = cJSON_CreateObject());
	cJSON_AddStringToObject(ae, "rn", ae_object->rn);
	cJSON_AddNumberToObject(ae, "ty", ae_object->ty);
	cJSON_AddStringToObject(ae, "pi", ae_object->pi);
	cJSON_AddStringToObject(ae, "ri", ae_object->ri);
	cJSON_AddStringToObject(ae, "ct", ae_object->ct);
	cJSON_AddStringToObject(ae, "lt", ae_object->lt);
	cJSON_AddStringToObject(ae, "et", ae_object->et);
	cJSON_AddStringToObject(ae, "api", ae_object->api);
	cJSON_AddBoolToObject(ae, "rr", ae_object->rr);
	cJSON_AddStringToObject(ae, "aei", ae_object->aei);

	json = cJSON_Print(root);

	cJSON_Delete(root);

	return json;
}
