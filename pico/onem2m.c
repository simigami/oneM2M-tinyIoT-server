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
#include "berkeleyDB.h"
#include "jsonparse.h"
#include "httpd.h"
#include "mqttClient.h"
#include "onem2mTypes.h"
#include "util.h"
#include "config.h"

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

RTNode* create_rtnode(void *resource, ResourceType ty){
	RTNode* rtnode = NULL;

	switch(ty) {
	case RT_CSE: rtnode = create_cse_rtnode((CSE*)resource); break;
	case RT_AE: rtnode = create_ae_rtnode((AE*)resource); break;
	case RT_CNT: rtnode = create_cnt_rtnode((CNT*)resource); break;
	case RT_CIN: rtnode = create_cin_rtnode((CIN*)resource); break;
	case RT_GRP: rtnode = create_grp_rtnode( (GRP *) resource); break;
	case RT_SUB: rtnode = create_sub_rtnode((Sub*)resource); break;
	case RT_ACP: rtnode = create_acp_rtnode((ACP*)resource); break;
	}

	rtnode->parent = NULL;
	rtnode->child = NULL;
	rtnode->sibling_left = NULL;
	rtnode->sibling_right = NULL;
	
	return rtnode;
}

RTNode* create_cse_rtnode(CSE *cse) {
	RTNode* rtnode = calloc(1, sizeof(RTNode));

	rtnode->rn = (char*)malloc((strlen(cse->rn) + 1) * sizeof(char));
	rtnode->ri = (char*)malloc((strlen(cse->ri) + 1) * sizeof(char));
	rtnode->pi = (char*)malloc((strlen(cse->pi) + 1) * sizeof(char));
	
	strcpy(rtnode->rn, cse->rn);
	strcpy(rtnode->ri, cse->ri);
	strcpy(rtnode->pi, cse->pi);

	rtnode->ty = RT_CSE;

	return rtnode;
}

RTNode* create_ae_rtnode(AE *ae) {
	RTNode* rtnode = calloc(1, sizeof(RTNode));

	rtnode->rn = (char*)malloc((strlen(ae->rn) + 1) * sizeof(char));
	rtnode->ri = (char*)malloc((strlen(ae->ri) + 1) * sizeof(char));
	rtnode->pi = (char*)malloc((strlen(ae->pi) + 1) * sizeof(char));
	
	strcpy(rtnode->rn, ae->rn);
	strcpy(rtnode->ri, ae->ri);
	strcpy(rtnode->pi, ae->pi);

	rtnode->ty = RT_AE;

	return rtnode;
}

RTNode* create_cnt_rtnode(CNT *cnt) {
	RTNode* rtnode = calloc(1, sizeof(RTNode));

	rtnode->rn = (char*)malloc((strlen(cnt->rn) + 1) * sizeof(char));
	rtnode->ri = (char*)malloc((strlen(cnt->ri) + 1) * sizeof(char));
	rtnode->pi = (char*)malloc((strlen(cnt->pi) + 1) * sizeof(char));
	
	strcpy(rtnode->rn, cnt->rn);
	strcpy(rtnode->ri, cnt->ri);
	strcpy(rtnode->pi, cnt->pi);

	if(cnt->acpi) {
		rtnode->acpi = (char*)malloc((strlen(cnt->acpi) + 1) * sizeof(char));
		strcpy(rtnode->acpi, cnt->acpi);
	}

	rtnode->ty = RT_CNT;
	rtnode->cni = cnt->cni;
	rtnode->cbs = cnt->cbs;
	rtnode->mni = cnt->mni;
	rtnode->mbs = cnt->mbs;

	return rtnode;
}

