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
#include "util.h"
#include "httpd.h"
#include "logger.h"
#include "onem2mTypes.h"
#include "config.h"
#include "berkeleyDB.h"
#include "jsonparse.h"

extern ResourceTree *rt;

RTNode* parse_uri(oneM2MPrimitive *o2pt, RTNode *cb) {
	logger("O2M", LOG_LEVEL_DEBUG, "Call parse_uri");
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

	int latest_oldest_flag = -1;

	if(!strcmp(uri_strtok[0], "viewer")) index_start++;
	if(!strcmp(uri_strtok[index_end], "la") || !strcmp(uri_strtok[index_end], "latest")) {
		latest_oldest_flag = 0; index_end--;
	} else if(!strcmp(uri_strtok[index_end], "ol") || !strcmp(uri_strtok[index_end], "oldest")) {
		latest_oldest_flag = 1; index_end--;
	}

	int fopt_flag = -1;
	if(!strcmp(uri_strtok[index_end], "fopt")){
		index_end--;
		fopt_flag = 1;
	}

	strcpy(uri_array, "\0");
	for(int i=index_start; i<=index_end; i++) {
		strcat(uri_array,"/"); strcat(uri_array,uri_strtok[i]);
	}
	RTNode* rtnode = find_rtnode_by_uri(cb, uri_array);
	
	if(rtnode && latest_oldest_flag != -1) rtnode = find_latest_oldest(rtnode, latest_oldest_flag);

	if(index_start == 1) o2pt->op = OP_VIEWER;

	return rtnode;
}

RTNode *find_rtnode_by_uri(RTNode *cb, char *target_uri) {
	RTNode *rtnode = cb, *parent_rtnode = NULL;
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
		if(i == index-1) parent_rtnode = rtnode;
		if(!rtnode) break;
		if(i != index) rtnode = rtnode->child;
	}
	if(rtnode) return rtnode;

	if(parent_rtnode) {
		CIN *cin = db_get_cin(uri_array[index]);
		if(cin) {
			if(!strcmp(cin->pi, parent_rtnode->ri)) {
				rtnode = create_rtnode(cin, RT_CIN);
				rtnode->parent = parent_rtnode;
			}
			free_cin(cin);
		}
	}

	return rtnode;
}

