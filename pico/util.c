#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "onem2m.h"
#include "util.h"
#include "httpd.h"
#include "logger.h"
#include "onem2mTypes.h"
#include "config.h"
#include "dbmanager.h"
#include "jsonparser.h"
#include "mqttClient.h"

extern ResourceTree *rt;


RTNode* parse_uri(oneM2MPrimitive *o2pt, RTNode *cb) {
	logger("O2M", LOG_LEVEL_DEBUG, "Call parse_uri %s", o2pt->to);
	
	char uri_array[MAX_URI_SIZE];
	char *uri_parse = uri_array;
	char *fopt_buf = NULL;

	if(!strncmp(o2pt->to, "~/", 2)){
        if(!strncmp(o2pt->to + 2, CSE_BASE_RI, strlen(CSE_BASE_RI))){
            char *temp = strdup(o2pt->to + 2 + strlen(CSE_BASE_RI) + 1);
            free(o2pt->to);
            o2pt->to = temp;
        }else{
            o2pt->isForwarding = true;
        }
    }

	strcpy(uri_array, o2pt->to);

	char uri_strtok[64][MAX_URI_SIZE] = {"\0", };
	int index_start = 0, index_end = -1, fopt_cnt = -1;

	uri_parse = strtok(uri_array, "/");

	while(uri_parse) {
		strcpy(uri_strtok[++index_end], uri_parse);
		if(!o2pt->isFopt && uri_parse && !strcmp(uri_parse, "fopt")){
			o2pt->isFopt = true;
		}
		if(uri_parse && o2pt->isFopt){
			fopt_cnt++;
		}
		uri_parse = strtok(NULL, "/");
		
	}

	int latest_oldest_flag = -1;
	if(o2pt->isFopt)
	{	
		index_end = index_end - 1 - fopt_cnt;
		if(fopt_cnt){

			fopt_buf = calloc(1, 256);
			for(int i = 0 ; i < fopt_cnt ; i++){
				strcat(fopt_buf, "/");
				strcat(fopt_buf, uri_strtok[i+index_end + 2]); //index end before fopt so +2
			}
			o2pt->fopt = strdup(fopt_buf);
			
			free(fopt_buf);
		}
	}else{

		if(!strcmp(uri_strtok[0], "viewer")) index_start++;
		if(!strcmp(uri_strtok[index_end], "la") || !strcmp(uri_strtok[index_end], "latest")) {
			latest_oldest_flag = 0; index_end--;
			
		} else if(!strcmp(uri_strtok[index_end], "ol") || !strcmp(uri_strtok[index_end], "oldest")) {
			latest_oldest_flag = 1; index_end--;
			
		}
	}

	strcpy(uri_array, "\0");
	for(int i=index_start; i<=index_end; i++) {
		strcat(uri_array,"/"); 
		strcat(uri_array,uri_strtok[i]);
	}

	RTNode * rtnode = NULL;
	if(o2pt->isForwarding){
		rtnode = find_csr_rtnode_by_uri(cb, uri_array);
	}else{
		rtnode = find_rtnode_by_uri(cb, uri_array);
	}

	
	if(rtnode && !o2pt->isFopt && latest_oldest_flag != -1) rtnode = find_latest_oldest(rtnode, latest_oldest_flag);

	if(index_start == 1) o2pt->op = OP_VIEWER;

	return rtnode;
}

RTNode *find_csr_rtnode_by_uri(RTNode *cb, char *uri){
	RTNode *rtnode = cb->child, *parent_rtnode = NULL;
	char *target_uri = strtok(uri+3, "/");
	target_uri -= 1;

	if(!target_uri) return NULL;


	logger("O2M", LOG_LEVEL_DEBUG, "target_uri : %s", target_uri);


	while(rtnode) {
		if(rtnode->ty != RT_CSR){
			rtnode = rtnode->sibling_right;
			continue;
		}
		cJSON *csi = cJSON_GetObjectItem(rtnode->obj, "csi");
		if(rtnode->obj && !strcmp(cJSON_GetStringValue(csi), target_uri)) break;
		rtnode = rtnode->sibling_right;
	}
	
	return rtnode;
}

RTNode *find_rtnode_by_uri(RTNode *cb, char *uri) {
	RTNode *rtnode = cb, *parent_rtnode = NULL;
	char *target_uri = strdup(uri);
	char *ptr = strtok(target_uri, "/");
	if(!ptr) return NULL;

	char ri[64];
	strcpy(ri, ptr);

	char uri_array[64][MAX_URI_SIZE];
	int index = -1;

	while(ptr) {
		strcpy(uri_array[++index], ptr);
		ptr = strtok(NULL, "/");
	}

	free(target_uri);

	for(int i=0; i<=index; i++) {
		while(rtnode) {
			if(!strcmp(get_rn_rtnode(rtnode), uri_array[i])) break;
			rtnode = rtnode->sibling_right;
		}
		if(i == index-1) parent_rtnode = rtnode;
		if(!rtnode) break;
		if(i != index) rtnode = rtnode->child;
	}
	if(rtnode) return rtnode;

	if(parent_rtnode) {
		cJSON *cin = db_get_resource(uri_array[index], RT_CIN);
		//CIN *cin = db_get_cin(uri_array[index]);
		if(cin) {
			cJSON *pi = cJSON_GetObjectItem(cin, "pi");
			if(!strcmp(cJSON_GetStringValue(pi), get_ri_rtnode(parent_rtnode))) {
				rtnode = create_rtnode(cin, RT_CIN);
				rtnode->parent = parent_rtnode;
			}
		}
	}

	if(!rtnode) {
		rtnode = find_rtnode_by_ri(cb, ri);
	}

	return rtnode;
}

RTNode *find_rtnode_by_ri(RTNode *cb, char *ri){
	RTNode *rtnode = cb;
	RTNode *ret = NULL;

	while(rtnode) {
		if(!strcmp(get_ri_rtnode(rtnode), ri)) {
			return rtnode;
		}
		
		ret = find_rtnode_by_ri(rtnode->child, ri);
		if(ret) {
			return ret;
		}
		rtnode = rtnode->sibling_right;
	}

	return ret;
}

char *ri_to_uri(char *ri){
	char *uri = NULL;
	RTNode *rtnode = find_rtnode_by_ri(rt->cb, ri);

	if(rtnode){
		uri = rtnode->uri;
	}
	
	return uri;
}


RTNode *find_latest_oldest(RTNode* rtnode, int flag) {
	logger("UTL", LOG_LEVEL_DEBUG, "latest oldest");
	RTNode *cin_rtnode = NULL;

	if(rtnode->ty == RT_CNT) {
		cJSON *cin_object = db_get_cin_laol(rtnode, flag);
		cin_rtnode = create_rtnode(cin_object, RT_CIN);
		cin_rtnode->parent = rtnode;
	}

	return cin_rtnode;
}

int net_to_bit(cJSON *net) {
	int ret = 0;
	cJSON *pjson = NULL;

	cJSON_ArrayForEach(pjson, net){
		int exp = atoi(pjson->valuestring);
		if(exp > 0) ret = (ret | (int)pow(2, exp - 1));
	}

	return ret;
}

int add_rr_list(RTNode *rtnode){

}

