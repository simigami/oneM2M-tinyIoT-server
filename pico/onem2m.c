#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <curl/curl.h>
#include <math.h>
#include <ctype.h>
#include <malloc.h>
#include <sys/timeb.h>
#include "onem2m.h"
#include "dbmanager.h"
#include "jsonparser.h"
#include "httpd.h"
#include "mqttClient.h"
#include "onem2mTypes.h"
#include "config.h"
#include "util.h"

extern ResourceTree *rt;

void init_cse(CSE* cse) {
	char *ct = get_local_time(0);
	char *ri = CSE_BASE_RI;
	char *rn = CSE_BASE_NAME;
	
	cse->ri = (char*)malloc((strlen(ri) + 1) * sizeof(char));
	cse->rn = (char*)malloc((strlen(rn) + 1) * sizeof(char));
	cse->ct = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	cse->lt = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	cse->csi = (char*)malloc((strlen(rn) + 2) * sizeof(char));
	cse->pi = (char*)malloc((strlen("NULL") + 1) * sizeof(char));
	
	strcpy(cse->ri, ri);
	strcpy(cse->rn, rn);
	strcpy(cse->ct, ct);
	strcpy(cse->lt, ct);
	strcpy(cse->csi,"/");
	strcat(cse->csi,rn);
	strcpy(cse->pi, "NULL");
	
	cse->ty = RT_CSE;
	
	free(ct); ct = NULL;
}

void init_ae(AE* ae, char *pi, char *origin) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char ri[128] = {'\0'};

	if(origin && strlen(origin) > 0) {
		if(origin[0] != 'C') strcpy(ri, "C");
		strcat(ri, origin);
	} else {
		struct timespec specific_time;

    	clock_gettime(CLOCK_REALTIME, &specific_time);
    	int millsec = floor(specific_time.tv_nsec/1.0e6);

		sprintf(ri, "%s%s%09d","CAE", ct, millsec);
	}

	if(!ae->rn) {
		ae->rn = (char*)malloc((strlen(ri) + 1) * sizeof(char));
		strcpy(ae->rn, ri);
	}
	
	ae->ri = (char*)malloc((strlen(ri) + 1) * sizeof(char));
	ae->pi = (char*)malloc((strlen(pi) + 1) * sizeof(char));
	ae->et = (char*)malloc((strlen(et) + 1) * sizeof(char));
	ae->ct = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	ae->lt = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	ae->aei = (char*)malloc((strlen(ri) + 1) * sizeof(char));
	strcpy(ae->ri, ri);
	strcpy(ae->pi, pi);
	strcpy(ae->et, et);
	strcpy(ae->ct, ct);
	strcpy(ae->lt, ct);
	strcpy(ae->aei, ri);
	
	ae->ty = RT_AE;
	
	free(ct); ct = NULL;
	free(et); et = NULL;
}

void init_cnt(CNT* cnt, char *pi) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(RT_CNT, ct);
	
	if(!cnt->rn) {
		cnt->rn = (char*)malloc((strlen(ri) + 1) * sizeof(char));
		strcpy(cnt->rn, ri);
	}
	
	cnt->ri = (char*)malloc((strlen(ri) + 1) * sizeof(char));
	cnt->pi = (char*)malloc((strlen(pi) + 1) * sizeof(char));
	cnt->et = (char*)malloc((strlen(et) + 1) * sizeof(char));
	cnt->ct = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	cnt->lt = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	strcpy(cnt->ri, ri);
	strcpy(cnt->pi, pi);
	strcpy(cnt->et, et);
	strcpy(cnt->ct, ct);
	strcpy(cnt->lt, ct);
	
	cnt->ty = RT_CNT;
	cnt->st = 0;
	cnt->cni = 0;
	cnt->cbs = 0;
	
	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

void init_cin(CIN* cin, char *pi) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(RT_CIN, ct);
	
	cin->rn = (char*)malloc((strlen(ri) + 1) * sizeof(char));
	cin->ri = (char*)malloc((strlen(ri) + 1) * sizeof(char));
	cin->pi = (char*)malloc((strlen(pi) + 1) * sizeof(char));
	cin->et = (char*)malloc((strlen(et) + 1) * sizeof(char));
	cin->ct = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	cin->lt = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	strcpy(cin->rn, ri);
	strcpy(cin->ri, ri);
	strcpy(cin->pi, pi);
	strcpy(cin->et, et);
	strcpy(cin->ct, ct);
	strcpy(cin->lt, ct);
	
	cin->ty = RT_CIN;
	cin->st = 0;
	cin->cs = strlen(cin->con);
	
	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

void init_sub(SUB* sub, char *pi, char *uri) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(RT_SUB, ct);
	if(!sub->rn) {
		sub->rn = (char*)malloc((strlen(ri) + 1) * sizeof(char));
		strcpy(sub->rn, ri);
	}

	if(!sub->net) {
		sub->net = (char*)malloc(2*sizeof(char));
		strcpy(sub->net,"1");
	}

	sub->ri = (char*)malloc((strlen(ri) + 1) * sizeof(char));
	sub->pi = (char*)malloc((strlen(pi) + 1) * sizeof(char));
	sub->et = (char*)malloc((strlen(et) + 1) * sizeof(char));
	sub->ct = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	sub->lt = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	sub->sur = (char *)malloc((strlen(uri) + strlen(sub->rn) + 2) * sizeof(char));

	sprintf(sub->sur, "/%s/%s",uri,sub->rn);
	strcpy(sub->ri, ri);
	strcpy(sub->pi, pi);
	strcpy(sub->et, et);
	strcpy(sub->ct, ct);
	strcpy(sub->lt, ct);
	sub->ty = RT_SUB;
	sub->nct = 0;

	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

