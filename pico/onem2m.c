#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <malloc.h>
#include <sys/timeb.h>
#include <limits.h>
#include "onem2m.h"
#include "dbmanager.h"
#include "httpd.h"
#include "mqttClient.h"
#include "onem2mTypes.h"
#include "config.h"
#include "util.h"
#include "cJSON.h"

extern ResourceTree *rt;

void init_cse(cJSON* cse) {
	char *ct = get_local_time(0);
	char *csi = (char*)malloc(strlen(CSE_BASE_RI) + 2);
	sprintf(csi, "/%s", CSE_BASE_RI);

	cJSON *srt = cJSON_CreateArray();
	cJSON_AddItemToArray(srt, cJSON_CreateNumber(1));
	cJSON_AddItemToArray(srt, cJSON_CreateNumber(2));
	cJSON_AddItemToArray(srt, cJSON_CreateNumber(3));
	cJSON_AddItemToArray(srt, cJSON_CreateNumber(4));
	cJSON_AddItemToArray(srt, cJSON_CreateNumber(5));
	cJSON_AddItemToArray(srt, cJSON_CreateNumber(9));
	cJSON_AddItemToArray(srt, cJSON_CreateNumber(16));

	
	cJSON_AddStringToObject(cse, "ct", ct);
	cJSON_AddStringToObject(cse, "ri", CSE_BASE_RI);
	cJSON_AddStringToObject(cse, "lt", ct);
	cJSON_AddStringToObject(cse, "rn", CSE_BASE_NAME);
	cJSON_AddNumberToObject(cse, "cst", SERVER_TYPE);
	cJSON_AddItemToObject(cse, "srt", srt);
	cJSON_AddStringToObject(cse, "srv", "2a");
	cJSON_AddItemToObject(cse, "pi", cJSON_CreateNull());
	cJSON_AddNumberToObject(cse, "ty", RT_CSE);
	cJSON_AddStringToObject(cse, "uri", CSE_BASE_NAME);
	cJSON_AddStringToObject(cse, "csi", csi);
	
	// TODO - add acpi, poa
	
	free(ct); ct = NULL;
	free(csi); csi = NULL;
}

void add_general_attribute(cJSON *root, RTNode *parent_rtnode, ResourceType ty){
	char *ct = get_local_time(0);
	char *ptr = NULL;

	cJSON_AddNumberToObject(root, "ty", ty);
	cJSON_AddStringToObject(root, "ct", ct);

	ptr = resource_identifier(ty, ct);
	cJSON_AddStringToObject(root, "ri", ptr);
	if(cJSON_GetObjectItem(root, "rn") == NULL){
		cJSON_AddStringToObject(root, "rn", ptr);
	}
	free(ptr); ptr = NULL;

	cJSON_AddStringToObject(root, "lt", ct);

	cJSON *pi = cJSON_GetObjectItem(parent_rtnode->obj, "ri");
	cJSON_AddStringToObject(root, "pi", pi->valuestring);

	ptr = get_local_time(DEFAULT_EXPIRE_TIME);
	cJSON_AddStringToObject(root, "et", ptr);
	free(ptr); ptr = NULL;
}

void init_csr(CSR* csr, RTNode *parent_rtnode){
	char *ct = get_local_time(0);
	
	csr->ri = resource_identifier(RT_CSR, ct);

	csr->ct = strdup(ct);
	csr->lt = strdup(ct);
	if(!csr->rn){
		csr->rn = (char*)malloc((strlen(csr->csi) + 1) * sizeof(char));
		strcpy(csr->rn, csr->csi);
	}

	csr->pi = strdup(get_ri_rtnode(parent_rtnode));

	logger("O2", LOG_LEVEL_DEBUG, "%s/%s", parent_rtnode->uri, csr->rn);
	csr->uri = (char*) malloc( (strlen(parent_rtnode->uri) + strlen(csr->rn) + 2) * sizeof(char));
	sprintf(csr->uri, "%s/%s", get_uri_rtnode(parent_rtnode), csr->rn);
	
	csr->ty = RT_CSR;
	
	free(ct); ct = NULL;
}

void init_ae(AE* ae, RTNode *parent_rtnode, char *origin) {
	int origin_size;
	char *uri[128];

	ae->ct = get_local_time(0);
	ae->et = get_local_time(DEFAULT_EXPIRE_TIME);

	if(origin && (origin_size = strlen(origin)) > 0) {
		ae->ri = (char *)malloc(sizeof(char) * (origin_size + 1));
		strcpy(ae->ri, origin);

	} else {
		ae->ri = resource_identifier(RT_AE, ae->ct);
	}

	if(!ae->rn) 
		ae->rn = strdup(ae->ri);
	
	ae->pi = strdup(get_ri_rtnode(parent_rtnode));
	ae->lt = strdup(ae->ct);
	ae->aei = strdup(ae->ri);

	sprintf(uri, "%s/%s", get_uri_rtnode(parent_rtnode), ae->rn);
	ae->uri = strdup(uri);

	
	ae->ty = RT_AE;
}

void init_cnt(CNT* cnt, RTNode *parent_rtnode) {
	char uri[128] = {0};
	cnt->ct = get_local_time(0);
	cnt->et = get_local_time(DEFAULT_EXPIRE_TIME);
	cnt->ri = resource_identifier(RT_CNT, cnt->ct);
	
	if(!cnt->rn) {
		cnt->rn = (char*)malloc((strlen(cnt->ri) + 1) * sizeof(char));
		strcpy(cnt->rn, cnt->ri);
	}
	
	cnt->pi = strdup(get_ri_rtnode(parent_rtnode));
	cnt->lt = strdup(cnt->ct);;
	
	cnt->ty = RT_CNT;
	cnt->st = 0;
	cnt->cni = 0;
	cnt->cbs = 0;

	sprintf(uri, "%s/%s", get_uri_rtnode(parent_rtnode), cnt->rn);
	cnt->uri = strdup(uri);
}

void init_cin(CIN* cin, RTNode *parent_rtnode) {
	char uri[256] = {0};
	cin->ct = get_local_time(0);
	cin->et = get_local_time(DEFAULT_EXPIRE_TIME);
	cin->ri = resource_identifier(RT_CIN, cin->ct);
	
	cin->rn = strdup(cin->ri);
	cin->pi = strdup(get_ri_rtnode(parent_rtnode));
	cin->lt = strdup(cin->ct);
	
	cin->ty = RT_CIN;
	cin->st = 0;
	cin->cs = strlen(cin->con);
	sprintf(uri, "%s/%s",get_uri_rtnode(parent_rtnode), cin->rn);
	cin->uri = strdup(uri);
}

