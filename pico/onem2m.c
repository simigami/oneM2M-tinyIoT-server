#include "httpd.h"
#include "onem2m.h"
#include "cJSON.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


static int print_preallocated(cJSON *root)
{
	/* declarations */
	char *out = NULL;
	char *buf = NULL;
	char *buf_fail = NULL;
	size_t len = 0;
	size_t len_fail = 0;

	/* formatted print */
	out = cJSON_Print(root);

	/* create buffer to succeed */
	/* the extra 5 bytes are because of inaccuracies when reserving memory */
	len = strlen(out) + 5;
	buf = (char*)malloc(len);
	if (buf == NULL)
	{
		printf("Failed to allocate memory.\n");
		exit(1);
	}

	/* create buffer to fail */
	len_fail = strlen(out);
	buf_fail = (char*)malloc(len_fail);
	if (buf_fail == NULL)
	{
		printf("Failed to allocate memory.\n");
		exit(1);
	}

	/* Print to buffer */
	if (!cJSON_PrintPreallocated(root, buf, (int)len, 1)) {
		printf("cJSON_PrintPreallocated failed!\n");
		if (strcmp(out, buf) != 0) {
			printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
			printf("cJSON_Print result:\n%s\n", out);
			printf("cJSON_PrintPreallocated result:\n%s\n", buf);
		}
		free(out);
		free(buf_fail);
		free(buf);
		return -1;
	}

	/* success */
	printf("%s\n", buf);

	/* force it to fail */
	if (cJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
		printf("cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
		printf("cJSON_Print result:\n%s\n", out);
		printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
		free(out);
		free(buf_fail);
		free(buf);
		return -1;
	}

	free(out);
	free(buf_fail);
	free(buf);
	return 0;
}

static void create_objects(AE* ae_object)
{
	/* declare a few. */
	cJSON *root = NULL;
	cJSON *ae = NULL;

	/* Our "ae" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:ae", ae = cJSON_CreateObject());
	cJSON_AddStringToObject(ae, "rn", ae_object->rn);
	cJSON_AddStringToObject(ae, "api", ae_object->api);
	if(ae_object->rr) cJSON_AddBoolToObject(ae, "rr", "true");
	else cJSON_AddBoolToObject(ae, "rr", "false");

	if (print_preallocated(root) != 0) {
		cJSON_Delete(root);
		exit(EXIT_FAILURE);
	}
	cJSON_Delete(root);
}

AE* Create_AE(char *json_payload) {	
	AE* created_ae = malloc(sizeof(AE));
	cJSON *root = NULL;
	cJSON *api = NULL;
	cJSON *rr = NULL;
	cJSON *rn = NULL;

	cJSON *json = cJSON_Parse(json_payload);
	if (json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	root = cJSON_GetObjectItem(json, "m2m:ae");	// payload에서 "m2m:ae" 부분을 파싱

	api = cJSON_GetObjectItem(root, "api");	//  "api" 파싱
	if (!cJSON_IsString(api) && (api->valuestring == NULL))
	{
		goto end;
	}
	
	created_ae->api = malloc(sizeof(api->valuestring));
	strcpy(created_ae->api,api->valuestring);
	
	rr = cJSON_GetObjectItemCaseSensitive(root, "rr");	// "rr" 파싱
	if (!cJSON_IsTrue(rr) && !cJSON_IsFalse(rr))
	{
		goto end;
	}
	else if (cJSON_IsTrue(rr))
	{
		rr->type = cJSON_True;
		rr->valueint = 1;
	}
	else if (cJSON_IsFalse(rr))
	{
		rr->type = cJSON_False;
		rr->valuedouble = 0;
	}
	
	created_ae->rr = malloc(sizeof(bool));
	
	if(rr->valueint == 1) created_ae->rr = true;
	else created_ae->rr = false; 
	
	rn = cJSON_GetObjectItem(root, "rn");	// "rn" 파싱
	if (!cJSON_IsString(rn) && (rn->valuestring == NULL))
	{
		goto end;
	}
	
	created_ae->rn = malloc(sizeof(rn->valuestring));
	strcpy(created_ae->rn, rn->valuestring);
	
	end:
	    cJSON_Delete(json);
	    
	create_objects(created_ae);
	
	return created_ae;
}