RTNode *find_latest_oldest(RTNode* rtnode, int flag) {
	if(rtnode->ty == RT_CNT) {
		RTNode *head = db_get_cin_rtnode_list_by_pi(rtnode->ri);
		RTNode *cin = head;

		if(cin) {
			if(flag == 1) {
				head = head->sibling_right;
				cin->sibling_right = NULL;			
			} else {
				while(cin->sibling_right) cin = cin->sibling_right;
				if(cin->sibling_left) {
					cin->sibling_left->sibling_right = NULL;
					cin->sibling_left = NULL;
				}
			}
			if(head != cin) free_rtnode_list(head);
			if(cin) {
				cin->parent = rtnode;
				return cin;
			} else {
				return NULL;
			}
		}
		return NULL;
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

	logger("O2M", LOG_LEVEL_DEBUG, "Add Resource Tree Node [Parent-ID] : %s, [Child-ID] : %s",parent->ri, child->ri);
	
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
	
	
	
	return 1;
}

ResourceType http_parse_object_type() {
	char *content_type = request_header("Content-Type");
	if(!content_type) return RT_MIXED;
	char *str_ty = strstr(content_type, "ty=");
	if(!str_ty) return RT_MIXED;
	int object_type = atoi(str_ty+3);

	ResourceType ty;
	
	switch(object_type) {
	case 1 : ty = RT_ACP; break;
	case 2 : ty = RT_AE; break;
	case 3 : ty = RT_CNT; break;
	case 4 : ty = RT_CIN; break;
	case 5 : ty = RT_CSE; break;
	case 9 : ty = RT_GRP; break;
	case 23 : ty = RT_SUB; break;
	default : ty = RT_MIXED; break;
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

void set_o2pt_pc(oneM2MPrimitive *o2pt, char *pc, ...){
	if(o2pt->pc) free(o2pt->pc);

	o2pt->pc = (char *)malloc((strlen(pc) + 1) * sizeof(char));
	strcpy(o2pt->pc, pc);
}

void set_o2pt_rsc(oneM2MPrimitive *o2pt, int rsc){
	o2pt->rsc = rsc;
}

int is_json_valid_char(char c){
	return (('!' <= c && c <= '~') || c == ' ');
}

void respond_to_client(oneM2MPrimitive *o2pt, int status) {
	if(!o2pt->pc) {
		logger("O2M", LOG_LEVEL_ERROR, "Response payload is NULL");
		return;
	}

	switch(o2pt->prot) {
		case PROT_HTTP:
			http_respond_to_client(o2pt, status); 
			break;
		#ifdef ENABLE_MQTT
		case PROT_MQTT:
			mqtt_respond_to_client(o2pt);
			break;
		#endif
	}
}

ResourceType parse_object_type_cjson(cJSON *cjson) {
	ResourceType ty;

	if(!cjson) return RT_MIXED;
	
	if(cJSON_GetObjectItem(cjson, "m2m:cb") || cJSON_GetObjectItem(cjson, "m2m:cse")) ty = RT_CSE;
	else if(cJSON_GetObjectItem(cjson, "m2m:ae")) ty = RT_AE;
	else if(cJSON_GetObjectItem(cjson, "m2m:cnt")) ty = RT_CNT;
	else if(cJSON_GetObjectItem(cjson, "m2m:cin")) ty = RT_CIN;
	else if(cJSON_GetObjectItem(cjson, "m2m:sub")) ty = RT_SUB;
	else if(cJSON_GetObjectItem(cjson, "m2m:grp")) ty = RT_GRP;
	else if(cJSON_GetObjectItem(cjson, "m2m:acp")) ty = RT_ACP;
	else ty = RT_MIXED;
	
	return ty;
}

char *resource_identifier(ResourceType ty, char *ct) {
	char *ri = (char *)calloc(24, sizeof(char));

	switch(ty) {
		case RT_CNT : strcpy(ri, "3-"); break;
		case RT_CIN : strcpy(ri, "4-"); break;
		case RT_SUB : strcpy(ri, "23-"); break;
		case RT_ACP : strcpy(ri, "1-"); break;
		case RT_GRP : strcpy(ri, "9-"); break;
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

void delete_cin_under_cnt_mni_mbs(CNT *cnt) {
	if(cnt->cni <= cnt->mni && cnt->cbs <= cnt->mbs) return;

	RTNode *head = db_get_cin_rtnode_list_by_pi(cnt->ri);
	RTNode *right;

	while((cnt->mni >= 0 && cnt->cni > cnt->mni) || (cnt->mbs >= 0 && cnt->cbs > cnt->mbs)) {
		if(head) {
			right = head->sibling_right;
			db_delete_onem2m_resource(head->ri);
			cnt->cbs -= head->cs;
			cnt->cni--;
			free_rtnode(head);
			head = right;
		} else {
			break;
		}
	}

	if(head) free_rtnode_list(head);
}

void too_large_content_size_error(oneM2MPrimitive *o2pt) {
	logger("O2M", LOG_LEVEL_ERROR, "Too large content size");
	set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"too large content size\"}");
	o2pt->rsc = RSC_NOT_ACCEPTABLE;
	respond_to_client(o2pt, 400);
}

void tree_viewer_api(oneM2MPrimitive *o2pt, RTNode *node) {
	logger("O2M", LOG_LEVEL_DEBUG, "\x1b[43mTree Viewer API\x1b[0m\n");
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
	
	logger("O2M", LOG_LEVEL_DEBUG,"Latest CIN Size : %d\n", cinSize);
	
	tree_viewer_data(node, &viewer_data, cinSize);
	strcat(viewer_data,"]\0");
	char *res;
	res = calloc(1, MAX_TREE_VIEWER_SIZE);
	int index = 0;

	for(int i=0; i<MAX_TREE_VIEWER_SIZE; i++) {
		if(i == 1) continue;
		if(is_json_valid_char(viewer_data[i])) {
			res[index++] = viewer_data[i];
		}
	}
	
	fprintf(stderr,"Content-Size : %ld\n",strlen(res));
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = res;
	respond_to_client(o2pt, 200);
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

	if(node->ty != RT_SUB && node->ty != RT_ACP) {
		RTNode *cin_list_head = db_get_cin_rtnode_list_by_pi(node->ri);

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

void log_runtime(double start) {
	double end = (((double)clock()) / CLOCKS_PER_SEC); // runtime check - end
	logger("UTIL", LOG_LEVEL_DEBUG, "Run time : %lf", end-start);
}


void init_server() {
	rt = (ResourceTree *)malloc(sizeof(rt));
	
	CSE *cse;

	if(access("./RESOURCE.db", 0) == -1) {
		cse = (CSE*)malloc(sizeof(CSE));
		init_cse(cse);
		db_store_cse(cse);
	} else {
		cse = db_get_cse(CSE_BASE_RI);
	}
	
	rt->cb = create_rtnode(cse, RT_CSE);
	free_cse(cse); cse = NULL;

 	restruct_resource_tree();
}

void restruct_resource_tree(){
	RTNode *rtnode_list = (RTNode *)calloc(1,sizeof(RTNode));
	RTNode *tail = rtnode_list;
	
	if(access("./RESOURCE.db", 0) != -1) {
		RTNode* ae_list = db_get_all_ae();
		tail->sibling_right = ae_list;
		if(ae_list) ae_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;

		RTNode* cnt_list = db_get_all_cnt();
		tail->sibling_right = cnt_list;
		if(cnt_list) cnt_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;
	} else {
		logger("MAIN", LOG_LEVEL_DEBUG, "RESOURCE.db does not exist");
	}
	
	if(access("./SUB.db", 0) != -1) {
		RTNode* sub_list = db_get_all_sub();
		tail->sibling_right = sub_list;
		if(sub_list) sub_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;
	} else {
		logger("MAIN", LOG_LEVEL_DEBUG, "SUB.db does not exist");
	}

	if(access("./ACP.db", 0) != -1) {
		RTNode* acp_list = db_get_all_acp();
		tail->sibling_right = acp_list;
		if(acp_list) acp_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;
	} else {
		logger("MAIN", LOG_LEVEL_DEBUG, "ACP.db does not exist");
	}

	if(access("./GROUP.db", 0) != -1) {
		RTNode* grp_list = db_get_all_grp();
		tail->sibling_right = grp_list;
		if(grp_list) grp_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;
	} else {
		logger("MAIN", LOG_LEVEL_DEBUG, "GROUP.db does not exist");
	}
	
	RTNode *prtnode = rtnode_list;
	rtnode_list = rtnode_list->sibling_right;
	if(rtnode_list) rtnode_list->sibling_left = NULL;
	free_rtnode(prtnode);
	
	if(rtnode_list) restruct_resource_tree_child(rt->cb, rtnode_list);
}

RTNode* restruct_resource_tree_child(RTNode *parent_rtnode, RTNode *list) {
	RTNode *rtnode = list;
	
	while(rtnode) {
		RTNode *right = rtnode->sibling_right;

		if(!strcmp(parent_rtnode->ri, rtnode->pi)) {
			RTNode *left = rtnode->sibling_left;
			
			if(!left) {
				list = right;
			} else {
				left->sibling_right = right;
			}
			
			if(right) right->sibling_left = left;
			rtnode->sibling_left = rtnode->sibling_right = NULL;
			add_child_resource_tree(parent_rtnode, rtnode);
		}
		rtnode = right;
	}
	RTNode *child = parent_rtnode->child;
	
	while(child) {
		list = restruct_resource_tree_child(child, list);
		child = child->sibling_right;
	}
	
	return list;
}

int check_payload_size(oneM2MPrimitive *o2pt) {

	if(o2pt->pc && strlen(o2pt->pc) > MAX_PAYLOAD_SIZE) {
		logger("UTIL", LOG_LEVEL_ERROR, "Request payload too large");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"payload is too large\"}");
		o2pt->rsc = RSC_BAD_REQUEST;
		respond_to_client(o2pt, 413);
		return -1;
	}
	return 0;
}

int result_parse_uri(oneM2MPrimitive *o2pt, RTNode *rtnode) {
	if(!rtnode) {
		logger("UTIL", LOG_LEVEL_ERROR, "URI is invalid");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"URI is invalid\"}");
		o2pt->rsc = RSC_NOT_FOUND;
		respond_to_client(o2pt, 404);
		return -1;
	} else {
		return 0;
	} 
}

int get_acop(oneM2MPrimitive *o2pt, RTNode *rtnode) {
    if(!rtnode->acpi) return ALL_ACOP;

	char origin[64];
	int acop = 0;
	
	if(o2pt->fr) {
		strcpy(origin, o2pt->fr);
	} else {
		strcpy(origin, "all");
	}

    if(!strcmp(origin, "CAdmin")) return ALL_ACOP;

	if(rtnode->ty == TY_ACP) {
		acop = (acop | get_acop_origin(origin, rtnode, 1));
		acop = (acop | get_acop_origin("all", rtnode, 1));
		return acop;
	}

	RTNode *cb = rtnode;
	while(cb->parent) cb = cb->parent;
	
	int uri_cnt = 0;
	char arr_acp_uri[512][1024] = {"\0", }, arr_acpi[MAX_PROPERTY_SIZE] = "\0";
	char *acp_uri = NULL;

	if(rtnode->acpi)  {
		strcpy(arr_acpi, rtnode->acpi);

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
			return 0;
		}
		strcpy(arr_acor, acp->pvs_acor);
		strcpy(arr_acop, acp->pvs_acop);
	} else {
		if(!acp->pv_acor) {
			return 0;
		}
		strcpy(arr_acor, acp->pv_acor);
		strcpy(arr_acop, acp->pv_acop);
	}

	acor = strtok(arr_acor, ",");
    acop = strtok(arr_acop, ",");

	while(acor && acop) {
		if(!strcmp(acor, origin)) break;
		acor = strtok(NULL, ",");
        acop = strtok(NULL, ",");
	}

	if(acop) ret_acop = (ret_acop | atoi(acop));

	return ret_acop;
}

int check_privilege(oneM2MPrimitive *o2pt, RTNode *rtnode, ACOP acop) {
	bool deny = false;

	RTNode *parent_rtnode = rtnode;

	while(parent_rtnode && parent_rtnode->ty != RT_AE) {
		parent_rtnode = parent_rtnode->parent;
	}

	if(!o2pt->fr) {
		if(!(o2pt->op == OP_CREATE && o2pt->ty == RT_AE)) {
			deny = true;
		}
	} else if(!strcmp(o2pt->fr, "CAdmin")) {
		deny = false;
	} else if((parent_rtnode && strcmp(o2pt->fr, parent_rtnode->ri))) {
		deny = true;
	}
  
	if(rtnode->ty == TY_CIN) rtnode = rtnode->parent;

    ResourceType ty = rtnode->ty;

	if(deny == true) {
        if(ty == RT_CNT || ty == RT_ACP) {
            if((get_acop(o2pt, rtnode) & acop) == acop) {
                deny = false;
            }
        }
	}

	if(deny) {
		logger("UTIL", LOG_LEVEL_ERROR, "Originator has no privilege");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"originator has no privilege.\"}");
		o2pt->rsc = RSC_ORIGINATOR_HAS_NO_PRIVILEGE;
		respond_to_client(o2pt, 403);
		return -1;
	}

	return 0;
}

int check_rn_duplicate(oneM2MPrimitive *o2pt, RTNode *rtnode) {
	if(!rtnode) return 0;
	cJSON *root = o2pt->cjson_pc;
	cJSON *resource, *rn;

	switch(o2pt->ty) {
		case RT_AE: resource = cJSON_GetObjectItem(root, "m2m:ae"); break;
		case RT_CNT: resource = cJSON_GetObjectItem(root, "m2m:cnt"); break;
		case RT_CIN: resource = cJSON_GetObjectItem(root, "m2m:cin"); break;
		case RT_SUB: resource = cJSON_GetObjectItem(root, "m2m:sub"); break;
		case RT_GRP: resource = cJSON_GetObjectItem(root, "m2m:grp"); break;
		case RT_ACP: resource = cJSON_GetObjectItem(root, "m2m:acp"); break;
	}

	RTNode *child = rtnode->child;
    bool flag = false;

	rn = cJSON_GetObjectItem(resource, "rn");
	if(rn) {
		char *resource_name = rn->valuestring;
		while(child) {
			if(!strcmp(child->rn, resource_name)) {
                flag = true; 
                break;
			}
			child = child->sibling_right;
		}
        if(rtnode->ty == TY_CNT) {
            char invalid_rn[][8] = {"la", "latest", "ol", "oldest"};
            int invalid_rn_size = sizeof(invalid_rn)/(8*sizeof(char));
            for(int i=0; i<invalid_rn_size; i++) {
                if(!strcmp(resource_name, invalid_rn[i])) {
                    flag = true;
                }
            }
        }
	}

    if(flag) {
        logger("UTIL", LOG_LEVEL_ERROR, "Attribute `rn` is duplicated");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"attribute `rn` is duplicated\"}");
		o2pt->rsc = RSC_CONFLICT;
		respond_to_client(o2pt, 209);
		return -1;
    }

	return 0;
}

int check_aei_duplicate(oneM2MPrimitive *o2pt, RTNode *rtnode) {
	if(!rtnode) return 0;

	char aei[1024] = {'\0', };
	if(!o2pt->fr) {
		return 0;
	} else if(o2pt->fr[0] != 'C'){
		aei[0] = 'C';
	}

	strcat(aei, o2pt->fr);

	RTNode *child = rtnode->child;

	while(child) {
		if(!strcmp(child->ri, aei)) {
			logger("UTIL", LOG_LEVEL_ERROR, "AE-ID is duplicated");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"attribute `aei` is duplicated\"}");
			o2pt->rsc = RSC_ORIGINATOR_HAS_ALREADY_REGISTERD;
			respond_to_client(o2pt, 209);
			return -1;
		}
		child = child->sibling_right;
	}

	return 0;
}