void init_sub(SUB* sub, RTNode *parent_rtnode) {
	char uri[256] = {0};
	sub->ct = get_local_time(0);
	sub->et = get_local_time(DEFAULT_EXPIRE_TIME);
	sub->ri = resource_identifier(RT_SUB, sub->ct);

	if(!sub->rn) {
		sub->rn = (char*)malloc((strlen(sub->ri) + 1) * sizeof(char));
		strcpy(sub->rn, sub->ri);
	}

	if(!sub->net) {
		sub->net = (char*)malloc(2*sizeof(char));
		strcpy(sub->net,"1");
	}

	sprintf(uri, "/%s/%s", get_uri_rtnode(parent_rtnode), sub->rn);
	sub->pi = strdup(get_ri_rtnode(parent_rtnode));
	sub->lt = strdup(sub->ct);

	sub->sur = strdup(uri);
	sub->uri = strdup(uri);

	sub->ty = RT_SUB;
	sub->nct = 0;
}

void init_acp(ACP* acp, RTNode *parent_rtnode) {
	char *ct = get_local_time(0);
	char *et = get_local_time(DEFAULT_EXPIRE_TIME);
	char *ri = resource_identifier(RT_ACP, ct);
	char uri[128] = {0};

	if(!acp->rn) {
		acp->rn = (char*)malloc((strlen(acp->ri) + 1) * sizeof(char));
		strcpy(acp->rn, acp->ri);
	}
	
	acp->ri = strdup(ri);
	acp->pi = strdup(get_ri_rtnode(parent_rtnode));
	acp->et = strdup(et);
	acp->ct = strdup(ct);
	acp->lt = strdup(ct);

	sprintf(uri, "%s/%s", get_uri_rtnode(parent_rtnode), acp->rn);
	acp->uri = strdup(uri);
	
	acp->ty = RT_ACP;
}

RTNode* create_rtnode(cJSON *obj, ResourceType ty){
	RTNode* rtnode = (RTNode *)calloc(1, sizeof(RTNode));
	cJSON *uri = NULL;

	rtnode->ty = ty;
	rtnode->obj = obj;

	rtnode->parent = NULL;
	rtnode->child = NULL;
	rtnode->sibling_left = NULL;
	rtnode->sibling_right = NULL;
	if(uri = cJSON_GetObjectItem(obj, "uri"))
	{
		rtnode->uri = strdup(uri->valuestring);
		cJSON_DeleteItemFromObject(obj, "uri");
	}
		
	
	return rtnode;
}

int create_csr(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	int e = check_rn_invalid(o2pt, RT_CSR);
	if(e == -1) return o2pt->rsc;

	if(parent_rtnode->ty != RT_CSE) {
		handle_error(o2pt, RSC_INVALID_CHILD_RESOURCETYPE, "child type is invalid");
		return o2pt->rsc;
	}

	if(SERVER_TYPE == ASN_CSE){
		handle_error(o2pt, RSC_OPERATION_NOT_ALLOWED, "operation not allowed");
		return o2pt->rsc;
	}

	cJSON *root = cJSON_Duplicate(o2pt->cjson_pc, 1);
	cJSON *csr = cJSON_GetObjectItem(root, "m2m:csr");

	add_general_attribute(csr, parent_rtnode, RT_CSR);
	cJSON_AddStringToObject(csr, "csi", o2pt->fr);

	int rsc = validate_csr(o2pt, parent_rtnode, csr, OP_CREATE);
	if(rsc != RSC_OK){
		cJSON_Delete(root);
		return rsc;
	}

	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	o2pt->rsc = RSC_CREATED;
	
	// Add uri attribute
	char *ptr = malloc(1024);
	cJSON *rn = cJSON_GetObjectItem(csr, "rn");
	sprintf(ptr, "%s/%s", get_uri_rtnode(parent_rtnode), rn->valuestring);	

	// Save to DB
	int result = db_store_resource(csr, ptr);
	if(result == -1) {
		cJSON_Delete(root);
		handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "database error");
		free(ptr);	ptr = NULL;
		return RSC_INTERNAL_SERVER_ERROR;
	}
	free(ptr);	ptr = NULL;

	RTNode* rtnode = create_rtnode(csr, RT_CSR);
	add_child_resource_tree(parent_rtnode, rtnode);
	//TODO: update descendent cse if update is needed
	return RSC_CREATED;
}

int create_ae(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	int e = check_aei_duplicate(o2pt, parent_rtnode);
	if(e != -1) e = check_rn_invalid(o2pt, RT_AE);
	if(e != -1) e = check_aei_invalid(o2pt);
	if(e == -1) return o2pt->rsc;

	if(parent_rtnode->ty != RT_CSE) {
		handle_error(o2pt, RSC_INVALID_CHILD_RESOURCETYPE, "child type is invalid");
		return o2pt->rsc;
	}
	cJSON *root = cJSON_Duplicate(o2pt->cjson_pc, 1);
	cJSON *ae = cJSON_GetObjectItem(root, "m2m:ae");
	// if(!is_attr_valid(ae, RT_AE)){
	// 	handle_error(o2pt, RSC_BAD_REQUEST, "wrong attribute(s) submitted");
	// 	cJSON_Delete(root);
	// 	return o2pt->rsc;
	// }

	add_general_attribute(ae, parent_rtnode, RT_AE);
	if(o2pt->fr && strlen(o2pt->fr) > 0) {
		cJSON* ri = cJSON_GetObjectItem(ae, "ri");
		cJSON_SetValuestring(ri, o2pt->fr);
	}
	cJSON_AddStringToObject(ae, "aei", cJSON_GetObjectItem(ae, "ri")->valuestring);
	
	int rsc = validate_ae(o2pt, ae, OP_CREATE);

	if(rsc != RSC_OK){
		cJSON_Delete(root);
		return rsc;
	}
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	// Add uri attribute
	char *ptr = malloc(1024);
	cJSON *rn = cJSON_GetObjectItem(ae, "rn");
	sprintf(ptr, "%s/%s", get_uri_rtnode(parent_rtnode), rn->valuestring);
	// Save to DB
	int result = db_store_resource(ae, ptr);
	if(result != 1) { 
		handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "DB store fail");
		cJSON_Delete(root);
		free(ptr); ptr = NULL;
		return RSC_INTERNAL_SERVER_ERROR;
	}

	free(ptr);	ptr = NULL;

	// Add to resource tree
	RTNode* child_rtnode = create_rtnode(ae, RT_AE);
	add_child_resource_tree(parent_rtnode, child_rtnode); 
	return o2pt->rsc = RSC_CREATED;
}