RTNode* create_cin_rtnode(CIN *cin) {
	RTNode* rtnode = calloc(1, sizeof(RTNode));

	rtnode->rn = (char*)malloc((strlen(cin->rn) + 1) * sizeof(char));
	rtnode->ri = (char*)malloc((strlen(cin->ri) + 1) * sizeof(char));
	rtnode->pi = (char*)malloc((strlen(cin->pi) + 1) * sizeof(char));
	
	strcpy(rtnode->rn, cin->rn);
	strcpy(rtnode->ri, cin->ri);
	strcpy(rtnode->pi, cin->pi);

	rtnode->ty = RT_CIN;
	rtnode->cs = cin->cs;

	return rtnode;
}

RTNode* create_sub_rtnode(Sub *sub) {
	RTNode* rtnode = calloc(1, sizeof(RTNode));

	rtnode->rn = (char*)malloc((strlen(sub->rn) + 1) * sizeof(char));
	rtnode->ri = (char*)malloc((strlen(sub->ri) + 1) * sizeof(char));
	rtnode->pi = (char*)malloc((strlen(sub->pi) + 1) * sizeof(char));
	rtnode->nu = (char*)malloc((strlen(sub->nu) + 1) * sizeof(char));
	rtnode->sur = (char*)malloc((strlen(sub->sur) + 1) * sizeof(char));
	
	strcpy(rtnode->rn, sub->rn);
	strcpy(rtnode->ri, sub->ri);
	strcpy(rtnode->pi, sub->pi);
	strcpy(rtnode->nu, sub->nu);
	strcpy(rtnode->sur, sub->sur);

	rtnode->ty = RT_SUB;
	rtnode->net = net_to_bit(sub->net);

	return rtnode;
}

RTNode* create_acp_rtnode(ACP *acp) {
	RTNode* rtnode = calloc(1, sizeof(RTNode));

	rtnode->rn = (char*)malloc((strlen(acp->rn) + 1) * sizeof(char));
	rtnode->ri = (char*)malloc((strlen(acp->ri) + 1) * sizeof(char));
	rtnode->pi = (char*)malloc((strlen(acp->pi) + 1) * sizeof(char));
	rtnode->pv_acor = (char*)malloc((strlen(acp->pv_acor) + 1) * sizeof(char));
	rtnode->pv_acop = (char*)malloc((strlen(acp->pv_acop) + 1) * sizeof(char));
	rtnode->pvs_acor = (char*)malloc((strlen(acp->pvs_acor) + 1) * sizeof(char));
	rtnode->pvs_acop = (char*)malloc((strlen(acp->pvs_acop) + 1) * sizeof(char));

	strcpy(rtnode->rn, acp->rn);
	strcpy(rtnode->ri, acp->ri);
	strcpy(rtnode->pi, acp->pi);
	strcpy(rtnode->pv_acor, acp->pv_acor);
	strcpy(rtnode->pv_acop, acp->pv_acop);
	strcpy(rtnode->pvs_acor, acp->pvs_acor);
	strcpy(rtnode->pvs_acop, acp->pvs_acop);

	rtnode->ty = RT_ACP;

	return rtnode;
}


RTNode *create_grp_rtnode(GRP *grp){
	RTNode *rtnode = calloc(1, sizeof(RTNode));

	rtnode->rn = strdup(grp->rn);
	rtnode->ri = strdup(grp->ri);
	rtnode->pi = strdup(grp->pi);

	if(grp->acpi){
		rtnode->acpi = strdup(grp->acpi);
	}
	rtnode->ty = RT_GRP;

	return rtnode;
	
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
	free(ae); ae = NULL;
}