int add_child_resource_tree(RTNode *parent, RTNode *child) {
	RTNode *node = parent->child;
	child->parent = parent;

	char *uri = malloc(strlen(parent->uri) + strlen(get_rn_rtnode(child)) + 2);
	strcpy(uri, parent->uri);
	strcat(uri, "/");
	strcat(uri, get_rn_rtnode(child));
	child->uri = uri;

	logger("O2M", LOG_LEVEL_DEBUG, "Add Resource Tree Node [Parent-ID] : %s, [Child-ID] : %s",get_ri_rtnode(parent), get_ri_rtnode(child));
	
	if(!node) {
		parent->child = child;
	} else if(node) {
		while(node->sibling_right && node->sibling_right->ty <= child->ty) { 	
				node = node->sibling_right;
		}

		if(parent->child == node && child->ty < node->ty) {
			parent->child = child;
			child->parent = parent;
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

ResourceType http_parse_object_type(header_t *headers) {
	char *content_type = search_header(headers, "Content-Type");
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
	case 16: ty = RT_CSR; break;
	case 23 : ty = RT_SUB; break;
	default : ty = RT_MIXED; break;
	}
	
	return ty;
}


char *get_local_time(int diff) {
	time_t t = time(NULL) - diff;
	struct tm tm = *localtime(&t);
	struct timespec specific_time;
    //int millsec;
    clock_gettime(0, &specific_time);
	
	char year[5], mon[3], day[3], hour[3], minute[3], sec[3], millsec[7]; 
	
	sprintf(year,"%d", tm.tm_year+1900);
	sprintf(mon,"%02d",tm.tm_mon+1);
	sprintf(day,"%02d",tm.tm_mday);
	sprintf(hour,"%02d",tm.tm_hour);
	sprintf(minute,"%02d",tm.tm_min);
	sprintf(sec,"%02d",tm.tm_sec);
	sprintf(millsec, "%06d", (int) floor(specific_time.tv_nsec/1.0e6));
	
	char* local_time = (char*)malloc(24 * sizeof(char));
	
	*local_time = '\0';
	strcat(local_time,year);
	strcat(local_time,mon);
	strcat(local_time,day);
	strcat(local_time,"T");
	strcat(local_time,hour);
	strcat(local_time,minute);
	strcat(local_time,sec);
	strcat(local_time,millsec);
	
	return local_time;
}

char *get_resource_key(ResourceType ty){
	char *key = NULL;
	switch(ty){
		case RT_CSE:
			key = "m2m:cb";
			break;
		case RT_AE:
			key = "m2m:ae";
			break;
		case RT_CNT:
			key = "m2m:cnt";
			break;
		case RT_CIN:
			key = "m2m:cin";
			break;
		case RT_SUB:
			key = "m2m:sub";
			break;
		case RT_GRP:
			key = "m2m:grp";
			break;
		case RT_ACP:
			key = "m2m:acp";
			break;
		case RT_CSR:
			key = "m2m:csr";
			break;
		case RT_CBA:
			key = "m2m:cbA";
			break;
		case RT_AEA:
			key = "m2m:aeA";
			break;
		default:
			key = "general";
			break;
	}
	return key;
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
	else if(cJSON_GetObjectItem(cjson, "m2m:csr")) ty = RT_CSR;
	else ty = RT_MIXED;
	
	return ty;
}

char *resource_identifier(ResourceType ty, char *ct) {
	char *ri = (char *)calloc(32, sizeof(char));

	switch(ty) {
		case RT_AE : strcpy(ri, "CAE"); break;
		case RT_CNT : strcpy(ri, "3-"); break;
		case RT_CIN : strcpy(ri, "4-"); break;
		case RT_SUB : strcpy(ri, "23-"); break;
		case RT_ACP : strcpy(ri, "1-"); break;
		case RT_GRP : strcpy(ri, "9-"); break;
		case RT_CSR : strcpy(ri, "16-"); break; 
	}

	// struct timespec specific_time;
    // int millsec;
	static int n = 0;

	char buf[32] = "\0";

    // clock_gettime(CLOCK_REALTIME, &specific_time);
    // millsec = floor(specific_time.tv_nsec/1.0e6);

	sprintf(buf, "%s%04d",ct, n);
	n = (n + 1) % 10000;

	strcat(ri, buf);

	return ri;
}

void delete_cin_under_cnt_mni_mbs(RTNode *rtnode) {
	logger("UTIL", LOG_LEVEL_DEBUG, "call delete_cin_under_cnt_mni_mbs");
	cJSON *cnt = rtnode->obj;
	cJSON *cni_obj = NULL;
	cJSON *cbs_obj = NULL;
	cJSON *mni_obj = NULL;
	cJSON *mbs_obj = NULL;
	int cni, mni, cbs, mbs, tmp = 0;

	if(cni_obj = cJSON_GetObjectItem(cnt, "cni")) {
		cni = cni_obj->valueint;
	}
	if(mni_obj = cJSON_GetObjectItem(cnt, "mni")) {
		mni = mni_obj->valueint;
	}else{
		mni = DEFAULT_MAX_NR_INSTANCES;
	}
	if(cbs_obj = cJSON_GetObjectItem(cnt, "cbs")) {
		cbs = cbs_obj->valueint;
	}
	if(mbs_obj = cJSON_GetObjectItem(cnt, "mbs")) {
		mbs = mbs_obj->valueint;
	}else{
		mbs = DEFAULT_MAX_BYTE_SIZE;
	}
	if(cni <= mni && cbs <= mbs) return;

	if(cni == mni+1){
		tmp = db_delete_one_cin_mni(rtnode);
		if(tmp == -1){
			logger("UTIL", LOG_LEVEL_ERROR, "delete_cin_under_cnt_mni_mbs error");
		}
		cbs -= tmp;
		cni--;
	}else{
		RTNode *head = db_get_cin_rtnode_list(rtnode);
		RTNode *right;

		while((mni >= 0 && cni > mni) || (mbs >= 0 && cbs > mbs)) {
			if(head) {
				right = head->sibling_right;
				db_delete_onem2m_resource(head);
				cbs -= cJSON_GetObjectItem(head->obj, "cs")->valueint;
				cni--;
				free_rtnode(head);
				head = right;
			} else {
				break;
			}
		}
		if(head) free_rtnode_list(head);
	}


	if( cni_obj->valueint != cni){
		cJSON_SetIntValue(cni_obj, cni);
	}
	if( cbs_obj->valueint != cbs ){
		cJSON_SetIntValue(cbs_obj, cbs);
	}
}

int tree_viewer_api(oneM2MPrimitive *o2pt, RTNode *node) {
	logger("O2M", LOG_LEVEL_DEBUG, "\x1b[43mTree Viewer API\x1b[0m\n");
	char arr_viewer_data[MAX_TREE_VIEWER_SIZE] = "[";
	char *viewer_data = arr_viewer_data;
	
	RTNode *p = node;
	while(p = p->parent) {
		char *json = rtnode_to_json(p);
		strcat(viewer_data,",");
		strcat(viewer_data,json);
		free(json); json = NULL;
	}
	
	int cinSize = 1;
	
	//char *la = strstr(qs,"la=");
	//if(la) cinSize = atoi(la+3);

	cinSize = cJSON_GetNumberValue(cJSON_GetObjectItem(o2pt->fc, "la"));
	
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
}

void tree_viewer_data(RTNode *rtnode, char **viewer_data, int cin_size) {
	char *json = rtnode_to_json(rtnode);

	strcat(*viewer_data, ",");
	strcat(*viewer_data, json);
	free(json); json = NULL;

	RTNode *child = rtnode->child;
	while(child) {
		tree_viewer_data(child, viewer_data, cin_size);
		child = child->sibling_right;
	}

	if(rtnode->ty == RT_CNT) {
		RTNode *cin_list_head = db_get_cin_rtnode_list(rtnode);

		if(cin_list_head) cin_list_head = latest_cin_list(cin_list_head, cin_size);

		RTNode *cin = cin_list_head;

		while(cin) {
			json = rtnode_to_json(cin);
			strcat(*viewer_data, ",");
			strcat(*viewer_data, json);
			free(json); json = NULL;
			cin = cin->sibling_right;		
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
	logger("UTIL", LOG_LEVEL_INFO, "Run time : %lf", end-start);
}


void init_server() {
	char poa[128] = {0};

	rt = (ResourceTree *)calloc(1, sizeof(rt));
	
	cJSON *cse;

	cse = db_get_resource(CSE_BASE_RI, RT_CSE);
	

	if(!cse){
		cse = cJSON_CreateObject();
		init_cse(cse);
		db_store_resource(cse, CSE_BASE_NAME);
	}else{
		cJSON *rr = cJSON_GetObjectItem(cse, "rr");
		if(rr->valueint == 1)
			rr->type = cJSON_True;
		else
			rr->type = cJSON_False;
			// cJSON_SetBoolValue(cJSON_GetObjectItem(cse, "rr"), false);
	}

	#ifdef NIC_NAME
	struct ifreq ifr;
	char ipstr[40];
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, NIC_NAME, IFNAMSIZ);

	if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
		printf("Error");
	} else {
		inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2,
				ipstr,sizeof(struct sockaddr));
	}
	

	logger("UTIL", LOG_LEVEL_INFO, "Server IP : %s", ipstr);
	close(s);
 
	sprintf(poa, "http://%s:%s", ipstr, SERVER_PORT);
	#else
	sprintf(poa, "http://%s:%s", SERVER_IP, SERVER_PORT);
	#endif

	cJSON *poa_obj = cJSON_CreateArray();
	cJSON_AddItemToArray(poa_obj, cJSON_CreateString(poa));
	cJSON_AddItemToObject(cse, "poa", poa_obj);

	rt->cb = create_rtnode(cse, RT_CSE);
	// rt->cb->uri = CSE_BASE_NAME;

 	init_resource_tree();
}

void init_resource_tree(){
	RTNode *rtnode_list = (RTNode *)calloc(1,sizeof(RTNode));
	RTNode *tail = rtnode_list;

	RTNode* csr_list = db_get_all_csr_rtnode();
	tail->sibling_right = csr_list;
	if(csr_list) csr_list->sibling_left = tail;
	while(tail->sibling_right) tail = tail->sibling_right;
	
	RTNode* ae_list = db_get_all_ae_rtnode();
	tail->sibling_right = ae_list;
	if(ae_list) ae_list->sibling_left = tail;
	while(tail->sibling_right) tail = tail->sibling_right;

	RTNode* cnt_list = db_get_all_cnt_rtnode();
	tail->sibling_right = cnt_list;
	if(cnt_list) cnt_list->sibling_left = tail;
	while(tail->sibling_right) tail = tail->sibling_right;
	

	RTNode* sub_list = db_get_all_sub_rtnode();
	tail->sibling_right = sub_list;
	if(sub_list) sub_list->sibling_left = tail;
	while(tail->sibling_right) tail = tail->sibling_right;


	RTNode* acp_list = db_get_all_acp_rtnode();
	tail->sibling_right = acp_list;
	if(acp_list) acp_list->sibling_left = tail;
	while(tail->sibling_right) tail = tail->sibling_right;

	RTNode* grp_list = db_get_all_grp_rtnode();
	tail->sibling_right = grp_list;
	if(grp_list) grp_list->sibling_left = tail;
	while(tail->sibling_right) tail = tail->sibling_right;
	
	RTNode *temp = rtnode_list;
	rtnode_list = rtnode_list->sibling_right;
	if(rtnode_list) rtnode_list->sibling_left = NULL;
	free_rtnode(temp);
	temp = NULL;
	if(rtnode_list) restruct_resource_tree(rt->cb, rtnode_list);
}

RTNode* restruct_resource_tree(RTNode *parent_rtnode, RTNode *list) {
	RTNode *rtnode = list;
	while(rtnode) {
		logger("UTIL", LOG_LEVEL_DEBUG, "restruct_resource_tree : %s", get_ri_rtnode(rtnode));
		RTNode *right = rtnode->sibling_right;
		if(!strcmp(get_ri_rtnode(parent_rtnode), get_pi_rtnode(rtnode))) {
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
		list = restruct_resource_tree(child, list);
		child = child->sibling_right;
	}
	
	return list;
}

int check_payload_size(oneM2MPrimitive *o2pt) {
	if(o2pt->pc && strlen(o2pt->pc) > MAX_PAYLOAD_SIZE) {
		handle_error(o2pt, RSC_BAD_REQUEST, "payload is too large");
		return -1;
	}
	return 0;
}

int result_parse_uri(oneM2MPrimitive *o2pt, RTNode *rtnode) {
	if(!rtnode) {
		handle_error(o2pt, RSC_NOT_FOUND, "URI is invalid");
		return -1;
	} else {
		return 0;
	} 
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
		return false;
	} else if((parent_rtnode && strcmp(o2pt->fr, get_ri_rtnode(parent_rtnode)))) {
		deny = true;
	}
  
	if(rtnode->ty == RT_CIN) rtnode = rtnode->parent;

	if(rtnode->ty != RT_CSE) {
		if(get_acpi_rtnode(rtnode) || rtnode->ty == RT_ACP) {
			deny = true;
			if((get_acop(o2pt, rtnode) & acop) == acop) {
				deny = false;
			}
		}
	}

	if(deny) {
		handle_error(o2pt, RSC_ORIGINATOR_HAS_NO_PRIVILEGE, "originator has no privilege");
		return -1;
	}

	return false;
}

int check_macp_privilege(oneM2MPrimitive *o2pt, RTNode *rtnode, ACOP acop){
	bool deny = false;
	if(!o2pt->fr) {
		deny = true;
	} else if(!strcmp(o2pt->fr, "CAdmin")) {
		return false;
	} else if(strcmp(o2pt->fr, get_ri_rtnode(rtnode))) {
		deny = true;
	}

	cJSON *macp = cJSON_GetObjectItem(rtnode->obj, "macp");
	if(macp && cJSON_GetArraySize(macp) > 0) {
		deny = true;
		if((get_acop_macp(o2pt, rtnode) & acop) == acop) {
			deny = false;
		}
	}else{
		deny = false;
	}

	if(deny) {
		handle_error(o2pt, RSC_ORIGINATOR_HAS_NO_PRIVILEGE, "originator has no privilege");
		return -1;
	}

	return 0;
}

int get_acop(oneM2MPrimitive *o2pt, RTNode *rtnode) {
	int acop = 0;
	logger("UTIL", LOG_LEVEL_DEBUG, "get_acop : %s", o2pt->fr);
	char *origin = NULL;
	if(!o2pt->fr) {
		origin = strdup("all");
	}else{
		origin = strdup(o2pt->fr);
	}

    if(!strcmp(origin, "CAdmin")) {
		free(origin);
		return ALL_ACOP;
	}

	if(rtnode->ty == RT_ACP) {
		acop = (acop | get_acop_origin(o2pt, rtnode, 1));
		free(origin);
		return acop;
	}

	cJSON *acpiArr = get_acpi_rtnode(rtnode);
    if(!acpiArr) return 0;
	
	RTNode *cb = rtnode;
	while(cb->parent) cb = cb->parent;
	cJSON *acpi = NULL;
	cJSON_ArrayForEach(acpi, acpiArr) {
		RTNode *acp = find_rtnode_by_uri(cb, acpi->valuestring);
		if(acp) {
			acop = (acop | get_acop_origin(o2pt, acp, 0));
		}
	}

	free(origin);
	return acop;
}

int get_acop_macp(oneM2MPrimitive *o2pt, RTNode *rtnode){
	int acop = 0;
	logger("UTIL", LOG_LEVEL_DEBUG, "get_acop_macp : %s", o2pt->fr);
	char *origin = NULL;
	if(!o2pt->fr) {
		origin = strdup("all");
	}else{
		origin = strdup(o2pt->fr);
	}

	if(!strcmp(origin, "CAdmin")) {
		free(origin);
		return ALL_ACOP;
	}

	cJSON *macp = cJSON_GetObjectItem(rtnode->obj, "macp");
	if(!macp) return 0;

	RTNode *cb = rtnode;
	while(cb->parent) cb = cb->parent;
	cJSON *acpi = NULL;
	cJSON_ArrayForEach(acpi, macp) {
		
		RTNode *acp = find_rtnode_by_uri(cb, acpi->valuestring);
		if(acp) {
			acop = (acop | get_acop_origin(o2pt, acp, 0));
		}
	}

	free(origin);
	return acop;
}

int check_acco(cJSON *acco, char *ip){
	if(!acco) return 1;
	if(!ip) return 1;

	cJSON *acip = cJSON_GetObjectItem(acco, "acip");
	cJSON *ipv4 = cJSON_GetObjectItem(acip, "ipv4");
	cJSON *pjson = NULL;
	char *ip_str = NULL;
	int res = 0;
	struct in_addr addr, addr2;
	char *subnet_ptr;
	int mask = 0xFFFFFFFF;

	cJSON_ArrayForEach(pjson, ipv4){
		ip_str = strdup(pjson->valuestring);
		mask = 1;

		subnet_ptr = strchr(ip_str, '/');

		if(subnet_ptr){
			subnet_ptr[0] = '\0';
			subnet_ptr++;
			for(int i = 0 ; i < atoi(subnet_ptr) - 1 ; i++){
				mask = mask << 1;
				mask = mask | 1;
			}


			res = inet_pton(AF_INET, ip_str, &addr);
			if(res == 0){
				logger("UTIL", LOG_LEVEL_DEBUG, "check_acco/ipv4 : %s is not valid ipv4 address", ip_str);
				continue;
			}else if(res == -1){
				logger("UTIL", LOG_LEVEL_DEBUG, "check_acco/ipv4 : inet_pton error");
				continue;
			}

			res = inet_pton(AF_INET, ip, &addr2);
			if(res == 0){
				logger("UTIL", LOG_LEVEL_DEBUG, "check_acco/ipv4 : %s is not valid ipv4 address", ip);
				continue;
			}else if(res == -1){
				logger("UTIL", LOG_LEVEL_DEBUG, "check_acco/ipv4 : inet_pton error");
				continue;
			}
			logger("UTIL", LOG_LEVEL_DEBUG, "addr & mask : %X, addr2 & mask : %X", (addr.s_addr & mask), (addr2.s_addr & mask));

			if((addr.s_addr & mask) == (addr2.s_addr & mask)){
				free(ip_str);
				return 1;
			}
		}else{
			if(!strcmp(ip_str, ip)) {
				free(ip_str);
				return 1;
			}
		}
		
		free(ip_str);
	}
	return 0;
}

int get_acop_origin(oneM2MPrimitive *o2pt, RTNode *acp_rtnode, int flag) {
	int ret_acop = 0, cnt = 0;
	cJSON *acp = acp_rtnode->obj;
	
	cJSON *privilege = NULL;
	cJSON *acr = NULL;
	cJSON *acor = NULL;
	bool found = false;
	char *origin = NULL;

	if(!o2pt->fr) {
		origin = strdup("all");
	}else{
		origin = strdup(o2pt->fr);
	}

	if(flag){
		privilege = cJSON_GetObjectItem(acp, "pvs");
	}else{
		privilege = cJSON_GetObjectItem(acp, "pv");
	}

	cJSON_ArrayForEach(acr, cJSON_GetObjectItem(privilege, "acr")){
		cJSON_ArrayForEach(acor, cJSON_GetObjectItem(acr, "acor")){
			if(acor->valuestring[strlen(acor->valuestring)-1] == '*') {
				if(!strncmp(acor->valuestring, origin, strlen(acor->valuestring)-1)) {
					if(check_acco(cJSON_GetObjectItem(acr, "acco"), o2pt->ip)){
						ret_acop = (ret_acop | cJSON_GetObjectItem(acr, "acop")->valueint);
					}
					break;
				}
			} else if(!strcmp(acor->valuestring, origin)) {
				if(check_acco(cJSON_GetObjectItem(acr, "acco"), o2pt->ip)){
					ret_acop = (ret_acop | cJSON_GetObjectItem(acr, "acop")->valueint);
				}
				break;
			}else if(!strcmp(acor->valuestring, "all")){
				if(check_acco(cJSON_GetObjectItem(acr, "acco"), o2pt->ip)){
					ret_acop = (ret_acop | cJSON_GetObjectItem(acr, "acop")->valueint);
				}
				break;
			}
		}
	}
	free(origin);
	return ret_acop;
}

int has_privilege(oneM2MPrimitive *o2pt, char *acpi, ACOP acop) {
	char *origin = o2pt->fr;
	if(!origin) return 0;
	if(!acpi) return 1;

	RTNode *acp = find_rtnode_by_uri(rt->cb, acpi);
	int result = get_acop_origin(o2pt, acp, 0);
	if( (result & acop) == acop) {
		return 1;
	}
	return 0;
}

int check_rn_duplicate(oneM2MPrimitive *o2pt, RTNode *rtnode) {
	if(!rtnode) return 0;
	cJSON *root = o2pt->cjson_pc;
	cJSON *resource, *rn;

	resource = getResource(root, o2pt->ty);

	RTNode *child = rtnode->child;
    bool flag = false;

	rn = cJSON_GetObjectItem(resource, "rn");
	if(rn) {
		char *resource_name = rn->valuestring;
		while(child) {
			if(get_rn_rtnode(child) && !strcmp(get_rn_rtnode(child), resource_name)) {
                flag = true; 
                break;
			}
			child = child->sibling_right;
		}
        if(rtnode->ty == RT_CNT) {
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
		handle_error(o2pt, RSC_CONFLICT, "attribute `rn` is duplicated");
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
		if(child->ty != RT_AE){
			child = child->sibling_right;
			continue;
		}
		if(!strcmp(get_ri_rtnode(child), aei)) {
			handle_error(o2pt, RSC_ORIGINATOR_HAS_ALREADY_REGISTERD, "attribute `aei` is duplicated");
			return -1;
		}
		child = child->sibling_right;
	}

	return 0;
}

int check_csi_duplicate(char *new_csi, RTNode *rtnode) {
	if(!rtnode || new_csi == NULL) return 0;

	RTNode *child = rtnode->child;

	while(child) {
		if(!strcmp(get_ri_rtnode(child), new_csi)) {
			return -1;
		}
		child = child->sibling_right;
	}

	return 0;
}

int check_aei_invalid(oneM2MPrimitive *o2pt) {
	char *aei = o2pt->fr;
	if(!aei || strlen(aei) == 0) return 0;
	cJSON *cjson = string_to_cjson_string_list_item(ALLOW_AE_ORIGIN);

	int size = cJSON_GetArraySize(cjson);
	for(int i=0; i<size; i++) {
		cJSON *item = cJSON_GetArrayItem(cjson, i);
		char *origin = strdup(item->valuestring);
		if(origin[strlen(origin)-1] == '*') {
			if(!strncmp(aei, origin, strlen(origin)-1)) {
				cJSON_Delete(cjson);
				free(origin);
				return 0;
			}
		} else if(!strcmp(origin, aei)){
			cJSON_Delete(cjson);
			free(origin);
			return 0;
		} 

		free(origin); origin = NULL;
	}
	o2pt->rsc = RSC_BAD_REQUEST;
	cJSON_Delete(cjson);
	if(o2pt->pc) free(o2pt->pc);
	o2pt->pc = strdup("{\"m2m:dbg\":\"originator is invalid\"}");
	return -1;
}

int check_payload_format(oneM2MPrimitive *o2pt) {
	cJSON *cjson = o2pt->cjson_pc;
	
	if(cjson == NULL) {
		handle_error(o2pt, RSC_BAD_REQUEST, "payload format is invalid");
		return -1;
	}
	return 0;
}

int check_payload_empty(oneM2MPrimitive *o2pt) {
	if(!o2pt->pc) {
		handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "payload is empty");
		return -1;
	}
	return 0;
}

int check_rn_invalid(oneM2MPrimitive *o2pt, ResourceType ty) {
	cJSON *root = o2pt->cjson_pc;
	cJSON *resource, *rn;

	resource = getResource(root, ty);

	rn = cJSON_GetObjectItem(resource, "rn");
	if(!rn) return 0;
	char *resource_name = rn->valuestring;
	int len_resource_name = strlen(resource_name);

	for(int i=0; i<len_resource_name; i++) {
		if(!is_rn_valid_char(resource_name[i])) {
			handle_error(o2pt, RSC_BAD_REQUEST, "attribute `rn` is invalid");
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
		handle_error(o2pt, RSC_BAD_REQUEST, "resource type is invalid");
		return -1;
	}
	return 0;
}

int check_resource_type_invalid(oneM2MPrimitive *o2pt) {
	if(o2pt->ty == RT_MIXED) {
		handle_error(o2pt, RSC_BAD_REQUEST, "resource type is invalid");
		return -1;
	}
	return 0;
}

/**
 * @brief set rsc and error msg to oneM2MPrimitive
 * @param o2pt oneM2MPrimitive
 * @param rsc error code
 * @param err error message
 * @return error code
*/
int handle_error(oneM2MPrimitive *o2pt, int rsc, char *err){
	logger("UTIL", LOG_LEVEL_ERROR, err);
	o2pt->rsc = rsc;
	o2pt->errFlag = true;
	char pc[MAX_PAYLOAD_SIZE];
	sprintf(pc, "{\"m2m:dbg\": \"%s\"}", err);
	set_o2pt_pc(o2pt, pc);
	return rsc;
}

bool isUriFopt(char *str){
	char *s;
	if(!str) return false;
	s = strrchr(str, '/');
	if(!s) return false;
	return !strcmp(s, "/fopt");
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

int validate_grp(oneM2MPrimitive *o2pt, cJSON *grp){
	logger("UTIL", LOG_LEVEL_DEBUG, "Validating GRP");
	int rsc = 0;
	bool hasFopt = false;
	bool isLocalResource = true;
	char *p = NULL;
	char *tStr = NULL;
	cJSON *pjson = NULL;

	int mt = 0;
	int csy = DEFAULT_CONSISTENCY_POLICY;
	
	if(pjson = cJSON_GetObjectItem(grp, "mt")){
		mt = pjson->valueint;
	}else{
		handle_error(o2pt, RSC_BAD_REQUEST, "`mt` is mandatory");
		return RSC_BAD_REQUEST;
	}


	if(pjson = cJSON_GetObjectItem(grp, "csy")){
		csy = pjson->valueint;
		if(csy < 0 || csy > 3){
			handle_error(o2pt, RSC_BAD_REQUEST, "`csy` is invalid");
			return RSC_BAD_REQUEST;
		}
	}

	RTNode *rt_node;

	if(pjson = cJSON_GetObjectItem(grp, "mtv")){
		if(pjson->valueint == 1)
			return RSC_OK;
	}



	cJSON *midArr = cJSON_GetObjectItem(grp, "mid");
	cJSON *mid_obj = NULL;
	
	if(!midArr){
		handle_error(o2pt, RSC_BAD_REQUEST, "`mid` is mandatory");
		return RSC_BAD_REQUEST;
	}

	if(midArr && !cJSON_IsArray(midArr)){
		handle_error(o2pt, RSC_BAD_REQUEST, "`mid` should be array");
		return RSC_BAD_REQUEST;
	}

	if(pjson = cJSON_GetObjectItem(grp, "mnm")){
		
		if(pjson->valueint < cJSON_GetArraySize(midArr)){
			handle_error(o2pt, RSC_BAD_REQUEST, "`mnm` is less than `mid` size");
			return RSC_BAD_REQUEST;
		}
	}

	// validate macp
	if(pjson = cJSON_GetObjectItem(grp, "macp")){
		if(!cJSON_IsArray(pjson)){
			handle_error(o2pt, RSC_BAD_REQUEST, "`macp` should be array");
			return RSC_BAD_REQUEST;
		}else if(cJSON_GetArraySize(pjson) == 0){
			handle_error(o2pt, RSC_BAD_REQUEST, "`macp` should not be empty");
		}else{
			if( validate_acpi(o2pt, pjson, OP_CREATE)  != RSC_OK )
				return o2pt->rsc;
		}
	}

	// member id check
	int i = 0;
	bool dup = false;
	cJSON_ArrayForEach(mid_obj, midArr){
		char *mid = cJSON_GetStringValue(mid_obj);

		for(int j = 0 ; j < i ; j++){
			if(cJSON_GetArrayItem(midArr, j) && !strcmp(mid, cJSON_GetStringValue(cJSON_GetArrayItem(midArr, j)))){
				dup = true;
				cJSON_DeleteItemFromArray(midArr, i);
				break;
			}
		}

		if(dup){ 
			dup = false;
			continue;
		}

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
			hasFopt = isUriFopt(mid);
			tStr = strdup(mid);
			if(hasFopt && strlen(mid) > 5)  // remove fopt 
				tStr[strlen(mid) - 5] = '\0';
			logger("util-t", LOG_LEVEL_DEBUG, "%s",tStr);

			if((rt_node = find_rtnode_by_uri(rt->cb, tStr))){
				if(mt != 0 && rt_node->ty != mt)
					if(handle_csy(grp, mid_obj, csy, i)) return RSC_GROUPMEMBER_TYPE_INCONSISTENT;
			}else{
				logger("UTIL", LOG_LEVEL_DEBUG, "GRP member %s not present", tStr);
				if(handle_csy(grp, mid_obj, csy, i)) return RSC_GROUPMEMBER_TYPE_INCONSISTENT;
			}

			if(rt_node && rt_node->ty == RT_GRP){
				//pGrp = db_get_grp(rt_node->ri);
				cJSON *pGrp = rt_node->obj;
				if((rsc = validate_grp(o2pt, pGrp)) >= 4000 ){
					if(handle_csy(grp, mid_obj, csy, i)) return RSC_GROUPMEMBER_TYPE_INCONSISTENT;
				}
				//free_grp(pGrp);
			}

			free(tStr);
			tStr = NULL;
		}else{
			return RSC_NOT_IMPLEMENTED;
		}
		i++;
	}
	return RSC_OK;
}

int validate_grp_update(oneM2MPrimitive *o2pt, cJSON *grp_old, cJSON *grp_new){
	logger("UTIL", LOG_LEVEL_DEBUG, "Validating GRP Update");
	int rsc = 0;
	bool hasFopt = false;
	bool isLocalResource = true;
	char *p = NULL;
	char *tStr = NULL;
	cJSON *pjson = NULL;

	int mt = 0;
	int csy = DEFAULT_CONSISTENCY_POLICY;

	if(pjson = cJSON_GetObjectItem(grp_new, "mt")){
		mt = pjson->valueint;
	}

	if(pjson = cJSON_GetObjectItem(grp_new, "csy")){
		csy = pjson->valueint;
		if(csy < 0 || csy > 3){
			handle_error(o2pt, RSC_BAD_REQUEST, "`csy` is invalid");
			return RSC_BAD_REQUEST;
		}
	}

	cJSON *midArr = cJSON_GetObjectItem(grp_new, "mid");
	cJSON *mid_obj = NULL;
	if(midArr && !cJSON_IsArray(midArr)){
		handle_error(o2pt, RSC_BAD_REQUEST, "`mid` should be array");
		return RSC_BAD_REQUEST;
	}


	if(pjson = cJSON_GetObjectItem(grp_new, "mnm")){
		if(pjson->valueint < cJSON_GetArraySize(midArr)){
			handle_error(o2pt, RSC_BAD_REQUEST, "`mnm` is less than `mid` size");
			return RSC_BAD_REQUEST;
		}

		if(pjson->valueint < cJSON_GetObjectItem(grp_old, "cnm")->valueint){
			handle_error(o2pt, RSC_BAD_REQUEST, "`mnm` can't be smaller than `cnm` size");
			return RSC_BAD_REQUEST;
		}
	}
	else if(pjson = cJSON_GetObjectItem(grp_old, "mnm")){
		if(pjson->valueint < cJSON_GetArraySize(midArr)){
			handle_error(o2pt, RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED, "`mnm` is less than `mid` size");
			return RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED;
		}
	}

	if(pjson = cJSON_GetObjectItem(grp_new, "macp")){
		if(validate_acpi(o2pt, pjson, OP_UPDATE) != RSC_OK)
		return RSC_BAD_REQUEST;

		if(cJSON_GetArraySize(pjson) == 0){
			cJSON_DeleteItemFromObject(grp_new, "macp");
		}
	}

	RTNode *rt_node;

	// member id check
	int i = 0;
	bool dup = false;
	cJSON_ArrayForEach(mid_obj, midArr){
		char *mid = cJSON_GetStringValue(mid_obj);
		logger("UTIL", LOG_LEVEL_DEBUG, "mid : %s, %d", mid, i);

		for(int j = 0 ; j < i ; j++){
			if(!strcmp(mid, cJSON_GetStringValue(cJSON_GetArrayItem(midArr, j)))){
				logger("UTIL", LOG_LEVEL_DEBUG, "Duplicated");
				cJSON_DeleteItemFromArray(midArr, i);
				dup = true;
				break;
			}
		}

		if(dup){ 
			dup = false;
			if(i == cJSON_GetArraySize(midArr)) break;
			else continue;
		}

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
			hasFopt = isUriFopt(mid);
			tStr = strdup(mid);
			if(hasFopt && strlen(mid) > 5)  // remove fopt 
				tStr[strlen(mid) - 5] = '\0';

			if((rt_node = find_rtnode_by_uri(rt->cb, tStr))){
				if(mt != 0 && rt_node->ty != mt)
					if(handle_csy(grp_old, mid_obj, csy, i)) return RSC_GROUPMEMBER_TYPE_INCONSISTENT;
			}else{
				logger("UTIL", LOG_LEVEL_DEBUG, "GRP member %s not present", tStr);
				if(handle_csy(grp_old, mid_obj, csy, i)) return RSC_GROUPMEMBER_TYPE_INCONSISTENT;
			}

			if(rt_node && rt_node->ty == RT_GRP){
				//pGrp = db_get_grp(rt_node->ri);
				cJSON *pGrp = rt_node->obj;
				if((rsc = validate_grp(o2pt, pGrp)) >= 4000 ){
					if(handle_csy(grp_old, mid_obj, csy, i)) return RSC_GROUPMEMBER_TYPE_INCONSISTENT;
				}
				//free_grp(pGrp);
			}

			free(tStr);
			tStr = NULL;
		}else{
			return handle_error(o2pt, RSC_NOT_IMPLEMENTED, "remote resource is not supported");
		}
		i++;
	}
	return RSC_OK;
}

int handle_csy(cJSON *grp, cJSON *mid, int csy, int i){
	cJSON *mt = cJSON_GetObjectItem(grp, "mt");
	cJSON *cnm = cJSON_GetObjectItem(grp, "cnm");

	switch(csy){
		case CSY_ABANDON_MEMBER:
			cJSON_DeleteItemFromArray(mid, i);
			cJSON_SetNumberValue(cnm, cnm->valueint - 1);
			break;
		case CSY_ABANDON_GROUP:
			return RSC_GROUPMEMBER_TYPE_INCONSISTENT;

		case CSY_SET_MIXED:
			cJSON_SetNumberValue(mt, RT_MIXED);
			break;
	}
	return 0;
}

bool isMinDup(char **mid, int idx, char *new_mid){
	if(!mid) return true;
	if(!new_mid) return true;

	for(int i = 0 ; i < idx ; i++){
		if( mid[i] != 0 && !strcmp(mid[i], new_mid))
			return true;
	}
	return false;
}

void remove_mid(char **mid, int idx, int cnm){
	char *del = mid[idx];
	for(int i = idx ; i < cnm-1; i++){
		mid[i] = mid[i+1];
	}
	//if(mid[cnm-1]) free(mid[cnm-1]);
	mid[cnm-1] = NULL;
	if(idx != cnm-1) free(del);
	del = NULL;
}

int rsc_to_http_status(int rsc){
	switch(rsc){
		case RSC_OK:
		case RSC_UPDATED:
			return 200;

		case RSC_CREATED:
			return 201;

		case RSC_BAD_REQUEST:
		case RSC_ORIGINATOR_HAS_NO_PRIVILEGE:
		case RSC_NOT_FOUND:
		case RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED:
			return 400;
		
		case RSC_OPERATION_NOT_ALLOWED:
			return 403;

		case RSC_INTERNAL_SERVER_ERROR:
		case RSC_TARGET_NOT_REACHABLE:
			return 500;

		default:
			return 200;
	}
}


cJSON *o2pt_to_json(oneM2MPrimitive *o2pt){
	cJSON *json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "rsc", o2pt->rsc);
    cJSON_AddStringToObject(json, "rqi", o2pt->rqi);
    cJSON_AddStringToObject(json, "to", o2pt->to);    
    cJSON_AddStringToObject(json, "fr", o2pt->fr);
    cJSON_AddStringToObject(json, "rvi", o2pt->rvi);
    if(o2pt->pc) cJSON_AddStringToObject(json, "pc", o2pt->pc);
	if(o2pt->cnot){
		cJSON_AddNumberToObject(json, "cnst", CS_PARTIAL_CONTENT);
		cJSON_AddNumberToObject(json, "cnot", o2pt->cnot);
	}
    //if(o2pt->ty >= 0) cJSON_AddNumberToObject(json, "ty", o2pt->ty);
	
	return json;
}

