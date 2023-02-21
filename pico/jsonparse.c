#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include "onem2m.h"
#include "jsonparse.h"
#include "cJSON.h"
#include "onem2mTypes.h"
#include "config.h"
#include "util.h"

void remove_quotation_mark(char *s){
	int len = strlen(s);
	int index = 0;

	for(int i=0; i<len; i++) {
		if(s[i] != '\\' && s[i] != '\"')
			s[index++] = s[i];
	}
	s[index] = '\0';
}

AE* cjson_to_ae(cJSON *cjson) {
	cJSON *root = NULL;
	cJSON *api = NULL;
	cJSON *rr = NULL;
	cJSON *rn = NULL;
	cJSON *lbl = NULL;
	cJSON *srv = NULL;

	if (cjson == NULL) {
		// const char *error_ptr = cJSON_GetErrorPtr();
		// if (error_ptr != NULL)
		// {
		// 	fprintf(stderr, "Error before: %s\n", error_ptr);
		// }
		return NULL;
	}

	root = cJSON_GetObjectItem(cjson, "m2m:ae");
	if(!root) return NULL;

	AE *ae = (AE *)calloc(1,sizeof(AE));

	// api (mandatory)
	api = cJSON_GetObjectItem(root, "api");
	rr = cJSON_GetObjectItemCaseSensitive(root, "rr");	// + rr
	if (api == NULL || api->valuestring[0] == 0 || isspace(api->valuestring[0])) {
		fprintf(stderr, "Invalid api\n");
		if (!cJSON_IsTrue(rr) && !cJSON_IsFalse(rr)) {
			fprintf(stderr, "Invalid rr\n");
		}
		free_ae(ae);
		return NULL;
	}
	else {
		ae->api = cJSON_PrintUnformatted(api);
		remove_quotation_mark(ae->api);
	}


	// rr (mandatory)
	if (!cJSON_IsTrue(rr) && !cJSON_IsFalse(rr))
	{
		fprintf(stderr, "Invalid rr\n");
		free_ae(ae);
		return NULL;
	}
	else if (cJSON_IsTrue(rr))
	{
		ae->rr = true;
	}
	else if (cJSON_IsFalse(rr))
	{
		ae->rr = false;
	}

	// srv (madatory)
	srv = cJSON_GetObjectItem(root, "srv");
	if(srv == NULL) {
		fprintf(stderr, "Invalid srv\n");
		free_ae(ae);
		return NULL;
	}
	ae->srv = cjson_list_item_to_string(srv);
	

	// rn (optional)
	rn = cJSON_GetObjectItem(root, "rn");
	if (rn == NULL || isspace(rn->valuestring[0])) {
		ae->rn = NULL;
	}
	else {
		ae->rn = cJSON_PrintUnformatted(rn);
		remove_quotation_mark(ae->rn);
	}

	// lbl (optional)
	lbl = cJSON_GetObjectItem(root, "lbl");
	ae->lbl = cjson_list_item_to_string(lbl);

	return ae;
}

CNT* cjson_to_cnt(cJSON *cjson) {
	cJSON *root = NULL;
	cJSON *rn = NULL;
	cJSON *acpi = NULL;
	cJSON *lbl = NULL;
	cJSON *mni = NULL;
	cJSON *mbs = NULL;

	if (cjson == NULL) {
		// const char *error_ptr = cJSON_GetErrorPtr();
		// if (error_ptr != NULL)
		// {
		// 	fprintf(stderr, "Error before: %s\n", error_ptr);
		// }
		return NULL;
	}

	root = cJSON_GetObjectItem(cjson, "m2m:cnt");
	if(!root) return NULL;

	CNT *cnt = (CNT *)calloc(1,sizeof(CNT));

	// rn (optional)
	rn = cJSON_GetObjectItem(root, "rn");
	if (rn == NULL || isspace(rn->valuestring[0])) {
		cnt->rn = NULL;
	}
	else {
		cnt->rn = cJSON_PrintUnformatted(rn);
		remove_quotation_mark(cnt->rn);
	}


	// acpi (optional)
	acpi = cJSON_GetObjectItem(root, "acpi");
	if (acpi == NULL) {
		cnt->acpi = NULL;
	}
	else {
		int acpi_size = cJSON_GetArraySize(acpi);
		int is_NULL_flag = 0;
		if (acpi_size == 0) {
			cnt->acpi = NULL;
		}
		else {
			char acpi_str[MAX_PROPERRT_SIZE] = { '\0' };
			for (int i = 0; i < acpi_size; i++) {
				if (isspace(cJSON_GetArrayItem(acpi, i)->valuestring[0]) || (cJSON_GetArrayItem(acpi, i)->valuestring[0] == 0)) {
					cnt->acpi = NULL;
					is_NULL_flag = 1;
					break;
				}
				else {
					strcat(acpi_str, cJSON_GetArrayItem(acpi, i)->valuestring);
					if (i < acpi_size - 1) {
						strcat(acpi_str, ",");
					}
				}
			}
			if (is_NULL_flag != 1) {
				cnt->acpi = (char *)malloc(sizeof(char *) * strlen(acpi_str) + 1);
				strcpy(cnt->acpi, acpi_str);
			}
		}
	}

	// lbl (optional)
	lbl = cJSON_GetObjectItem(root, "lbl");
	if (lbl == NULL) {
		cnt->lbl = NULL;
	}
	else {
		int lbl_size = cJSON_GetArraySize(lbl);
		int is_NULL_flag = 0;
		if (lbl_size == 0) {
			cnt->lbl = NULL;
		}
		else {
			char lbl_str[MAX_PROPERRT_SIZE] = { '\0' };
			for (int i = 0; i < lbl_size; i++) {
				if (isspace(cJSON_GetArrayItem(lbl, i)->valuestring[0]) || (cJSON_GetArrayItem(lbl, i)->valuestring[0] == 0)) {
					cnt->lbl = NULL;
					is_NULL_flag = 1;
					break;
				}
				else {
					strcat(lbl_str, cJSON_GetArrayItem(lbl, i)->valuestring);
					if (i < lbl_size - 1) {
						strcat(lbl_str, ",");
					}
				}
			}
			if (is_NULL_flag != 1) {
				cnt->lbl = (char *)malloc(sizeof(char *) * strlen(lbl_str) + 1);
				strcpy(cnt->lbl, lbl_str);
			}
		}
	}

	//mni (Optional)
	mni = cJSON_GetObjectItem(root, "mni");
	if(mni) {
		cnt->mni = mni->valueint;
	} else {
		cnt->mni = INT_MIN;
	}

	//mbs (Optional)
	mbs = cJSON_GetObjectItem(root, "mbs");
	if(mbs) {
		cnt->mbs = mbs->valueint;
	} else {
		cnt->mbs = INT_MIN;
	}

	return cnt;
}