int check_payload_format(oneM2MPrimitive *o2pt) {
	cJSON *cjson = o2pt->cjson_pc;
	
	if(cjson == NULL) {
		logger("UTIL", LOG_LEVEL_ERROR, "Payload format is invalid");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"payload format is invalid\"}");
		o2pt->rsc = RSC_BAD_REQUEST;
		respond_to_client(o2pt, 400);
		return -1;
	}
	return 0;
}

int check_payload_empty(oneM2MPrimitive *o2pt) {
	if(!o2pt->pc) {
		logger("UTIL", LOG_LEVEL_ERROR, "Payload is empty");
		set_o2pt_pc(o2pt,  "{\"m2m:dbg\": \"payload is empty\"}");
		o2pt->rsc = RSC_INTERNAL_SERVER_ERROR;
		respond_to_client(o2pt, 500);
		return -1;
	}
	return 0;
}

int check_rn_invalid(oneM2MPrimitive *o2pt, ResourceType ty) {
	cJSON *root = o2pt->cjson_pc;
	cJSON *resource, *rn;

	switch(ty) {
		case RT_AE: resource = cJSON_GetObjectItem(root, "m2m:ae"); break;
		case RT_CNT: resource = cJSON_GetObjectItem(root, "m2m:cnt"); break;
		case RT_SUB: resource = cJSON_GetObjectItem(root, "m2m:sub"); break;
		case RT_ACP: resource = cJSON_GetObjectItem(root, "m2m:acp"); break;
		case RT_GRP: resource = cJSON_GetObjectItem(root, "m2m:grp"); break;
	}

	rn = cJSON_GetObjectItem(resource, "rn");
	if(!rn) return 0;
	char *resource_name = rn->valuestring;
	int len_resource_name = strlen(resource_name);

	for(int i=0; i<len_resource_name; i++) {
		if(!is_rn_valid_char(resource_name[i])) {
			logger("UTIL", LOG_LEVEL_ERROR, "Resource name is invalid");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"attribute `rn` is invalid\"}");
			o2pt->rsc = RSC_BAD_REQUEST;
			respond_to_client(o2pt, 406);
			return -1;
		}
	}

	return 0;
}