void init_acp(ACP* acp, char *pi) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(RT_ACP, ct);

	if(!acp->rn) {
		acp->rn = (char*)malloc((strlen(ri) + 1) * sizeof(char));
		strcpy(acp->rn, ri);
	}
	
	acp->ri = (char*)malloc((strlen(ri) + 1) * sizeof(char));
	acp->pi = (char*)malloc((strlen(pi) + 1) * sizeof(char));
	acp->et = (char*)malloc((strlen(et) + 1) * sizeof(char));
	acp->ct = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	acp->lt = (char*)malloc((strlen(ct) + 1) * sizeof(char));
	strcpy(acp->ri, ri);
	strcpy(acp->pi, pi);
	strcpy(acp->et, et);
	strcpy(acp->ct, ct);
	strcpy(acp->lt, ct);
	
	acp->ty = RT_ACP;
	
	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

RTNode* create_rtnode(void *obj, ResourceType ty){
	RTNode* rtnode = (RTNode *)calloc(1, sizeof(RTNode));

	rtnode->ty = ty;
	rtnode->obj = obj;

	rtnode->parent = NULL;
	rtnode->child = NULL;
	rtnode->sibling_left = NULL;
	rtnode->sibling_right = NULL;
	
	return rtnode;
}

int create_ae(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	int e = check_aei_duplicate(o2pt, parent_rtnode);
	if(e != -1) e = check_rn_invalid(o2pt, RT_AE);
	if(e == -1) return o2pt->rsc;

	if(parent_rtnode->ty != RT_CSE) {
		child_type_error(o2pt);
		return o2pt->rsc;
	}
	AE* ae = cjson_to_ae(o2pt->cjson_pc);
	if(!ae) {
		no_mandatory_error(o2pt);
		return o2pt->rsc = RSC_CONTENTS_UNACCEPTABLE;
	}
	if(ae->api[0] != 'R' && ae->api[0] != 'N') {
		free_ae(ae);
		api_prefix_invalid(o2pt);
		return o2pt->rsc = RSC_BAD_REQUEST;
	}
	init_ae(ae, get_ri_rtnode(parent_rtnode), o2pt->fr);
	
	int result = db_store_ae(ae);
	if(result != 1) { 
		db_store_fail(o2pt); free_ae(ae); ae = NULL;
		return o2pt->rsc = RSC_INTERNAL_SERVER_ERROR;
	}
	
	RTNode* child_rtnode = create_rtnode(ae, RT_AE);
	add_child_resource_tree(parent_rtnode, child_rtnode);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = ae_to_json(ae);
	return o2pt->rsc = RSC_CREATED;
	// notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	
}

int create_cnt(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty != RT_CNT && parent_rtnode->ty != RT_AE && parent_rtnode->ty != RT_CSE) {
		child_type_error(o2pt);
		return RSC_INVALID_CHILD_RESOURCETYPE;
	}
	CNT* cnt = cjson_to_cnt(o2pt->cjson_pc);
	if(!cnt) {
		no_mandatory_error(o2pt);
		return o2pt->rsc;
	}
	if(cnt->mbs != INT_MIN && cnt->mbs < 0) {
		mni_mbs_invalid(o2pt, "mbs"); free(cnt); cnt = NULL;
		return o2pt->rsc;
	}
	if(cnt->mni != INT_MIN && cnt->mni < 0) {
		mni_mbs_invalid(o2pt, "mni"); free(cnt); cnt = NULL;
		return o2pt->rsc;
	}
	init_cnt(cnt,get_ri_rtnode(parent_rtnode));

	int result = db_store_cnt(cnt);
	if(result != 1) { 
		db_store_fail(o2pt); free_cnt(cnt); cnt = NULL;
		return o2pt->rsc;
	}
	
	RTNode* child_rtnode = create_rtnode(cnt, RT_CNT);
	add_child_resource_tree(parent_rtnode,child_rtnode);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cnt_to_json(cnt);
	o2pt->rsc = RSC_CREATED;
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	return RSC_CREATED;
}

int create_cin(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty != RT_CNT) {
		child_type_error(o2pt);
		return o2pt->rsc;
	}
	CIN* cin = cjson_to_cin(o2pt->cjson_pc);
	CNT* cnt = (CNT *)parent_rtnode->obj;
	if(!cin) {
		no_mandatory_error(o2pt);
		return o2pt->rsc;
	} else if(cnt->mbs >= 0 && cin->cs > cnt->mbs) {
		too_large_content_size_error(o2pt); free(cin); cin = NULL;
		return o2pt->rsc;
	}
	init_cin(cin,get_ri_rtnode(parent_rtnode));

	int result = db_store_cin(cin);
	if(result != 1) { 
		db_store_fail(o2pt); free_cin(cin); cin = NULL;
		return o2pt->rsc;
	}

	RTNode *cin_rtnode = create_rtnode(cin, RT_CIN);
	update_cnt_cin(parent_rtnode, cin_rtnode, 1);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cin_to_json(cin);
	o2pt->rsc = RSC_CREATED;

	free_rtnode(cin_rtnode);
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	return RSC_CREATED;
}

int create_sub(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty == RT_CIN || parent_rtnode->ty == RT_SUB) {
		child_type_error(o2pt);
		return o2pt->rsc;
	}
	SUB* sub = cjson_to_sub(o2pt->cjson_pc);
	if(!sub) {
		no_mandatory_error(o2pt);
		return o2pt->rsc;
	}
	init_sub(sub, get_ri_rtnode(parent_rtnode), o2pt->to);
	
	int result = db_store_sub(sub);
	if(result != 1) { 
		db_store_fail(o2pt); free_sub(sub); sub = NULL;
		return o2pt->rsc;
	}

	RTNode* child_rtnode = create_rtnode(sub, RT_SUB);
	add_child_resource_tree(parent_rtnode,child_rtnode);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = sub_to_json(sub);
	o2pt->rsc = RSC_CREATED;
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	return RSC_CREATED;
}