CIN* cjson_to_cin(cJSON *cjson) {
	CIN *cin = (CIN *)calloc(1,sizeof(CIN));

	cJSON *root = NULL;
	cJSON *rn = NULL;
	cJSON *con = NULL;

	const char *error_ptr = NULL;

	if (cjson == NULL) {
		// error_ptr = cJSON_GetErrorPtr();
		// if (error_ptr != NULL)
		// {
		// 	fprintf(stderr, "Error before: %s\n", error_ptr);
		// }
		return NULL;
	}

	root = cJSON_GetObjectItem(cjson, "m2m:cin");
	if(!root) return NULL;

	// con (mandatory)
	con = cJSON_GetObjectItem(root, "con");
	if (con == NULL || con->valuestring[0] == 0 || isspace(con->valuestring[0]))
	{
		fprintf(stderr, "Invalid con\n");
		return NULL;
	}
	cin->con = cJSON_PrintUnformatted(con);
	remove_quotation_mark(cin->con);
	cin->cs = strlen(cin->con);

	return cin;
}

Sub* cjson_to_sub(cJSON *cjson) {
	Sub *sub = (Sub *)calloc(1,sizeof(Sub));

	cJSON *root = NULL;
	cJSON *rn = NULL;
	cJSON *enc = NULL;
	cJSON *net = NULL;
	cJSON *nu = NULL;

	if (cjson == NULL) {
		// const char *error_ptr = cJSON_GetErrorPtr();
		// if (error_ptr != NULL)
		// {
		// 	fprintf(stderr, "Error before: %s\n", error_ptr);
		// }
		return NULL;
	}

	root = cJSON_GetObjectItem(cjson, "m2m:sub");

	// nu (mandatory)
	nu = cJSON_GetObjectItem(root, "nu");
	int nu_size = cJSON_GetArraySize(nu);
	if (nu == NULL || nu_size == 0) {
		fprintf(stderr, "Invalid nu\n");
		return NULL;
	}
	else {
		char nu_str[MAX_PROPERRT_SIZE] = { '\0' };
		int is_NULL = 0;
		for (int i = 0; i < nu_size; i++) {
			if (isspace(cJSON_GetArrayItem(nu, i)->valuestring[0]) || (cJSON_GetArrayItem(nu, i)->valuestring[0] == 0)) {
				fprintf(stderr, "Invalid nu\n");
				is_NULL = 1;
				return NULL;
			}
			strcat(nu_str, cJSON_GetArrayItem(nu, i)->valuestring);
			if (i < nu_size - 1) {
				strcat(nu_str, ",");
			}
		}
		sub->nu = (char *)malloc(sizeof(char) * strlen(nu_str) + 1);
		strcpy(sub->nu, nu_str);
	}


	// rn (optional)
	rn = cJSON_GetObjectItem(root, "rn");
	if (rn == NULL || isspace(rn->valuestring[0])) {
		sub->rn = NULL;
	}
	else {
		sub->rn = cJSON_PrintUnformatted(rn);
		remove_quotation_mark(sub->rn);
	}

	// enc (optional)
	enc = cJSON_GetObjectItem(root, "enc");
	if (enc == NULL) {
		sub->net = NULL;
	}
	else {
		// net (optional)
		net = cJSON_GetObjectItem(enc, "net");
		int net_size = cJSON_GetArraySize(net);
		if (net == NULL || net_size == 0) {
			sub->net = NULL;
		}
		else {
			char net_str[10] = { '\0' };
			char tmp[10] = { '\0' };
			for (int i = 0; i < net_size; i++) {
				sprintf(tmp, "%d", cJSON_GetArrayItem(net, i)->valueint);
				strcat(net_str, tmp);
				if (i < net_size - 1) {
					strcat(net_str, ",");
				}
			}
			sub->net = (char *)malloc(sizeof(char) * strlen(net_str) + 1);
			strcpy(sub->net, net_str);
		}
	}

	return sub;
}