void free_o2pt(oneM2MPrimitive *o2pt){
	if(o2pt->rqi)
		free(o2pt->rqi);
	if(o2pt->origin)
		free(o2pt->origin);
	if(o2pt->pc)
		free(o2pt->pc);
	if(o2pt->fr)
		free(o2pt->fr);
	if(o2pt->to)
		free(o2pt->to);
	if(o2pt->fc)
		cJSON_Delete(o2pt->fc);
	if(o2pt->fopt)
		free(o2pt->fopt);
	if(o2pt->rvi)
		free(o2pt->rvi);
	if(o2pt->ip)
		free(o2pt->ip);
	if(o2pt->cjson_pc)
		cJSON_Delete(o2pt->cjson_pc);
	memset(o2pt, 0, sizeof(o2pt));
	free(o2pt);
	o2pt = NULL;
}

void o2ptcpy(oneM2MPrimitive **dest, oneM2MPrimitive *src){
	if(src == NULL) return;

	(*dest) = (oneM2MPrimitive *) calloc(1, sizeof(oneM2MPrimitive));

	(*dest)->fr = strdup(src->fr);
	(*dest)->to = strdup(src->to);
	if(src->rqi) (*dest)->rqi = strdup(src->rqi);
	if(src->origin) (*dest)->origin = strdup(src->origin);
	if(src->pc) (*dest)->pc = strdup(src->pc);
	if(src->cjson_pc) (*dest)->cjson_pc = cJSON_Parse((*dest)->pc);
	if(src->rvi) (*dest)->rvi = strdup(src->rvi);
	if(src->fopt) (*dest)->fopt = strdup(src->fopt);
	(*dest)->ty = src->ty;
	(*dest)->op = src->op;
	(*dest)->isFopt = src->isFopt;
	(*dest)->rsc = src->rsc;
	(*dest)->cnot = src->cnot;
	(*dest)->fc = src->fc;
}


