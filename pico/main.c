#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <limits.h>
#include "onem2m.h"
#include "jsonparse.h"
#include "berkeleyDB.h"
#include "httpd.h"
#include "cJSON.h"
#include "config.h"
#include "onem2mTypes.h"
#include "mqttClient.h"
#include "util.h"

ResourceTree *rt;

int main(int c, char **v) {
	pthread_t mqtt;
	init_server();
	
	#ifdef ENABLE_MQTT
	int mqtt_thread_id;
	mqtt_thread_id = pthread_create(&mqtt, NULL, (void*) &mqtt_serve, "mqtt Client");
	if(mqtt_thread_id < 0){
		fprintf(stderr, "MQTT thread create error\n");
		return 0;
	}
	#endif

	serve_forever(SERVER_PORT); // main oneM2M operation logic in void route()    

	return 0;
}

void route(oneM2MPrimitive *o2pt) {
    double start;

    start = (double)clock() / CLOCKS_PER_SEC; // runtime check - start
	RTNode* target_rtnode = parse_uri(o2pt, rt->cb);
	int e = result_parse_uri(o2pt, target_rtnode);

	if(e != -1) e = check_payload_size(o2pt);
	if(e == -1) {
		log_runtime(start);
		return;
	}

	switch(o2pt->op) {
	
	case OP_CREATE:	
		create_onem2m_resource(o2pt, target_rtnode); break;
	
	case OP_RETRIEVE:
		retrieve_onem2m_resource(o2pt, target_rtnode); break;
		
	case OP_UPDATE: 
		update_onem2m_resource(o2pt, target_rtnode); break;
		
	case OP_DELETE:
		delete_onem2m_resource(o2pt, target_rtnode); break;

	case OP_VIEWER:
		tree_viewer_api(o2pt, target_rtnode); break;
	
	//case OP_OPTIONS:
		//respond_to_client(200, "{\"m2m:dbg\": \"response about options method\"}", "2000");
		//break;
	
	default:
		logger("MAIN", LOG_LEVEL_ERROR, "Internal server error");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"internal server error\"}");
		o2pt->rsc = RSC_INTERNAL_SERVER_ERROR;
		respond_to_client(o2pt, 500);
	}
	if(target_rtnode->ty == TY_CIN) free_rtnode(target_rtnode);

	log_runtime(start);

}

void create_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *parent_rtnode) {
	int e = check_resource_type_invalid(o2pt);
	if(e != -1) e = check_payload_empty(o2pt);
	if(e != -1) e = check_payload_format(o2pt);
	if(e != -1) e = check_resource_type_equal(o2pt);
	if(e != -1) e = check_privilege(o2pt, parent_rtnode, ACOP_CREATE);
	if(e != -1) e = check_rn_duplicate(o2pt, parent_rtnode);
	if(e == -1) return;

	switch(o2pt->ty) {	
	case TY_AE :
		logger("MAIN", LOG_LEVEL_INFO, "Create AE");
		create_ae(o2pt, parent_rtnode);
		break;	

	case TY_CNT :
		logger("MAIN", LOG_LEVEL_INFO, "Create CNT");
		create_cnt(o2pt, parent_rtnode);
		break;
		
	case TY_CIN :
		logger("MAIN", LOG_LEVEL_INFO, "Create CIN");
		create_cin(o2pt, parent_rtnode);
		break;

	case TY_SUB :
		logger("MAIN", LOG_LEVEL_INFO, "Create SUB");
		create_sub(o2pt, parent_rtnode);
		break;
	
	case TY_ACP :
		logger("MAIN", LOG_LEVEL_INFO, "Create ACP");
		create_acp(o2pt, parent_rtnode);
		break;

	case TY_NONE :
		logger("MAIN", LOG_LEVEL_ERROR, "Resource type is invalid");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"resource type error\"}");
		o2pt->rsc = RSC_BAD_REQUEST;
		respond_to_client(o2pt, 400);
	}	
}