int create_cnt(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	cJSON *root = cJSON_Duplicate(o2pt->cjson_pc, 1);
	cJSON *pjson = NULL;
	if(parent_rtnode->ty != RT_CNT && parent_rtnode->ty != RT_AE && parent_rtnode->ty != RT_CSE) {
		handle_error(o2pt, RSC_INVALID_CHILD_RESOURCETYPE, "child type is invalid");
		return RSC_INVALID_CHILD_RESOURCETYPE;
	}

	cJSON *cnt = cJSON_GetObjectItem(root, "m2m:cnt");

	add_general_attribute(cnt, parent_rtnode, RT_CNT);

	int rsc = validate_cnt(o2pt, cnt, OP_CREATE);
	if(rsc != RSC_OK){
		cJSON_Delete(root);
		return rsc;
	}
	
	// Add cr attribute
	if(pjson = cJSON_GetObjectItem(cnt, "cr")){
		if(pjson->type == cJSON_NULL){
			cJSON_AddStringToObject(cnt, "cr", o2pt->fr);
		}else{
			handle_error(o2pt, RSC_BAD_REQUEST, "creator attribute with arbitary value is not allowed");
			cJSON_Delete(root);
			return o2pt->rsc;
		}
	}

	// Add st, cni, cbs, mni, mbs attribute
	cJSON_AddNumberToObject(cnt, "st", 0);
	cJSON_AddNumberToObject(cnt, "cni", 0);
	cJSON_AddNumberToObject(cnt, "cbs", 0);

	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	o2pt->rsc = RSC_CREATED;

	// Add uri attribute
	char *ptr = malloc(1024);
	cJSON *rn = cJSON_GetObjectItem(cnt, "rn");
	sprintf(ptr, "%s/%s", get_uri_rtnode(parent_rtnode), rn->valuestring);

	// Store to DB
	int result = db_store_resource(cnt, ptr);
	if(result != 1) { 
		handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "DB store fail"); 
		cJSON_Delete(root);
		free(ptr);	ptr = NULL;
		return o2pt->rsc;
	}
	free(ptr);	ptr = NULL;

	cJSON *uri = cJSON_GetObjectItem(cnt, "uri");
	cJSON_DeleteItemFromObject(cnt, "uri");

	RTNode* child_rtnode = create_rtnode(cnt, RT_CNT);
	add_child_resource_tree(parent_rtnode,child_rtnode);

	return RSC_CREATED;
}

int create_cin(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty != RT_CNT) {
		handle_error(o2pt, RSC_INVALID_CHILD_RESOURCETYPE, "child type is invalid");
		return o2pt->rsc;
	}

	cJSON *root = cJSON_Duplicate(o2pt->cjson_pc, 1);
	cJSON *cin = cJSON_GetObjectItem(root, "m2m:cin");

	add_general_attribute(cin, parent_rtnode, RT_CIN);
	
	// add cs attribute
	cJSON *con = cJSON_GetObjectItem(cin, "con");
	if(cJSON_IsString(con))
		cJSON_AddNumberToObject(cin, "cs", strlen(cJSON_GetStringValue(con)));

	// Add st attribute
	cJSON *st = cJSON_GetObjectItem(parent_rtnode->obj, "st");
	cJSON_AddNumberToObject(cin, "st", st->valueint);

	int rsc = validate_cin(o2pt, parent_rtnode->obj, cin, OP_CREATE);
	if(rsc != RSC_OK){
		cJSON_Delete(root);
		return rsc;
	}

	RTNode *cin_rtnode = create_rtnode(cin, RT_CIN);
	update_cnt_cin(parent_rtnode, cin_rtnode, 1);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	o2pt->rsc = RSC_CREATED;

	// Add uri attribute
	char *ptr = malloc(1024);
	cJSON *ri = cJSON_GetObjectItem(cin, "ri");
	sprintf(ptr, "%s/%s", get_uri_rtnode(parent_rtnode), ri->valuestring);

	// Store to DB
	int result = db_store_resource(cin, ptr);
	if(result != 1) { 
		handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "DB store fail"); 
		free_rtnode(cin_rtnode);
		cJSON_Delete(root);
		free(ptr);	ptr = NULL;
		return o2pt->rsc;
	}

	cJSON_Delete(root);
	free(ptr);	ptr = NULL;

	return RSC_CREATED;
}

int create_sub(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty == RT_CIN || parent_rtnode->ty == RT_SUB) {
		handle_error(o2pt, RSC_INVALID_CHILD_RESOURCETYPE, "child type is invalid");
		return o2pt->rsc;
	}

	cJSON *root = cJSON_Duplicate(o2pt->cjson_pc, 1);
	cJSON *sub = cJSON_GetObjectItem(root, "m2m:sub");


	add_general_attribute(sub, parent_rtnode, RT_SUB);

	int rsc = validate_sub(o2pt, sub, OP_CREATE);

	if(rsc != RSC_OK){
		cJSON_Delete(root);
		return rsc;
	}

	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	o2pt->rsc = RSC_CREATED;

	// Add uri attribute
	char *ptr = malloc(1024);
	cJSON *rn = cJSON_GetObjectItem(sub, "rn");
	sprintf(ptr, "%s/%s", get_uri_rtnode(parent_rtnode), rn->valuestring);	

	// Store to DB
	int result = db_store_resource(sub, ptr);
	if(result != 1) { 
		handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "DB store fail"); 
		cJSON_Delete(root);
		free(ptr);	ptr = NULL;
		return o2pt->rsc;
	}

	RTNode* child_rtnode = create_rtnode(sub, RT_SUB);
	add_child_resource_tree(parent_rtnode,child_rtnode);
	free(ptr);	ptr = NULL;
	return RSC_CREATED;
}

int create_acp(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty != RT_CSE && parent_rtnode->ty != RT_AE) {
		handle_error(o2pt, RSC_INVALID_CHILD_RESOURCETYPE, "child type is invalid");
		return o2pt->rsc;
	}

	cJSON *root = cJSON_Duplicate(o2pt->cjson_pc, 1);
	cJSON *acp = cJSON_GetObjectItem(root, "m2m:acp");
	// if(!is_attr_valid(acp, RT_ACP)){
	// 	handle_error(o2pt, RSC_BAD_REQUEST, "wrong attribute(s) submitted");
	// 	cJSON_Delete(root);
	// 	return o2pt->rsc;
	// }

	add_general_attribute(acp, parent_rtnode, RT_ACP);

	int rsc = validate_acp(o2pt, acp, OP_CREATE);

	if(rsc != RSC_OK){
		cJSON_Delete(root);
		return rsc;
	}
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);

	// Add uri attribute
	char *ptr = malloc(1024);
	cJSON *rn = cJSON_GetObjectItem(acp, "rn");
	sprintf(ptr, "%s/%s", get_uri_rtnode(parent_rtnode), rn->valuestring);
	// Save to DB
	int result = db_store_resource(acp, ptr);
	if(result != 1) { 
		handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "DB store fail");
		cJSON_Delete(root);
		free(ptr); ptr = NULL;
		return RSC_INTERNAL_SERVER_ERROR;
	}

	free(ptr);	ptr = NULL;

	// Add to resource tree
	RTNode* child_rtnode = create_rtnode(acp, RT_ACP);
	add_child_resource_tree(parent_rtnode, child_rtnode); 
	return o2pt->rsc = RSC_CREATED;
}

