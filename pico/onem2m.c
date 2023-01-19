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
#include "config.h"

RTNode* parse_uri(oneM2MPrimitive *o2pt, RTNode *cb) {
	fprintf(stderr,"parse_uri \x1b[33m%s\x1b[0m...",uri);
	char uri_array[MAX_URI_SIZE];
	char *uri_parse = uri_array;
	strcpy(uri_array, o2pt->to);

	char uri_strtok[64][MAX_URI_SIZE] = {"\0", };
	int index_start = 0, index_end = -1;

	uri_parse = strtok(uri_array, "/");

	while(uri_parse) {
		strcpy(uri_strtok[++index_end], uri_parse);
		uri_parse = strtok(NULL, "/");
	}

	if(!strcmp(uri_strtok[0], "viewer")) index_start++;
	if(!strcmp(uri_strtok[index_end], "la") || !strcmp(uri_strtok[index_end], "latest")) {
		o2pt->op = OP_LATEST; index_end--;
	} else if(!strcmp(uri_strtok[index_end], "ol") || !strcmp(uri_strtok[index_end], "oldest")) {
		o2pt->op = OP_OLDEST; index_end--;
	}

	strcpy(uri_array, "\0");
	for(int i=index_start; i<=index_end; i++) {
		strcat(uri_array,"/"); strcat(uri_array,uri_strtok[i]);
	}
	RTNode* node = find_rtnode_by_uri(cb, uri_array);
	
	//if(node && (*op == OP_LATEST || *op == OP_OLDEST)) node = find_latest_oldest(node, op);

	if(index_start == 1) o2pt->op = OP_VIEWER;

	return node;
}

RTNode *find_rtnode_by_uri(RTNode *cb, char *target_uri) {
	RTNode *rtnode = cb, *prtnode = NULL;
	target_uri = strtok(target_uri, "/");

	if(!target_uri) return NULL;

	char uri_array[64][MAX_URI_SIZE];
	int index = -1;

	while(target_uri) {
		strcpy(uri_array[++index], target_uri);
		target_uri = strtok(NULL, "/");
	}

	for(int i=0; i<=index; i++) {
		while(rtnode) {
			if(!strcmp(rtnode->rn, uri_array[i])) break;
			rtnode = rtnode->sibling_right;
		}
		if(i == index-1) prtnode = rtnode;
		if(!rtnode) break;
		if(i != index) rtnode = rtnode->child;
	}

	if(rtnode) return rtnode;

	RTNode *head;

	if(prtnode) {
		head = db_get_cin_list_by_pi(prtnode->ri);
		rtnode = head;
		while(rtnode) {
			if(!strcmp(rtnode->rn, uri_array[index])) break;
			rtnode = rtnode->sibling_right;
		}
	}

	if(rtnode) {
		if(rtnode->sibling_left) rtnode->sibling_left->sibling_right = rtnode->sibling_right;
		if(rtnode->sibling_right) rtnode->sibling_right->sibling_left = rtnode->sibling_left;
		rtnode->parent = prtnode;
	}

	if(head) free_rtnode_list(head);

	return rtnode;
}

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
	
	cse->ty = TY_CSE;
	
	free(ct); ct = NULL;
}

RTNode* create_node(void *obj, ObjectType ty){
	RTNode* node = NULL;

	switch(ty) {
	case TY_CSE: node = create_cse_node((CSE*)obj); break;
	case TY_AE: node = create_ae_node((AE*)obj); break;
	case TY_CNT: node = create_cnt_node((CNT*)obj); break;
	case TY_CIN: node = create_cin_node((CIN*)obj); break;
	case TY_SUB: node = create_sub_node((Sub*)obj); break;
	case TY_ACP: node = create_acp_node((ACP*)obj); break;
	}

	node->parent = NULL;
	node->child = NULL;
	node->sibling_left = NULL;
	node->sibling_right = NULL;
	
	fprintf(stderr,"OK\n");
	
	return node;
}