bool is_rn_valid_char(char c) {
	return ((48 <= c && c <=57) || (65 <= c && c <= 90) || (97 <= c && c <= 122) || (c == '_' || c == '-'));
}

int check_resource_type_equal(oneM2MPrimitive *o2pt) {	
	if(o2pt->ty != parse_object_type_cjson(o2pt->cjson_pc)) {
		logger("UTIL", LOG_LEVEL_ERROR, "Resource type is invalid");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"resource type is invalid\"}");
		o2pt->rsc = RSC_BAD_REQUEST;
		respond_to_client(o2pt, 400);
		return -1;
	}
	return 0;
}

int check_resource_type_invalid(oneM2MPrimitive *o2pt) {
	if(o2pt->ty == RT_MIXED) {
		logger("UTIL", LOG_LEVEL_ERROR, "Resource type is invalid");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"resource type is invalid\"}");
		o2pt->rsc = RSC_BAD_REQUEST;
		respond_to_client(o2pt, 400);
		return -1;
	}
	return 0;
}

void child_type_error(oneM2MPrimitive *o2pt){
	logger("UTIL", LOG_LEVEL_ERROR, "Child type is invalid");
	set_o2pt_pc(o2pt,"{\"m2m:dbg\": \"child can not be created under the type of parent\"}");
	o2pt->rsc = RSC_INVALID_CHILD_RESOURCETYPE;
	respond_to_client(o2pt, 403);
}