int update_ae(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][8] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_ae = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:ae");
	int invalid_key_size = sizeof(invalid_key)/(8*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_ae, invalid_key[i])) {
			handle_error(o2pt, RSC_BAD_REQUEST, "unsupported attribute on update");
			return RSC_BAD_REQUEST;
		}
	}

	//TODO - validation process
	int result = validate_ae(o2pt, m2m_ae, OP_UPDATE);
	if(result != RSC_OK){
		logger("O2", LOG_LEVEL_ERROR, "validation failed");
		return result;
	}

	update_resource(target_rtnode->obj, m2m_ae);


	result = db_update_resource(m2m_ae, cJSON_GetObjectItem(target_rtnode->obj, "ri")->valuestring, RT_AE);

	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:ae", target_rtnode->obj);

	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	o2pt->rsc = RSC_UPDATED;

	cJSON_DetachItemFromObject(root, "m2m:ae");
	cJSON_Delete(root);
	return RSC_UPDATED;
}

int update_cnt(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][9] = {"ty", "pi", "ri", "rn", "ct", "cr"};
	cJSON *m2m_cnt = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:cnt");
	int invalid_key_size = sizeof(invalid_key)/(9*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_cnt, invalid_key[i])) {
			handle_error(o2pt, RSC_BAD_REQUEST, "unsupported attribute on update");
			return RSC_BAD_REQUEST;
		}
	}

	cJSON* cnt = target_rtnode->obj;
	int result;
	cJSON *pjson = NULL;

	result = validate_cnt(o2pt, m2m_cnt, OP_UPDATE); //TODO - add UPDATE validation
	if(result != RSC_OK) return result;


	// if(cnt->mbs != INT_MIN && cnt->mbs < 0) {
	// 	handle_error(o2pt, RSC_BAD_REQUEST, "attribute `mbs` is invalid");
	// 	return o2pt->rsc;
	// }
	// if(cnt->mni != INT_MIN && cnt->mni < 0) {
	// 	handle_error(o2pt, RSC_BAD_REQUEST, "attribute `mni` is invalid");
	// 	return o2pt->rsc;
	// }
	//set_rtnode_update(target_rtnode, after);
	

	cJSON_AddNumberToObject(m2m_cnt, "st", cJSON_GetObjectItem(cnt, "st")->valueint + 1);
	update_resource(target_rtnode->obj, m2m_cnt);

	delete_cin_under_cnt_mni_mbs(target_rtnode);

	result = db_update_resource(m2m_cnt, cJSON_GetObjectItem(target_rtnode->obj, "ri")->valuestring, RT_CNT);

	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:cnt", target_rtnode->obj);

	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	o2pt->rsc = RSC_UPDATED;

	cJSON_DetachItemFromObject(root, "m2m:cnt");
	cJSON_Delete(root);
	return RSC_UPDATED;
}

int update_acp(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][8] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_acp = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:acp");
	int invalid_key_size = sizeof(invalid_key)/(8*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_acp, invalid_key[i])) {
			handle_error(o2pt, RSC_BAD_REQUEST, "unsupported attribute on update");
			return RSC_BAD_REQUEST;
		}
	}

	int result = validate_acp(o2pt, m2m_acp, OP_UPDATE);
	if(result != RSC_OK) return result;

	cJSON* acp = target_rtnode->obj;
	cJSON *pjson = NULL;

	update_resource(target_rtnode->obj, m2m_acp);
	
	result = db_update_resource(m2m_acp, cJSON_GetObjectItem(target_rtnode->obj, "ri")->valuestring, RT_ACP);

	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:acp", target_rtnode->obj);

	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	o2pt->rsc = RSC_UPDATED;

	cJSON_DetachItemFromObject(root, "m2m:acp");
	cJSON_Delete(root);
	return RSC_UPDATED;
}

//Todo
int update_csr(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][8] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_csr = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:csr");
	int invalid_key_size = sizeof(invalid_key)/(8*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_csr, invalid_key[i])) {
			handle_error(o2pt, RSC_BAD_REQUEST, "unsupported attribute on update");
			return RSC_BAD_REQUEST;
		}
	}
	cJSON* csr = target_rtnode->obj;
	int result;

	//result = validate_csr(o2pt, target_rtnode->p)
	//if(result != 1) return result;
	

	result = db_update_resource(m2m_csr, cJSON_GetStringValue(cJSON_GetObjectItem(csr, "ri")), RT_CSR);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(csr);
	o2pt->rsc = RSC_UPDATED;
	return RSC_UPDATED;
}

int set_ae_update(oneM2MPrimitive *o2pt, cJSON *m2m_ae, AE* ae) {
	cJSON *rr = cJSON_GetObjectItemCaseSensitive(m2m_ae, "rr");
	cJSON *lbl = cJSON_GetObjectItem(m2m_ae, "lbl");
	cJSON *srv = cJSON_GetObjectItem(m2m_ae, "srv");
	cJSON *et = cJSON_GetObjectItem(m2m_ae, "et");
	cJSON *acpi = cJSON_GetObjectItem(m2m_ae, "acpi");

	if(acpi) {
		if(rr || lbl || srv || et) {
			handle_error(o2pt, RSC_BAD_REQUEST, "`acpi` must be only attribute in update");
			return -1;
		} else if(!ae->acpi && strcmp(o2pt->fr, ae->origin)) {
			handle_error(o2pt, RSC_ORIGINATOR_HAS_NO_PRIVILEGE, "originator has no privilege on udpate `acpi`");
			return -1;
		} else if(!check_acpi_valid(o2pt, acpi)) {
			return -1;
		} else {
			if(ae->acpi) free(ae->acpi);
			ae->acpi = cjson_string_list_item_to_string(acpi);
		}
	}

	if(rr) {
		if(cJSON_IsTrue(rr)) {
			ae->rr = true;
		} else {
			ae->rr = false;
		}
	}

	if(et) {
		if(ae->et) free(ae->et);
		ae->et = (char *)malloc((strlen(et->valuestring) + 1) * sizeof(char));
		strcpy(ae->et, et->valuestring);
	}

	if(lbl) {
		if(ae->lbl) free(ae->lbl);
		ae->lbl = cjson_string_list_item_to_string(lbl);
	}

	if(srv) {
		if(ae->srv) free(ae->srv);
		ae->srv = cjson_string_list_item_to_string(srv);
	}

	if(ae->lt) free(ae->lt);
	ae->lt = get_local_time(0);
	return 1;
}

