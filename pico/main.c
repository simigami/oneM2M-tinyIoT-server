#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "onem2m.h"
#include "jsonparse.h"
#include "berkeleyDB.h"
#include "httpd.h"
#include "cJSON.h"
#include "config.h"
#include "mqttClient.h"

ResourceTree *rt;
void *mqtt_serve();

int main(int c, char **v) {
	pthread_t mqtt;
	int mqtt_thread_id;
	init();

	mqtt_thread_id = pthread_create(&mqtt, NULL, mqtt_serve, "mqtt Client");
	if(mqtt_thread_id < 0){
		fprintf(stderr, "MQTT thread create error\n");
		return 0;
	}

	serve_forever(SERVER_PORT); // main oneM2M operation logic in void route()    

	return 0;
}

void handle_http_request() {
	oneM2Mprimitive o2pt;
	route(o2pt);
}

void route(oneM2Mprimitive o2pt) {
    double start;

    start = (double)clock() / CLOCKS_PER_SEC; // runtime check - start

	Operation op = OP_NONE;

	memset(response_headers, 0, sizeof(response_headers));

	char *header_value;
	if((header_value = request_header("X-M2M-RI"))) {
		set_response_header("X-M2M-RI", header_value);
	}
	if(header_value = request_header("X-M2M-RVI")) {
		set_response_header("X-M2M-RVI", header_value);
	}

	Node* pnode = parse_uri(rt->cb, uri, &op); // return tree node by URI
	int e = result_parse_uri(pnode);

	if(e != -1) e = check_payload_size();
	if(e == -1) {
		log_runtime(start);
		return;
	}

	if(op == OP_NONE) op = parse_operation(); // parse operation by HTTP method

	switch(op) {
	
	case OP_CREATE:	
		create_object(pnode); break;
	
	case OP_RETRIEVE:
		retrieve_object(pnode);	break;
		
	case OP_UPDATE: 
		update_object(pnode); break;
		
	case OP_DELETE:
		delete_object(pnode); break;

	case OP_VIEWER:
		tree_viewer_api(pnode); break;
	
	case OP_OPTIONS:
		respond_to_client(200, "{\"m2m:dbg\": \"response about options method\"}", "2000");
		break;
	
	default:
		respond_to_client(500, "{\"m2m:dbg\": \"internal server error\"}", "5000");
	}
	if(pnode->ty == TY_CIN) free_node(pnode);

	log_runtime(start);
}

void log_runtime(double start) {
	double end = (((double)clock()) / CLOCKS_PER_SEC); // runtime check - end
    fprintf(stderr,"Run time :%lf\n", (end-start));
}

void init_server() {
	response_headers = (char *)calloc(4096,sizeof(char));

	rt = (ResourceTree *)malloc(sizeof(rt));
	
	CSE *cse;

	if(access("./RESOURCE.db", 0) == -1) {
		cse = (CSE*)malloc(sizeof(CSE));
		init_cse(cse);
		db_store_cse(cse);
	} else {
		cse = db_get_cse(CSE_BASE_RI);
	}
	
	rt->cb = create_node(cse, TY_CSE);
	free_cse(cse); cse = NULL;

 	restruct_resource_tree();


}

void create_object(Node *pnode) {
	ObjectType ty = parse_object_type(); 
	
	int e = check_request_body_empty();
	if(e != -1) e = check_resource_type_equal(ty, parse_object_type_in_request_body());
	if(e != -1) e = check_privilege(pnode, ACOP_CREATE, OP_CREATE, ty);
	if(e != -1) e = check_json_format();
	if(e != -1) e = check_resource_name_duplicate(pnode);
	if(e == -1) return;

	switch(ty) {	
	case TY_AE :
		fprintf(stderr,"\x1b[42mCreate AE\x1b[0m\n");
		create_ae(pnode);
		break;	
					
	case TY_CNT :
		fprintf(stderr,"\x1b[42mCreate CNT\x1b[0m\n");
		create_cnt(pnode);
		break;
			
	case TY_CIN :
		fprintf(stderr,"\x1b[42mCreate CIN\x1b[0m\n");
		create_cin(pnode);
		break;

	case TY_SUB :
		fprintf(stderr,"\x1b[42mCreate Sub\x1b[0m\n");
		create_sub(pnode);
		break;
	
	case TY_ACP :
		fprintf(stderr,"\x1b[42mCreate ACP\x1b[0m\n");
		create_acp(pnode);
		break;

	default :
		fprintf(stderr,"Resource type error (Content-Type Header Invalid)\n");
		respond_to_client(400, "{\"m2m:dbg\": \"resource type error (Content-Type header invalid)\"}", "4000");
	}	
}