void no_mandatory_error(oneM2MPrimitive *o2pt){
	logger("UTIL", LOG_LEVEL_ERROR, "Insufficient mandatory attribute(s)");
	set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"insufficient mandatory attribute(s)\"}");
	o2pt->rsc = RSC_CONTENTS_UNACCEPTABLE;
	respond_to_client(o2pt, 400);
}

void api_prefix_invalid(oneM2MPrimitive *o2pt) {
	logger("UTIL", LOG_LEVEL_ERROR, "API prefix is invalid");
	set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"attribute `api` prefix is invalid\"}");
	o2pt->rsc = RSC_BAD_REQUEST;
	respond_to_client(o2pt, 400);
}

void mni_mbs_invalid(oneM2MPrimitive *o2pt, char *attribute) {
	logger("UTIL", LOG_LEVEL_ERROR, "attribute `%s` is invalid", attribute);
	set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"attribute `%s` is invalid\"}", attribute);
	o2pt->rsc = RSC_BAD_REQUEST;
	respond_to_client(o2pt, 400);
}

void db_store_fail(oneM2MPrimitive *o2pt) {
	logger("UTIL", LOG_LEVEL_ERROR, "DB store fail");
	set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"DB store fail\"}");
	o2pt->rsc = RSC_INTERNAL_SERVER_ERROR;
	respond_to_client(o2pt, 500);
}