int set_cnt_update(oneM2MPrimitive *o2pt, cJSON *m2m_cnt, CNT* cnt) {
	cJSON *lbl = cJSON_GetObjectItem(m2m_cnt, "lbl");
	cJSON *acpi = cJSON_GetObjectItem(m2m_cnt, "acpi");
	cJSON *mni = cJSON_GetObjectItem(m2m_cnt, "mni");
	cJSON *mbs = cJSON_GetObjectItem(m2m_cnt, "mbs");
	cJSON *et = cJSON_GetObjectItem(m2m_cnt, "et");

	if(et) {
		if(cnt->et) free(cnt->et);
		cnt->et = (char *)malloc((strlen(et->valuestring) + 1) * sizeof(char));
		strcpy(cnt->et, et->valuestring);
	}

	if(acpi) {
		if(cnt->acpi) free(cnt->acpi);
		cnt->acpi = cjson_string_list_item_to_string(acpi);
	}
	
	if(lbl) {
		if(cnt->lbl) free(cnt->lbl);
		cnt->lbl = cjson_string_list_item_to_string(lbl);
	}

	if(mni) {
		cnt->mni = mni->valueint;
	}

	if(mbs) {
		cnt->mbs = mbs->valueint;
	}

	if(cnt->lt) free(cnt->lt);
	cnt->lt = get_local_time(0);

	return 1;
}

int set_acp_update(oneM2MPrimitive *o2pt, cJSON *m2m_acp, ACP* acp) {
	cJSON *pv_acr = cJSON_GetObjectItem(m2m_acp, "pv");
	cJSON *pvs_acr = cJSON_GetObjectItem(m2m_acp, "pvs");
	cJSON *et = cJSON_GetObjectItem(m2m_acp, "et");
	cJSON *lbl = cJSON_GetObjectItem(m2m_acp, "lbl");
	cJSON *pv_acor = NULL;
	cJSON *pv_acop = NULL;
	cJSON *pvs_acor = NULL;
	cJSON *pvs_acop = NULL;
	char pv_acor_str[256] = {'\0'};
	char pv_acop_str[256] = {'\0'};
	char pvs_acor_str[256] =  {'\0'};
	char pvs_acop_str[256] =  {'\0'};

	if(pvs_acr) {
		pvs_acr = cJSON_GetObjectItem(pvs_acr, "acr");

		if(pvs_acr) {
			int acr_size = cJSON_GetArraySize(pvs_acr);

			for(int i=0; i<acr_size; i++) {
				cJSON *arr_item = cJSON_GetArrayItem(pvs_acr, i);
				pvs_acor = cJSON_GetObjectItem(arr_item, "acor");
				pvs_acop = cJSON_GetObjectItem(arr_item, "acop");

				if(pvs_acor && pvs_acop) {
					char acop[3];
					sprintf(acop, "%d", pvs_acop->valueint);
					int acor_size = cJSON_GetArraySize(pvs_acor);

					for(int j=0; j<acor_size; j++) {
						char *acor = cJSON_GetArrayItem(pvs_acor, j)->valuestring;
						strcat(pvs_acor_str, acor);
						strcat(pvs_acop_str, acop);
						if(j != acor_size - 1) {
							strcat(pvs_acor_str, ",");
							strcat(pvs_acop_str, ",");
						}
					}
					logger("O2", LOG_LEVEL_DEBUG, "pvs_acor_str: %s", pvs_acor_str);
					if(i < acr_size - 1) {
						strcat(pvs_acor_str, ",");
						strcat(pvs_acop_str, ",");
					}
				}

				
			}
		}
		if(pvs_acor_str[0] != '\0' && pvs_acop_str[0] != '\0') {
			if(acp->pvs_acor) free(acp->pvs_acor);
			if(acp->pvs_acop) free(acp->pvs_acop);
			acp->pvs_acor = strdup(pvs_acor_str);
			acp->pvs_acop = strdup(pvs_acop_str);
		} else {
			handle_error(o2pt, RSC_BAD_REQUEST, "attribute `pvs` is mandatory");
			return RSC_BAD_REQUEST;
		}
	}

	if(lbl) {
		if(acp->lbl) free(acp->lbl);
		acp->lbl = cjson_string_list_item_to_string(lbl);
	}

	if(et) {
		if(acp->et) free(acp->et);
		acp->et = (char *)malloc((strlen(et->valuestring) + 1) * sizeof(char));
		strcpy(acp->et, et->valuestring);
	}

	if(pv_acr) {
		pv_acr = cJSON_GetObjectItem(pv_acr, "acr");

		if(pv_acr) {
			int acr_size = cJSON_GetArraySize(pv_acr);

			for(int i=0; i<acr_size; i++) {
				cJSON *arr_item = cJSON_GetArrayItem(pv_acr, i);
				pv_acor = cJSON_GetObjectItem(arr_item, "acor");
				pv_acop = cJSON_GetObjectItem(arr_item, "acop");

				if(pv_acor && pv_acop) {
					char acop[3];
					sprintf(acop, "%d", pv_acop->valueint);
					int acor_size = cJSON_GetArraySize(pv_acor);

					for(int j=0; j<acor_size; j++) {
						char *acor = cJSON_GetArrayItem(pv_acor, j)->valuestring;
						strcat(pv_acor_str, acor);
						strcat(pv_acop_str, acop);
						if(j != acor_size - 1) {
							strcat(pv_acor_str, ",");
							strcat(pv_acop_str, ",");
						}
					}
					if(i < acr_size - 1) {
						strcat(pvs_acor_str, ",");
						strcat(pvs_acop_str, ",");
					}
				}
			}
		}
		if(pv_acor_str[0] != '\0' && pv_acop_str[0] != '\0') {
			if(acp->pv_acor) free(acp->pv_acor);
			if(acp->pv_acop) free(acp->pv_acop);
			acp->pv_acor = strdup(pv_acor_str);
			acp->pv_acop = strdup(pv_acop_str);
		} else {
			acp->pv_acor = acp->pv_acop = NULL;
		}
	}	

	if(acp->lt) free(acp->lt);
	acp->lt = get_local_time(0);

	return 1;
}

int set_csr_update(oneM2MPrimitive *o2pt, cJSON *m2m_csr, CSR* csr){
	cJSON *lbl = cJSON_GetObjectItem(m2m_csr, "lbl");
	cJSON *acpi = cJSON_GetObjectItem(m2m_csr, "acpi");
	cJSON *poa = cJSON_GetObjectItem(m2m_csr, "poa");
	cJSON *csi = cJSON_GetObjectItem(m2m_csr, "csi");
	cJSON *cb = cJSON_GetObjectItem(m2m_csr, "cb");
	cJSON *et = cJSON_GetObjectItem(m2m_csr, "et");

	if(et) {
		if(csr->et) free(csr->et);
		csr->et = (char *)malloc((strlen(et->valuestring) + 1) * sizeof(char));
		strcpy(csr->et, et->valuestring);
	}

	if(acpi) {
		if(csr->acpi) free(csr->acpi);
		csr->acpi = cjson_string_list_item_to_string(acpi);
	}

	if(lbl) {
		if(csr->lbl) free(csr->lbl);
		csr->lbl = cjson_string_list_item_to_string(lbl);
	}

	if(poa) {
		if(csr->poa) free(csr->poa);
		csr->poa = cjson_string_list_item_to_string(poa);
	}

	if(csi) {
		if(csr->csi) free(csr->csi);
		csr->csi = cjson_string_list_item_to_string(csi);
	}

	if(cb) {
		if(csr->cb) free(csr->cb);
		csr->cb = cjson_string_list_item_to_string(cb);
	}
	
	return 1;
}