ACP* cjson_to_acp(cJSON *cjson) {
	ACP *acp = (ACP *)calloc(1, sizeof(ACP));

	cJSON *root = NULL;
	cJSON *rn = NULL;
	cJSON *pv = NULL;
	cJSON *pvs = NULL;
	cJSON *acrs = NULL;
	cJSON *acr = NULL;
	cJSON *acor = NULL;
	cJSON *acop = NULL;

	if (cjson == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		return NULL;
	}

	root = cJSON_GetObjectItem(cjson, "m2m:acp");

	// pv
	pv = cJSON_GetObjectItem(root, "pv");
	if (pv == NULL) {
		fprintf(stderr, "Invalid pv\n");
		return NULL;
	}
	else {
		// acr
		acrs = cJSON_GetObjectItem(pv, "acr");
		int acr_size = cJSON_GetArraySize(acrs);
		if (acrs == NULL || acr_size == 0) {
			fprintf(stderr, "Invalid pv-acr\n");
			return NULL;
		}
		else {
			char acor_str[1024] = { '\0' };
			char acop_str[1024] = { '\0' };
			int i = 0;
			cJSON_ArrayForEach(acr, acrs) {
				acor = cJSON_GetObjectItem(acr, "acor");
				int acor_size = cJSON_GetArraySize(acor);
				if (acor == NULL || acor_size == 0) {
					fprintf(stderr, "Invalid pv-acr-acor\n");
					if (acor_size == 0) {
						fprintf(stderr, "Invalid pv-acr-acop\n");
					}
					return NULL;
				}
				else {
					for (int j = 0; j < acor_size; j++) {
						if (isspace(cJSON_GetArrayItem(acor, j)->valuestring[0]) || (cJSON_GetArrayItem(acor, j)->valuestring[0] == 0)) {
							fprintf(stderr, "Invalid pv-acr-acor\n");
							return NULL;
						}
						else {
							strcat(acor_str, cJSON_GetArrayItem(acor, j)->valuestring);
							if (j < acor_size - 1) {
								strcat(acor_str, ",");
							}
						}
					}
					if (i < acr_size - 1)
						strcat(acor_str, ",");
				}

				acop = cJSON_GetObjectItem(acr, "acop");
				int acop_int = acop->valueint;
				char tmp[1024]={'\0'};
				for (int j = 0; j < acor_size; j++) {
					sprintf(tmp, "%d", acop_int);
					strcat(acop_str, tmp);
						if (j < acor_size - 1) {
							strcat(acop_str, ",");
						}
					}
					if (i < acr_size - 1)
						strcat(acop_str, ",");
					i++;
			
			}
			acp->pv_acor = (char *)malloc(sizeof(char) * strlen(acor_str) + 1);
			strcpy(acp->pv_acor, acor_str);
		
			acp->pv_acop = (char *)malloc(sizeof(char) * strlen(acop_str) + 1);
			strcpy(acp->pv_acop, acop_str);
		}
	}

	// pvs
	pvs = cJSON_GetObjectItem(root, "pvs");
	if (pvs == NULL) {
		fprintf(stderr, "Invalid pvs\n");
		return NULL;
	}
	else {
		// acr
		acrs = cJSON_GetObjectItem(pvs, "acr");
		int acr_size = cJSON_GetArraySize(acrs);
		if (acrs == NULL || acr_size == 0) {
			fprintf(stderr, "Invalid pvs-acr\n");
			return NULL;
		}
		else {
			char acor_str[100] = { '\0' };
			char acop_str[100] = { '\0' };
			int i = 0;
			cJSON_ArrayForEach(acr, acrs) {
				acor = cJSON_GetObjectItem(acr, "acor");
				int acor_size = cJSON_GetArraySize(acor);
				if (acor == NULL || acor_size == 0) {
					fprintf(stderr, "Invalid pvs-acr-acor\n");
					if (acor_size == 0) {
						fprintf(stderr, "Invalid pvs-acr-acop\n");
					}
					return NULL;
				}
				else {
					for (int j = 0; j < acor_size; j++) {
						if (isspace(cJSON_GetArrayItem(acor, j)->valuestring[0]) || (cJSON_GetArrayItem(acor, j)->valuestring[0] == 0)) {
							fprintf(stderr, "Invalid pvs-acr-acor\n");
							return NULL;
						}
						else {
							strcat(acor_str, cJSON_GetArrayItem(acor, j)->valuestring);
							if (j < acor_size - 1) {
								strcat(acor_str, ",");
							}
						}
					}
					if (i < acr_size - 1)
						strcat(acor_str, ",");
				}

				acop = cJSON_GetObjectItem(acr, "acop");
				int acop_int = acop->valueint;
				char tmp[1024]={'\0'};
				for (int j = 0; j < acor_size; j++) {
					sprintf(tmp, "%d", acop_int);
					strcat(acop_str, tmp);
						if (j < acor_size - 1) {
							strcat(acop_str, ",");
						}
					}
					if (i < acr_size - 1)
						strcat(acop_str, ",");
					i++;
			}
			acp->pvs_acor = (char *)malloc(sizeof(char) * strlen(acor_str) + 1);
			strcpy(acp->pvs_acor, acor_str);

			acp->pvs_acop = (char *)malloc(sizeof(char) * strlen(acop_str) + 1);
			strcpy(acp->pvs_acop, acop_str);
		}
	}

	// rn (optional)
	rn = cJSON_GetObjectItem(root, "rn");
	if (rn == NULL || isspace(rn->valuestring[0])) {
		acp->rn = NULL;
	}
	else {
		acp->rn = cJSON_PrintUnformatted(rn);
		remove_quotation_mark(acp->rn);
	}

	return acp;
}