void retrieve_object(Node *pnode) {
	int e = check_privilege(pnode, ACOP_RETRIEVE, OP_RETRIEVE, pnode->ty);

	if(e == -1) return;

	int fu = get_value_querystring_int("fu");

	if(fu == 1) {
		fprintf(stderr,"\x1b[43mRetrieve FilterCriteria\x1b[0m\n");
		retrieve_object_filtercriteria(pnode);
		return;
	}

	switch(pnode->ty) {
		
	case TY_CSE :
        fprintf(stderr,"\x1b[43mRetrieve CSE\x1b[0m\n");
        retrieve_cse(pnode);
      	break;
	case TY_AE : 
		fprintf(stderr,"\x1b[43mRetrieve AE\x1b[0m\n");
		retrieve_ae(pnode);	
		break;	
			
	case TY_CNT :
		fprintf(stderr,"\x1b[43mRetrieve CNT\x1b[0m\n");
		retrieve_cnt(pnode);			
		break;
			
	case TY_CIN :
		fprintf(stderr,"\x1b[43mRetrieve CIN\x1b[0m\n");
		retrieve_cin(pnode);			
		break;

	case TY_SUB :
		fprintf(stderr,"\x1b[43mRetrieve Sub\x1b[0m\n");
		retrieve_sub(pnode);			
		break;

	case TY_ACP :
		fprintf(stderr,"\x1b[43mRetrieve ACP\x1b[0m\n");
		retrieve_acp(pnode);			
		break;
	}	
}

void update_object(Node *pnode) {
	ObjectType ty = parse_object_type_in_request_body();

	int e = check_request_body_empty();
	if(e != -1) e = check_privilege(pnode, ACOP_UPDATE, OP_UPDATE, ty);
	if(e != -1) e = check_json_format();
	if(e != -1) e = check_resource_name_duplicate(pnode->parent);
	if(e != -1) e = check_resource_type_equal(ty, pnode->ty);
	if(e == -1) return;
	
	switch(ty) {
	case TY_AE :
		fprintf(stderr,"\x1b[45mUpdate AE\x1b[0m\n");
		update_ae(pnode);
		break;

	case TY_CNT :
		fprintf(stderr,"\x1b[45mUpdate CNT\x1b[0m\n");
		update_cnt(pnode);
		break;

	case TY_SUB :
		fprintf(stderr,"\x1b[45mUpdate Sub\x1b[0m\n");
		update_sub(pnode);
		break;
	
	case TY_ACP :
		fprintf(stderr,"\x1b[45mUpdate ACP\x1b[0m\n");
		update_acp(pnode);
		break;

	default :
		fprintf(stderr,"Resource type do not support PUT method\n");
		respond_to_client(400, "{\"m2m:dbg\": \"`PUT` method unsupported\"}","4005");
	}
}