int update_cnt_cin(RTNode *cnt_rtnode, RTNode *cin_rtnode, int sign) {
	cJSON *cnt = cnt_rtnode->obj;
	cJSON *cin = cin_rtnode->obj;
	cJSON *cni = cJSON_GetObjectItem(cnt, "cni");
	cJSON *cbs = cJSON_GetObjectItem(cnt, "cbs");
	cJSON *st = cJSON_GetObjectItem(cnt, "st");

	cJSON_SetIntValue(cni, cni->valueint + sign);
	cJSON_SetIntValue(cbs, cbs->valueint + sign * cJSON_GetObjectItem(cin, "cs")->valueint);
	cJSON_SetIntValue(st, st->valueint + 1);
	logger("O2", LOG_LEVEL_DEBUG, "cni: %d, cbs: %d, st: %d", cJSON_GetObjectItem(cnt, "cni")->valueint, cJSON_GetObjectItem(cnt, "cbs")->valueint, cJSON_GetObjectItem(cnt, "st")->valueint);
	delete_cin_under_cnt_mni_mbs(cnt_rtnode);	

	db_update_resource(cnt, cJSON_GetObjectItem(cnt, "ri")->valuestring, RT_CNT);


	return 1;
}

int delete_onem2m_resource(oneM2MPrimitive *o2pt, RTNode* target_rtnode) {
	logger("O2M", LOG_LEVEL_INFO, "Delete oneM2M resource");
	if(target_rtnode->ty == RT_CSE) {
		handle_error(o2pt, RSC_OPERATION_NOT_ALLOWED, "CSE can not be deleted");
		return RSC_OPERATION_NOT_ALLOWED;
	}
	if(target_rtnode->ty == RT_AE || target_rtnode->ty == RT_CNT || target_rtnode->ty == RT_GRP || target_rtnode->ty == RT_ACP) {
		if(check_privilege(o2pt, target_rtnode, ACOP_DELETE) == -1) {
			return o2pt->rsc;
		}
	}
	delete_rtnode_and_db_data(o2pt, target_rtnode,1);
	target_rtnode = NULL;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = NULL;
	o2pt->rsc = RSC_DELETED;
	return RSC_DELETED;
}

int delete_rtnode_and_db_data(oneM2MPrimitive *o2pt, RTNode *rtnode, int flag) {
	switch(rtnode->ty) {
	case RT_AE : 
	case RT_CNT : 
	case RT_SUB :
	case RT_ACP :
	case RT_GRP:
	case RT_CSR:
		db_delete_onem2m_resource(rtnode); 
		break;
	case RT_CIN :
		db_delete_onem2m_resource(rtnode);
		update_cnt_cin(rtnode->parent, rtnode,-1);
		break;
	}

	notify_onem2m_resource(o2pt, rtnode);
	if(rtnode->ty == RT_CIN) return 1;

	RTNode *left = rtnode->sibling_left;
	RTNode *right = rtnode->sibling_right;
	
	if(rtnode->ty != RT_CIN) {
		if(flag == 1) {
			if(left) left->sibling_right = right;
			else rtnode->parent->child = right;
			if(right) right->sibling_left = left;
		} else {
			if(right) delete_rtnode_and_db_data(o2pt, right, 0);
		}
	}
	
	if(rtnode->child) delete_rtnode_and_db_data(o2pt, rtnode->child, 0);
	
	free_rtnode(rtnode); rtnode = NULL;
	return 1;
}

void free_rtnode(RTNode *rtnode) {
	if(rtnode->uri && rtnode->ty != RT_CSE)
		free(rtnode->uri);
	cJSON_Delete(rtnode->obj);
	if(rtnode->parent && rtnode->parent->child == rtnode){
		rtnode->parent->child = rtnode->sibling_right;
	}
	if(rtnode->sibling_left){
		rtnode->sibling_left->sibling_right = rtnode->sibling_right;
	}
	if(rtnode->sibling_right){
		rtnode->sibling_right->sibling_left = rtnode->sibling_left;
	}
	if(rtnode->child){
		free_rtnode_list(rtnode->child);
	}

	
	free(rtnode);
}

void free_rtnode_list(RTNode *rtnode) {
	RTNode *right = NULL;
	while(rtnode) {
		right = rtnode->sibling_right;
		free_rtnode(rtnode);
		rtnode = right;
	}
}


/* GROUP IMPLEMENTATION */
int update_grp(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	int rsc = 0, result = 0;
	char invalid_key[6][4] = {"ty", "pi", "ri", "rn", "ct", "mtv"};
	cJSON *m2m_grp = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:grp");
	cJSON *pjson = NULL;

	int invalid_key_size = 6;
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_grp, invalid_key[i])) {
			handle_error(o2pt, RSC_BAD_REQUEST, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			return RSC_BAD_REQUEST;
		}
	}

	if( (rsc = validate_grp_update(o2pt, target_rtnode->obj, m2m_grp))  >= 4000){
		o2pt->rsc = rsc;
		return rsc;
	}

	if(pjson = cJSON_GetObjectItem(m2m_grp, "mid")){
		cJSON_SetIntValue(cJSON_GetObjectItem(target_rtnode->obj, "cnm"), cJSON_GetArraySize(pjson));
	}

	update_resource(target_rtnode->obj, m2m_grp);

	result = db_update_resource(m2m_grp, cJSON_GetObjectItem(target_rtnode->obj, "ri")->valuestring, RT_GRP);

	if(!result){
		logger("O2M", LOG_LEVEL_ERROR, "DB update Failed");
		return RSC_INTERNAL_SERVER_ERROR;
	}

	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:grp", target_rtnode->obj);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	o2pt->rsc = RSC_UPDATED;

	cJSON_DetachItemFromObject(root, "m2m:grp");
	cJSON_Delete(root);
	root = NULL;

	return RSC_UPDATED;
}

int create_grp(oneM2MPrimitive *o2pt, RTNode *parent_rtnode){
	int e = 1;
	int rsc = 0;
	if( parent_rtnode->ty != RT_AE && parent_rtnode->ty != RT_CSE ) {
		return o2pt->rsc = RSC_INVALID_CHILD_RESOURCETYPE;
	}

	cJSON *root = cJSON_Duplicate(o2pt->cjson_pc, 1);
	cJSON *grp = cJSON_GetObjectItem(root, "m2m:grp");

	add_general_attribute(grp, parent_rtnode, RT_GRP);

	cJSON_AddItemToObject(grp, "cnm", cJSON_CreateNumber(cJSON_GetArraySize(cJSON_GetObjectItem(grp, "mid"))));

	rsc = validate_grp(o2pt, grp);
	if(rsc >= 4000){
		logger("O2M", LOG_LEVEL_DEBUG, "Group Validation failed");
		return o2pt->rsc = rsc;
	}
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(o2pt->cjson_pc);
	o2pt->rsc = RSC_CREATED;

	cJSON *rn = cJSON_GetObjectItem(grp, "rn");
	char *uri = (char *)malloc((strlen(rn->valuestring) + strlen(parent_rtnode->uri) + 2) * sizeof(char));
	sprintf(uri, "%s/%s", parent_rtnode->uri, rn->valuestring);

	int result = db_store_resource(grp, uri);
	if(result != 1){
		handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "DB store fail");
		return RSC_INTERNAL_SERVER_ERROR;
	}
	

	RTNode *child_rtnode = create_rtnode(grp, RT_GRP);
	add_child_resource_tree(parent_rtnode, child_rtnode);


	free(uri); uri = NULL;
	return rsc;
}