void retrieve_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	int e = check_privilege(o2pt, target_rtnode, ACOP_RETRIEVE);

	if(e == -1) return;
	/*
	int fu = get_value_querystring_int("fu");

	if(fu == 1) {
		fprintf(stderr,"\x1b[43mRetrieve FilterCriteria\x1b[0m\n");
		retrieve_object_filtercriteria(pnode);
		return;
	}
	*/

	switch(target_rtnode->ty) {
		
	case TY_CSE :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve CSE");
        retrieve_cse(o2pt, target_rtnode);
      	break;
	
	case TY_AE : 
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve AE");
		retrieve_ae(o2pt, target_rtnode);	
		break;	
			
	case TY_CNT :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve CNT");
		retrieve_cnt(o2pt, target_rtnode);			
		break;
			
	case TY_CIN :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve CIN");
		retrieve_cin(o2pt, target_rtnode);			
		break;

	case TY_SUB :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve SUB");
		retrieve_sub(o2pt, target_rtnode);			
		break;

	case TY_ACP :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve ACP");
		retrieve_acp(o2pt, target_rtnode);			
		break;
	}	
}

void update_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	o2pt->ty = target_rtnode->ty;
	int e = check_payload_empty(o2pt);
	if(e != -1) e = check_payload_format(o2pt);
	ResourceType ty = parse_object_type_cjson(o2pt->cjson_pc);
	if(e != -1) e = check_resource_type_equal(o2pt);
	if(e != -1) e = check_privilege(o2pt, target_rtnode, ACOP_UPDATE);
	if(e != -1) e = check_rn_duplicate(o2pt, target_rtnode->parent);
	if(e == -1) return;
	
	switch(ty) {
	case TY_AE :
		logger("MAIN", LOG_LEVEL_INFO, "Update AE");
		update_ae(o2pt, target_rtnode);
		break;

	case TY_CNT :
		logger("MAIN", LOG_LEVEL_INFO, "Update CNT");
		update_cnt(o2pt, target_rtnode);
		break;

	// case TY_SUB :
	//	logger("MAIN", LOG_LEVEL_INFO, "Update SUB");
	// 	update_sub(pnode);
	// 	break;
	
	// case TY_ACP :
	// 	logger("MAIN", LOG_LEVEL_INFO, "Update ACP");
	// 	update_acp(pnode);
	// 	break;

	default :
		logger("MAIN", LOG_LEVEL_ERROR, "Resource type does not support Update");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"`Update` operation is unsupported\"}");
		o2pt->rsc = RSC_OPERATION_NOT_ALLOWED;
		respond_to_client(o2pt, 400);
	}
}
/*
void update_sub(RTNode *pnode) {
	Sub* after = db_get_sub(pnode->ri);
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

void update_acp(RTNode *pnode) {
	ACP* after = db_get_acp(pnode->ri);
	int result;
	
	set_acp_update(after);
	set_rtnode_update(pnode, after);
	result = db_delete_acp(after->ri);
	result = db_store_acp(after);
	
	response_payload = acp_to_json(after);
	respond_to_client(200, NULL, "2004");
	notify_onem2m_resource(pnode->child, response_payload, NOTIFICATION_EVENT_1);
	free(response_payload); response_payload = NULL;
	free_acp(after); after = NULL;
}

void retrieve_filtercriteria_data(RTNode *node, ResourceType ty, char **discovery_list, int *size, int level, int curr, int flag) {
	if(!node || curr > level) return;

	RTNode *child[1024];
	RTNode *sibling[1024];
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
			RTNode *cin_list_head = db_get_cin_rtnode_list_by_pi(sibling[i]->ri);
			RTNode *p = cin_list_head;

			while(p) {
				discovery_list[*size] = (char*)malloc(MAX_URI_SIZE*sizeof(char));
				strcpy(discovery_list[*size], sibling[i]->uri);
				strcat(discovery_list[*size], "/");
				strcat(discovery_list[*size], p->ri);
				(*size)++;
				p = p->sibling_right;
			}

			free_rtnode_list(cin_list_head);
		}
	}

	for(int i = 0; i<=index; i++) {
		retrieve_filtercriteria_data(child[i], ty, discovery_list, size, level, curr+1, 0);
	} 
}

void retrieve_object_filtercriteria(RTNode *pnode) {
	char **discovery_list = (char**)malloc(65536*sizeof(char*));
	int size = 0;
	ResourceType ty = get_value_querystring_int("ty");
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
*/