void free_all_resource(RTNode *rtnode){
	RTNode *del;
	while(rtnode) {
		del = rtnode;
		if(rtnode->child) free_all_resource(rtnode->child);
		rtnode = rtnode->sibling_right;
		free_rtnode(del);
		del = NULL;
	}
}

char *get_pi_rtnode(RTNode *rtnode) {
	cJSON *pi = cJSON_GetObjectItem(rtnode->obj, "pi");
	if(pi){
		return pi->valuestring;
	}else{
		return "";
	}
}

char *get_ri_rtnode(RTNode *rtnode) {
	cJSON *ri = cJSON_GetObjectItem(rtnode->obj, "ri");
	if(ri){
		return ri->valuestring;
	}else{
		return NULL;
	}
}

char *get_rn_rtnode(RTNode *rtnode) {
	cJSON *rn = cJSON_GetObjectItem(rtnode->obj, "rn");
	if(rn){
		return rn->valuestring;
	}else{
		return NULL;
	}
}

cJSON *get_acpi_rtnode(RTNode *rtnode) {
	cJSON *acpi = cJSON_GetObjectItem(rtnode->obj, "acpi");
	if(acpi){
		return acpi;
	}else{
		return NULL;
	}
}

char *get_ct_rtnode(RTNode *rtnode){
	cJSON *ct = cJSON_GetObjectItem(rtnode->obj, "ct");
	if(ct){
		return ct->valuestring;
	}else{
		return NULL;
	}
}

char *get_et_rtnode(RTNode *rtnode){
	cJSON *et = cJSON_GetObjectItem(rtnode->obj, "et");
	if(et){
		return et->valuestring;
	}else{
		return NULL;
	}
}

char *get_lt_rtnode(RTNode *rtnode){
	cJSON *lt = cJSON_GetObjectItem(rtnode->obj, "lt");
	if(lt){
		return lt->valuestring;
	}else{
		return NULL;
	}
}

char *get_uri_rtnode(RTNode *rtnode){
	return rtnode->uri;
}

cJSON *getResource(cJSON *root, ResourceType ty){
	switch(ty){
		case RT_CSE:
			return cJSON_GetObjectItem(root, "m2m:cse");
		case RT_AE:
			return cJSON_GetObjectItem(root, "m2m:ae");
		case RT_CNT:
			return cJSON_GetObjectItem(root, "m2m:cnt");
		case RT_CIN:
			return cJSON_GetObjectItem(root, "m2m:cin");
		case RT_ACP:
			return cJSON_GetObjectItem(root, "m2m:acp");
		case RT_SUB:
			return cJSON_GetObjectItem(root, "m2m:sub");
		case RT_GRP:
			return cJSON_GetObjectItem(root, "m2m:grp");
		case RT_CSR:
			return cJSON_GetObjectItem(root, "m2m:csr");
	}
}