RTNode* create_cse_node(CSE *cse) {
	fprintf(stderr,"Create Tree RTNode\n[rn] %s\n[ri] %s...",cse->rn, cse->ri);

	RTNode* node = calloc(1, sizeof(RTNode));

	node->rn = (char*)malloc((strlen(cse->rn) + 1) * sizeof(char));
	node->ri = (char*)malloc((strlen(cse->ri) + 1) * sizeof(char));
	node->pi = (char*)malloc((strlen(cse->pi) + 1) * sizeof(char));
	
	strcpy(node->rn, cse->rn);
	strcpy(node->ri, cse->ri);
	strcpy(node->pi, cse->pi);

	node->ty = TY_CSE;

	return node;
}

RTNode* create_ae_node(AE *ae) {
	fprintf(stderr,"Create Tree RTNode\n[rn] %s\n[ri] %s...",ae->rn, ae->ri);

	RTNode* node = calloc(1, sizeof(RTNode));

	node->rn = (char*)malloc((strlen(ae->rn) + 1) * sizeof(char));
	node->ri = (char*)malloc((strlen(ae->ri) + 1) * sizeof(char));
	node->pi = (char*)malloc((strlen(ae->pi) + 1) * sizeof(char));
	
	strcpy(node->rn, ae->rn);
	strcpy(node->ri, ae->ri);
	strcpy(node->pi, ae->pi);

	node->ty = TY_AE;

	return node;
}

RTNode* create_cnt_node(CNT *cnt) {
	fprintf(stderr,"Create Tree RTNode\n[rn] %s\n[ri] %s...",cnt->rn, cnt->ri);

	RTNode* node = calloc(1, sizeof(RTNode));

	node->rn = (char*)malloc((strlen(cnt->rn) + 1) * sizeof(char));
	node->ri = (char*)malloc((strlen(cnt->ri) + 1) * sizeof(char));
	node->pi = (char*)malloc((strlen(cnt->pi) + 1) * sizeof(char));
	
	strcpy(node->rn, cnt->rn);
	strcpy(node->ri, cnt->ri);
	strcpy(node->pi, cnt->pi);

	if(cnt->acpi) {
		node->acpi = (char*)malloc((strlen(cnt->acpi) + 1) * sizeof(char));
		strcpy(node->acpi, cnt->acpi);
	}

	node->ty = TY_CNT;

	return node;
}

RTNode* create_cin_node(CIN *cin) {
	fprintf(stderr,"Create Tree RTNode\n[rn] %s\n[ri] %s...",cin->rn, cin->ri);

	RTNode* node = calloc(1, sizeof(RTNode));

	node->rn = (char*)malloc((strlen(cin->rn) + 1) * sizeof(char));
	node->ri = (char*)malloc((strlen(cin->ri) + 1) * sizeof(char));
	node->pi = (char*)malloc((strlen(cin->pi) + 1) * sizeof(char));
	
	strcpy(node->rn, cin->rn);
	strcpy(node->ri, cin->ri);
	strcpy(node->pi, cin->pi);

	node->ty = TY_CIN;

	return node;
}

RTNode* create_sub_node(Sub *sub) {
	fprintf(stderr,"Create Tree RTNode\n[rn] %s\n[ri] %s...",sub->rn, sub->ri);

	RTNode* node = calloc(1, sizeof(RTNode));

	node->rn = (char*)malloc((strlen(sub->rn) + 1) * sizeof(char));
	node->ri = (char*)malloc((strlen(sub->ri) + 1) * sizeof(char));
	node->pi = (char*)malloc((strlen(sub->pi) + 1) * sizeof(char));
	node->nu = (char*)malloc((strlen(sub->nu) + 1) * sizeof(char));
	node->sur = (char*)malloc((strlen(sub->sur) + 1) * sizeof(char));
	
	strcpy(node->rn, sub->rn);
	strcpy(node->ri, sub->ri);
	strcpy(node->pi, sub->pi);
	strcpy(node->nu, sub->nu);
	strcpy(node->sur, sub->sur);

	node->ty = TY_SUB;
	node->net = net_to_bit(sub->net);

	return node;
}