int cjson_to_grp(cJSON *cjson, GRP *grp){
	cJSON *root = NULL;
	cJSON *rn = NULL;
	cJSON *mt = NULL;
	cJSON *mnm = NULL;
	cJSON *acpi = NULL;
	cJSON *mid = NULL;
	cJSON *pmid = NULL;

	cJSON *pjson = NULL;

	if(cjson == NULL){
		return 0;
	}

	root = cJSON_GetObjectItem(cjson, "m2m:grp");

	rn = cJSON_GetObjectItem(root, "rn");
	mt = cJSON_GetObjectItem(root, "mt");
	mnm = cJSON_GetObjectItem(root, "mnm");
	mid = cJSON_GetObjectItem(root, "mid");
	acpi = cJSON_GetObjectItem(root, "acpi");

	if(pjson = cJSON_GetObjectItem(root, ""))


	// mnm & mid mandatory
	if(mnm == NULL || mid == NULL){
		logger("JSON", LOG_LEVEL_DEBUG, "Invalid request");
		return RSC_INVALID_ARGUMENTS;
	}

	// validate mnm >= 0
	if(mnm->valueint < 0){
		logger("JSON", LOG_LEVEL_DEBUG, "Invalid Maximum Member");
		return RSC_INVALID_ARGUMENTS;
	} 

	// set rn(optional);
	if(rn){
		grp->rn = (char*) malloc( sizeof(char) * strlen(rn->valuestring) + 1 );
		strcpy(grp->rn, rn->valuestring);
	}

	if(mt){
		grp->mt = (unsigned) mt->valueint;
	}

	if(acpi){
		grp->acpi = cjson_list_item_to_string(acpi);
	}
	grp->mnm = mnm->valueint;

	grp->mid = (char **) malloc(sizeof(char *) * grp->mnm);

	int mid_size = cJSON_GetArraySize(mid);
	if(mid_size > grp->mnm){
		return RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED;
	}
	
	grp->cnm = mid_size;
	for(int i = 0 ; i < grp->mnm; i++){
		if(i < mid_size){
			pmid = cJSON_GetArrayItem(mid, i);
			if(validate_mid_dup(grp->mid, i, pmid->valuestring))
				grp->mid[i] = strdup(pmid->valuestring);
			else
				grp->cnm--;
			
		}else{
			grp->mid[i] = NULL;
		}
	}

	return RSC_CREATED;
}

bool validate_mid_dup(char **mid, int idx, char *new){
	if(!mid) return true;
	if(!new) return false;

	for(int i = 0 ; i < idx; i++){
		if( !strcmp(mid[i], new) ){
			return false;
		}
	}
	return true;
}

char* node_to_json(RTNode *node) {
	char *json = NULL;

	cJSON *obj = NULL;
	cJSON *child = NULL;

	/* Our "obj" item: */
	obj = cJSON_CreateObject();
	cJSON_AddStringToObject(obj, "ri", node->ri);
	cJSON_AddStringToObject(obj, "rn", node->rn);
	cJSON_AddStringToObject(obj, "pi", node->pi);
	cJSON_AddNumberToObject(obj, "ty", node->ty);

	child = cJSON_CreateArray();
	cJSON_AddItemToObject(obj, "children", child);

	json = cJSON_PrintUnformatted(obj);

	cJSON_Delete(obj);

	return json;
}