int create_acp(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty != RT_CSE && parent_rtnode->ty != RT_AE) {
		child_type_error(o2pt);
		return o2pt->rsc;
	}
	ACP* acp = cjson_to_acp(o2pt->cjson_pc);
	if(!acp) {
		no_mandatory_error(o2pt);
		return o2pt->rsc;
	}
	init_acp(acp, get_ri_rtnode(parent_rtnode));
	
	int result = db_store_acp(acp);
	if(result != 1) { 
		db_store_fail(o2pt); free_acp(acp); acp = NULL;
		return o2pt->rsc;
	}
	
	RTNode* child_rtnode = create_rtnode(acp, RT_ACP);
	add_child_resource_tree(parent_rtnode, child_rtnode);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = acp_to_json(acp);
	o2pt->rsc = RSC_CREATED;
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	return RSC_CREATED;
}

int retrieve_cse(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	CSE* cse = (CSE *)target_rtnode->obj;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cse_to_json(cse);
	o2pt->rsc = RSC_OK;
	//free_cse(gcse); gcse = NULL;
	return RSC_OK;
}

int retrieve_ae(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	AE* ae = (AE *)target_rtnode->obj;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = ae_to_json(ae);
	o2pt->rsc = RSC_OK;
	return RSC_OK;
}

int retrieve_cnt(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	CNT* cnt = (CNT *)target_rtnode->obj;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cnt_to_json(cnt);
	o2pt->rsc = RSC_OK;

	return RSC_OK;
}

int retrieve_cin(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	CIN* cin = (CIN *)target_rtnode->obj;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cin_to_json(cin);
	o2pt->rsc = RSC_OK;

	return RSC_OK;
}

int retrieve_sub(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	SUB* sub = (SUB *)target_rtnode->obj;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = sub_to_json(sub);
	o2pt->rsc = RSC_OK;

	return RSC_OK;
}

int retrieve_acp(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	ACP* acp = (ACP *)target_rtnode->obj;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = acp_to_json(acp);
	o2pt->rsc = RSC_OK;

	return RSC_OK;
}

int retrieve_grp(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	GRP *grp = (GRP *)target_rtnode->obj;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = grp_to_json(grp);
	o2pt->rsc = RSC_OK;

	return RSC_OK;
}


int update_ae(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][8] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_ae = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:ae");
	int invalid_key_size = sizeof(invalid_key)/(8*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_ae, invalid_key[i])) {
			logger("O2M", LOG_LEVEL_ERROR, "Unsupported attribute on update");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			o2pt->rsc = RSC_BAD_REQUEST;
			return RSC_BAD_REQUEST;
		}
	}

	int result;
	AE *ae = (AE*)target_rtnode->obj;

	set_ae_update(m2m_ae, ae);
	result = db_delete_onem2m_resource(ae);
	result = db_store_ae(ae);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = ae_to_json(ae);
	o2pt->rsc = RSC_UPDATED;
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	return RSC_UPDATED;
}

int update_cnt(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][8] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_cnt = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:cnt");
	int invalid_key_size = sizeof(invalid_key)/(8*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_cnt, invalid_key[i])) {
			logger("O2M", LOG_LEVEL_ERROR, "Unsupported attribute on update");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			o2pt->rsc = RSC_BAD_REQUEST;
			return RSC_BAD_REQUEST;
		}
	}

	CNT* cnt = (CNT *)target_rtnode->obj;
	int result;

	set_cnt_update(m2m_cnt, cnt);
	if(cnt->mbs != INT_MIN && cnt->mbs < 0) {
		mni_mbs_invalid(o2pt, "mbs");
		return o2pt->rsc;
	}
	if(cnt->mni != INT_MIN && cnt->mni < 0) {
		mni_mbs_invalid(o2pt, "mni");
		return o2pt->rsc;
	}
	//set_rtnode_update(target_rtnode, after);
	delete_cin_under_cnt_mni_mbs(cnt);
	cnt->st++;
	result = db_delete_onem2m_resource(cnt->ri);
	result = db_store_cnt(cnt);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cnt_to_json(cnt);
	o2pt->rsc = RSC_UPDATED;
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	return RSC_UPDATED;
}

int update_acp(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][8] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_acp = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:acp");
	int invalid_key_size = sizeof(invalid_key)/(8*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_acp, invalid_key[i])) {
			logger("O2M", LOG_LEVEL_ERROR, "Unsupported attribute on update");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			o2pt->rsc = RSC_BAD_REQUEST;
			return RSC_BAD_REQUEST;
		}
	}
	ACP* acp = (ACP *)target_rtnode->obj;
	int result;
	
	set_acp_update(m2m_acp, acp);
	result = db_delete_acp(acp->ri);
	result = db_store_acp(acp);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = acp_to_json(acp);
	o2pt->rsc = RSC_UPDATED;
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	return RSC_UPDATED;
}

int set_ae_update(cJSON *m2m_ae, AE* after) {
	cJSON *rr = cJSON_GetObjectItemCaseSensitive(m2m_ae, "rr");
	cJSON *lbl = cJSON_GetObjectItem(m2m_ae, "lbl");
	cJSON *srv = cJSON_GetObjectItem(m2m_ae, "srv");
	cJSON *et = cJSON_GetObjectItem(m2m_ae, "et");

	if(rr) {
		if(cJSON_IsTrue(rr)) {
			after->rr = true;
		} else {
			after->rr = false;
		}
	}

	if(et) {
		if(after->et) free(after->et);
		after->et = (char *)malloc((strlen(et->valuestring) + 1) * sizeof(char));
		strcpy(after->et, et->valuestring);
	}

	if(lbl) {
		if(after->lbl) free(after->lbl);
		after->lbl = cjson_list_item_to_string(lbl);
	}

	if(srv) {
		if(after->srv) free(after->srv);
		after->srv = cjson_list_item_to_string(srv);
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);
	return 1;
}