void free_cnt(CNT *cnt) {
	if(cnt->et) free(cnt->et);
	if(cnt->ct) free(cnt->ct);
	if(cnt->lt) free(cnt->lt);
	if(cnt->rn) free(cnt->rn);
	if(cnt->ri) free(cnt->ri);
	if(cnt->pi) free(cnt->pi);
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

void free_sub(Sub* sub) {
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
	if(grp->rn) free(grp->rn);
	if(grp->ri) free(grp->ri);
	if(grp->ct) free(grp->ct);
	if(grp->et) free(grp->et);
	if(grp->lt) free(grp->lt);
	if(grp->pi) free(grp->pi);
	if(grp->acpi) free(grp->acpi);

	
	if(grp->mid){
		for(int i = 0 ; i < grp->cnm ; i++){
			if(grp->mid[i])
				free(grp->mid[i]);
			grp->mid[i] = NULL;
		}
		free(grp->mid); grp->mid = NULL;
	}
	free(grp); grp = NULL;
}

void free_rtnode(RTNode *rtnode) {
	free(rtnode->ri);
	free(rtnode->rn);
	free(rtnode->pi);
	if(rtnode->nu) free(rtnode->nu);
	if(rtnode->sur) free(rtnode->sur);
	if(rtnode->acpi) free(rtnode->acpi);
	if(rtnode->pv_acop) free(rtnode->pv_acop);
	if(rtnode->pv_acor) free(rtnode->pv_acor);
	if(rtnode->pvs_acor) free(rtnode->pvs_acor);
	if(rtnode->pvs_acop) free(rtnode->pvs_acop);
	if(rtnode->uri) free(rtnode->uri);
	free(rtnode); rtnode = NULL;
}

void free_rtnode_list(RTNode *rtnode) {
	while(rtnode) {
		RTNode *right = rtnode->sibling_right;
		free_rtnode(rtnode);
		rtnode = right;
	}
}


void create_ae(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	int e = check_aei_duplicate(o2pt, parent_rtnode);
	if(e != -1) e = check_rn_invalid(o2pt, RT_AE);
	if(e == -1) return;

	if(parent_rtnode->ty != RT_CSE) {
		child_type_error(o2pt);
		return;
	}
	AE* ae = cjson_to_ae(o2pt->cjson_pc);
	if(!ae) {
		no_mandatory_error(o2pt);
		return;
	}
	if(ae->api[0] != 'R' && ae->api[0] != 'N') {
		free_ae(ae);
		api_prefix_invalid(o2pt);
		return;
	}
	init_ae(ae,parent_rtnode->ri, o2pt->fr);
	
	int result = db_store_ae(ae);
	if(result != 1) { 
		db_store_fail(o2pt); free_ae(ae); ae = NULL;
		return;
	}
	
	RTNode* child_rtnode = create_rtnode(ae, RT_AE);
	add_child_resource_tree(parent_rtnode, child_rtnode);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = ae_to_json(ae);
	o2pt->rsc = RSC_CREATED;
	respond_to_client(o2pt, 201);
	// notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free_ae(ae); ae = NULL;
}

void create_cnt(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty != RT_CNT && parent_rtnode->ty != RT_AE && parent_rtnode->ty != RT_CSE) {
		child_type_error(o2pt);
		return;
	}
	CNT* cnt = cjson_to_cnt(o2pt->cjson_pc);
	if(!cnt) {
		no_mandatory_error(o2pt);
		return;
	}
	if(cnt->mbs != INT_MIN && cnt->mbs < 0) {
		mni_mbs_invalid(o2pt, "mbs"); free(cnt); cnt = NULL;
		return;
	}
	if(cnt->mni != INT_MIN && cnt->mni < 0) {
		mni_mbs_invalid(o2pt, "mni"); free(cnt); cnt = NULL;
		return;
	}
	init_cnt(cnt,parent_rtnode->ri);

	int result = db_store_cnt(cnt);
	if(result != 1) { 
		db_store_fail(o2pt); free_cnt(cnt); cnt = NULL;
		return;
	}
	
	RTNode* child_rtnode = create_rtnode(cnt, RT_CNT);
	add_child_resource_tree(parent_rtnode,child_rtnode);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cnt_to_json(cnt);
	o2pt->rsc = RSC_CREATED;
	respond_to_client(o2pt, 201);
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free_cnt(cnt); cnt = NULL;
}