int get_number_from_cjson(cJSON *json){
	if(!json) return 0;

	if(json->valueint)
		return json->valueint;
	if(json->valuestring)
		return atoi(json->valuestring);
}

cJSON *qs_to_json(char* qs){
	if(!qs) return NULL;

	int prevb = 0;
	char *qStr = strdup(qs);
	char *buf = calloc(1, 256);
	char *temp = calloc(1, 256);
	char *key = NULL, *value = NULL;

	size_t qslen = strlen(qs);
	cJSON *json;

	buf[0] = '{';

	for(int i = 0 ; i <= qslen; i++){
		if(qStr[i] == '='){
			key = qStr + prevb;
			qStr[i] = '\0';
			prevb = i+1;
		}
		else if(qStr[i] == '&' || i == qslen){
			value = qStr + prevb;
			qStr[i] = '\0';
			prevb = i+1;
		}

		if(key != NULL && value != NULL){
			if(value[0] == '['){
				sprintf(temp, "\"%s\":%s,", key, value);
			}else{
				sprintf(temp, "\"%s\":\"%s\",", key, value);
			}

			strcat(buf, temp);
			key = NULL;
			value = NULL;
		}

	}
	buf[strlen(buf)-1] = '}';
	json = cJSON_Parse(buf);
	if(!json){
		logger("UTIL", LOG_LEVEL_DEBUG, "ERROR before %10s\n", cJSON_GetErrorPtr());
		return NULL;
	}
	free(temp);
	free(buf);
	free(qStr);

	return json;
}


int is_in_uril(cJSON *uril, char* new){
	if(!uril) return false;
	if(!new) return false;
	int result = -1;
	cJSON *pjson = NULL;

	int urilSize = cJSON_GetArraySize(uril);

	for(int i = 0 ; i < urilSize ; i++){
		pjson = cJSON_GetArrayItem(uril, i);
		if(!strcmp(pjson->valuestring, new)){
			result = i;
			break;
		}
	}

	return result;
}


void filterOptionStr(FilterOperation fo , char *sql){
	switch(fo){
		case FO_AND:
			strcat(sql, " AND ");
			break;
		case FO_OR:
			strcat(sql, " OR ");
			break;
	}
}

bool check_acpi_valid(oneM2MPrimitive *o2pt, cJSON *acpi) {
	bool ret = true;

	int acpi_size = cJSON_GetArraySize(acpi);

	for(int i=0; i<acpi_size; i++) {
		char *acp_uri = cJSON_GetArrayItem(acpi, i)->valuestring;
		RTNode *acp_rtnode = find_rtnode_by_uri(rt->cb, acp_uri);
		if(!acp_rtnode) {
			ret = false;
			handle_error(o2pt, RSC_NOT_FOUND, "resource `acp` does not found");
			break;
		} else {
			if((get_acop_origin(o2pt, acp_rtnode, 1) & ACOP_UPDATE) != ACOP_UPDATE) {
				ret = false;
				handle_error(o2pt, RSC_ORIGINATOR_HAS_NO_PRIVILEGE, "originator has no privilege");
				break;
			}
		}
	}

	return ret;
}

char** http_split_uri(char *uri){
	char **ret = (char **)malloc(3 * sizeof(char *));
	char host[MAX_URI_SIZE] = {'\0'};
	char remain[MAX_URI_SIZE] = {'\0'};
	char port[8];

	int index = 0;
	int uri_size = strlen(uri);

	char *p = strstr(uri, "http://");

	if(p) {
		p = p+7;
		while(p < uri + uri_size && p != ':') {
			host[index++] = *(p++);
		}
		p = p+1;
		index = 0;
		while(p < uri + uri_size && p != '/' && p != '?') {
			port[index++] = *(p++);
		}
	}
}

cJSON *getNonDiscoverableAcp(oneM2MPrimitive *o2pt, RTNode *rtnode){
	cJSON *acp_list = cJSON_CreateArray();
	while(rtnode){
		if(rtnode->ty == RT_ACP){
			if( (get_acop_origin(o2pt, rtnode, 0) & ACOP_DISCOVERY) != ACOP_DISCOVERY){
				cJSON_AddItemToArray(acp_list, cJSON_CreateString(get_uri_rtnode(rtnode)));
			}
		}
		if(rtnode->child){
			cJSON *pjson = NULL;
			cJSON *child_acp_list = getNonDiscoverableAcp(o2pt, rtnode->child);
			cJSON_ArrayForEach(pjson, child_acp_list){
				cJSON_AddItemToArray(acp_list, cJSON_CreateString(pjson->valuestring));
			}
			cJSON_Delete(child_acp_list);
		}
		rtnode = rtnode->sibling_right;
	}
	return acp_list;
}

cJSON *getNoPermAcopDiscovery(oneM2MPrimitive *o2pt, RTNode *rtnode, ACOP acop){
	cJSON *acp_list = cJSON_CreateArray();

	while(rtnode){
		if(rtnode->ty == RT_ACP){
			if( (get_acop_origin(o2pt, rtnode, 0) & acop) != acop){
				cJSON_AddItemToArray(acp_list, cJSON_CreateString(get_uri_rtnode(rtnode)));
			}
		}
		if(rtnode->child){
			cJSON *pjson = NULL;
			cJSON *child_acp_list = getNoPermAcopDiscovery(o2pt, rtnode->child, acop);
			cJSON_ArrayForEach(pjson, child_acp_list){
				cJSON_AddItemToArray(acp_list, cJSON_CreateString(pjson->valuestring));
			}
			cJSON_Delete(child_acp_list);
		}
		rtnode = rtnode->sibling_right;
	}
	return acp_list;
}

void notify_to_nu(oneM2MPrimitive *o2pt, RTNode *sub_rtnode, cJSON *noti_cjson, int net) {
	logger("UTIL", LOG_LEVEL_DEBUG, "notify_to_nu");
	cJSON *sub = sub_rtnode->obj;
	int uri_len = 0, index = 0;
	char *noti_json = cJSON_PrintUnformatted(noti_cjson);
	char *p = NULL;
	char port[10] = {'\0'};
	bool isNoti = false;
	NotiTarget *nt = NULL;
	cJSON *pjson = NULL;

	cJSON *nu = cJSON_GetObjectItem(sub, "nu");
	if(!nu) return;

	cJSON *net_obj = cJSON_GetObjectItem(sub, "net");
	if(!net_obj) return;

	cJSON_ArrayForEach(pjson, net_obj){
		if(pjson->valueint == net) {
			isNoti = true;
			break;
		}
	}

	cJSON_ArrayForEach(pjson, nu){
		char *noti_uri = pjson->valuestring;
		logger("UTIL", LOG_LEVEL_DEBUG, "noti_uri : %s", noti_uri);
		index = 0;
		nt = calloc(1, sizeof(NotiTarget));
		uri_len = strlen(noti_uri);
		p = noti_uri+7;

		if(!strncmp(noti_uri, "http://", 7)) {
			nt->prot = PROT_HTTP;
		}else if(!strncmp(noti_uri, "mqtt://", 7)) {
			nt->prot = PROT_MQTT;
		}

		while(noti_uri + uri_len > p && *p != ':' && *p != '/' && *p != '?'){
			nt->host[index++] = *(p++);
		}
		if(noti_uri + uri_len > p && *p == ':') {
			p++;
			index = 0;
			while(noti_uri + uri_len > p && *p != '/' && *p != '?'){
				port[index++] = *(p++);
			}
			nt->port = atoi(port);
		}
		if(noti_uri + uri_len > p) {
			strcpy(nt->target, p);
			logger("UTIL", LOG_LEVEL_DEBUG, "%s", nt->target);
			p = strchr(nt->target, '?');
			if(p)
				memset(p, 0, strlen(p));
			// if(*p == '?') {
			// 	sprintf(nt->target, "/%s", p);
			// } else if(*p == '/') {
			// 	strcpy(nt->target, p);
			// }
		}

		switch(nt->prot){
			case PROT_HTTP:
				if(!nt->port)
					nt->port = 80;
				if(nt->target[0] == '\0')
					nt->target[0] = '/';
				http_notify(o2pt, noti_json, nt);
				break;
			#ifdef ENABLE_MQTT
			case PROT_MQTT:
				if(!nt->port)
					nt->port = 1883;
				mqtt_notify(o2pt, noti_json, nt);
				break;
			#endif
		}
		noti_uri = strtok(NULL, ",");
		free(nt);
		nt = NULL;
	}

	free(noti_json);
}

void update_resource(cJSON *old, cJSON *new){
	cJSON *pjson = new->child;
	while(pjson){	
		if(cJSON_GetObjectItem(old, pjson->string)){
			cJSON_ReplaceItemInObject(old, pjson->string, cJSON_Duplicate(pjson, 1));
		}else{
			cJSON_AddItemToObject(old, pjson->string, cJSON_Duplicate(pjson, 1));
		}
		pjson = pjson->next;
	}
}

/**
 * @brief validate sub attribute
 * @param obj attribute supported
 * @param attr attribute to validate
 * @param err_msg error message
 * @return true if valid, false if invalid
 * */
bool validate_sub_attr(cJSON *obj, cJSON *attr, char* err_msg){
	if(!attr) return false;
	if(!obj) return false;
	cJSON *verifier = NULL;
	cJSON *verifiee = NULL;

	verifier = cJSON_GetObjectItem(obj, attr->string);
	if(!verifier && obj->type == cJSON_Array){
		verifier = obj;
	}
	
	if(!verifier){
		if(err_msg){
			sprintf(err_msg, "invalid attribute : %s", attr->string);
		}
		return false;
	}
	if(attr->type != cJSON_NULL && verifier->type != attr->type){ // if attribute type is null it is deleting attribute(on update)
		if(verifier->type == cJSON_True || verifier->type == cJSON_False){
			if(attr->type == cJSON_True || attr->type == cJSON_False){
			}else{
				if(err_msg){
					sprintf(err_msg, "invalid attribute type : %s", attr->string);
				}
				return false;
			}
		}else{
			if(err_msg){
				sprintf(err_msg, "invalid attribute type : %s", attr->string);
			}
			return false;
		}
		
	} 
	if(attr->type == cJSON_Object){
		cJSON_ArrayForEach(verifiee, attr){
			if(cJSON_GetArraySize(verifiee) > 0){
				if(!validate_sub_attr(verifier, verifiee, err_msg)){
					return false;
				}
			}
		}
	}else if(attr->type == cJSON_Array){
		cJSON_ArrayForEach(verifiee, attr){
			if(verifiee->type != verifier->child->type){
				if(err_msg){
					sprintf(err_msg, "invalid attribute type : %s", attr->string);
				}
				return false;
			}
			if(verifiee->type == cJSON_Object){
				cJSON *verifiee_child = NULL;
				cJSON_ArrayForEach(verifiee_child, verifiee){
					if(!validate_sub_attr(verifier->child, verifiee_child, err_msg)){
						return false;
					}
				}
			}
		}
	}
	
	return true;
}

/**
 * @brief validate requested attribute
 * @param obj attribute received
 * @param ty resource type
 * @param err_msg buffer for error message
 * @return true if valid, false if invalid
*/
bool is_attr_valid(cJSON *obj, ResourceType ty, char *err_msg){
	extern cJSON* ATTRIBUTES;
	cJSON *attrs = NULL;
	cJSON *general_attrs = NULL;
	bool flag = false;
	attrs = cJSON_GetObjectItem(ATTRIBUTES, get_resource_key(ty));
	general_attrs = cJSON_GetObjectItem(ATTRIBUTES, "general");
	if(!attrs) return false;
	if(!general_attrs) return false;
	if(!cJSON_IsObject(attrs)) return false;

	cJSON *pjson = cJSON_GetObjectItem(obj, get_resource_key(ty));
	cJSON *attr = NULL;
	if(!pjson) return false;
	pjson = pjson->child;
	while(pjson){
		if(validate_sub_attr(attrs, pjson, err_msg)){
			flag = true;
		}
		if(flag){
			pjson = pjson->next;
			flag = false;
			continue;
		}
		if(validate_sub_attr(general_attrs, pjson, err_msg)){
			flag = true;
		}
		if(!flag){
			return false;
		}
		pjson = pjson->next;
		flag = false;
	}

	return true;
}