char* cse_to_json(CSE* cse_object) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *cse = NULL;

	/* Our "cse" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:cb", cse = cJSON_CreateObject());
	cJSON_AddStringToObject(cse, "pi", cse_object->pi);
	cJSON_AddStringToObject(cse, "ri", cse_object->ri);
	cJSON_AddNumberToObject(cse, "ty", cse_object->ty);
	cJSON_AddStringToObject(cse, "ct", cse_object->ct);
	cJSON_AddStringToObject(cse, "rn", cse_object->rn);
	cJSON_AddStringToObject(cse, "lt", cse_object->lt);
	cJSON_AddStringToObject(cse, "csi", cse_object->csi);

	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char* ae_to_json(AE *ae_object) {
	char *json = NULL;
	char str_array[MAX_PROPERRT_SIZE];

	cJSON *root = NULL;
	cJSON *ae = NULL;
	cJSON *lbl = NULL;
	cJSON *srv = NULL;

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

	//lbl
	if(ae_object->lbl) {
		lbl = cJSON_CreateArray();
		strcpy(str_array, ae_object->lbl);
		char *lbl_str = strtok(str_array, ",");
		do {
			cJSON_AddItemToArray(lbl, cJSON_CreateString(lbl_str));
			lbl_str = strtok(NULL, ",");
		} while(lbl_str != NULL);
		cJSON_AddItemToObject(ae, "lbl", lbl);
	}

	//srv
	if(ae_object->srv) {
		srv = cJSON_CreateArray();
		strcpy(str_array, ae_object->srv);
		char *srv_str = strtok(str_array, ",");
		do {
			cJSON_AddItemToArray(srv, cJSON_CreateString(srv_str));
			srv_str = strtok(NULL, ",");
		} while(srv_str != NULL);
		cJSON_AddItemToObject(ae, "srv", srv);
	}

	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char* cnt_to_json(CNT* cnt_object) {
	logger("cJSON", LOG_LEVEL_DEBUG, "Call cnt_to_json");
	char *json = NULL;
	char str_array[MAX_PROPERRT_SIZE];

	cJSON *root = NULL;
	cJSON *cnt = NULL;
	cJSON *acpi = NULL;
	cJSON *lbl = NULL;

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
	if(cnt_object->mni != INT_MIN) cJSON_AddNumberToObject(cnt, "mni", cnt_object->mni);
	if(cnt_object->mbs != INT_MIN) cJSON_AddNumberToObject(cnt, "mbs", cnt_object->mbs);

	// acpi
	if(cnt_object->acpi) {
		acpi = cJSON_CreateArray();
		strcpy(str_array, cnt_object->acpi);
		char *acpi_str = strtok(str_array, ",");
		do {
			cJSON_AddItemToArray(acpi, cJSON_CreateString(acpi_str));
			acpi_str = strtok(NULL, ",");
		} while (acpi_str != NULL);
		cJSON_AddItemToObject(cnt, "acpi", acpi);
	}

	//lbl
	if(cnt_object->lbl) {
		lbl = cJSON_CreateArray();
		strcpy(str_array, cnt_object->lbl);
		char *lbl_str = strtok(str_array, ",");
		do {
			cJSON_AddItemToArray(lbl, cJSON_CreateString(lbl_str));
			lbl_str = strtok(NULL, ",");
		} while(lbl_str != NULL);
		cJSON_AddItemToObject(cnt, "lbl", lbl);
	}
	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char* cin_to_json(CIN* cin_object) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *cin = NULL;

	/* Our "cin" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:cin", cin = cJSON_CreateObject());
	cJSON_AddStringToObject(cin, "rn", cin_object->rn);
	cJSON_AddNumberToObject(cin, "ty", cin_object->ty);
	cJSON_AddStringToObject(cin, "pi", cin_object->pi);
	cJSON_AddStringToObject(cin, "ri", cin_object->ri);
	cJSON_AddStringToObject(cin, "ct", cin_object->ct);
	cJSON_AddStringToObject(cin, "lt", cin_object->lt);
	cJSON_AddNumberToObject(cin, "st", cin_object->st);
	cJSON_AddStringToObject(cin, "et", cin_object->et);
	cJSON_AddNumberToObject(cin, "cs", cin_object->cs);
	cJSON_AddStringToObject(cin, "con", cin_object->con);

	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char* sub_to_json(Sub *sub_object) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *sub = NULL;
	cJSON *nu = NULL;
	cJSON *enc = NULL;
	cJSON *net = NULL;

	/* Our "sub" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:sub", sub = cJSON_CreateObject());
	cJSON_AddStringToObject(sub, "rn", sub_object->rn);
	cJSON_AddNumberToObject(sub, "ty", sub_object->ty);
	cJSON_AddStringToObject(sub, "pi", sub_object->pi);
	cJSON_AddStringToObject(sub, "ri", sub_object->ri);
	cJSON_AddStringToObject(sub, "ct", sub_object->ct);
	cJSON_AddStringToObject(sub, "lt", sub_object->lt);
	cJSON_AddStringToObject(sub, "et", sub_object->et);

	// nu
	if(sub_object->nu) {
		nu = cJSON_CreateArray();
		char *nu_str = strtok(sub_object->nu, ",");
		do {
			cJSON_AddItemToArray(nu, cJSON_CreateString(nu_str));
			nu_str = strtok(NULL, ",");
		} while (nu_str != NULL);
		cJSON_AddItemToObject(sub, "nu", nu);
	}

	// net
	cJSON_AddItemToObject(sub, "enc", enc = cJSON_CreateObject());

		net = cJSON_CreateArray();
	if(sub_object->net) {
		char *net_str = strtok(sub_object->net, ",");
		do {
			cJSON_AddItemToArray(net, cJSON_CreateNumber(atof(net_str)));
			net_str = strtok(NULL, ",");
		} while (net_str != NULL);
	}
	cJSON_AddItemToObject(enc, "net", net);

	// nct
	cJSON_AddNumberToObject(sub, "nct", sub_object->nct);

	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char* notification_to_json(char *sur, int net, char *rep) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *sgn = NULL;
	cJSON *nev = NULL;

	/* Our "noti" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:sgn", sgn = cJSON_CreateObject());
	cJSON_AddStringToObject(sgn, "sur", sur);
	cJSON_AddItemToObject(sgn, "nev", nev = cJSON_CreateObject());
	cJSON_AddNumberToObject(nev, "net", net);
	cJSON_AddStringToObject(nev, "rep", rep);

	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char* acp_to_json(ACP *acp_object) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *acp = NULL;
	cJSON *pv = NULL;
	cJSON *pvs = NULL;
	cJSON *acrs = NULL;
	cJSON *acr = NULL;
	cJSON *acor = NULL;

	char *acor_copy = NULL;
	char *acor_remainder = NULL;
	char *acor_str = NULL;

	char *acop_copy = NULL;
	char *acop_remainder = NULL;
	char *acop_str = NULL;
	
	/* Our "acp" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:acp", acp = cJSON_CreateObject());
	cJSON_AddStringToObject(acp, "rn", acp_object->rn);
	cJSON_AddNumberToObject(acp, "ty", acp_object->ty);
	cJSON_AddStringToObject(acp, "pi", acp_object->pi);
	cJSON_AddStringToObject(acp, "ri", acp_object->ri);
	cJSON_AddStringToObject(acp, "ct", acp_object->ct);
	cJSON_AddStringToObject(acp, "lt", acp_object->lt);
	cJSON_AddStringToObject(acp, "et", acp_object->et);

	// pv
	if(acp_object->pv_acor && acp_object->pv_acop) {
		cJSON_AddItemToObject(acp, "pv", pv = cJSON_CreateObject());

		// acr
		acrs = cJSON_CreateArray();
		cJSON_AddItemToObject(pv, "acr", acrs);

		// acor
		acor_copy = (char *)malloc(sizeof(char) * strlen(acp_object->pv_acor) + 1);
		acor_copy = strcpy(acor_copy, acp_object->pv_acor);
		acor_str = strtok_r(acor_copy, ",", &acor_remainder);

		// acop
		acop_copy = (char *)malloc(sizeof(char) * strlen(acp_object->pv_acop) + 1);
		acop_copy = strcpy(acop_copy, acp_object->pv_acop);
		acop_str = strtok_r(acop_copy, ",", &acop_remainder);

		while (1) {
			if (acop_str == NULL) {
				break;
			}

			char *acop = acop_str;

			cJSON_AddItemToArray(acrs, acr = cJSON_CreateObject());

			acor = cJSON_CreateArray();

			do {
				cJSON_AddItemToArray(acor, cJSON_CreateString(acor_str));
				acor_str = strtok_r(NULL, ",", &acor_remainder);

				acop_str = strtok_r(NULL, ",", &acop_remainder);
			} while (acop_str != NULL && strcmp(acop, acop_str) == 0);
			cJSON_AddItemToObject(acr, "acor", acor);
			cJSON_AddItemToObject(acr, "acop", cJSON_CreateNumber(atoi(acop)));
		}
	}

	// pvs
	cJSON_AddItemToObject(acp, "pvs", pvs = cJSON_CreateObject());

	// acr
	acrs = cJSON_CreateArray();
	cJSON_AddItemToObject(pvs, "acr", acrs);

	// acor
	acor_copy = (char *)malloc(sizeof(char) * strlen(acp_object->pvs_acor) + 1);
	acor_copy = strcpy(acor_copy, acp_object->pvs_acor);
	acor_remainder = NULL;
	acor_str = strtok_r(acor_copy, ",", &acor_remainder);

	// acop
	acop_copy = (char *)malloc(sizeof(char) * strlen(acp_object->pvs_acop) + 1);
	acop_copy = strcpy(acop_copy, acp_object->pvs_acop);
	acop_remainder = NULL;
	acop_str = strtok_r(acop_copy, ",", &acop_remainder);

	while (1) {
		if (acop_str == NULL) {
			break;
		}

		char *acop = acop_str;

		cJSON_AddItemToArray(acrs, acr = cJSON_CreateObject());

		acor = cJSON_CreateArray();

		do {
			cJSON_AddItemToArray(acor, cJSON_CreateString(acor_str));
			acor_str = strtok_r(NULL, ",", &acor_remainder);

			acop_str = strtok_r(NULL, ",", &acop_remainder);
		} while (acop_str != NULL && strcmp(acop, acop_str) == 0);
		cJSON_AddItemToObject(acr, "acor", acor);
		cJSON_AddItemToObject(acr, "acop", cJSON_CreateNumber(atoi(acop)));
	}

	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char* discovery_to_json(char **result, int size) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *uril = NULL;

	/* Our "cnt" item: */
	root = cJSON_CreateObject();

	// uril
	uril = cJSON_CreateArray();

	for(int i=0; i<size; i++) {
		cJSON_AddItemToArray(uril, cJSON_CreateString(result[i]));
	}
	cJSON_AddItemToObject(root, "m2m:uril", uril);

	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char *grp_to_json(GRP *grp_object){

	char *json = NULL;

	cJSON *root;
	cJSON *grp;
	cJSON *mid;

	int cnm = 0;

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:grp", grp = cJSON_CreateObject());
	cJSON_AddStringToObject(grp, "ri", grp_object->ri);
	cJSON_AddStringToObject(grp, "pi", grp_object->pi);
	cJSON_AddStringToObject(grp, "rn", grp_object->rn);
	cJSON_AddStringToObject(grp, "ct", grp_object->ct);
	cJSON_AddStringToObject(grp, "lt", grp_object->lt);
	cJSON_AddStringToObject(grp, "et", grp_object->et);
	if(grp_object->acpi) cJSON_AddStringToObject(grp, "acpi", grp_object->acpi);
	cJSON_AddBoolToObject(grp, "mtv", grp_object->mtv);

	cJSON_AddNumberToObject(grp, "mnm", grp_object->mnm);
	cJSON_AddNumberToObject(grp, "cnm", grp_object->cnm);
	cJSON_AddNumberToObject(grp, "mt", grp_object->mt);
	cJSON_AddNumberToObject(grp, "ty", 9);
	
	if(grp_object->mid){
		mid = cJSON_CreateArray();
		for(int i = 0 ; i < grp_object->cnm ; i++){
			cJSON_AddItemToArray(mid, cJSON_CreateString(grp_object->mid[i]));
		}
	}
	cJSON_AddItemToObject(grp, "mid", mid);
	

	json = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return json;
}