void create_cin(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty != RT_CNT) {
		child_type_error(o2pt);
		return;
	}
	CIN* cin = cjson_to_cin(o2pt->cjson_pc);
	if(!cin) {
		no_mandatory_error(o2pt);
		return;
	} else if(parent_rtnode->mbs >= 0 && cin->cs > parent_rtnode->mbs) {
		too_large_content_size_error(o2pt); free(cin); cin = NULL;
		return;
	}
	init_cin(cin,parent_rtnode->ri);

	int result = db_store_cin(cin);
	if(result != 1) { 
		db_store_fail(o2pt); free_cin(cin); cin = NULL;
		return;
	}

	RTNode *cin_rtnode = create_rtnode(cin, RT_CIN);
	update_cnt_cin(parent_rtnode, cin_rtnode, 1);
	free_rtnode(cin_rtnode);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cin_to_json(cin);
	o2pt->rsc = RSC_CREATED;
	respond_to_client(o2pt, 201);
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free_cin(cin); cin = NULL;
}

void create_sub(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty == RT_CIN || parent_rtnode->ty == RT_SUB) {
		child_type_error(o2pt);
		return;
	}
	Sub* sub = cjson_to_sub(o2pt->cjson_pc);
	if(!sub) {
		no_mandatory_error(o2pt);
		return;
	}
	init_sub(sub, parent_rtnode->ri, o2pt->to);
	
	int result = db_store_sub(sub);
	if(result != 1) { 
		db_store_fail(o2pt); free_sub(sub); sub = NULL;
		return;
	}
	
	free_sub(sub); sub = NULL;

	RTNode* child_rtnode = create_rtnode(sub, RT_SUB);
	add_child_resource_tree(parent_rtnode,child_rtnode);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = sub_to_json(sub);
	o2pt->rsc = RSC_CREATED;
	respond_to_client(o2pt, 201);
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
}

void create_acp(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	if(parent_rtnode->ty != RT_CSE && parent_rtnode->ty != RT_AE) {
		child_type_error(o2pt);
		return;
	}
	ACP* acp = cjson_to_acp(o2pt->cjson_pc);
	if(!acp) {
		no_mandatory_error(o2pt);
		return;
	}
	init_acp(acp, parent_rtnode->ri);
	
	int result = db_store_acp(acp);
	if(result != 1) { 
		db_store_fail(o2pt); free_acp(acp); acp = NULL;
		return;
	}
	
	RTNode* child_rtnode = create_rtnode(acp, RT_ACP);
	add_child_resource_tree(parent_rtnode, child_rtnode);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = acp_to_json(acp);
	o2pt->rsc = RSC_CREATED;
	respond_to_client(o2pt, 201);
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free_acp(acp); acp = NULL;
}

void retrieve_cse(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	CSE* gcse = db_get_cse(target_rtnode->ri);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cse_to_json(gcse);
	o2pt->rsc = RSC_OK;
	respond_to_client(o2pt, 200);
	free_cse(gcse); gcse = NULL;
}


void retrieve_ae(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	AE* gae = db_get_ae(target_rtnode->ri);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = ae_to_json(gae);
	o2pt->rsc = RSC_OK;
	respond_to_client(o2pt, 200);
	free_ae(gae); gae = NULL;
}

void retrieve_cnt(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	CNT* gcnt = db_get_cnt(target_rtnode->ri);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cnt_to_json(gcnt);
	o2pt->rsc = RSC_OK;
	respond_to_client(o2pt, 200);
	free_cnt(gcnt); gcnt = NULL;
}

void retrieve_cin(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	CIN* gcin = db_get_cin(target_rtnode->ri);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cin_to_json(gcin);
	o2pt->rsc = RSC_OK;
	respond_to_client(o2pt, 200); 
	free_cin(gcin); gcin = NULL;
}

void retrieve_grp(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	GRP *grp = db_get_grp(target_rtnode->ri);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = grp_to_json(grp);
	o2pt->rsc = RSC_OK;
	respond_to_client(o2pt, 200);
	free_grp(grp);
}

void retrieve_sub(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	Sub* gsub = db_get_sub(target_rtnode->ri);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = sub_to_json(gsub);
	o2pt->rsc = RSC_OK;
	respond_to_client(o2pt, 200); 
	free_sub(gsub); gsub = NULL;
}