int update_sub(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	char invalid_key[][8] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_sub = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:sub");
	int invalid_key_size = sizeof(invalid_key)/(8*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_sub, invalid_key[i])) {
			handle_error(o2pt, RSC_BAD_REQUEST, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			return RSC_BAD_REQUEST;
		}
	}

	cJSON* sub = target_rtnode->obj;
	int result;
	
	validate_sub(o2pt, m2m_sub, o2pt->op);
	update_resource(sub, m2m_sub);
	db_update_resource(m2m_sub, cJSON_GetObjectItem(sub, "ri")->valuestring, RT_SUB);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(sub);
	o2pt->rsc = RSC_UPDATED;
	return RSC_UPDATED;
}

int create_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	int rsc = 0;
	int e = check_resource_type_invalid(o2pt);
	if(e != -1) e = check_payload_empty(o2pt);
	if(e != -1) e = check_payload_format(o2pt);
	if(e != -1) e = check_resource_type_equal(o2pt);
	if(e != -1) e = check_privilege(o2pt, parent_rtnode, ACOP_CREATE);
	if(e != -1) e = check_rn_duplicate(o2pt, parent_rtnode);
	if(e == -1) return o2pt->rsc;

	switch(o2pt->ty) {	
	case RT_AE :
		logger("O2M", LOG_LEVEL_INFO, "Create AE");
		rsc = create_ae(o2pt, parent_rtnode);
		break;	

	case RT_CNT :
		logger("O2M", LOG_LEVEL_INFO, "Create CNT");
		rsc = create_cnt(o2pt, parent_rtnode);
		break;
		
	case RT_CIN :
		logger("O2M", LOG_LEVEL_INFO, "Create CIN");
		rsc = create_cin(o2pt, parent_rtnode);
		break;

	case RT_SUB :
		logger("O2M", LOG_LEVEL_INFO, "Create SUB");
		rsc = create_sub(o2pt, parent_rtnode);
		break;
	
	case RT_ACP :
		logger("O2M", LOG_LEVEL_INFO, "Create ACP");
		rsc = create_acp(o2pt, parent_rtnode);
		break;

	case RT_GRP:
		logger("O2M", LOG_LEVEL_INFO, "Create GRP");
		rsc = create_grp(o2pt, parent_rtnode);
		break;

	case RT_CSR:
		logger("O2M", LOG_LEVEL_INFO, "Create CSR");
		rsc = create_csr(o2pt, parent_rtnode);
		break;

	case RT_MIXED :
		handle_error(o2pt, RSC_BAD_REQUEST, "resource type error");
		rsc = o2pt->rsc;
	}	
	return rsc;
}

int retrieve_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	int rsc = 0;
	int e = check_privilege(o2pt, target_rtnode, ACOP_RETRIEVE);

	if(e == -1) return o2pt->rsc;
	cJSON *root = cJSON_CreateObject();
	cJSON *obj = cJSON_Duplicate(target_rtnode->obj, 1);

	cJSON_AddItemToObject(root, get_resource_key(target_rtnode->ty), obj);

	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);
	logger("O2M", LOG_LEVEL_INFO, "retrieve pc : %s", o2pt->pc);
	cJSON_DetachItemFromObject(root, get_resource_key(target_rtnode->ty));
	o2pt->rsc = RSC_OK;
	return RSC_OK;
}

int update_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	int rsc = 0;
	o2pt->ty = target_rtnode->ty;
	int e = check_payload_empty(o2pt);
	if(e != -1) e = check_payload_format(o2pt);
	ResourceType ty = parse_object_type_cjson(o2pt->cjson_pc);
	if(e != -1) e = check_resource_type_equal(o2pt);
	if(e != -1) e = check_privilege(o2pt, target_rtnode, ACOP_UPDATE);
	if(e != -1) e = check_rn_duplicate(o2pt, target_rtnode->parent);
	if(e == -1) return o2pt->rsc;
	
	switch(ty) {
		case RT_AE :
			logger("O2M", LOG_LEVEL_INFO, "Update AE");
			rsc = update_ae(o2pt, target_rtnode);
			break;

		case RT_CNT :
			logger("O2M", LOG_LEVEL_INFO, "Update CNT");
			rsc = update_cnt(o2pt, target_rtnode);
			break;

		case RT_SUB :
			logger("O2M", LOG_LEVEL_INFO, "Update SUB");
			rsc = update_sub(o2pt, target_rtnode);
			break;
		
		case RT_ACP :
			logger("O2M", LOG_LEVEL_INFO, "Update ACP");
			rsc = update_acp(o2pt, target_rtnode);
			break;

		case RT_GRP:
			logger("O2M", LOG_LEVEL_INFO, "Update GRP");
			rsc = update_grp(o2pt, target_rtnode);
			break;

		default :
			handle_error(o2pt, RSC_OPERATION_NOT_ALLOWED, "operation `update` is unsupported");
			rsc = o2pt->rsc;
		}
	return rsc;
}