int set_cnt_update(cJSON *m2m_cnt, CNT* after) {
	cJSON *lbl = cJSON_GetObjectItem(m2m_cnt, "lbl");
	cJSON *acpi = cJSON_GetObjectItem(m2m_cnt, "acpi");
	cJSON *mni = cJSON_GetObjectItem(m2m_cnt, "mni");
	cJSON *mbs = cJSON_GetObjectItem(m2m_cnt, "mbs");
	cJSON *et = cJSON_GetObjectItem(m2m_cnt, "et");

	if(et) {
		if(after->et) free(after->et);
		after->et = (char *)malloc((strlen(et->valuestring) + 1) * sizeof(char));
		strcpy(after->et, et->valuestring);
	}

	if(acpi) {
		if(after->acpi) free(after->acpi);
		after->acpi = cjson_list_item_to_string(acpi);
	}
	
	if(lbl) {
		if(after->lbl) free(after->lbl);
		after->lbl = cjson_list_item_to_string(lbl);
	}

	if(mni) {
		after->mni = mni->valueint;
	}

	if(mbs) {
		after->mbs = mbs->valueint;
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);

	return 1;
}

int set_acp_update(cJSON *m2m_acp, ACP* after) {
	cJSON *pv_acr = cJSON_GetObjectItem(m2m_acp, "pv");
	cJSON *pvs_acr = cJSON_GetObjectItem(m2m_acp, "pvs");
	cJSON *et = cJSON_GetObjectItem(m2m_acp, "et");
	cJSON *pv_acor = NULL;
	cJSON *pv_acop = NULL;
	cJSON *pvs_acor = NULL;
	cJSON *pvs_acop = NULL;
	char pv_acor_str[256] = {'\0'};
	char pv_acop_str[256] = {'\0'};
	char pvs_acor_str[256] =  {'\0'};
	char pvs_acop_str[256] =  {'\0'};

	if(et) {
		if(after->et) free(after->et);
		after->et = (char *)malloc((strlen(et->valuestring) + 1) * sizeof(char));
		strcpy(after->et, et->valuestring);
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
				}
			}
		}
		if(pv_acor_str[0] != '\0' && pv_acop_str[0] != '\0') {
			if(after->pv_acor) free(after->pv_acor);
			if(after->pv_acop) free(after->pv_acop);
			after->pv_acor = strdup(pv_acor_str);
			after->pv_acop = strdup(pv_acop_str);
		} else {
			after->pv_acor = after->pv_acop = NULL;
		}
	}

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
				}
			}
		}
		if(pvs_acor_str[0] != '\0' && pvs_acop_str[0] != '\0') {
			if(after->pvs_acor) free(after->pvs_acor);
			if(after->pvs_acop) free(after->pvs_acop);
			after->pvs_acor = strdup(pvs_acor_str);
			after->pvs_acop = strdup(pvs_acop_str);
		} else {
			after->pvs_acor = after->pvs_acop = NULL;
		}	
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);

	return 1;
}

int update_cnt_cin(RTNode *cnt_rtnode, RTNode *cin_rtnode, int sign) {
	CNT *cnt = (CNT *)cnt_rtnode->obj;
	CIN *cin = (CIN *)cin_rtnode->obj;
	cnt->cni += sign;
	cnt->cbs += sign*(cin->cs);
	delete_cin_under_cnt_mni_mbs(cnt);	
	cnt->st++;
	db_delete_onem2m_resource(cnt->ri);
	db_store_cnt(cnt);
	return 1;
}

int delete_onem2m_resource(oneM2MPrimitive *o2pt, RTNode* target_rtnode) {
	logger("O2M", LOG_LEVEL_INFO, "Delete oneM2M resource");
	if(target_rtnode->ty == RT_AE || target_rtnode->ty == RT_CNT || target_rtnode->ty == RT_GRP) {
		if(check_privilege(o2pt, target_rtnode, ACOP_DELETE) == -1) {
			return o2pt->rsc;
		}
	}
	if(target_rtnode->ty == RT_CSE) {
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"CSE can not be deleted\"}");
		o2pt->rsc = RSC_OPERATION_NOT_ALLOWED;
		return RSC_OPERATION_NOT_ALLOWED;
	}
	delete_rtnode_and_db_data(target_rtnode,1);
	target_rtnode = NULL;
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = NULL;
	//set_o2pt_pc(o2pt,"{\"m2m:dbg\": \"resource is deleted successfully\"}");
	o2pt->rsc = RSC_DELETED;
	return RSC_DELETED;
}

int delete_rtnode_and_db_data(RTNode *rtnode, int flag) {
	switch(rtnode->ty) {
	case RT_AE : 
		db_delete_onem2m_resource(((AE *)rtnode->obj)->ri); 
		break;
	case RT_CNT : 
		db_delete_onem2m_resource(((CNT *)rtnode->obj)->ri); 
		break;
	case RT_CIN :
		db_delete_onem2m_resource(((CIN *)rtnode->obj)->ri);
		update_cnt_cin(rtnode->parent, rtnode,-1);
		return 1;
	case RT_SUB :
		db_delete_sub(((SUB *)rtnode->obj)->ri);
		break;
	case RT_ACP :
		db_delete_acp(((ACP *)rtnode->obj)->ri);
		break;
	case RT_GRP:
		db_delete_grp(((GRP *)rtnode->obj)->ri);
		break;
	}

	RTNode *left = rtnode->sibling_left;
	RTNode *right = rtnode->sibling_right;
	
	if(rtnode->ty != RT_CIN) {
		if(flag == 1) {
			if(left) left->sibling_right = right;
			else rtnode->parent->child = right;
			if(right) right->sibling_left = left;
		} else {
			if(right) delete_rtnode_and_db_data(right, 0);
		}
	}
	
	if(rtnode->child) delete_rtnode_and_db_data(rtnode->child, 0);
	
	free_rtnode(rtnode); rtnode = NULL;
	return 1;
}