char* get_json_value_char(char *key, char *json) {
	char key_exist_check[16];
	sprintf(key_exist_check,"\"%s\"",key);
	if(!strstr(json,key_exist_check)) return NULL;

	char json_copy[MAX_PAYLOAD_SIZE];
	char *resource = NULL;
	char *value = NULL;

	cJSON *root = NULL;
	cJSON *ckey = NULL;

	cJSON *cjson = cJSON_Parse(json);
	if (cjson == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			//fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		return NULL;
	}

	//Extracting resources from json
	strcpy(json_copy, json);
	resource = strstr(json_copy, "m2m:");
	resource = strtok(resource, "\"");

	root = cJSON_GetObjectItem(cjson, resource);

	ckey = cJSON_GetObjectItem(root, key);
	if (!cJSON_IsString(ckey) && ckey->valuestring == NULL) {
		goto end;
	}
	value = cJSON_PrintUnformatted(ckey);
	remove_quotation_mark(value);

end:
	cJSON_Delete(cjson);

	return value;
}

char* get_json_value_string(char *json, char *key) {
	cJSON *root = cJSON_Parse(json);

	if(root == NULL) {
		return NULL;
	}

	char key_str[64], keys[8][8];
	strcpy(key_str, key);

	char *k = strtok(key_str, "-");
	int index = 0;
	while(k) {
		strcpy(keys[index++], k);
		k = strtok(NULL, "-");
	}

	cJSON *json_key, *parent = root;

	for(int i=0; i<index; i++) {
		json_key = cJSON_GetObjectItem(parent, keys[i]);
		if(!json_key) { 
			cJSON_Delete(root);
			return NULL;
		}
		parent = json_key;
	}

	char *value_string = cJSON_PrintUnformatted(json_key);
	remove_quotation_mark(value_string);
	cJSON_Delete(root);
	return value_string;
}