int fopt_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *parent_rtnode){
	int rsc = 0;
	int cnt = 0;
	int cnm = 0;

	RTNode *target_rtnode = NULL;
	oneM2MPrimitive *req_o2pt = NULL;
	cJSON *new_pc = NULL;
	cJSON *agr = NULL;
	cJSON *rsp = NULL;
	cJSON *json = NULL;
	cJSON *grp = NULL;
	
	if(parent_rtnode == NULL){
		o2pt->rsc = RSC_NOT_FOUND;
		return RSC_NOT_FOUND;
	}
	logger("O2M", LOG_LEVEL_DEBUG, "handle fopt");


	grp = parent_rtnode->obj;
	if(!grp){
		o2pt->rsc = RSC_INTERNAL_SERVER_ERROR;
		return RSC_INTERNAL_SERVER_ERROR;
	}
	
	if((cnm = cJSON_GetObjectItem(grp, "cnm")->valueint) == 0){
		logger("O2M", LOG_LEVEL_DEBUG, "No member to fanout");
		return o2pt->rsc = RSC_NO_MEMBERS;
	}

	o2ptcpy(&req_o2pt, o2pt);

	new_pc = cJSON_CreateObject();
	cJSON_AddItemToObject(new_pc, "m2m:agr", agr = cJSON_CreateObject());
	cJSON_AddItemToObject(agr, "m2m:rsp", rsp = cJSON_CreateArray());
	cJSON *mid_obj = NULL;

	cJSON_ArrayForEach(mid_obj, cJSON_GetObjectItem(grp, "mid")){
		char *mid = cJSON_GetStringValue(mid_obj);
		if(req_o2pt->to) free(req_o2pt->to);
		if(o2pt->fopt){
			if(strncmp(mid, CSE_BASE_NAME, strlen(CSE_BASE_NAME))){
				mid = ri_to_uri(mid);
			}
			req_o2pt->to = malloc(strlen(mid) + strlen(o2pt->fopt) + 1);
		}else{
			req_o2pt->to = malloc(strlen(mid) + 1);
		}
		
		strcpy(req_o2pt->to, mid);
		if(o2pt->fopt) strcat(req_o2pt->to, o2pt->fopt);

		req_o2pt->isFopt = false;
		
		target_rtnode = parse_uri(req_o2pt, rt->cb);
		if(target_rtnode && target_rtnode->ty == RT_AE){
			req_o2pt->fr = strdup(get_ri_rtnode(target_rtnode));
		}
		
		if(target_rtnode){
			rsc = handle_onem2m_request(req_o2pt, target_rtnode);
			if(rsc < 4000) cnt++;
			json = o2pt_to_json(req_o2pt);
			if(json) {
				cJSON_AddItemToArray(rsp, json);
			}
			if(req_o2pt->op != OP_DELETE && target_rtnode->ty == RT_CIN){
				free_rtnode(target_rtnode);
				target_rtnode = NULL;
			}

		} else{
			logger("O2M", LOG_LEVEL_DEBUG, "rtnode not found");
		}
	}

	if(o2pt->pc) free(o2pt->pc); //TODO double free bug
	o2pt->pc = cJSON_PrintUnformatted(new_pc);

	cJSON_Delete(new_pc);
	
	o2pt->rsc = RSC_OK;	

	free_o2pt(req_o2pt);
	req_o2pt = NULL;
	return RSC_OK;
}

/**
 * Discover Resources based on Filter Criteria
*/
int discover_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	logger("MAIN", LOG_LEVEL_DEBUG, "Discover Resource");
	cJSON *fc = o2pt->fc;
	cJSON *pjson = NULL;
	cJSON *root = cJSON_CreateObject();
	cJSON *uril = NULL;
	int urilSize = 0;
	if(!o2pt->fc){
		logger("O2M", LOG_LEVEL_WARN, "Empty Filter Criteria");
		return RSC_BAD_REQUEST;
	}

	uril = db_get_filter_criteria(o2pt->to, o2pt->fc);

	// TODO - Check OPS
	if(pjson = cJSON_GetObjectItem(o2pt->fc, "ops")){
	}



	
	
	urilSize = cJSON_GetArraySize(uril);
	int lim = cJSON_GetNumberValue(cJSON_GetObjectItem(fc, "lim"));
	int ofst = cJSON_GetNumberValue(cJSON_GetObjectItem(fc, "ofst"));
	if(lim < urilSize - ofst){
		logger("O2M", LOG_LEVEL_DEBUG, "limit exceeded");
		for(int i = 0 ; i < ofst ; i++){
			cJSON_DeleteItemFromArray(uril, 0);
		}
		urilSize = cJSON_GetArraySize(uril);
		for(int i = lim ; i < urilSize; i++){
			cJSON_DeleteItemFromArray(uril, lim);
		}
		o2pt->cnst = CS_PARTIAL_CONTENT;
		o2pt->cnot = ofst + lim;
	}
	cJSON_AddItemToObject(root, "m2m:uril", uril);

	if(o2pt->pc)
		free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return o2pt->rsc = RSC_OK;

}

int notify_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	if(!target_rtnode) {
		logger("O2M", LOG_LEVEL_ERROR, "target_rtnode is NULL");
		return -1;
	}
	int net = NET_NONE;

	switch(o2pt->op) {
		case OP_CREATE:
			net = NET_CREATE_OF_DIRECT_CHILD_RESOURCE;
			break;
		case OP_UPDATE:
			net = NET_UPDATE_OF_RESOURCE;
			break;
		case OP_DELETE:
			net = NET_DELETE_OF_RESOURCE;
			break;
	}

	cJSON *noti_cjson, *sgn, *nev;
	noti_cjson = cJSON_CreateObject();
	cJSON_AddItemToObject(noti_cjson, "m2m:sgn", sgn = cJSON_CreateObject());
	cJSON_AddItemToObject(sgn, "nev", nev = cJSON_CreateObject());
	cJSON_AddNumberToObject(nev, "net", net);
	cJSON_AddStringToObject(nev, "rep", o2pt->pc);


	RTNode *child = target_rtnode->child;

	while(child) {
		if(child->ty == RT_SUB) {
			cJSON_AddStringToObject(sgn, "sur", child->uri);
			notify_to_nu(o2pt, child, noti_cjson, net);
			cJSON_DeleteItemFromObject(sgn, "sur");
		}
		child = child->sibling_right;
	}

	if(net == NET_DELETE_OF_RESOURCE) {
		net = NET_DELETE_OF_DIRECT_CHILD_RESOURCE;
		cJSON_SetNumberValue(cJSON_GetObjectItem(nev, "net"), net);
		child = target_rtnode->parent->child;
		while(child) {
			if(child->ty == RT_SUB) {
				cJSON_AddStringToObject(sgn, "sur", child->uri);
				notify_to_nu(o2pt, child, noti_cjson, net);
				cJSON_DeleteItemFromObject(sgn, "sur");
			}
			child = child->sibling_right;
		}
	}

	cJSON_Delete(noti_cjson);

	return 1;
}

int forwarding_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	logger("O2M", LOG_LEVEL_DEBUG, "Forwarding Resource");
	char *host = NULL;
	char *port = NULL;
	logger("O2M", LOG_LEVEL_DEBUG, "target_rtnode->ty : %d", target_rtnode->ty);
	if(target_rtnode->ty != RT_CSR){
		logger("O2M", LOG_LEVEL_ERROR, "target_rtnode is not CSR");
		return o2pt->rsc = RSC_NOT_FOUND;
	}

	CSR *csr = (CSR *)target_rtnode->obj;
	char *poa_list = strdup(csr->poa);
	char *ptr = strtok(poa_list, ",");
	while(ptr){
		if(strncmp(ptr, "http://", 7) == 0){
			host = ptr + 7;
			port = strchr(host, ':');
			if(port){
				*port = '\0';
				port++;
			}else{
				port = "80";
			}
			http_forwarding(o2pt, host, port);
		}else if (strncmp(ptr, "mqtt://", 7) == 0){
			host = ptr + 7;
			port = strchr(host, ':');
			if(port){
				*port = '\0';
				port++;
			}
			mqtt_forwarding(o2pt, host, port, csr);
		}
		ptr = strtok(NULL, ",");
	}

	free(poa_list);	
}