/**
 * Get Request Primitive and acpi attribute and validate it.
 * @param o2pt oneM2M request primitive
 * @param acpiAttr acpi attribute cJSON object
 * @param op operation type
 * @return 0 if valid, -1 if invalid
*/
int validate_acpi(oneM2MPrimitive *o2pt, cJSON *acpiAttr, Operation op){
	if(!acpiAttr) {
		return handle_error(o2pt, RSC_BAD_REQUEST, "insufficient mandatory attribute(s)");
	}
	if( !cJSON_IsArray(acpiAttr) && !cJSON_IsNull(acpiAttr)){
		return handle_error(o2pt, RSC_BAD_REQUEST, "attribute `acpi` is in invalid form");
	}
	if(cJSON_IsArray(acpiAttr) && cJSON_GetArraySize(acpiAttr) == 0){
		return handle_error(o2pt, RSC_BAD_REQUEST, "attribute `acpi` is empty");
	}


	cJSON *acpi = NULL;
	cJSON_ArrayForEach(acpi, acpiAttr){
		RTNode *acp = NULL;
		acp = find_rtnode_by_uri(rt->cb, acpi->valuestring);
		if(!acp){
			return handle_error(o2pt, RSC_BAD_REQUEST, "resource `acp` is not found");
		}else if(op == OP_UPDATE){
			int acop = 0;
			acop = (acop | get_acop_origin(o2pt, acp, 1));
			logger("UTIL", LOG_LEVEL_DEBUG, "acop-pvs : %d, op : %d", acop, op);
			if(!strcmp(o2pt->fr, "CAdmin")){

			}
			else if((acop & op) != op){
				handle_error(o2pt, RSC_ORIGINATOR_HAS_NO_PRIVILEGE, "originator has no privilege");
				return RSC_ORIGINATOR_HAS_NO_PRIVILEGE;
			}
		}
	}
	

	return RSC_OK;
}

/**
 * @brief validate acr attribute especially acip
 * @param o2pt oneM2M request primitive
 * @param acr_attr acr attribute cJSON object
 * @return RSC_OK if valid, else if invalid
*/
int validate_acr(oneM2MPrimitive *o2pt, cJSON *acr_attr){
	cJSON *acr = NULL;
	cJSON *acop = NULL;
	cJSON *acco = NULL;
	cJSON *acip = NULL;
	cJSON *ipv4 = NULL;
	char *ptr = NULL;

	int mask = 0;

	cJSON_ArrayForEach(acr, acr_attr){
		acop = cJSON_GetObjectItem(acr, "acop");
		if( acop->valueint > 63 || acop->valueint < 0){
			return handle_error(o2pt, RSC_BAD_REQUEST, "attribute `acop` is invalid");
		}
		acco = cJSON_GetObjectItem(acr, "acco");

		if(acco){
			if(acip = cJSON_GetObjectItem(acco, "acip") ){
				cJSON_ArrayForEach(ipv4, cJSON_GetObjectItem(acip, "ipv4")){
					if(ptr = strchr(ipv4->valuestring, '/')){
						mask = atoi(ptr+1);
						if(mask > 32 || mask < 0){
							return handle_error(o2pt, RSC_BAD_REQUEST, "ip in attribute `acip` is invalid");
						}
						*ptr = '\0';
					}
					struct sockaddr_in sa;
					if(inet_pton(AF_INET, ipv4->valuestring, &(sa.sin_addr)) != 1){
						*ptr = '/';
						return handle_error(o2pt, RSC_BAD_REQUEST, "ip in attribute `acip` is invalid");
					}
					if(ptr)
						*ptr = '/';
				}
			}
		}
	}

	return RSC_OK;
}

/**
 * @brief validate acp resource
 * @param o2pt oneM2M request primitive
 * @param acp acp attribute cJSON object
 * @param op operation type
 * @return 0 if valid, -1 if invalid
*/
int validate_acp(oneM2MPrimitive *o2pt, cJSON *acp, Operation op){
	cJSON *pjson = NULL;
	char *ptr = NULL;
	if(!acp) {
		return handle_error(o2pt, RSC_BAD_REQUEST, "insufficient mandatory attribute(s)");
	}


	if(op == OP_CREATE){
		pjson = cJSON_GetObjectItem(acp, "pv");
		if(!pjson){
			return handle_error(o2pt, RSC_BAD_REQUEST, "insufficient mandatory attribute(s)");
		}else if(pjson->type == cJSON_NULL || !cJSON_GetObjectItem(pjson, "acr")){
			return handle_error(o2pt, RSC_BAD_REQUEST, "empty `pv` is not allowed");
		}else{
			if(validate_acr(o2pt, cJSON_GetObjectItem(pjson, "acr")) != RSC_OK)
				return o2pt->rsc;
		}

		pjson = cJSON_GetObjectItem(acp, "pvs");
		if(!pjson){
			return handle_error(o2pt, RSC_BAD_REQUEST, "insufficient mandatory attribute(s)");
		}else if( cJSON_IsNull(pjson) || !cJSON_GetObjectItem(pjson, "acr")){
			return handle_error(o2pt, RSC_BAD_REQUEST, "empty `pvs` is not allowed");
		}else{
			if(validate_acr(o2pt, cJSON_GetObjectItem(pjson, "acr")) != RSC_OK){
				return o2pt->rsc;
			}
		}
	}
	if(op == OP_UPDATE){
		pjson = cJSON_GetObjectItem(acp, "pv");
		if(pjson){
			if( cJSON_IsNull(pjson) || !cJSON_GetObjectItem(pjson, "acr")){
				return handle_error(o2pt, RSC_BAD_REQUEST, "empty `pv` is not allowed");
			}
		}else{
			if(validate_acr(o2pt, cJSON_GetObjectItem(pjson, "acr")) != RSC_OK){
				return o2pt->rsc;
			}
		}

		pjson = cJSON_GetObjectItem(acp, "pvs");
		if (pjson){
			if( cJSON_IsNull(pjson) || !cJSON_GetObjectItem(pjson, "acr")){
				return handle_error(o2pt, RSC_BAD_REQUEST, "empty `pvs` is not allowed");
			}
		}else{
			if(validate_acr(o2pt, cJSON_GetObjectItem(pjson, "acr")) != RSC_OK){
				return o2pt->rsc;
			}
		}
	}

	return RSC_OK;
}

int validate_ae(oneM2MPrimitive *o2pt, cJSON *ae, Operation op){
	cJSON *pjson = NULL;
	char *ptr = NULL;
	if(!ae) {
		handle_error(o2pt, RSC_CONTENTS_UNACCEPTABLE, "insufficient mandatory attribute(s)");
		return RSC_CONTENTS_UNACCEPTABLE;
	}
	if(op == OP_CREATE){
		pjson = cJSON_GetObjectItem(ae, "api");
		if(!pjson){
			handle_error(o2pt, RSC_CONTENTS_UNACCEPTABLE, "insufficient mandatory attribute(s)");
			return RSC_CONTENTS_UNACCEPTABLE;
		}
		ptr = pjson->valuestring;
		if(ptr[0] != 'R' && ptr[0] != 'N') {
			handle_error(o2pt, RSC_BAD_REQUEST, "attribute `api` prefix is invalid");
			return RSC_BAD_REQUEST;
		}
	}
	pjson = cJSON_GetObjectItem(ae, "acpi");
	if(pjson){
		int result = validate_acpi(o2pt, pjson, op);
		if(result != RSC_OK) return result;
	}

	if(op == OP_UPDATE){
		if(pjson && cJSON_GetArraySize(pjson) > 1){
			handle_error(o2pt, RSC_BAD_REQUEST, "only attribute `acpi` is allowed when updating `acpi`");
			return RSC_BAD_REQUEST;
		}
	}

	return RSC_OK;
}

int validate_cnt(oneM2MPrimitive *o2pt, cJSON *cnt, Operation op){
	cJSON *pjson = NULL;
	char *ptr = NULL;
	if(!cnt) {
		handle_error(o2pt, RSC_CONTENTS_UNACCEPTABLE, "insufficient mandatory attribute(s)");
		return RSC_CONTENTS_UNACCEPTABLE;
	}
	
	pjson = cJSON_GetObjectItem(cnt, "acpi");
	if(pjson){
		int result = validate_acpi(o2pt, pjson, op);
		if(result != RSC_OK) return result;
	}

	if(op == OP_UPDATE){
		if(pjson && cJSON_GetArraySize(pjson) > 1){
			handle_error(o2pt, RSC_BAD_REQUEST, "only attribute `acpi` is allowed when updating `acpi`");
			return RSC_BAD_REQUEST;
		}
	}
	
	pjson = cJSON_GetObjectItem(cnt, "mni");
	if(pjson && pjson->valueint < 0){
		handle_error(o2pt, RSC_BAD_REQUEST, "attribute `mni` is invalid");
		return RSC_BAD_REQUEST;
	}
	
	pjson = cJSON_GetObjectItem(cnt, "mbs");
	if(pjson && pjson->valueint < 0) {
		handle_error(o2pt, RSC_BAD_REQUEST, "attribute `mbs` is invalid");
		return RSC_BAD_REQUEST;
	}

	return RSC_OK;
}

int validate_cin(oneM2MPrimitive *o2pt, cJSON *parent_cnt, cJSON *cin, Operation op){
	cJSON *pjson = NULL, *pjson2 = NULL;
	char *ptr = NULL;

	cJSON *mbs = NULL;
	cJSON *cs = NULL;
	if(mbs = cJSON_GetObjectItem(parent_cnt, "mbs")){
		logger("UTIL", LOG_LEVEL_DEBUG, "mbs %d", mbs->valueint);
		if(cs = cJSON_GetObjectItem(cin, "cs")){
			logger("UTIL", LOG_LEVEL_DEBUG, "cs %d", cs->valueint);
			if(mbs->valueint >= 0 && cs->valueint > mbs->valueint) {
				handle_error(o2pt, RSC_NOT_ACCEPTABLE, "contentInstance size exceed `mbs`");
				return RSC_NOT_ACCEPTABLE;
			}
		}
	}
	

	return RSC_OK;
}

int validate_sub(oneM2MPrimitive *o2pt, cJSON *sub, Operation op){
	cJSON *pjson = NULL;
	char *ptr = NULL;

	return RSC_OK;
}

int validate_csr(oneM2MPrimitive *o2pt, RTNode *parent_rtnode, cJSON *csr, Operation op){
	char *mandatory[4] = {""}; // TODO - Add mandatory check
	char *optional[5] = {""}; // TODO - Add optional check
	cJSON *pjson = NULL;
	char *ptr = NULL;

	char *csi = NULL;

	if(pjson = cJSON_GetObjectItem(csr, "csi")){
		csi = pjson->valuestring;
		if(check_csi_duplicate(csi, parent_rtnode) == -1){
			handle_error(o2pt, RSC_OPERATION_NOT_ALLOWED, "originator has already registered");
			return o2pt->rsc;
		}
	}else{
		return handle_error(o2pt, RSC_BAD_REQUEST, "insufficient mandatory attribute(s)");
	}

	if(pjson = cJSON_GetObjectItem(csr, "cb")){
		// TODO - check cb
	}else{
		return handle_error(o2pt, RSC_BAD_REQUEST, "insufficient mandatory attribute(s)");
	}
	

	return RSC_OK;
}