void retrieve_acp(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	ACP* gacp = db_get_acp(target_rtnode->ri);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = acp_to_json(gacp);
	o2pt->rsc = RSC_OK;
	respond_to_client(o2pt, 200);
	free_acp(gacp); gacp = NULL;
}

void delete_rtnode_and_db_data(RTNode *rtnode, int flag) {
	switch(rtnode->ty) {
	case RT_AE : 
		db_delete_onem2m_resource(rtnode->ri); 
		break;
	case RT_CNT : 
		db_delete_onem2m_resource(rtnode->ri); 
		//char *noti_json = (char*)malloc(sizeof("resource is deleted successfully") + 1);
		//strcpy(noti_json, "resource is deleted successfully");
		//notify_onem2m_resource(node->child,noti_json,NOTIFICATION_EVENT_2); 
		//free(noti_json); noti_json = NULL;
		break;
	case RT_CIN :
		db_delete_onem2m_resource(rtnode->ri);
		update_cnt_cin(rtnode->parent, rtnode,-1);
		return;
	case RT_SUB :
		db_delete_sub(rtnode->ri);
		break;
	case RT_ACP :
		db_delete_acp(rtnode->ri);
		break;
	case RT_GRP :
		db_delete_grp(rtnode->ri);
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
	
}

void init_ae(AE* ae, char *pi, char *origin) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char ri[128] = {'\0'};

	if(origin) {
		if(origin[0] != 'C') strcpy(ri, "C");
		strcat(ri, origin);
	} else {
		strcpy(ri, "CAE");
		strcat(ri, ct);
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

void init_sub(Sub* sub, char *pi, char *uri) {
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

void set_ae_update(cJSON *m2m_ae, AE* after) {
	cJSON *rr = cJSON_GetObjectItemCaseSensitive(m2m_ae, "rr");
	cJSON *lbl = cJSON_GetObjectItem(m2m_ae, "lbl");
	cJSON *srv = cJSON_GetObjectItem(m2m_ae, "srv");

	if(rr) {
		if(cJSON_IsTrue(rr)) {
			after->rr = true;
		} else {
			after->rr = false;
		}
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
}

void set_cnt_update(cJSON *m2m_cnt, CNT* after) {
	cJSON *lbl = cJSON_GetObjectItem(m2m_cnt, "lbl");
	cJSON *acpi = cJSON_GetObjectItem(m2m_cnt, "acpi");
	cJSON *mni = cJSON_GetObjectItem(m2m_cnt, "mni");
	cJSON *mbs = cJSON_GetObjectItem(m2m_cnt, "mbs");

	if(acpi) {
		if(after->acpi) free(after->acpi);
		after->acpi = cjson_list_item_to_string(acpi);
	}
	
	if(lbl) {
		if(after->lbl) 
			free(after->lbl);
		after->lbl = cjson_list_item_to_string(lbl);
	}

	if(mni) {
		after->mni = mni->valueint;
	}

	if(mbs) {
		after->mbs = mbs->valueint;
	}

	if(after->lt) 
		free(after->lt);

	after->lt = get_local_time(0);
}

void set_rtnode_update(RTNode *rtnode, void *after) {
	ResourceType ty = rtnode->ty;
	if(rtnode->acpi) {free(rtnode->acpi); rtnode->acpi = NULL;}
	if(rtnode->nu) {free(rtnode->nu); rtnode->nu = NULL;}
	if(rtnode->pv_acor && rtnode->pv_acop) {
		free(rtnode->pv_acor); rtnode->pv_acor = NULL; 
		free(rtnode->pv_acop); rtnode->pv_acop = NULL;
	}
	if(rtnode->pvs_acor && rtnode->pvs_acop) {
		free(rtnode->pvs_acor); rtnode->pvs_acor = NULL;
		free(rtnode->pvs_acop); rtnode->pvs_acop = NULL;
	}
	
	switch(ty) {
	case RT_CNT:
		CNT *cnt = (CNT*)after;
		if(cnt->acpi) {
			rtnode->acpi = (char*)malloc((strlen(cnt->acpi) + 1)*sizeof(char));
			strcpy(rtnode->acpi, cnt->acpi);
		}
		break;

	case RT_SUB:
		Sub *sub = (Sub*)after;
		rtnode->net = net_to_bit(sub->net);
		if(sub->nu) {
			rtnode->nu = (char*)malloc((strlen(sub->nu) + 1)*sizeof(char));
			strcpy(rtnode->nu, sub->nu);
		}
		break;

	case RT_ACP:
		ACP *acp = (ACP*)after;
		if(acp->pv_acor && acp->pv_acop) {
			rtnode->pv_acor = (char*)malloc((strlen(acp->pv_acor) + 1)*sizeof(char));
			rtnode->pv_acop = (char*)malloc((strlen(acp->pv_acop) + 1)*sizeof(char));
			strcpy(rtnode->pv_acor, acp->pv_acor);
			strcpy(rtnode->pv_acop, acp->pv_acop);
		}
		if(acp->pvs_acor && acp->pvs_acop) {
			rtnode->pvs_acor = (char*)malloc((strlen(acp->pvs_acor) + 1)*sizeof(char));
			rtnode->pvs_acop = (char*)malloc((strlen(acp->pvs_acop) + 1)*sizeof(char));
			strcpy(rtnode->pvs_acor, acp->pvs_acor);
			strcpy(rtnode->pvs_acop, acp->pvs_acop);
		}
		break;

	case RT_GRP:
		GRP *grp = (GRP*) after;
		if(grp->acpi) {
			rtnode->acpi = strdup(grp->acpi);
		}
		break;

	}
}

void update_ae(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][4] = {"ty", "pi", "ri", "rn"};
	cJSON *m2m_ae = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:ae");
	int invalid_key_size = sizeof(invalid_key)/(4*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_ae, invalid_key[i])) {
			logger("MAIN", LOG_LEVEL_ERROR, "Unsupported attribute on update");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			o2pt->rsc = RSC_BAD_REQUEST;
			respond_to_client(o2pt, 200);
			return;
		}
	}

	AE* after = db_get_ae(target_rtnode->ri);
	int result;

	set_ae_update(m2m_ae, after);
	set_rtnode_update(target_rtnode, after);
	result = db_delete_onem2m_resource(after->ri);
	result = db_store_ae(after);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = ae_to_json(after);
	o2pt->rsc = RSC_UPDATED;
	respond_to_client(o2pt,200);
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	free_ae(after); after = NULL;
}

void update_cnt(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	char invalid_key[][4] = {"ty", "pi", "ri", "rn"};
	cJSON *m2m_cnt = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:cnt");
	int invalid_key_size = sizeof(invalid_key)/(4*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_cnt, invalid_key[i])) {
			logger("MAIN", LOG_LEVEL_ERROR, "Unsupported attribute on update");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			o2pt->rsc = 4000;
			respond_to_client(o2pt, 200);
			return;
		}
	}

	CNT* after = db_get_cnt(target_rtnode->ri);
	int result;

	set_cnt_update(m2m_cnt, after);
	if(after->mbs != INT_MIN && after->mbs < 0) {
		mni_mbs_invalid(o2pt, "mbs"); free(after); after = NULL;
		return;
	}
	if(after->mni != INT_MIN && after->mni < 0) {
		mni_mbs_invalid(o2pt, "mni"); free(after); after = NULL;
		return;
	}
	target_rtnode->mbs = after->mbs;
	target_rtnode->mni = after->mni;
	delete_cin_under_cnt_mni_mbs(after);
	after->st++;
	result = db_delete_onem2m_resource(after->ri);
	result = db_store_cnt(after);
	
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = cnt_to_json(after);
	o2pt->rsc = 2004;
	respond_to_client(o2pt, 200);
	//notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	free_cnt(after); after = NULL;
}