void create_ae(Node *pnode) {
	if(check_resource_aei_duplicate(pnode)) return;
	if(check_resource_name_invalid(TY_AE)) return;
	if(pnode->ty != TY_CSE) {
		child_type_error();
		return;
	}
	AE* ae = json_to_ae(payload);
	if(!ae) {
		no_mandatory_error();
		return;
	}
	init_ae(ae,pnode->ri);
	
	int result = db_store_ae(ae);
	if(result != 1) { 
		respond_to_client(500, "{\"m2m:dbg\": \"DB store fail\"}", "5000");
		free_ae(ae); ae = NULL;
		return;
	}
	
	Node* node = create_node(ae, TY_AE);
	add_child_resource_tree(pnode,node);
	response_payload = ae_to_json(ae);

	respond_to_client(201, NULL, "2001");
	notify_object(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free(response_payload); response_payload = NULL;
	free_ae(ae); ae = NULL;
}

void create_cnt(Node *pnode) {
	if(pnode->ty != TY_CNT && pnode->ty != TY_AE && pnode->ty != TY_CSE) {
		child_type_error();
		return;
	}
	CNT* cnt = json_to_cnt(payload);
	if(!cnt) {
		no_mandatory_error();
		return;
	}
	init_cnt(cnt,pnode->ri);

	int result = db_store_cnt(cnt);
	if(result != 1) { 
		respond_to_client(500, "{\"m2m:dbg\": \"DB store fail\"}", "5000");
		free_cnt(cnt); cnt = NULL;
		return;
	}
	
	Node* node = create_node(cnt, TY_CNT);
	add_child_resource_tree(pnode,node);

	response_payload = cnt_to_json(cnt);
	respond_to_client(201, NULL, "2001");
	notify_object(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free(response_payload); response_payload = NULL; 
	free_cnt(cnt); cnt = NULL;
}

void create_cin(Node *pnode) {
	if(pnode->ty != TY_CNT) {
		child_type_error();
		return;
	}
	CIN* cin = json_to_cin(payload);
	if(!cin) {
		no_mandatory_error();
		return;
	}
	init_cin(cin,pnode->ri);

	int result = db_store_cin(cin);
	if(result != 1) { 
		respond_to_client(500, "{\"m2m:dbg\": \"DB store fail\"}", "5000");
		free_cin(cin);
		cin = NULL;
		return;
	}
	
	response_payload = cin_to_json(cin);
	respond_to_client(201, NULL, "2001");
	notify_object(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free(response_payload); response_payload = NULL; 
	free_cin(cin); cin = NULL;
}

void create_sub(Node *pnode) {
	if(pnode->ty == TY_CIN || pnode->ty == TY_SUB) {
		child_type_error();
		return;
	}
	Sub* sub = json_to_sub(payload);
	if(!sub) {
		no_mandatory_error();
		return;
	}
	init_sub(sub, pnode->ri);
	
	int result = db_store_sub(sub);
	if(result != 1) { 
		respond_to_client(500, "{\"m2m:dbg\": \"DB store fail\"}", "5000");
		free_sub(sub); sub = NULL;
		return;
	}
	
	Node* node = create_node(sub, TY_SUB);
	add_child_resource_tree(pnode,node);
	
	response_payload = sub_to_json(sub);
	respond_to_client(201, NULL, "2001");
	notify_object(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free(response_payload); response_payload = NULL;
	free_sub(sub); sub = NULL;
}

void create_acp(Node *pnode) {
	if(pnode->ty != TY_CSE && pnode->ty != TY_AE) {
		child_type_error();
		return;
	}
	ACP* acp = json_to_acp(payload);
	if(!acp) {
		no_mandatory_error();
		return;
	}
	init_acp(acp, pnode->ri);
	
	int result = db_store_acp(acp);
	if(result != 1) { 
		respond_to_client(500, "{\"m2m:dbg\": \"DB store fail\"}", "5000");
		free_acp(acp); acp = NULL;
		return;
	}
	
	Node* node = create_node(acp, TY_ACP);
	add_child_resource_tree(pnode,node);
	
	response_payload = acp_to_json(acp);
	respond_to_client(201, NULL, "2001");
	notify_object(pnode->child, response_payload, NOTIFICATION_EVENT_3);
	free(response_payload); response_payload = NULL; 
	free_acp(acp); acp = NULL;
}

void retrieve_cse(Node *pnode){
	CSE* gcse = db_get_cse(pnode->ri);
	response_payload = cse_to_json(gcse);
	respond_to_client(200, NULL, "2000");
	free(response_payload); response_payload = NULL; 
	free_cse(gcse); gcse = NULL;
}

void retrieve_ae(Node *pnode){
	AE* gae = db_get_ae(pnode->ri);
	response_payload = ae_to_json(gae);
	respond_to_client(200, NULL, "2000");
	free(response_payload); response_payload = NULL;
	free_ae(gae); gae = NULL;
}

void retrieve_cnt(Node *pnode){
	CNT* gcnt = db_get_cnt(pnode->ri);
	response_payload = cnt_to_json(gcnt);
	respond_to_client(200, NULL, "2000");
	free(response_payload); response_payload = NULL;
	free_cnt(gcnt); gcnt = NULL;
}

void retrieve_cin(Node *pnode){
	CIN* gcin = db_get_cin(pnode->ri);
	response_payload = cin_to_json(gcin);
	respond_to_client(200, NULL, "2000");
	free(response_payload); response_payload = NULL; 
	free_cin(gcin); gcin = NULL;
}

void retrieve_sub(Node *pnode){
	Sub* gsub = db_get_sub(pnode->ri);
	response_payload = sub_to_json(gsub);
	respond_to_client(200, NULL, "2000");
	free(response_payload); response_payload = NULL; 
	free_sub(gsub); gsub = NULL;
}

void retrieve_acp(Node *pnode){
	ACP* gacp = db_get_acp(pnode->ri);
	response_payload = acp_to_json(gacp);
	respond_to_client(200, NULL, "2000");
	free(response_payload); response_payload = NULL; 
	free_acp(gacp); gacp = NULL;
}

void update_ae(Node *pnode) {
	char invalid_key[][16] = {"m2m:ae-ty", "m2m:ae-pi", "m2m:ae-ri"};
	int invalid_key_size = sizeof(invalid_key)/(16*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(json_key_exist(payload, invalid_key[i])) {
			respond_to_client(200, "{\"m2m:dbg\": \"unsupported attribute on update\"}", "4000");
			return;
		}
	}

	AE* after = db_get_ae(pnode->ri);
	int result;

	set_ae_update(after);
	set_node_update(pnode, after);
	result = db_delete_object(after->ri);
	result = db_store_ae(after);
	
	response_payload = ae_to_json(after);
	respond_to_client(200, NULL, "2004");
	notify_object(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	free(response_payload); response_payload = NULL; 
	free_ae(after); after = NULL;
}

void update_cnt(Node *pnode) {
	char invalid_key[][16] = {"m2m:cnt-ty", "m2m:cnt-pi", "m2m:cnt-ri"};
	int invalid_key_size = sizeof(invalid_key)/(16*sizeof(char));
	for(int i=0; i<invalid_key_size; i++) {
		if(json_key_exist(payload, invalid_key[i])) {
			respond_to_client(200, "{\"m2m:dbg\": \"unsupported attribute on update\"}", "4000");
			return;
		}
	}

	CNT* after = db_get_cnt(pnode->ri);
	int result;

	set_cnt_update(after);
	set_node_update(pnode, after);
	result = db_delete_object(after->ri);
	result = db_store_cnt(after);
	
	response_payload = cnt_to_json(after);
	respond_to_client(200, NULL, "2004");
	notify_object(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	free(response_payload); response_payload = NULL;
	free_cnt(after); after = NULL;
}

void update_sub(Node *pnode) {
	Sub* after = db_get_sub(pnode->ri);
	int result;
	
	set_sub_update(after);
	set_node_update(pnode, after);
	result = db_delete_sub(after->ri);
	result = db_store_sub(after);
	
	response_payload = sub_to_json(after);
	respond_to_client(200, NULL, "2004");
	free(response_payload); response_payload = NULL;
	free_sub(after); after = NULL;
}

void update_acp(Node *pnode) {
	ACP* after = db_get_acp(pnode->ri);
	int result;
	
	set_acp_update(after);
	set_node_update(pnode, after);
	result = db_delete_acp(after->ri);
	result = db_store_acp(after);
	
	response_payload = acp_to_json(after);
	respond_to_client(200, NULL, "2004");
	notify_object(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	free(response_payload); response_payload = NULL;
	free_acp(after); after = NULL;
}

void delete_object(Node* pnode) {
	fprintf(stderr,"\x1b[41mDelete Object\x1b[0m\n");
	if(pnode->ty == TY_AE || pnode->ty == TY_CNT) {
		if(check_privilege(pnode, ACOP_DELETE, OP_DELETE, pnode->ty) == -1) {
			return;
		}
	}
	if(pnode->ty == TY_CSE) {
		respond_to_client(403, "{\"m2m:dbg\": \"CSE can not be deleted\"}", "4005");
		return;
	}
	delete_node_and_db_data(pnode,1);
	pnode = NULL;
	respond_to_client(200, "{\"m2m:dbg\": \"resource is deleted successfully\"}", "2002");
}

void restruct_resource_tree(){
	Node *node_list = (Node *)calloc(1,sizeof(Node));
	Node *tail = node_list;
	
	if(access("./RESOURCE.db", 0) != -1) {
		Node* ae_list = db_get_all_ae();
		tail->sibling_right = ae_list;
		if(ae_list) ae_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;

		Node* cnt_list = db_get_all_cnt();
		tail->sibling_right = cnt_list;
		if(cnt_list) cnt_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;
	} else {
		fprintf(stderr,"RESOURCE.db is not exist\n");
	}
	
	if(access("./SUB.db", 0) != -1) {
		Node* sub_list = db_get_all_sub();
		tail->sibling_right = sub_list;
		if(sub_list) sub_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;
	} else {
		fprintf(stderr,"SUB.db is not exist\n");
	}

	if(access("./ACP.db", 0) != -1) {
		Node* acp_list = db_get_all_acp();
		tail->sibling_right = acp_list;
		if(acp_list) acp_list->sibling_left = tail;
		while(tail->sibling_right) tail = tail->sibling_right;
	} else {
		fprintf(stderr,"ACP.db is not exist\n");
	}
	
	Node *fnode = node_list;
	node_list = node_list->sibling_right;
	if(node_list) node_list->sibling_left = NULL;
	free_node(fnode);
	
	if(node_list) restruct_resource_tree_child(rt->cb, node_list);
}

Node* restruct_resource_tree_child(Node *pnode, Node *list) {
	Node *node = list;
	
	while(node) {
		Node *right = node->sibling_right;

		if(!strcmp(pnode->ri, node->pi)) {
			Node *left = node->sibling_left;
			
			if(!left) {
				list = right;
			} else {
				left->sibling_right = right;
			}
			
			if(right) right->sibling_left = left;
			node->sibling_left = node->sibling_right = NULL;
			add_child_resource_tree(pnode, node);
		}
		node = right;
	}
	Node *child = pnode->child;
	
	while(child) {
		list = restruct_resource_tree_child(child, list);
		child = child->sibling_right;
	}
	
	return list;
}

void no_mandatory_error(){
	fprintf(stderr,"No Mandatory Error\n");
	respond_to_client(400, "{\"m2m:dbg\": \"insufficient mandatory attribute\"}", "4102");
}

void child_type_error(){
	fprintf(stderr,"Child Type Error\n");
	respond_to_client(403, "{\"m2m:dbg\": \"child can not be created under the type of parent\"}", "4108");
}

int check_json_format() {
	cJSON *json = cJSON_Parse(payload);
	if(json == NULL) {
		fprintf(stderr,"Body Format Invalid\n");
		respond_to_client(400,"{\"m2m:dbg\": \"body format invalid\"}", "4000");
		cJSON_Delete(json);
		return -1;
	}
	cJSON_Delete(json);
	return 0;
}

int check_privilege(Node *node, ACOP acop, Operation op, ObjectType target_ty) {
	bool deny = false;

	Node *parent_node = node;

	while(parent_node && parent_node->ty != TY_AE) {
		parent_node = parent_node->parent;
	}

	char *origin = request_header("X-M2M-Origin");

	if(!origin) {
		if(!(op == OP_CREATE && target_ty == TY_AE)) {
			deny = true;
		}
	} else if(!strcmp(origin, "CAdmin")) {
		deny = false;
	} else if((parent_node && strcmp(origin, parent_node->ri))) {
		deny = true;
	}

	if(node->ty == TY_CIN) node = node->parent;

	if((node->ty == TY_CNT || node->ty == TY_ACP) && (get_acop(node) & acop) != acop) {
		deny = true;
	}

	if(deny) {
		fprintf(stderr,"Originator has no privilege\n");
		respond_to_client(403, "{\"m2m:dbg\": \"originator has no privilege.\"}", "4103");
		return -1;
	}

	return 0;
}

int check_request_body_empty() {
	if(!payload) {
		fprintf(stderr,"Request body empty error\n");
		respond_to_client(400, "{\"m2m:dbg\": \"request body is empty\"}", "5000");
		return -1;
	}
	return 0;
}

int check_resource_aei_duplicate(Node *node) {
	if(!node) return 0;

	if(check_same_resource_aei_exists(node)) {
		fprintf(stderr,"Resource aei duplicate error\n");
		respond_to_client(209, "{\"m2m:dbg\": \"attribute `aei` is duplicated\"}", "4117");
		return -1;
	}
	return 0;
}

int check_same_resource_aei_exists(Node *node) {
	char *origin = request_header("X-M2M-Origin");
	if(!origin) return 0;

	char aei[128] = {'\0'};
	if(origin[0] != 'C') {
		aei[0] = 'C';
	}
	strcpy(aei, origin);
	node = node->child;

	while(node) {
		if(!strcmp(node->ri, aei)) {
			return 1;
		}
		node = node->sibling_right;
	}
	return 0;
}

int check_resource_name_duplicate(Node *node) {
	if(!node) return 0;

	if(check_same_resource_name_exists(node)) {
		fprintf(stderr,"Resource name duplicate error\n");
		respond_to_client(209, "{\"m2m:dbg\": \"attribute `rn` is duplicated\"}", "4105");
		return -1;
	}
	return 0;
}

int check_same_resource_name_exists(Node *pnode) {
	Node* node = pnode->child;
	char* rn = get_json_value_char("rn",payload);
	if(!rn) return 0;

	while(node) {
		if(!strcmp(node->rn, rn)) return 1;
		node = node->sibling_right;
	}

	return 0;
}

int check_resource_type_equal(ObjectType ty1, ObjectType ty2) {	
	if(ty1 != ty2) {
		fprintf(stderr,"Resource type error\n");
		respond_to_client(400, "{\"m2m:dbg\": \"resource type error\"}", "4000");
		return -1;
	}
	return 0;
}

int result_parse_uri(Node *node) {
	if(!node) {
		fprintf(stderr,"Invalid\n");
		respond_to_client(404, "{\"m2m:dbg\": \"URI is invalid\"}", "4004");
		return -1;
	} else {
		fprintf(stderr,"OK\n");
		return 0;
	} 
}

int check_payload_size() {
	if(payload && payload_size > MAX_PAYLOAD_SIZE) {
		fprintf(stderr,"Request payload too large\n");
		respond_to_client(413, "{\"m2m:dbg\": \"payload is too large\"}", "5207");
		return -1;
	}
	return 0;
}

void retrieve_filtercriteria_data(Node *node, ObjectType ty, char **discovery_list, int *size, int level, int curr, int flag) {
	if(!node || curr > level) return;

	Node *child[1024];
	Node *sibling[1024];
	int index = -1;

	if(flag == 1) {
		if(!node->uri) set_node_uri(node);
		child[++index] = node->child;
		sibling[index] = node;
	}

	while(node && flag == 0) {
		if(!node->uri) set_node_uri(node);

		if(ty != -1) {
			if(node->ty == ty) {
				discovery_list[*size] = (char*)malloc(MAX_URI_SIZE*sizeof(char));
				strcpy(discovery_list[(*size)++], node->uri);
			}
		} else {
			discovery_list[*size] = (char*)malloc(MAX_URI_SIZE*sizeof(char));
			strcpy(discovery_list[(*size)++], node->uri);
		}
		child[++index] = node->child;
		sibling[index] = node;
		node = node->sibling_right;
	}

	if((ty == -1 || ty == TY_CIN) && curr < level) {
		for(int i= 0; i <= index; i++) {
			Node *cin_list_head = db_get_cin_list_by_pi(sibling[i]->ri);
			Node *p = cin_list_head;

			while(p) {
				discovery_list[*size] = (char*)malloc(MAX_URI_SIZE*sizeof(char));
				strcpy(discovery_list[*size], sibling[i]->uri);
				strcat(discovery_list[*size], "/");
				strcat(discovery_list[*size], p->ri);
				(*size)++;
				p = p->sibling_right;
			}

			free_node_list(cin_list_head);
		}
	}

	for(int i = 0; i<=index; i++) {
		retrieve_filtercriteria_data(child[i], ty, discovery_list, size, level, curr+1, 0);
	} 
}

void retrieve_object_filtercriteria(Node *pnode) {
	char **discovery_list = (char**)malloc(65536*sizeof(char*));
	int size = 0;
	ObjectType ty = get_value_querystring_int("ty");
	int level = get_value_querystring_int("lvl");
	if(level == -1) level = INT32_MAX;

	retrieve_filtercriteria_data(pnode, ty, discovery_list, &size, level, 0, 1);

	response_payload = discovery_to_json(discovery_list, size);
	respond_to_client(200, NULL, "2000");
	for(int i=0; i<size; i++) free(discovery_list[i]);
	free(discovery_list);
	free(response_payload); response_payload = NULL;

	return;
}


void *mqtt_serve(){
	int result = 0;
	result = mqtt_ser();
}

int check_resource_name_invalid(ObjectType ty) {
	char key[16];

	switch(ty) {
		case TY_AE: strcpy(key, "m2m:ae"); break;
		case TY_CNT: strcpy(key, "m2m:cnt"); break;
		case TY_SUB: strcpy(key, "m2m:sub"); break;
		case TY_ACP: strcpy(key, "m2m:acp"); break;
	}

	strcat(key, "-rn");

	char *rn = get_json_value_string(payload, key);
	if(!rn) return 0;
	int len_rn = strlen(rn);

	for(int i=0; i<len_rn; i++) {
		if(rn[i] != '_' && rn[i] != '-' && !is_rn_valid_char(rn[i])) {
			fprintf(stderr,"Resource name is invalid");
			respond_to_client(406, "{\"m2m:dbg\": \"attribute `rn` is invalid\"}", "4000");
			free(rn);
			return -1;
		}
	}

	free(rn);
	return 0;
}

bool is_rn_valid_char(char c) {
	return ((48 <= c && c <=57) || (65 <= c && c <= 90) || (97 <= c && c <= 122));
}