int register_remote_cse(){
	char buf[1024];
	HTTPRequest *req = (HTTPRequest *)calloc(sizeof(HTTPRequest), 1);
	HTTPResponse *res = (HTTPResponse *)calloc(sizeof(HTTPResponse), 1);
	int status_code = 0;
	sprintf(buf, "/%s/%s", REMOTE_CSE_NAME, CSE_BASE_RI);

	req->method = "GET";
	req->uri = strdup(buf);
	req->qs = NULL;
	req->prot = strdup("HTTP/1.1");
	req->payload = NULL;
	req->payload_size = 0;
	req->headers = calloc(sizeof(header_t), 1);
	add_header("X-M2M-Origin", "/"CSE_BASE_RI, req->headers);
	add_header("X-M2M-RI", "check-cse-registered", req->headers);
	add_header("Accept", "application/json", req->headers);
	add_header("Content-Type", "application/json", req->headers);
	add_header("X-M2M-RVI", "2a", req->headers);

	send_http_request(REMOTE_CSE_HOST, REMOTE_CSE_PORT, req, res);
	logger("UTIL", LOG_LEVEL_DEBUG, "Remote CSE registration check: %d", res->status_code);
	status_code = res->status_code;
	if(status_code == 999 || status_code == 500){
		logger("UTIL", LOG_LEVEL_ERROR, "Remote CSE is not running");
		free_HTTPRequest(req);
		free_HTTPResponse(res);
		return status_code;
	}

	if(res->status_code != 200 ){ 
		free_HTTPRequest(req);
		free_HTTPResponse(res);
		logger("UTIL", LOG_LEVEL_DEBUG, "Remote CSE is not registered");
		req = (HTTPRequest *)calloc(sizeof(HTTPRequest), 1);
		res = (HTTPResponse *)calloc(sizeof(HTTPResponse), 1);
		//register MN-CSE
		req->method = "POST";
		sprintf(buf, "/%s", REMOTE_CSE_NAME);
		req->uri = strdup(buf);
		req->qs = NULL;
		req->prot = strdup("HTTP/1.1");

		req->headers = calloc(sizeof(header_t), 1);
		add_header("X-M2M-Origin", "/"CSE_BASE_RI, req->headers);
		add_header("X-M2M-RI", "register-cse", req->headers);
		add_header("Accept", "application/json", req->headers);
		add_header("Content-Type", "application/json;ty=16", req->headers);
		add_header("X-M2M-RVI", "2a", req->headers);

		cJSON *root = cJSON_CreateObject();
		cJSON *csr = cJSON_Duplicate(rt->cb->obj, 1);
		init_csr(csr);
		cJSON_AddItemToObject(root, get_resource_key(RT_CSR), csr);
		req->payload =  cJSON_PrintUnformatted(root);
		cJSON_Delete(root);

		req->payload_size = strlen(req->payload);
		send_http_request(REMOTE_CSE_HOST, REMOTE_CSE_PORT, req, res);

		char *rsc = 0;
		if( rsc = search_header(res->headers, "X-M2M-RSC")){
			if( atoi(rsc) != RSC_CREATED){
				logger("UTIL", LOG_LEVEL_ERROR, "Remote CSE registration failed");
				free_HTTPRequest(req);
				free_HTTPResponse(res);
				return atoi(rsc);
			}
		}else{
			logger("UTIL", LOG_LEVEL_ERROR, "Remote CSE registration failed");
			free_HTTPRequest(req);
			free_HTTPResponse(res);
			return -1;
		}
		
	}

	free_HTTPRequest(req);
	free_HTTPResponse(res);

	return 0;
}

int create_local_csr(){
	char buf[256] = {0};

	HTTPRequest *req = (HTTPRequest *)malloc(sizeof(HTTPRequest));
	HTTPResponse *res = (HTTPResponse *)malloc(sizeof(HTTPResponse));

	req->method = "GET";
	req->uri = strdup("/"REMOTE_CSE_NAME);
	req->qs = NULL;
	req->prot = strdup("HTTP/1.1");
	req->payload = NULL;
	req->payload_size = 0;
	req->headers = calloc(sizeof(header_t), 1);
	add_header("X-M2M-Origin", "/"CSE_BASE_RI, req->headers);
	add_header("X-M2M-RI", "retrieve-cb", req->headers);
	add_header("Accept", "application/json", req->headers);
	add_header("Content-Type", "application/json", req->headers);
	add_header("X-M2M-RVI", "2a", req->headers);

	send_http_request(REMOTE_CSE_HOST, REMOTE_CSE_PORT, req, res);

	if(res->status_code != 200 ){ 
		logger("MAIN", LOG_LEVEL_ERROR, "Remote CSE is not online : %d", res->status_code);
		free_HTTPRequest(req);
		free_HTTPResponse(res);
		return res->status_code;
	}
	
	cJSON * root = cJSON_Parse(res->payload);
	cJSON *remote_cb = cJSON_GetObjectItem(root, get_resource_key(RT_CSE));

	if(!remote_cb){
		logger("MAIN", LOG_LEVEL_ERROR, "Remote CSE not valid");
		free_HTTPRequest(req);
		free_HTTPResponse(res);
		return -1;
	}



	cJSON *remote_ri = cJSON_GetObjectItem(remote_cb, "ri");
	cJSON *remote_rn = cJSON_GetObjectItem(remote_cb, "rn");
	cJSON *remote_csi = cJSON_GetObjectItem(remote_cb, "csi");
	if(!remote_rn || !remote_csi){
		logger("UTIL", LOG_LEVEL_ERROR, "Remote CSE not valid");
		free_HTTPRequest(req);
		free_HTTPResponse(res);
		return -1;
	}

	cJSON *csr = cJSON_Duplicate(remote_cb, 1);
	cJSON_DeleteItemFromObject(csr, "ct");
	cJSON_DeleteItemFromObject(csr, "lt");
	cJSON_DeleteItemFromObject(csr, "et");
	cJSON_DeleteItemFromObject(csr, "acpi");
	cJSON_DeleteItemFromObject(csr, "pi");
	cJSON_DeleteItemFromObject(csr, "srt");
	cJSON_DeleteItemFromObject(csr, "ctm");


	cJSON_SetValuestring(cJSON_GetObjectItem(csr, "rn"), remote_ri->valuestring);
	cJSON_SetValuestring(cJSON_GetObjectItem(csr, "ri"), remote_rn->valuestring);

	cJSON_SetIntValue(cJSON_GetObjectItem(csr, "ty"), RT_CSR);

	sprintf(buf, "%s/%s", remote_csi->valuestring, remote_rn->valuestring);
	cJSON_AddItemToObject(csr, "cb", cJSON_CreateString(buf));

	cJSON_Delete(root);

	add_general_attribute(csr, rt->cb, RT_CSR);

	// int rsc = validate_csr(o2pt, parent_rtnode, csr, OP_CREATE);
	// if(rsc != RSC_OK){
	// 	cJSON_Delete(root);
	// 	return rsc;
	// }

	char *ptr = malloc(1024);
	sprintf(ptr, "%s/%s", CSE_BASE_NAME, remote_rn->valuestring);	
	cJSON_AddStringToObject(csr, "uri", cJSON_CreateString(ptr));
	int result = db_store_resource(csr, ptr);
	if(result == -1) {
		cJSON_Delete(root);
		free(ptr);	ptr = NULL;
		return RSC_INTERNAL_SERVER_ERROR;
	}
	free(ptr);	ptr = NULL;

	RTNode* rtnode = create_rtnode(csr, RT_CSR);
	add_child_resource_tree(rt->cb, rtnode);

	
	//TODO: update descendent cse if update is needed

	return 0;
}

int deRegister_csr(){
	RTNode *rtnode = rt->cb->child;
	while(rtnode){
		if(rtnode->ty == RT_CSR){
			cJSON *csr = rtnode->obj;
			cJSON *csi = cJSON_GetObjectItem(csr, "csi");
			cJSON *cb = cJSON_GetObjectItem(csr, "cb");

			cJSON *poa = cJSON_GetObjectItem(csr, "poa");
			cJSON *pjson = NULL;
			cJSON_ArrayForEach(pjson, poa){
				char *poa_str = pjson->valuestring;
				Protocol prot = PROT_HTTP;

				if(!strncmp(poa_str, "http://", 7)) {
					prot = PROT_HTTP;
				}else if(!strncmp(poa_str, "mqtt://", 7)) {
					prot = PROT_MQTT;
				}
				char *p = poa_str + 7;
				char *ptr = strchr(p, ':');
				char *host = NULL;
				int port = 80;
				if(ptr){
					*ptr = '\0';
					port = atoi(ptr+1);
				}
				host = strdup(p);

				*ptr = ':';

				char buf[1024] = {0};

				if(prot == PROT_HTTP){

					HTTPRequest *req = (HTTPRequest *)calloc(sizeof(HTTPRequest), 1);
					HTTPResponse *res = (HTTPResponse *)calloc(sizeof(HTTPResponse), 1);

					req->method = "DELETE";
					sprintf(buf, "/~%s/%s", cb->valuestring , CSE_BASE_RI);
					req->uri = strdup(buf);
					req->qs = NULL;
					req->prot = strdup("HTTP/1.1");
					req->payload = NULL;
					req->payload_size = 0;
					req->headers = calloc(sizeof(header_t), 1);
					add_header("X-M2M-Origin", "/"CSE_BASE_RI, req->headers);
					add_header("X-M2M-RI", "delete-csr", req->headers);
					add_header("Content-Type", "application/json", req->headers);
					add_header("X-M2M-RVI", "2a", req->headers);

					send_http_request(host, port, req, res);

					if(res->status_code != 200 ){ 
						logger("MAIN", LOG_LEVEL_ERROR, "Remote CSE is not online : %d", res->status_code);
						free_HTTPRequest(req);
						free_HTTPResponse(res);
						return res->status_code;
					}
					free_HTTPRequest(req);
					free_HTTPResponse(res);
					free(host);
				}else if(prot == PROT_MQTT){
					//TODO - implement MQTT delete
					#ifdef ENABLE_MQTT
					mqtt_delete_csr(host, port, cb->valuestring);
					#endif
				}

				db_delete_onem2m_resource(rtnode);
			}
		}
		rtnode = rtnode->sibling_right;
	}
}

int create_remote_cba(char *poa, char **cbA_url){
	logger("UTIL", LOG_LEVEL_DEBUG, "create_remote_cba");
	Protocol prot = PROT_HTTP;
	char *host = NULL;
	int port = 80;
	char *path = NULL;


	if (parsePoa(poa, &prot, &host, &port, &path) == -1){
		logger("UTIL", LOG_LEVEL_ERROR, "poa is invalid");
		return -1;
	}
	logger("UTIL", LOG_LEVEL_DEBUG, "%d %s %d %s", prot, host, port, path);

	char buf[1024] = {0};

	if(prot == PROT_HTTP){
		HTTPRequest *req = (HTTPRequest *)calloc(sizeof(HTTPRequest), 1);
		HTTPResponse *res = (HTTPResponse *)calloc(sizeof(HTTPResponse), 1);

		req->method = "POST";
		req->uri = strdup(path);
		req->qs = NULL;
		req->prot = strdup("HTTP/1.1");
		req->payload = NULL;
		req->payload_size = 0;
		req->headers = calloc(sizeof(header_t), 1);
		add_header("X-M2M-Origin", "/"CSE_BASE_RI, req->headers);
		add_header("X-M2M-RI", "create-cba", req->headers);
		add_header("Accept", "application/json", req->headers);
		add_header("Content-Type", "application/json;ty=10005", req->headers);
		add_header("X-M2M-RVI", "2a", req->headers);

		cJSON *root = cJSON_CreateObject();
		cJSON *cba = cJSON_CreateObject();
		cJSON_AddItemToObject(root, get_resource_key(RT_CBA), cba);
		cJSON_AddItemToObject(cba, "lnk", cJSON_CreateString("/"CSE_BASE_RI"/"CSE_BASE_NAME));
		cJSON *srv = cJSON_Duplicate( cJSON_GetObjectItem(rt->cb->obj, "srv"), true);
		cJSON_AddItemToObject(cba, "srv", srv);

		req->payload =  cJSON_PrintUnformatted(root);
		cJSON_Delete(root);

		req->payload_size = strlen(req->payload);
		logger("UTIL", LOG_LEVEL_DEBUG, "payload: %s", req->payload);
		send_http_request(host, port, req, res);

		if(res->status_code != 201 ){ 
			logger("MAIN", LOG_LEVEL_ERROR, "cbA creation failed : %d", res->status_code);
			logger("MAIN", LOG_LEVEL_ERROR, "%s", res->payload);
			free_HTTPRequest(req);
			free_HTTPResponse(res);
			return -1;
		}
		cJSON *result = cJSON_Parse(res->payload);
		cJSON *obj = cJSON_GetObjectItem(result, get_resource_key(RT_CBA));
		*cbA_url =  strdup(cJSON_GetObjectItem(obj, "ri")->valuestring);

		free_HTTPRequest(req);
		free_HTTPResponse(res);
	}else if(prot == PROT_MQTT){
		//TODO - implement MQTT delete
		#ifdef ENABLE_MQTT
		mqtt_create_cba(host, port, cb->valuestring);
		#endif
	}
	

	free(host);
	free(path);
	return 0;
} 