void update_cnt_cin(RTNode *cnt_rtnode, RTNode *cin_rtnode, int sign) {
	CNT *cnt = db_get_cnt(cnt_rtnode->ri);
	cnt->cni += sign;
	cnt->cbs += sign*(cin_rtnode->cs);
	delete_cin_under_cnt_mni_mbs(cnt);	
	cnt_rtnode->cni = cnt->cni;
	cnt_rtnode->cbs = cnt->cbs;
	cnt->st++;
	db_delete_onem2m_resource(cnt_rtnode->ri);
	db_store_cnt(cnt);
	free_cnt(cnt);
}


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


	if(!grp->rn) {
		grp->rn = (char *) malloc((strlen(ri) + 1) * sizeof(char));
		strcpy(grp->rn, ri);
	} 

	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

int set_grp_update(cJSON *m2m_grp, GRP* after){
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
	
	if(mnm)
		after->mnm = mnm->valueint;

	int new_cnm = 0;

	if(mid){
		size_t mid_size = cJSON_GetArraySize(mid);
		if(mid_size > after->mnm) return -1;
		new_cnm = mid_size;
		for(int i = 0 ; i < after->mnm; i++){
			if(i < after->cnm) 
				free(after->mid[i]);
			if(i < mid_size){
				mc = cJSON_GetArrayItem(mid, i);
				if(validate_mid_dup(after->mid, i, mc->valuestring))
					after->mid[i] = strdup(mc->valuestring);
				else
					new_cnm--;
				
			}else{
				after->mid[i] = NULL;
			}
		}
		after->cnm = new_cnm;
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);
}