bool isFopt(char *str){
	char *s;
	s = strrchr(str, '/');
	return strcmp(s, "/fopt");
}

bool endswith(char *str, char *match){
	size_t str_len = 0;
	size_t match_len = 0;
	if(!str || !match) 
		return false;

	str_len = strlen(str);
	match_len = strlen(match);

	if(!str_len || !match_len)
		return false;

	return strncmp(str + str_len - match_len, match, match_len);
}

int validate_grp(RTNode* cb,  GRP *grp){
	bool hasFopt = false;
	bool isLocalResource = true;
	char *mid = NULL;
	char *p = NULL;
	char *tStr = NULL;

	RTNode *rt_node;

	if(grp->mtv) return 1;
	if(grp->mt == RT_MIXED) {
		grp->mtv = true;
		return 1;
	}


	for(int i = 0 ; i < grp->mnm; i++){
		mid = grp->mid[i];
		if(!mid) break;

		// Check is local resource
		isLocalResource = true;
		if(strlen(mid) >= 2 && mid[0] == '/' && mid[1] != '/'){
			tStr = strdup(mid);
			strtok(tStr, "/");
			p = strtok(NULL, "/");
			if( strcmp(p, CSE_BASE_NAME) != 0){
				isLocalResource = false;
			}

			free(tStr); tStr = NULL;
			p = NULL;
		}

		// resource check
		if(isLocalResource) {
			hasFopt = isFopt(mid);
			tStr = strdup(mid);
			if(hasFopt && strlen(mid) > 5)  // remove fopt 
				tStr[strlen(mid) - 5] = '\0';

			if(! (rt_node = find_rtnode_by_uri(cb, mid)))
				return RSC_NOT_FOUND;
		}else {
			// Todo - remote resource
		}

	}
}

void handle_error(oneM2MPrimitive *o2pt) {
	int rcode = 400;
	switch(o2pt->rsc){
		case RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED:
			logger("MAIN", LOG_LEVEL_DEBUG, "Max Number of member exceeded");
			break;
		
		case RSC_INVALID_ARGUMENTS:
			logger("MAIN", LOG_LEVEL_DEBUG, "Invalid Arguments");
			break;

		default:
			logger("MAIN", LOG_LEVEL_DEBUG, "Internal Server Error");
			break;
	}

	respond_to_client(o2pt, rcode);
}