void free_cse(CSE *cse) {
	if(cse->ct) free(cse->ct);
	if(cse->lt) free(cse->lt);
	if(cse->rn) free(cse->rn);
	if(cse->ri) free(cse->ri);
	if(cse->csi) free(cse->csi);
	if(cse->pi) free(cse->pi);
	free(cse); cse = NULL;
}

void free_ae(AE *ae) {
	if(ae->et) free(ae->et);
	if(ae->ct) free(ae->ct);
	if(ae->lt) free(ae->lt);
	if(ae->rn) free(ae->rn);
	if(ae->ri) free(ae->ri);
	if(ae->pi) free(ae->pi);
	if(ae->api) free(ae->api);
	if(ae->aei) free(ae->aei);
	if(ae->lbl) free(ae->lbl);
	if(ae->srv) free(ae->srv);
	if(ae->acpi) free(ae->acpi);
	free(ae); ae = NULL;
}

void free_cnt(CNT *cnt) {
	if(cnt->et) free(cnt->et);
	if(cnt->ct) free(cnt->ct);
	if(cnt->lt) free(cnt->lt);
	if(cnt->rn) free(cnt->rn);
	if(cnt->ri) free(cnt->ri);
	if(cnt->pi) free(cnt->pi);
	if(cnt->acpi) free(cnt->acpi);
	if(cnt->lbl) free(cnt->lbl);
	free(cnt); cnt = NULL;
}

void free_cin(CIN* cin) {
	if(cin->et) free(cin->et);
	if(cin->ct) free(cin->ct);
	if(cin->lt) free(cin->lt);
	if(cin->rn) free(cin->rn);
	if(cin->ri) free(cin->ri);
	if(cin->pi) free(cin->pi);
	if(cin->con) free(cin->con);
	free(cin); cin = NULL;
}

void free_sub(SUB* sub) {
	if(sub->et) free(sub->et);
	if(sub->ct) free(sub->ct);
	if(sub->lt) free(sub->lt);
	if(sub->rn) free(sub->rn);
	if(sub->ri) free(sub->ri);
	if(sub->pi) free(sub->pi);
	if(sub->nu) free(sub->nu);
	if(sub->net) free(sub->net);
	free(sub); sub = NULL;
}

void free_acp(ACP* acp) {
	if(acp->et) free(acp->et);
	if(acp->ct) free(acp->ct);
	if(acp->lt) free(acp->lt);
	if(acp->rn) free(acp->rn);
	if(acp->ri) free(acp->ri);
	if(acp->pi) free(acp->pi);
	if(acp->pv_acor) free(acp->pv_acor);
	if(acp->pv_acop) free(acp->pv_acop);
	if(acp->pvs_acor) free(acp->pvs_acor);
	if(acp->pvs_acop) free(acp->pvs_acop);
	free(acp); acp = NULL;
}

void free_grp(GRP *grp) {
	if(!grp) return;
	if(grp->rn) free(grp->rn);
	if(grp->ri) free(grp->ri);
	if(grp->ct) free(grp->ct);
	if(grp->et) free(grp->et);
	if(grp->lt) free(grp->lt);
	if(grp->pi) free(grp->pi);
	if(grp->acpi) free(grp->acpi);

	
	if(grp->mid){
		for(int i = 0 ; i < grp->cnm ; i++){
			if(grp->mid[i]){
				free(grp->mid[i]);
				grp->mid[i] = NULL;
			}
		}
		free(grp->mid); 
		grp->mid = NULL;
	}
	free(grp); 
	grp = NULL;
}