void update_grp(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	char invalid_key[][4] = {"ty", "pi", "ri", "rn", "ct"};
	cJSON *m2m_grp = cJSON_GetObjectItem(o2pt->cjson_pc, "m2m:grp");
	int invalid_key_size = sizeof(invalid_key)/(4*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(cJSON_GetObjectItem(m2m_grp, invalid_key[i])) {
			logger("MAIN", LOG_LEVEL_ERROR, "Unsupported attribute on update");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"unsupported attribute on update\"}");
			o2pt->rsc = RSC_BAD_REQUEST;
			respond_to_client(o2pt, 200);
			return;
		}
	}

	GRP *after = db_get_grp(target_rtnode->ri);
	int result;
	if( set_grp_update(m2m_grp, after) == -1){
		o2pt->rsc = RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED;
		respond_to_client(o2pt, 400);

		free_grp(after);
		after = NULL;
		return;
	}
	set_rtnode_update(target_rtnode, after);

	result = db_delete_grp(after->ri);
	result = db_store_grp(after);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = grp_to_json(after);
	o2pt->rsc = RSC_UPDATED;
	respond_to_client(o2pt, 200);

	free_grp(after); after = NULL;

}

void create_grp(oneM2MPrimitive *o2pt, RTNode *parent_rtnode){
	int e = 1;
	if(parent_rtnode->ty != RT_CNT && parent_rtnode->ty != RT_AE && parent_rtnode->ty != RT_CSE) {
		child_type_error(o2pt);
		return;
	}

	GRP *grp = (GRP *)calloc(1, sizeof(GRP));
	o2pt->rsc = cjson_to_grp(o2pt->cjson_pc, grp); // TODO Validation(mid dup chk etc.)
	init_grp(grp, parent_rtnode->ri);
	validate_grp(parent_rtnode, grp);


	if(o2pt->rsc >= 5000 && o2pt->rsc <= 7000){
		handle_error(o2pt);
		free_grp(grp);
		grp = NULL;
		return;
	}

	int result = db_store_grp(grp);
	if(result != 1){
		db_store_fail(o2pt); free_grp(grp); grp = NULL;
		return;
	}

	RTNode *child_rtnode = create_rtnode(grp, RT_GRP);
	add_child_resource_tree(parent_rtnode, child_rtnode);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = grp_to_json(grp);
	respond_to_client(o2pt, 201);

	free_grp(grp); grp = NULL;
}