int create_remote_aea(RTNode *parent_rtnode, cJSON *ae_obj){
	logger("UTIL", LOG_LEVEL_DEBUG, "create_remote_aea");
	char buf[256] = {0};
	bool pannc = false;

	// Check Parent Resource has attribute at
	cJSON *at_obj = cJSON_GetObjectItem(ae_obj, "at");
	cJSON *at = NULL;
	cJSON *pat = cJSON_GetObjectItem(parent_rtnode->obj, "at");

	cJSON_ArrayForEach(at, at_obj){
		pannc = false;
		char *cba_target = NULL;
		cJSON *pjson = NULL;
		cJSON_ArrayForEach(pjson, pat){
			if(!strncmp(pjson->valuestring, at->valuestring, strlen(at->valuestring))){
				pannc = true;
				cba_target = strdup(pjson->valuestring);
				break;
			}
		}
		
		if(!pannc){
			if(at->valuestring[0] == '/'){
				oneM2MPrimitive *o2pt = (oneM2MPrimitive *)calloc(sizeof(oneM2MPrimitive), 1);
				o2pt->fr = strdup("/"CSE_BASE_RI);
				sprintf(buf, "~%s%s", at->valuestring, at->valuestring);
				o2pt->to = strdup(buf);
				o2pt->op = OP_CREATE;
				o2pt->ty = RT_CBA;
				o2pt->rqi = strdup("create-cba");
				o2pt->rvi = strdup("2a");

				cJSON *root = cJSON_CreateObject();
				cJSON *cba = cJSON_CreateObject();
				cJSON_AddItemToObject(root, get_resource_key(RT_CBA), cba);
				cJSON_AddStringToObject(cba, "lnk", cJSON_CreateString("/"CSE_BASE_RI"/"CSE_BASE_NAME));
				cJSON *srv = cJSON_Duplicate( cJSON_GetObjectItem(parent_rtnode->obj, "srv"), true);
				cJSON_AddItemToObject(cba, "srv", srv);
				// cJSON_AddItemToObject(cba, "ty", cJSON_CreateNumber(RT_CBA));

				o2pt->pc = cJSON_PrintUnformatted(root);
				logger("UTIL", LOG_LEVEL_DEBUG, "create remote cba: %s", o2pt->pc);
				cJSON_Delete(root);
				o2pt->isForwarding = true;

				route(o2pt);

				logger("UTIL", LOG_LEVEL_DEBUG, "create remote cba: %s", o2pt->pc);
				logger("UTIL", LOG_LEVEL_DEBUG, "rsc: %d", o2pt->rsc);
				if(o2pt->rsc != RSC_CREATED){
					logger("UTIL", LOG_LEVEL_ERROR, "Creation failed");
					return -1;
				}
				cJSON *result = cJSON_Parse(o2pt->pc);
				if(result){
					// logger("UTIL", LOG_LEVEL_DEBUG, "cba_target: %s", cJSON_PrintUnformatted(result));
					root = cJSON_GetObjectItem(result, get_resource_key(RT_CBA));
					cba_target = strdup(cJSON_GetObjectItem(root, "ri")->valuestring);
				}else{
					logger("UTIL", LOG_LEVEL_ERROR, "%s", cJSON_GetErrorPtr());
				}
				// cJSO cJSON_CreateArray();
				// rt->cb->ob;
				cJSON_Delete(result);
				free_o2pt(o2pt);

				o2pt = (oneM2MPrimitive *)calloc(sizeof(oneM2MPrimitive), 1);
				sprintf(buf, "~%s/%s", at->valuestring, cba_target);
				o2pt->fr = strdup("/"CSE_BASE_RI);
				o2pt->to = strdup(buf);
				o2pt->op = OP_CREATE;
				o2pt->ty = RT_AEA;
				o2pt->rqi = strdup("create-aea");
				o2pt->rvi = strdup("2a");

				root = cJSON_CreateObject();
				cJSON *aea = cJSON_CreateObject();
				cJSON_AddItemToObject(root, get_resource_key(RT_AEA), aea);
				sprintf(buf, "/%s/%s/%s", CSE_BASE_RI, get_uri_rtnode(parent_rtnode), cJSON_GetObjectItem(ae_obj, "rn")->valuestring);
				cJSON_AddItemToObject(aea, "lnk", cJSON_CreateString(buf));
				logger("UTIL", LOG_LEVEL_DEBUG, "tt %s", buf);
				srv = cJSON_Duplicate( cJSON_GetObjectItem(ae_obj, "srv"), true);
				cJSON_AddItemToObject(aea, "srv", srv);
				cJSON *lbl = cJSON_Duplicate( cJSON_GetObjectItem(ae_obj, "lbl"), true);
				cJSON_AddItemToObject(aea, "lbl", lbl);

				cJSON* aa = cJSON_GetObjectItem(ae_obj, "aa");
				cJSON_ArrayForEach(pjson, aa){
					cJSON *temp =  cJSON_GetObjectItem(ae_obj, pjson->valuestring);
					cJSON_AddItemToObject(aea, pjson->valuestring, cJSON_Duplicate(temp, true));
				}

				o2pt->pc = cJSON_PrintUnformatted(root);
				logger("UTIL", LOG_LEVEL_DEBUG, "create remote aea: %s", o2pt->pc);
				cJSON_Delete(root);
				o2pt->isForwarding = true;

				route(o2pt);

				logger("UTIL", LOG_LEVEL_DEBUG, "create remote aea: %s", o2pt->pc);

				if(o2pt->rsc != RSC_CREATED){
					logger("UTIL", LOG_LEVEL_ERROR, "Creation failed");
					return -1;
				}

				result = cJSON_Parse(o2pt->pc);
				cJSON *aea_obj = cJSON_GetObjectItem(result, get_resource_key(RT_AEA));
				char *aea_ri = cJSON_GetObjectItem(aea_obj, "ri")->valuestring;
				logger("UTIL", LOG_LEVEL_DEBUG, "aea_ri: %s", aea_ri);
				sprintf(buf, "%s/%s", at->valuestring, aea_ri);
				cJSON_SetValuestring(at, buf);

				cJSON_Delete(result);
				free_o2pt(o2pt);
			}else {
				// create parent announcement resource
				if ( create_remote_cba(at->valuestring, &cba_target) != 0 ){
					logger("UTIL", LOG_LEVEL_ERROR, "Creation failed");
					return -1;
				}
		

				HTTPRequest *req = (HTTPRequest *)calloc(sizeof(HTTPRequest), 1);
				HTTPResponse *res = (HTTPResponse *)calloc(sizeof(HTTPResponse), 1);
				Protocol prot = PROT_HTTP;
				char *host = NULL;
				int port = 80;
				char *path = NULL;

				parsePoa(at->valuestring, &prot, &host, &port, &path);
				sprintf(buf, "/%s",cba_target);
				logger("UTIL", LOG_LEVEL_DEBUG, "cba_target: %s", buf);
				req->method = "POST";
				req->uri = strdup(buf);
				req->qs = NULL;
				req->prot = strdup("HTTP/1.1");
				req->payload = NULL;
				req->payload_size = 0;
				req->headers = calloc(sizeof(header_t), 1);
				add_header("X-M2M-Origin", "/"CSE_BASE_RI, req->headers);
				add_header("X-M2M-RI", "create-aeA", req->headers);
				add_header("Accept", "application/json", req->headers);
				add_header("Content-Type", "application/json;ty=10002", req->headers);
				add_header("X-M2M-RVI", "2a", req->headers);

				cJSON *root = cJSON_CreateObject();
				cJSON *aea = cJSON_CreateObject();
				cJSON_AddItemToObject(root, get_resource_key(RT_AEA), aea);
				sprintf(buf, "/%s/%s/%s", CSE_BASE_RI, get_uri_rtnode(parent_rtnode), cJSON_GetObjectItem(ae_obj, "rn")->valuestring);
				cJSON_AddItemToObject(aea, "lnk", cJSON_CreateString(buf));
				logger("UTIL", LOG_LEVEL_DEBUG, "tt %s", buf);
				cJSON *srv = cJSON_Duplicate( cJSON_GetObjectItem(ae_obj, "srv"), true);
				cJSON_AddItemToObject(aea, "srv", srv);
				cJSON *lbl = cJSON_Duplicate( cJSON_GetObjectItem(ae_obj, "lbl"), true);
				cJSON_AddItemToObject(aea, "lbl", lbl);

				cJSON* aa = cJSON_GetObjectItem(ae_obj, "aa");
				cJSON_ArrayForEach(pjson, aa){
					cJSON *temp =  cJSON_GetObjectItem(ae_obj, pjson->valuestring);
					cJSON_AddItemToObject(aea, pjson->valuestring, cJSON_Duplicate(temp, true));
				}

				req->payload =  cJSON_PrintUnformatted(root);
				logger("UTIL", LOG_LEVEL_DEBUG, "payload: %s", req->payload);

				req->payload_size = strlen(req->payload);
				send_http_request(host, port, req, res);
				cJSON_Delete(root);


				if(res->status_code != 201 ){ 
					logger("MAIN", LOG_LEVEL_ERROR, "aeA creation failed: %d", res->status_code);
					logger("MAIN", LOG_LEVEL_ERROR, "%s", res->payload);
					free_HTTPRequest(req);
					free_HTTPResponse(res);
					return -1;
				}
				cJSON *result = cJSON_Parse(res->payload);
				cJSON *obj = cJSON_GetObjectItem(result, get_resource_key(RT_AEA));

				char *aea_rn = cJSON_GetObjectItem(obj, "rn")->valuestring;
				logger("UTIL", LOG_LEVEL_DEBUG, "aea_rn: %s", aea_rn);
				sprintf(buf, "%s/%s", at->valuestring, aea_rn);
				cJSON_SetValuestring(at, buf);

				free_HTTPRequest(req);
				free_HTTPResponse(res);

			}

			// create announcement resource
			
			
		}
		free(cba_target);
		
	}
}

int parsePoa(char *poa_str, Protocol *prot, char **host, int *port, char **path){
	logger("UTIL", LOG_LEVEL_DEBUG, "parsePoa");
	char *p = strdup(poa_str);
	char *ptr = NULL;
	if(!strncmp(poa_str, "http://", 7)) {
		*prot = PROT_HTTP;
	}else if(!strncmp(poa_str, "mqtt://", 7)) {
		*prot = PROT_MQTT;
	}else{
		free(p);
		return -1;
	}
	ptr = strchr(p + 7, ':');
	if(ptr){
		*ptr = '\0';
		*port = atoi(ptr+1);
	}
	*host = strdup(p+7);
	*ptr = ':';
	ptr = strchr(p+7, '/');
	if(ptr){
		*path = strdup(ptr);
		*ptr = '\0';
	}
	if(!(*path)) {
		*path = strdup("/");
	}
	free(p);
	return 0;
}

void add_rrnode(RRNode *rrnode){
	RRNode *prrnode = rt->rr_list;
	if(!prrnode){
		rt->rr_list = rrnode;
		return;
	}
	while(prrnode->next){
		prrnode = prrnode->next;
	}
	prrnode->next = rrnode;
}

void detach_rrnode(RRNode *rrnode){
	RRNode *prrnode = rt->rr_list;
	if(!prrnode){
		return;
	}
	if(prrnode == rrnode){
		rt->rr_list = rrnode->next;
		return;
	}
	while(prrnode->next){
		if(prrnode->next == rrnode){
			prrnode->next = rrnode->next;
			return;
		}
		prrnode = prrnode->next;
	}
	if(rrnode->uri){
		free(rrnode->uri);
		rrnode->uri = NULL;
	}
	free(rrnode);
	rrnode = NULL;
}