char* get_json_value_list_v2(char *json, char *key) {
	cJSON *root = cJSON_Parse(json);

	if(root == NULL) {
		return NULL;
	}

	char key_str[64], keys[8][8];
	strcpy(key_str, key);

	char *k = strtok(key_str, "-");
	int index = 0;
	while(k) {
		strcpy(keys[index++], k);
		k = strtok(NULL, "-");
	}

	cJSON *json_key, *parent = root;

	for(int i=0; i<index; i++) {
		json_key = cJSON_GetObjectItem(parent, keys[i]);
		if(!json_key) { 
			cJSON_Delete(root);
			return NULL;
		}
		parent = json_key;
	}

	int list_size = cJSON_GetArraySize(json_key);
	if(list_size == 0) return NULL;
	char value_list_str[256] = "\0";

	for(int i=0; i<list_size; i++) {
		strcat(value_list_str, cJSON_GetArrayItem(json_key, i)->valuestring);
		if(i < list_size - 1) {
			strcat(value_list_str, ",");
		}
	}

	char *value_list = (char*)malloc(strlen((value_list_str) + 1) * sizeof(char));
	strcpy(value_list, value_list_str);
	remove_quotation_mark(value_list);
	cJSON_Delete(root);
	return value_list;
}

int get_json_value_int(char *key, char *json) {
	char key_exist_check[16];
	sprintf(key_exist_check,"\"%s\"",key);
	if(!strstr(json,key_exist_check)) return -1;

	char json_copy[MAX_PAYLOAD_SIZE];
	char *resource = NULL;
	int value = 0;

	cJSON *root = NULL;
	cJSON *ckey = NULL;

	cJSON *cjson = cJSON_Parse(json);
	if (cjson == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	// Extracting resources from json
	strcpy(json_copy, json);
	resource = strstr(json_copy, "m2m:");
	resource = strtok(resource, "\"");

	root = cJSON_GetObjectItem(cjson, resource);

	ckey = cJSON_GetObjectItem(root, key);
	if (!cJSON_IsNumber(ckey)) {
		goto end;
	}
	value = ckey->valueint;

end:
	cJSON_Delete(cjson);

	return value;
}

int get_json_value_bool(char *key, char *json) {
	char key_exist_check[16];
	sprintf(key_exist_check,"\"%s\"",key);
	if(!strstr(json,key_exist_check)) return -1;

	char json_copy[MAX_PAYLOAD_SIZE];
	char *resource = NULL;

	cJSON *root = NULL;
	cJSON *ckey = NULL;

	cJSON *cjson = cJSON_Parse(json);
	if (cjson == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	//Extracting resources from json
	strcpy(json_copy, json);
	resource = strstr(json_copy, "m2m:");
	resource = strtok(resource, "\"");

	root = cJSON_GetObjectItem(cjson, resource);

	ckey = cJSON_GetObjectItem(root, key);
	if (!cJSON_IsTrue(ckey) && !cJSON_IsFalse(ckey))
	{
		goto end;
	}
	else if (cJSON_IsTrue(ckey))
	{
		return 1;
	}
	else if (cJSON_IsFalse(ckey))
	{
		return 0;
	}

end:
	cJSON_Delete(cjson);
}

char *get_json_value_list(char *key_str, char *json) {
	char key_exist_check[16];
	sprintf(key_exist_check,"\"%s\"",key_str);
	if(!strstr(json,key_exist_check)) return NULL;

	char key_arr[16], *key;
	char json_copy[MAX_PAYLOAD_SIZE];
	char *resource = NULL;
	char *value = NULL;

	strcpy(key_arr, key_str);
	key = key_arr;

	cJSON *root = NULL;
	cJSON *ckey = NULL;

	cJSON *cjson = cJSON_Parse(json);
	if (cjson == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	//Extracting resources from json
	strcpy(json_copy, json);
	resource = strstr(json_copy, "m2m:");
	resource = strtok(resource, "\"");

	root = cJSON_GetObjectItem(cjson, resource);

	if (strstr(resource, "acp") != NULL) {	// acp ó��  ex) key: pv-acr-acor, pv-acr-acop, pvs-acr-acor, pvs-acr-acop
		// pv / pvs
		char *pv_str = strtok(key, "-");
		cJSON *pv = cJSON_GetObjectItem(root, pv_str);
		// acr
		char *acr_str = strtok(NULL, "-");
		cJSON *acrs = cJSON_GetObjectItem(pv, acr_str);
		int acr_size = cJSON_GetArraySize(acrs);
		// acor / acop
		char *ckey_str = strtok(NULL, "-");
		cJSON *acr = NULL;
		cJSON *acor = NULL;
		cJSON *acop = NULL;
		char acorp_str[256] = { "\0" };
		int i = 0;
		cJSON_ArrayForEach(acr, acrs) {
			acor = cJSON_GetObjectItem(acr, "acor");
			int acor_size = cJSON_GetArraySize(acor);
			if (!strcmp(ckey_str, "acop")) {
				acop = cJSON_GetObjectItem(acr, "acop");
			}
			for (int j = 0; j < acor_size; j++) {
				if (strstr(ckey_str, "acor") != NULL) {	// acor
					strcat(acorp_str, cJSON_GetArrayItem(acor, j)->valuestring);
				}
				else {	// acop
					strcat(acorp_str, cJSON_PrintUnformatted(acop));
				}

				if (j < acor_size - 1) {
					strcat(acorp_str, ",");
				}
			}
			if (i < acr_size - 1)
				strcat(acorp_str, ",");
			i++;
		}

		value = (char *)malloc(sizeof(char) * strlen(acorp_str) + 1);
		strcpy(value, acorp_str);
	}
	else {	// acp �̿� ó��
		// key �� �˻� : '-' ���� Ȯ��
		if (strstr(key, "-") != NULL) {	// '-' ���� ex) key: enc-net
			char *subRoot_str = strtok(key, "-");
			cJSON *subRoot = cJSON_GetObjectItem(root, subRoot_str);
			key = strtok(NULL, "-");
			ckey = cJSON_GetObjectItem(subRoot, key);
		}
		else {	// '-' ���� X ex) acpi
			ckey = cJSON_GetObjectItem(root, key);
		}

		int ckey_size = cJSON_GetArraySize(ckey);
		char ckey_str[256] = { '\0' };
		char tmp[64] = { '\0' };
		for (int i = 0; i < ckey_size; i++) {
			if (cJSON_GetArrayItem(ckey, i)->valuestring != NULL) {	// ���ڿ� ó��
				strcat(ckey_str, cJSON_GetArrayItem(ckey, i)->valuestring);
			}
			else {	// ���� ó��
				sprintf(tmp, "%d", cJSON_GetArrayItem(ckey, i)->valueint);
				strcat(ckey_str, tmp);
			}

			if (i < ckey_size - 1) {
				strcat(ckey_str, ",");
			}
		}
		value = (char *)malloc(sizeof(char) * strlen(ckey_str) + 1);
		strcpy(value, ckey_str);
	}


end:
	cJSON_Delete(cjson);

	return value;
}

bool json_key_exist(char *json, char *key) {
	cJSON *root = cJSON_Parse(json);

	if(root == NULL) {
		return false;
	}

	char key_str[64], keys[8][8];
	strcpy(key_str, key);

	char *k = strtok(key_str, "-");
	int index = 0;
	while(k) {
		strcpy(keys[index++], k);
		k = strtok(NULL, "-");
	}

	cJSON *json_key, *parent = root;

	for(int i=0; i<index; i++) {
		json_key = cJSON_GetObjectItem(parent, keys[i]);
		if(!json_key) { 
			cJSON_Delete(root);
			return false;
		}
		parent = json_key;
	}

	cJSON_Delete(root);
	return true;
}

char *cjson_list_item_to_string(cJSON *key) {
	if (key == NULL) {
		return NULL;
	}
	else {
		int item_size = cJSON_GetArraySize(key);
		if (item_size == 0) {
			return NULL;
		}
		else {
			char item_str[MAX_PROPERRT_SIZE] = { '\0' };
			for (int i = 0; i < item_size; i++) {
				if (isspace(cJSON_GetArrayItem(key, i)->valuestring[0]) || (cJSON_GetArrayItem(key, i)->valuestring[0] == 0)) {
					return NULL;
				}
				else {
					strcat(item_str, cJSON_GetArrayItem(key, i)->valuestring);
					if (i < item_size - 1) {
						strcat(item_str, ",");
					}
				}
			}
			char *ret = (char *)malloc(sizeof(char *) * strlen(item_str) + 1);
			strcpy(ret, item_str);

			return ret;
		}
	}
}