void free_rtnode(RTNode *rtnode) {
	if(rtnode->uri && rtnode->ty != RT_CSE)
		free(rtnode->uri);

	switch(rtnode->ty) {
		case RT_CSE:
			free_cse((CSE *)rtnode->obj);
			break;
		case RT_AE:
			free_ae((AE *)rtnode->obj);
			break;
		case RT_CNT:
			free_cnt((CNT *)rtnode->obj);
			break;
		case RT_CIN:
			free_cin((CIN *)rtnode->obj);
			break;
		case RT_SUB:
			free_sub((SUB *)rtnode->obj);
			break;
		case RT_ACP:
			free_acp((ACP *)rtnode->obj);
			break;
		case RT_GRP:
			free_grp((GRP *)rtnode->obj);
			break;
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

/*
void set_sub_update(SUB* after) {
	char *rn = get_json_value_char("rn", payload);
	char *nu = NULL;
	char *net = NULL;

	if(strstr(payload,"\"nu\"") != NULL) {
		nu = get_json_value_list("nu", payload);
		if(!strcmp(nu, "\0")) {
			free(nu); nu = after->nu = NULL;
		}
	}
	if(strstr(payload,"\"enc\"") != NULL) {
		if(strstr(payload, "\"net\"") != NULL) {
			net = get_json_value_list("enc-net", payload);
			if(!strcmp(net, "\0")) {
				free(net); net = after->net = NULL;
			}
		}
	}

	if(rn) {
		free(after->rn);
		after->rn = (char*)malloc((strlen(rn) + 1) * sizeof(char));
		strcpy(after->rn, rn);
	}

	if(nu) {
		if(after->nu) free(after->nu);
		after->nu = (char*)malloc((strlen(nu) + 1) * sizeof(char));
		strcpy(after->nu, nu);
	}

	if(net) {
		if(after->net) free(after->net);
		after->net = (char*)malloc((strlen(net) + 1) * sizeof(char));
		strcpy(after->net, net);
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);
}

void notify_onem2m_resource(RTNode *node, char *response_payload, NET net) {
	remove_invalid_char_json(response_payload);
	while(node) {
		if(node->ty == RT_SUB && (net & node->net) == net) {
			if(!node->uri) set_node_uri(node);
			char *notify_json = notification_to_json(node->uri, (int)log2((double)net ) + 1, response_payload);
			int result = send_http_packet(node->nu, notify_json);
			free(notify_json); notify_json = NULL;
		}
		node = node->sibling_right;
	}
}

void remove_invalid_char_json(char* json) { 
	int size = (int)malloc_usable_size(json); // segmentation fault if json memory not in heap (malloc)
	int index = 0;

	for(int i=0; i<size; i++) {
		if(is_json_valid_char(json[i]) && json[i] != '\\') {
			json[index++] = json[i];
		}
	}

	json[index] = '\0';
}

size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data) {
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;

    data->size += (size * nmemb);
    tmp = realloc(data->data, data->size + 1); // +1 for '\0' 

    if(tmp) {
        data->data = tmp;
    } else {
        if(data->data) {
            free(data->data);
        }
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}

int send_http_packet(char* target, char *post_data) {
    CURL *curl;
    struct url_data data;

    data.size = 0;
    data.data = malloc(4096 * sizeof(char)); // reasonable size initial buffer

    if(NULL == data.data) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return EXIT_FAILURE;
    }
	if(post_data) remove_invalid_char_json(post_data);

	char nu[MAX_PROPERRT_SIZE];
	strcpy(nu, target);

	target = strtok(nu, ",");

    CURLcode res;

    curl = curl_easy_init();

    if (curl) {
		if(post_data) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
		
		while(target) {
			data.data[0] = '\0';
			curl_easy_setopt(curl, CURLOPT_URL, target);
			res = curl_easy_perform(curl);
			
			if(res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			}
			target = strtok(NULL, ",");
		}
		
        curl_easy_cleanup(curl);
    }

	if(data.data) free(data.data);

    return EXIT_SUCCESS;
}

int get_value_querystring_int(char *key) {
	char *value = strstr(qs, key);
	if(!value) return -1;

	value = value + strlen(key) + 1;

	return atoi(value);
}

void set_node_uri(RTNode* node) {
	if(!node->uri) node->uri = (char*)calloc(MAX_URI_SIZE,sizeof(char));

	RTNode *p = node;
	char uri_copy[32][MAX_URI_SIZE];
	int index = -1;

	while(p) {
		strcpy(uri_copy[++index],p->rn);
		p = p->parent;
	}

	for(int i=index; i>=0; i--) {
		strcat(node->uri,"/");
		strcat(node->uri,uri_copy[i]);
	}

	return;
}

int check_origin() {
	if(request_header("X-M2M-Origin")) {
		return 1;
	} else {
		HTTP_403;
		printf("{\"m2m:dbg\": \"DB store fail\"}");
		return 0;
	}
}
*/
/* GROUP IMPLEMENTATION */

void init_grp(GRP *grp, char *pi){
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(RT_GRP, ct);

	grp->ct = (char *) malloc((strlen(ct) + 1) * sizeof(char));
	grp->et = (char *) malloc((strlen(et) + 1) * sizeof(char));
	grp->ri = (char *) malloc((strlen(ri) + 1) * sizeof(char));
	grp->lt = (char *) malloc((strlen(ri) + 1) * sizeof(char));
	grp->pi = (char *) malloc((strlen(pi) + 1) * sizeof(char));

	strcpy(grp->ct, ct);
	strcpy(grp->et, et);
	strcpy(grp->lt, ct);
	strcpy(grp->ri, ri);
	strcpy(grp->pi, pi);

	if(grp->csy == 0) grp->csy = CSY_ABANDON_MEMBER;


	if(!grp->rn) grp->rn = strdup(ri);

	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

int set_grp_update(cJSON *m2m_grp, GRP* after){
	if(!after) return RSC_BAD_REQUEST;
	cJSON *acpi = cJSON_GetObjectItem(m2m_grp, "acpi");
	cJSON *et = cJSON_GetObjectItem(m2m_grp, "et");
	cJSON *mt = cJSON_GetObjectItem(m2m_grp, "mt");
	cJSON *mnm = cJSON_GetObjectItem(m2m_grp, "mnm");
	cJSON *mid = cJSON_GetObjectItem(m2m_grp, "mid");
	cJSON *mc = NULL;

	after->mtv = false;

	if(acpi){
		if(after->acpi) 
			free(after->acpi);
		after->acpi = cjson_list_item_to_string(acpi);
	}

	if(et) {
		if(after->et)
			free(after->et);
		after->et = strdup(et->valuestring);
	}

	if(mt)
		after->mt = mt->valueint;
	
	if(mnm){
		if(mnm->valueint < after->cnm){
			return RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED;
		}
		after->mnm = mnm->valueint;
		
	}

	int new_cnm = 0;

	if(mid){
		size_t mid_size = cJSON_GetArraySize(mid);
		if(mid_size > after->mnm) return RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED;
		new_cnm = mid_size;
		int midx = 0;
		for(int i = 0 ; i < after->mnm ; i++){ //re initialize mid
			if(after->mid[i])
				free(after->mid[i]);

			after->mid[i] = NULL;
		}
		for(int i = 0 ; i < after->mnm; i++){
			
			if(i < mid_size){
				mc = cJSON_GetArrayItem(mid, i);
				if(!isMinDup(after->mid, midx, mc->valuestring)){
					
					logger("o2m-t", LOG_LEVEL_DEBUG, "updating %s to mid[%d]", mc->valuestring, midx);
					after->mid[midx] = strdup(mc->valuestring);
					midx++;
				}
				else{
					logger("json-t", LOG_LEVEL_DEBUG, "declining %s", mc->valuestring);
					after->mid[i] = NULL;
					new_cnm--;
				}
				
			}else{
				after->mid[i] = NULL;
			}
		}
		after->cnm = new_cnm;
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);
	return RSC_OK;
}

int update_grp(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	int rsc = 0;
	char invalid_key[][4] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_grp = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:grp");
	int invalid_key_size = sizeof(invalid_key)/(4*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_grp, invalid_key[i])) {
			handle_error(o2pt, RSC_BAD_REQUEST, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			return RSC_BAD_REQUEST;
		}
	}

	GRP *after = db_get_grp(get_ri_rtnode(target_rtnode));
	int result;
	if( (o2pt->rsc = set_grp_update(m2m_grp, after)) >= 4000){
		free_grp(after);
		after = NULL;
		return o2pt->rsc;
	}
	after->mtv = false;
	if( (rsc = validate_grp(after))  >= 4000){
		o2pt->rsc = rsc;
		return rsc;
	}

	result = db_delete_grp(after->ri);
	result = db_store_grp(after);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = grp_to_json(after);
	o2pt->rsc = RSC_UPDATED;
	if(target_rtnode->obj)
		free_grp((GRP *) target_rtnode->obj);

	target_rtnode->obj = after;

	//free_grp(after); after = NULL;
	return RSC_UPDATED;
}

int create_grp(oneM2MPrimitive *o2pt, RTNode *parent_rtnode){
	int e = 1;
	int rsc = 0;
	if( parent_rtnode->ty != RT_AE && parent_rtnode->ty != RT_CSE ) {
		return o2pt->rsc = RSC_INVALID_CHILD_RESOURCETYPE;
	}

	GRP *grp = (GRP *)calloc(1, sizeof(GRP));
	o2pt->rsc = rsc = cjson_to_grp(o2pt->cjson_pc, grp); 
	if(rsc >= 4000){
		free_grp(grp);
		grp = NULL;
		o2pt->rsc = rsc;
		return rsc;
	}
	init_grp(grp, get_ri_rtnode(parent_rtnode));
	rsc = validate_grp(grp);
	if(rsc >= 4000){
		logger("O2M", LOG_LEVEL_DEBUG, "Group Validation failed");
		
		free_grp(grp);
		grp = NULL;
		return o2pt->rsc = rsc;
	}



	int result = db_store_grp(grp);
	if(result != 1){
		db_store_fail(o2pt); 
		free_grp(grp); 
		grp = NULL;
		return RSC_INTERNAL_SERVER_ERROR;
	}

	RTNode *child_rtnode = create_rtnode(grp, RT_GRP);
	add_child_resource_tree(parent_rtnode, child_rtnode);

	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = grp_to_json(grp);

	//free_grp(grp); grp = NULL;
	return rsc;
}

bool isResourceAptFC(RTNode *rtnode, FilterCriteria *fc){
    void *obj;
    int flag = 0;
	RTNode *prtnode = NULL;
	FilterOperation fo = fc->fo;
    if(!rtnode || !fc) return false;

	// check Created Time
	if(fc->cra && fc->crb){
		if(strcmp(fc->cra, fc->crb) >= 0 && fo == FO_AND) return false;
	}
    if(fc->cra){
		if(!FC_isAptCra(fc->cra, rtnode)) {
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
    }
	if(fc->crb){
		if(!FC_isAptCrb(fc->crb, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	// check Last Modified
	if(fc->ms && fc->us){
		if(strcmp(fc->ms, fc->us) >= 0 && fo == FO_AND) return false;
	}
	if(fc->ms){
		if(!FC_isAptMs(fc->ms, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}
	if(fc->us){
		if(!FC_isAptUs(fc->us, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	// check state tag
	if(fc->stb && fc->sts){
		if(fc->stb >= fc->sts && fo == FO_AND) 
			return false;
	}
	if(fc->stb){
		if(!FC_isAptStb(fc->stb, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}
	if(fc->sts){
		if(!FC_isAptSts(fc->sts, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	// check Expiration Time
	if(fc->exa){
		if(!FC_isAptExa(fc->exa, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}
	
	if(fc->exb){
		if(!FC_isAptExb(fc->exb, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	// check label
	if(fc->lbl){
		if(!FC_isAptLbl(fc->lbl, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	if(fc->clbl){
		if(!FC_isAptClbl(fc->clbl, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	if(fc->palb){
		if(!FC_isAptPalb(fc->palb, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	// check TY
    if(fc->tycnt > 0){
        if(!FC_isAptTy(fc->ty, fc->tycnt, rtnode->ty)){
            return false;
		}else{
			if(fo == FO_OR){
				return true;
			}
		}
    }
	// check chty
	if(fc->chtycnt > 0){
		int flag = 0;
		prtnode = rtnode->child;
		if(!prtnode){
			if(fo == FO_AND)
				return false;
		}else{
			while(prtnode){
				if(FC_isAptChty(fc->chty, fc->chtycnt, prtnode->ty)){
					flag = 1;
					break;
				}
				prtnode = prtnode->sibling_right;
			}
			if(flag){
				if(fo == FO_OR)
					return true;
			}else{
				if(fo == FO_AND)
					return false;
			}
		}
		
	}
	// check pty
	if(fc->ptycnt > 0){
		if(!rtnode->parent){
			if(fo == FO_AND)
				return false;
		}
		else if(!FC_isAptChty(fc->pty, fc->ptycnt, rtnode->parent->ty)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	//check cs
	if(fc->sza && fc->szb){
		if(fc->sza >= fc->szb && fo == FO_AND){
			return false;
		}
	}
	if(fc->sza){
		if(!FC_isAptSza(fc->sza, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}
	if(fc->szb){
		if(!FC_isAptSzb(fc->szb, rtnode)){
			if(fo == FO_AND)
				return false;
		}else{
			if(fo == FO_OR)
				return true;
		}
	}

	// TODO - Check Attr

    return true;
}
/*
void update_sub(RTNode *pnode) {
	SUB* after = db_get_sub(pnode->ri);
	int result;
	
	set_sub_update(after);
	set_rtnode_update(pnode, after);
	result = db_delete_sub(after->ri);
	result = db_store_sub(after);
	
	response_payload = sub_to_json(after);
	respond_to_client(200, NULL, "2004");
	free(response_payload); response_payload = NULL;
	free_sub(after); after = NULL;
}
*/

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

	case RT_MIXED :
		logger("O2M", LOG_LEVEL_ERROR, "Resource type is invalid");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"resource type error\"}");
		rsc = o2pt->rsc = RSC_BAD_REQUEST;
	}	
	return rsc;
}

int retrieve_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	int rsc = 0;
	int e = check_privilege(o2pt, target_rtnode, ACOP_RETRIEVE);

	if(e == -1) return o2pt->rsc;

	switch(target_rtnode->ty) {
		
	case RT_CSE :
		logger("O2M", LOG_LEVEL_INFO, "Retrieve CSE");
        rsc = retrieve_cse(o2pt, target_rtnode);
      	break;
	
	case RT_AE : 
		logger("O2M", LOG_LEVEL_INFO, "Retrieve AE");
		rsc = retrieve_ae(o2pt, target_rtnode);	
		break;	
			
	case RT_CNT :
		logger("O2M", LOG_LEVEL_INFO, "Retrieve CNT");
		rsc = retrieve_cnt(o2pt, target_rtnode);			
		break;
			
	case RT_CIN :
		logger("O2M", LOG_LEVEL_INFO, "Retrieve CIN");
		rsc = retrieve_cin(o2pt, target_rtnode);			
		break;

	case RT_GRP :
		logger("O2M", LOG_LEVEL_INFO, "Retrieve GRP");
		rsc = retrieve_grp(o2pt, target_rtnode);	
		break;

	case RT_SUB :
		logger("O2M", LOG_LEVEL_INFO, "Retrieve SUB");
		rsc = retrieve_sub(o2pt, target_rtnode);			
		break;

	case RT_ACP :
		logger("O2M", LOG_LEVEL_INFO, "Retrieve ACP");
		rsc = retrieve_acp(o2pt, target_rtnode);			
		break;
	}	

	return rsc;
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

		// case RT_SUB :
		//	logger("O2M", LOG_LEVEL_INFO, "Update SUB");
		// 	rsc = update_sub(pnode);
		// 	break;
		
		case RT_ACP :
			logger("O2M", LOG_LEVEL_INFO, "Update ACP");
			rsc = update_acp(o2pt, target_rtnode);
			break;

		case RT_GRP:
			logger("O2M", LOG_LEVEL_INFO, "Update GRP");
			rsc = update_grp(o2pt, target_rtnode);
			break;

		default :
			logger("O2M", LOG_LEVEL_ERROR, "Resource type does not support Update");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"operation `update` is unsupported\"}");
			rsc = o2pt->rsc = RSC_OPERATION_NOT_ALLOWED;
		}
	return rsc;
}

int fopt_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *parent_rtnode){
	int rsc = 0;
	int cnt = 0;

	RTNode *target_rtnode = NULL;
	oneM2MPrimitive *req_o2pt = NULL;
	cJSON *new_pc = NULL;
	cJSON *agr = NULL;
	cJSON *rsp = NULL;
	cJSON *json = NULL;
	GRP *grp = NULL;
	
	if(parent_rtnode == NULL){
		o2pt->rsc = RSC_NOT_FOUND;
		return RSC_NOT_FOUND;
	}
	logger("O2M", LOG_LEVEL_DEBUG, "handle fopt");


	grp = db_get_grp(get_ri_rtnode(parent_rtnode));
	if(!grp){
		o2pt->rsc = RSC_INTERNAL_SERVER_ERROR;
		return RSC_INTERNAL_SERVER_ERROR;
	}
	
	if(grp->cnm == 0){
		logger("O2M", LOG_LEVEL_DEBUG, "No member to fanout");
		free_grp(grp);
		return o2pt->rsc = RSC_NO_MEMBERS;
	}

	o2ptcpy(&req_o2pt, o2pt);

	new_pc = cJSON_CreateObject();
	cJSON_AddItemToObject(new_pc, "m2m:agr", agr = cJSON_CreateObject());
	cJSON_AddItemToObject(agr, "m2m:rsp", rsp = cJSON_CreateArray());

	for(int i = 0 ; i < grp->cnm ; i++){
		if(req_o2pt->to) free(req_o2pt->to);
		if(o2pt->fopt)
			req_o2pt->to = malloc(strlen(grp->mid[i]) + strlen(o2pt->fopt) + 1);
		else
			req_o2pt->to = malloc(strlen(grp->mid[i]) + 1);
		
		strcpy(req_o2pt->to, grp->mid[i]);
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
	free_grp(grp);
	return RSC_OK;
}

/**
 * Discover Resources based on Filter Criteria
*/
int discover_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	logger("MAIN", LOG_LEVEL_DEBUG, "Discover Resource");
	RTNode *pn = target_rtnode->child;
	cJSON *root = cJSON_CreateObject();
	cJSON *uril = NULL;
	int urilSize = 0;

	// TODO - IMPLEMENT OFFSET
	// for(int i = 0 ; i < o2pt->fc->ofst ; i++){ 
	// 	if(pn->child){
	// 		pn = pn->child;
	// 	}else{
	// 		break;
	// 	}
	// }

	uril = fc_scan_resource_tree(pn, o2pt->fc, 1);
	
	
	urilSize = cJSON_GetArraySize(uril);	//Todo : contentStatus(cnst) set to Partial_content, cnot too 
	if(o2pt->fc->lim < urilSize){
		logger("MAIN", LOG_LEVEL_DEBUG, "limit exceeded");
		for(int i = o2pt->fc->lim ; i < urilSize; i++){
			cJSON_DeleteItemFromArray(uril, o2pt->fc->lim);
		}
	}
	cJSON_AddItemToObject(root, "m2m:uril", uril);
	

	if(o2pt->pc)
		free(o2pt->pc);
	o2pt->pc = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return o2pt->rsc = RSC_OK;

}