RTNode* create_acp_node(ACP *acp) {
	fprintf(stderr,"Create Tree RTNode\n[rn] %s\n[ri] %s...",acp->rn, acp->ri);

	RTNode* node = calloc(1, sizeof(RTNode));

	node->rn = (char*)malloc((strlen(acp->rn) + 1) * sizeof(char));
	node->ri = (char*)malloc((strlen(acp->ri) + 1) * sizeof(char));
	node->pi = (char*)malloc((strlen(acp->pi) + 1) * sizeof(char));
	node->pv_acor = (char*)malloc((strlen(acp->pv_acor) + 1) * sizeof(char));
	node->pv_acop = (char*)malloc((strlen(acp->pv_acop) + 1) * sizeof(char));
	node->pvs_acor = (char*)malloc((strlen(acp->pvs_acor) + 1) * sizeof(char));
	node->pvs_acop = (char*)malloc((strlen(acp->pvs_acop) + 1) * sizeof(char));

	strcpy(node->rn, acp->rn);
	strcpy(node->ri, acp->ri);
	strcpy(node->pi, acp->pi);
	strcpy(node->pv_acor, acp->pv_acor);
	strcpy(node->pv_acop, acp->pv_acop);
	strcpy(node->pvs_acor, acp->pvs_acor);
	strcpy(node->pvs_acop, acp->pvs_acop);

	node->ty = TY_ACP;

	return node;
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

int net_to_bit(char *net) {
	int netLen = strlen(net);
	int ret = 0;

	for(int i=0; i<netLen; i++) {
		int exp = atoi(net+i);
		if(exp > 0) ret = (ret | (int)pow(2, exp - 1));
	}

	return ret;
}

int add_child_resource_tree(RTNode *parent, RTNode *child) {
	RTNode *node = parent->child;
	child->parent = parent;

	fprintf(stderr,"\nAdd Child\n[P] %s\n[C] %s...",parent->rn, child->rn);
	
	if(!node) {
		parent->child = child;
	} else if(node) {
		while(node->sibling_right && node->sibling_right->ty <= child->ty) { 	
				node = node->sibling_right;
		}

		if(parent->child == node && child->ty < node->ty) {
			parent->child = child;
			child->sibling_right = node;
			node->sibling_left = child;
		} else {
			if(node->sibling_right) {
				node->sibling_right->sibling_left = child;
				child->sibling_right = node->sibling_right;
			}

			node->sibling_right = child;
			child->sibling_left = node;
		}
	}
	
	fprintf(stderr,"OK\n");
	
	return 1;
}

ObjectType http_parse_object_type() {
	char *content_type = request_header("Content-Type");
	if(!content_type) return TY_NONE;
	char *str_ty = strstr(content_type, "ty=");
	if(!str_ty) return TY_NONE;
	int object_type = atoi(str_ty+3);

	ObjectType ty;
	
	switch(object_type) {
	case 1 : ty = TY_ACP; break;
	case 2 : ty = TY_AE; break;
	case 3 : ty = TY_CNT; break;
	case 4 : ty = TY_CIN; break;
	case 5 : ty = TY_CSE; break;
	case 23 : ty = TY_SUB; break;
	}
	
	return ty;
}

char *get_local_time(int diff) {
	time_t t = time(NULL) - diff;
	struct tm tm = *localtime(&t);
	
	char year[5], mon[3], day[3], hour[3], minute[3], sec[3]; 
	
	sprintf(year,"%d", tm.tm_year+1900);
	sprintf(mon,"%02d",tm.tm_mon+1);
	sprintf(day,"%02d",tm.tm_mday);
	sprintf(hour,"%02d",tm.tm_hour);
	sprintf(minute,"%02d",tm.tm_min);
	sprintf(sec,"%02d",tm.tm_sec);
	
	char* local_time = (char*)malloc(16 * sizeof(char));
	
	*local_time = '\0';
	strcat(local_time,year);
	strcat(local_time,mon);
	strcat(local_time,day);
	strcat(local_time,"T");
	strcat(local_time,hour);
	strcat(local_time,minute);
	strcat(local_time,sec);
	
	return local_time;
}

void set_o2pt_pc(oneM2MPrimitive *o2pt, char *pc){
	if(o2pt->pc) free(o2pt->pc);

	o2pt->pc = (char *)malloc((strlen(pc) + 1) * sizeof(char));
	strcpy(o2pt->pc, pc);
}

void set_o2pt_rsc(oneM2MPrimitive *o2pt, char *rsc){
	if(o2pt->rsc) free(o2pt->rsc);

	o2pt->rsc = (char *)malloc((strlen(rsc) + 1) * sizeof(char));
	strcpy(o2pt->rsc, rsc);
}

int is_json_valid_char(char c){
	return (('!' <= c && c <= '~') || c == ' ');
}

/*

RTNode *find_latest_oldest(RTNode* node, Operation *op) {
	if(node->ty == TY_CNT) {
		RTNode *head = db_get_cin_list_by_pi(node->ri);
		RTNode *cin = head;

		if(cin) {
			if(*op == OP_OLDEST) {
				head = head->sibling_right;
				cin->sibling_right = NULL;
			} else {
				while(cin->sibling_right) cin = cin->sibling_right;
				if(cin->sibling_left) cin->sibling_left->sibling_right = NULL;
				cin->sibling_left = NULL;
			}
			if(head != cin) free_rtnode_list(head);
			*op = OP_NONE;
			if(cin) cin->parent = node;
			return cin;
		}
	} else if(node->ty == TY_AE){
		node = node->child;
		while(node) {
			if(node->ty == TY_CNT) break;
			node = node->sibling_right;
		}
		if(node && *op == OP_LATEST) {
			while(node->sibling_right && node->sibling_right->ty == TY_CNT) {
				node = node->sibling_right;
			}
		}
		*op = OP_NONE;
		return node;
	}
	return NULL;
}

void tree_viewer_api(RTNode *node) {
	fprintf(stderr,"\x1b[43mTree Viewer API\x1b[0m\n");
	char arr_viewer_data[MAX_TREE_VIEWER_SIZE] = "[";
	char *viewer_data = arr_viewer_data;
	
	RTNode *p = node;
	while(p = p->parent) {
		char *json = node_to_json(p);
		strcat(viewer_data,",");
		strcat(viewer_data,json);
		free(json); json = NULL;
	}
	
	int cinSize = 1;
	
	char *la = strstr(qs,"la=");
	if(la) cinSize = atoi(la+3);
	
	fprintf(stderr,"Latest CIN Size : %d\n", cinSize);
	
	tree_viewer_data(node, &viewer_data, cinSize);
	strcat(viewer_data,"]\0");
	char res[MAX_TREE_VIEWER_SIZE] = "";
	int index = 0;
	
	for(int i=0; i<MAX_TREE_VIEWER_SIZE; i++) {
		if(i == 1) continue;
		if(is_json_valid_char(viewer_data[i])) {
			res[index++] = viewer_data[i];
		}
	}
	
	fprintf(stderr,"Content-Size : %ld\n",strlen(res));

	respond_to_client(200, res, "2000");
}

void tree_viewer_data(RTNode *node, char **viewer_data, int cin_size) {
	char *json = node_to_json(node);

	strcat(*viewer_data, ",");
	strcat(*viewer_data, json);
	free(json); json = NULL;

	RTNode *child = node->child;
	while(child) {
		tree_viewer_data(child, viewer_data, cin_size);
		child = child->sibling_right;
	}

	if(node->ty != TY_SUB && node->ty != TY_ACP) {
		RTNode *cin_list_head = db_get_cin_list_by_pi(node->ri);

		if(cin_list_head) cin_list_head = latest_cin_list(cin_list_head, cin_size);

		RTNode *p = cin_list_head;

		while(p) {
			json = node_to_json(p);
			strcat(*viewer_data, ",");
			strcat(*viewer_data, json);
			free(json); json = NULL;
			p = p->sibling_right;		
		}
		free_rtnode_list(cin_list_head);
	}
}

RTNode *latest_cin_list(RTNode* cinList, int num) {
	RTNode *head, *tail;
	head = tail = cinList;
	int cnt = 1;
	
	while(tail->sibling_right) {
		tail = tail->sibling_right;
		cnt++;
	}
	
	for(int i=0; i < cnt-num; i++) {
		head = head->sibling_right;
		free_rtnode(head->sibling_left); head->sibling_left = NULL;
	}
	
	return head;
}

ObjectType parse_object_type_in_request_body() {
	ObjectType ty;

	if(payload == NULL) return TY_NONE;
	
	cJSON *json = cJSON_Parse(payload);
	if(!json) {
		return TY_NONE;
	}
	
	if(cJSON_GetObjectItem(json, "m2m:cb") || cJSON_GetObjectItem(json, "m2m:cse")) ty = TY_CSE;
	else if(cJSON_GetObjectItem(json, "m2m:ae")) ty = TY_AE;
	else if(cJSON_GetObjectItem(json, "m2m:cnt")) ty = TY_CNT;
	else if(cJSON_GetObjectItem(json, "m2m:cin")) ty = TY_CIN;
	else if(cJSON_GetObjectItem(json, "m2m:sub")) ty = TY_SUB;
	else if(cJSON_GetObjectItem(json, "m2m:acp")) ty = TY_ACP;
	else ty = TY_NONE;

	cJSON_Delete(json);
	
	return ty;
}

void delete_node_and_db_data(RTNode *node, int flag) {
	switch(node->ty) {
	case TY_AE : 
		db_delete_object(node->ri); 
		break;
	case TY_CNT : 
		db_delete_object(node->ri); 
		char *noti_json = (char*)malloc(sizeof("resource is deleted successfully") + 1);
		strcpy(noti_json, "resource is deleted successfully");
		notify_object(node->child,noti_json,NOTIFICATION_EVENT_2); 
		free(noti_json); noti_json = NULL;
		break;
	case TY_SUB :
		db_delete_sub(node->ri);
		break;
	case TY_ACP :
		db_delete_acp(node->ri);
		break;
	}

	RTNode *left = node->sibling_left;
	RTNode *right = node->sibling_right;
	
	if(flag == 1) {
		if(left) left->sibling_right = right;
		else node->parent->child = right;
		if(right) right->sibling_left = left;
	} else {
		if(right) delete_node_and_db_data(right, 0);
	}
	
	if(node->child) delete_node_and_db_data(node->child, 0);
	
	fprintf(stderr,"[free_rtnode] %s...",node->rn);
	free_rtnode(node); node = NULL;
	fprintf(stderr,"OK\n");
}

void init_ae(AE* ae, char *pi) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(TY_AE, ct);

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
	
	ae->ty = TY_AE;
	
	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

void init_cnt(CNT* cnt, char *pi) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(TY_CNT, ct);
	
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
	
	cnt->ty = TY_CNT;
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
	char *ri = resource_identifier(TY_CIN, ct);
	
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
	
	cin->ty = TY_CIN;
	cin->st = 0;
	cin->cs = strlen(cin->con);
	
	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

void init_sub(Sub* sub, char *pi) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(TY_SUB, ct);

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

	strcpy(sub->sur, uri);
	strcat(sub->sur, "/");
	strcat(sub->sur, sub->rn);
	strcpy(sub->ri, ri);
	strcpy(sub->pi, pi);
	strcpy(sub->et, et);
	strcpy(sub->ct, ct);
	strcpy(sub->lt, ct);
	sub->ty = TY_SUB;
	sub->nct = 0;

	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

void init_acp(ACP* acp, char *pi) {
	char *ct = get_local_time(0);
	char *et = get_local_time(EXPIRE_TIME);
	char *ri = resource_identifier(TY_ACP, ct);

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
	
	acp->ty = TY_ACP;
	
	free(ct); ct = NULL;
	free(et); et = NULL;
	free(ri); ri = NULL;
}

void set_ae_update(AE* after) {
	char *rn = get_json_value_char("rn", payload);
	int rr = get_json_value_bool("rr", payload);

	if(rn) {
		free(after->rn);
		after->rn = (char*)malloc((strlen(rn) + 1) * sizeof(char));
		strcpy(after->rn, rn);
		free(rn);
	}

	switch(rr) {
		case 0: after->rr = false; break;
		case 1: after->rr = true; break;
		default: break;
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);
}


void set_cnt_update(CNT* after) {
	char *rn = get_json_value_char("rn", payload);
	char *acpi = NULL;
	char *lbl = NULL;

	if(json_key_exist(payload, "m2m:cnt-acpi")) {
		acpi = get_json_value_list(payload, "m2m:cnt-acpi");
	}
	
	if(json_key_exist(payload, "m2m:cnt-lbl")) {
		lbl = get_json_value_list_v2(payload, "m2m:cnt-lbl");
	}

	if(rn) {
		free(after->rn);
		after->rn = (char*)malloc((strlen(rn) + 1) * sizeof(char));
		strcpy(after->rn, rn);
	}

	if(acpi) {
		if(after->acpi) free(after->acpi);
		after->acpi = (char*)malloc((strlen(acpi) + 1) * sizeof(char)); 
		strcpy(after->acpi, acpi);
	}

	if(lbl) {
		if(after->lbl) free(after->lbl);
		after->lbl = (char*)malloc((strlen(lbl) + 1) * sizeof(char)); 
		strcpy(after->lbl, lbl);
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);
}

void set_sub_update(Sub* after) {
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

void set_acp_update(ACP* after) {
	char *rn = get_json_value_char("rn", payload);
	char *pv_acor = NULL;
	char *pv_acop = NULL;
	char *pvs_acor = NULL;
	char *pvs_acop = NULL;

	if(strstr(payload, "\"pv\"")) {
		if(strstr(payload, "\"acr\"")) {
			if(strstr(payload, "\"acor\"") && strstr(payload, "acop")) {
				pv_acor = get_json_value_list("pv-acr-acor", payload); 
				pv_acop = get_json_value_list("pv-acr-acop", payload);
				if(!strcmp(pv_acor, "\0") || !strcmp(pv_acop, "\0")) {
					free(pv_acor); pv_acor = after->pv_acor = NULL;
					free(pv_acop); pv_acop = after->pv_acop = NULL;
				}
			}
		}
	}

	if(strstr(payload, "\"pvs\"")) {
		if(strstr(payload, "\"acr\"")) {
			if(strstr(payload, "\"acor\"") && strstr(payload, "\"acop\"")) {
				pvs_acor = get_json_value_list("pvs-acr-acor", payload);
				pvs_acop = get_json_value_list("pvs-acr-acop", payload);
				if(!strcmp(pvs_acor, "\0") || !strcmp(pvs_acop, "\0")) {
					free(pvs_acor); pvs_acor = after->pvs_acor = NULL;
					free(pvs_acop); pvs_acop = after->pvs_acop = NULL;
				}
			}
		}
	}

	if(rn) {
		free(after->rn);
		after->rn = (char*)malloc((strlen(rn) + 1) * sizeof(char));
		strcpy(after->rn, rn);
	}

	if(pv_acor && pv_acop) {
		if(after->pv_acor) free(after->pv_acor);
		if(after->pv_acop) free(after->pv_acop);
		after->pv_acor = (char*)malloc((strlen(pv_acor) + 1) * sizeof(char));
		after->pv_acop = (char*)malloc((strlen(pv_acop) + 1) * sizeof(char));
		strcpy(after->pv_acor, pv_acor);
		strcpy(after->pv_acop, pv_acop);
	}

	if(pvs_acor && pvs_acop) {
		if(after->pvs_acor) free(after->pvs_acor);
		if(after->pvs_acop) free(after->pvs_acop);
		after->pvs_acor = (char*)malloc((strlen(pvs_acor) + 1) * sizeof(char));
		after->pvs_acop = (char*)malloc((strlen(pvs_acop) + 1) * sizeof(char));
		strcpy(after->pvs_acor, pvs_acor);
		strcpy(after->pvs_acop, pvs_acop);
	}

	if(after->lt) free(after->lt);
	after->lt = get_local_time(0);
}

void set_node_update(RTNode *node, void *after) {
	ObjectType ty = node->ty;
	if(node->rn) {free(node->rn); node->rn = NULL;}
	if(node->uri) {free(node->uri); node->uri = NULL;}
	if(node->acpi) {free(node->acpi); node->acpi = NULL;}
	if(node->nu) {free(node->nu); node->nu = NULL;}
	if(node->pv_acor && node->pv_acop) {
		free(node->pv_acor); node->pv_acor = NULL; 
		free(node->pv_acop); node->pv_acop = NULL;
	}
	if(node->pvs_acor && node->pvs_acop) {
		free(node->pvs_acor); node->pvs_acor = NULL;
		free(node->pvs_acop); node->pvs_acop = NULL;
	}
	
	switch(ty) {
	case TY_AE:
		AE *ae = (AE*)after;
		node->rn = (char*)malloc((strlen(ae->rn) + 1)*sizeof(char));
		strcpy(node->rn, ae->rn);
		break;

	case TY_CNT:
		CNT *cnt = (CNT*)after;
		node->rn = (char*)malloc((strlen(cnt->rn) + 1)*sizeof(char));
		strcpy(node->rn, cnt->rn);
		if(cnt->acpi) {
			node->acpi = (char*)malloc((strlen(cnt->acpi) + 1)*sizeof(char));
			strcpy(node->acpi, cnt->acpi);
		}
		break;

	case TY_SUB:
		Sub *sub = (Sub*)after;
		node->rn = (char*)malloc((strlen(sub->rn) + 1)*sizeof(char));
		strcpy(node->rn, sub->rn);
		node->net = net_to_bit(sub->net);
		if(sub->nu) {
			node->nu = (char*)malloc((strlen(sub->nu) + 1)*sizeof(char));
			strcpy(node->nu, sub->nu);
		}
		break;

	case TY_ACP:
		ACP *acp = (ACP*)after;
		node->rn = (char*)malloc((strlen(acp->rn) + 1)*sizeof(char));
		strcpy(node->rn, acp->rn);
		if(acp->pv_acor && acp->pv_acop) {
			node->pv_acor = (char*)malloc((strlen(acp->pv_acor) + 1)*sizeof(char));
			node->pv_acop = (char*)malloc((strlen(acp->pv_acop) + 1)*sizeof(char));
			strcpy(node->pv_acor, acp->pv_acor);
			strcpy(node->pv_acop, acp->pv_acop);
		}
		if(acp->pvs_acor && acp->pvs_acop) {
			node->pvs_acor = (char*)malloc((strlen(acp->pvs_acor) + 1)*sizeof(char));
			node->pvs_acop = (char*)malloc((strlen(acp->pvs_acop) + 1)*sizeof(char));
			strcpy(node->pvs_acor, acp->pvs_acor);
			strcpy(node->pvs_acop, acp->pvs_acop);
		}
		break;
	}
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

void notify_object(RTNode *node, char *response_payload, NET net) {
	remove_invalid_char_json(response_payload);
	while(node) {
		if(node->ty == TY_SUB && (net & node->net) == net) {
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

char *resource_identifier(ObjectType ty, char *ct) {
	char *ri = (char *)calloc(24, sizeof(char));

	switch(ty) {
		case TY_AE : 
			char *origin = request_header("X-M2M-Origin");
			if(origin) {
				if(origin[0] != 'C') strcpy(ri, "C");
				strcat(ri, origin);
				return ri;
			} else {
				strcpy(ri, "CAE");
			} break;
		case TY_CNT : strcpy(ri, "3-"); break;
		case TY_CIN : strcpy(ri, "4-"); break;
		case TY_SUB : strcpy(ri, "23-"); break;
		case TY_ACP : strcpy(ri, "1-"); break;
	}

	struct timespec specific_time;
    int millsec;

	char buf[32] = "\0";

    clock_gettime(CLOCK_REALTIME, &specific_time);
    millsec = floor(specific_time.tv_nsec/1.0e6);

	sprintf(buf, "%s%03d",ct, millsec);

	strcat(ri, buf);

	return ri;
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

	char nu[MAX_PROPERTY_SIZE];
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

int get_acop(RTNode *node) {
	char *request_header_origin = request_header("X-M2M-Origin");
	char origin[128];
	int acop = 0;
	
	if(request_header_origin) {
		strcpy(origin, request_header_origin);
	} else {
		strcpy(origin, "all");
	}

	if(node->ty == TY_ACP) {
		acop = (acop | get_acop_origin(origin, node, 1));
		acop = (acop | get_acop_origin("all", node, 1));
		return acop;
	}

	if(!node->acpi) return ALL_ACOP;

	RTNode *cb = node;
	while(cb->parent) cb = cb->parent;
	
	int uri_cnt = 0;
	char arr_acp_uri[512][1024] = {"\0", }, arr_acpi[MAX_PROPERTY_SIZE] = "\0";
	char *acp_uri = NULL;

	if(node->acpi)  {
		strcpy(arr_acpi, node->acpi);

		acp_uri = strtok(arr_acpi, ",");

		while(acp_uri) { 
			strcpy(arr_acp_uri[uri_cnt++],acp_uri);
			acp_uri = strtok(NULL, ",");
		}
	}

	for(int i=0; i<uri_cnt; i++) {
		RTNode *acp = find_rtnode_by_uri(cb, arr_acp_uri[i]);

		if(acp) {
			acop = (acop | get_acop_origin(origin, acp, 0));
			acop = (acop | get_acop_origin("all", acp, 0));
		}
	}

	return acop;
}

int get_acop_origin(char *origin, RTNode *acp, int flag) {
	if(!origin) return 0;

	int ret_acop = 0, cnt = 0;
	char *acor, *acop, arr_acor[1024], arr_acop[1024];

	if(flag) {
		if(!acp->pvs_acor) {
			fprintf(stderr,"pvs_acor is NULL\n"); 
			return 0;
		}
		strcpy(arr_acor, acp->pvs_acor);
		strcpy(arr_acop, acp->pvs_acop);
	} else {
		if(!acp->pv_acor) {
			fprintf(stderr,"pv_acor is NULL\n"); 
			return 0;
		}
		strcpy(arr_acor, acp->pv_acor);
		strcpy(arr_acop, acp->pv_acop);
	}

	acor = strtok(arr_acor, ",");

	while(acor) {
		if(!strcmp(acor, origin)) break;
		acor = strtok(NULL, ",");
		cnt++;
	}

	acop = strtok(arr_acop, ",");

	for(int i=0; i<cnt; i++) acop = strtok(NULL,",");

	if(acop) ret_acop = (ret_acop | atoi(acop));

	return ret_acop;
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