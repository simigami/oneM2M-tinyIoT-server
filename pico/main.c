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
#include <signal.h>
#include "onem2m.h"
#include "jsonparse.h"
#include "berkeleyDB.h"
#include "httpd.h"
#include "cJSON.h"
#include "util.h"
#include "config.h"
#include "onem2mTypes.h"
#include "mqttClient.h"

ResourceTree *rt;
extern void *mqtt_serve();
void stop_server(int sig);
int terminate = 0;
#ifdef ENABLE_MQTT
pthread_t mqtt;
#endif


int main(int c, char **v) {
	signal(SIGINT, stop_server);
	
	if(!init_dbp()){
		logger("MAIN", LOG_LEVEL_ERROR, "DB Error");
		return 0;
	}
	init_server();
	
	#ifdef ENABLE_MQTT
	int mqtt_thread_id;
	mqtt_thread_id = pthread_create(&mqtt, NULL, mqtt_serve, "mqtt Client");
	if(mqtt_thread_id < 0){
		fprintf(stderr, "MQTT thread create error\n");
		return 0;
	}
	#endif

	serve_forever(SERVER_PORT); // main oneM2M operation logic in void route()    

	return 0;
}

void route(oneM2MPrimitive *o2pt) {
	int rsc = 0;
    double start;

    start = (double)clock() / CLOCKS_PER_SEC; // runtime check - start
	RTNode* target_rtnode = parse_uri(o2pt, rt->cb);
	int e = result_parse_uri(o2pt, target_rtnode);

	if(e != -1) e = check_payload_size(o2pt);
	if(e == -1) {
		log_runtime(start);
		respond_to_client(o2pt, rsc_to_http_status(rsc));
		return;
	}

	if(o2pt->isFopt)
		rsc = fopt_onem2m_resource(o2pt, target_rtnode);
	else{
		rsc = handle_onem2m_request(o2pt, target_rtnode);
	
		if(o2pt->op != OP_DELETE && target_rtnode->ty == RT_CIN){
			free_rtnode(target_rtnode);
			target_rtnode = NULL;
		}
	}

	
	respond_to_client(o2pt, rsc_to_http_status(rsc));
	log_runtime(start);
}

int handle_onem2m_request(oneM2MPrimitive *o2pt, RTNode *target_rtnode){
	int rsc = 0;

	switch(o2pt->op) {
		
		case OP_CREATE:	
			rsc = create_onem2m_resource(o2pt, target_rtnode); break;
		
		case OP_RETRIEVE:
			rsc = retrieve_onem2m_resource(o2pt, target_rtnode); break;
			
		case OP_UPDATE: 
			rsc = update_onem2m_resource(o2pt, target_rtnode); break;
			
		case OP_DELETE:
			rsc = delete_onem2m_resource(o2pt, target_rtnode); break;

		case OP_VIEWER:
			rsc = tree_viewer_api(o2pt, target_rtnode); break;
		
		//case OP_OPTIONS:
			//respond_to_client(200, "{\"m2m:dbg\": \"response about options method\"}", "2000");
			//break;
		
		default:
			handle_error(o2pt, RSC_INTERNAL_SERVER_ERROR, "{\"m2m:dbg\": \"internal server error\"}");
			// logger("MAIN", LOG_LEVEL_ERROR, "Internal server error");
			// set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"internal server error\"}");
			// o2pt->rsc = RSC_INTERNAL_SERVER_ERROR;
			//respond_to_client(o2pt, 500);
			return RSC_INTERNAL_SERVER_ERROR;
		}

		return rsc;
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
		logger("MAIN", LOG_LEVEL_INFO, "Create AE");
		rsc = create_ae(o2pt, parent_rtnode);
		break;	

	case RT_CNT :
		logger("MAIN", LOG_LEVEL_INFO, "Create CNT");
		rsc = create_cnt(o2pt, parent_rtnode);
		break;
		
	case RT_CIN :
		logger("MAIN", LOG_LEVEL_INFO, "Create CIN");
		rsc = create_cin(o2pt, parent_rtnode);
		break;

	case RT_SUB :
		logger("MAIN", LOG_LEVEL_INFO, "Create SUB");
		rsc = create_sub(o2pt, parent_rtnode);
		break;
	
	case RT_ACP :
		logger("MAIN", LOG_LEVEL_INFO, "Create ACP");
		rsc = create_acp(o2pt, parent_rtnode);
		break;

	case RT_GRP:
		logger("MAIN", LOG_LEVEL_INFO, "Create GRP");
		rsc = create_grp(o2pt, parent_rtnode);
		break;

	case RT_MIXED :
		logger("MAIN", LOG_LEVEL_ERROR, "Resource type is invalid");
		set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"resource type error\"}");
		rsc = o2pt->rsc = RSC_BAD_REQUEST;
		//respond_to_client(o2pt, 400);
	}	
	return rsc;
}

int retrieve_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode) {
	int rsc = 0;
	int e = check_privilege(o2pt, target_rtnode, ACOP_RETRIEVE);

	if(e == -1) return o2pt->rsc;
	/*
	int fu = get_value_querystring_int("fu");

	if(fu == 1) {
		fprintf(stderr,"\x1b[43mRetrieve FilterCriteria\x1b[0m\n");
		retrieve_object_filtercriteria(pnode);
		return;
	}
	*/

	switch(target_rtnode->ty) {
		
	case RT_CSE :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve CSE");
        rsc = retrieve_cse(o2pt, target_rtnode);
      	break;
	
	case RT_AE : 
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve AE");
		rsc = retrieve_ae(o2pt, target_rtnode);	
		break;	
			
	case RT_CNT :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve CNT");
		rsc = retrieve_cnt(o2pt, target_rtnode);			
		break;
			
	case RT_CIN :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve CIN");
		rsc = retrieve_cin(o2pt, target_rtnode);			
		break;

	case RT_GRP :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve GRP");
		rsc = retrieve_grp(o2pt, target_rtnode);	
		break;

	case RT_SUB :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve SUB");
		rsc = retrieve_sub(o2pt, target_rtnode);			
		break;

	case RT_ACP :
		logger("MAIN", LOG_LEVEL_INFO, "Retrieve ACP");
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
			logger("MAIN", LOG_LEVEL_INFO, "Update AE");
			rsc = update_ae(o2pt, target_rtnode);
			break;

		case RT_CNT :
			logger("MAIN", LOG_LEVEL_INFO, "Update CNT");
			rsc = update_cnt(o2pt, target_rtnode);
			break;

		// case RT_SUB :
		//	logger("MAIN", LOG_LEVEL_INFO, "Update SUB");
		// 	update_sub(pnode);
		// 	break;
		
		case RT_ACP :
			logger("MAIN", LOG_LEVEL_INFO, "Update ACP");
			rsc = update_acp(o2pt, target_rtnode);
			break;

		case RT_GRP:
			logger("MAIN", LOG_LEVEL_INFO, "Update GRP");
			rsc = update_grp(o2pt, target_rtnode);
			break;

		default :
			logger("MAIN", LOG_LEVEL_ERROR, "Resource type does not support Update");
			set_o2pt_pc(o2pt, "{\"m2m:dbg\": \"`Update` operation is unsupported\"}");
			rsc = o2pt->rsc = RSC_OPERATION_NOT_ALLOWED;
			//respond_to_client(o2pt, 400);
		}
	return rsc;
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

void retrieve_filtercriteria_data(RTNode *node, ResourceType ty, char **discovery_list, int *size, int level, int curr, int flag) {
	if(!node || curr > level) return;

	RTNode *child[1024];
	RTNode *sibling[1024];
	int index = -1;

void *mqtt_serve(){
	int result = 0;
	result = mqtt_ser();
}
*/

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
	logger("MAIN", LOG_LEVEL_DEBUG, "handle fopt");


	grp = db_get_grp(parent_rtnode->ri);
	if(!grp){
		o2pt->rsc = RSC_INTERNAL_SERVER_ERROR;
		return RSC_INTERNAL_SERVER_ERROR;
	}
	
	if(grp->cnm == 0){
		logger("MAIN", LOG_LEVEL_DEBUG, "No member to fanout");
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
			req_o2pt->fr = strdup(target_rtnode->ri);
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
			logger("MAIN", LOG_LEVEL_DEBUG, "rtnode not found");
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

void stop_server(int sig){
	logger("MAIN", LOG_LEVEL_INFO, "Shutting down server...");
	#ifdef ENABLE_MQTT
	terminate = 1;
	logger("MAIN", LOG_LEVEL_INFO, "Waiting for MQTT Client to shut down...");
	pthread_join(mqtt, NULL);
	#endif
	logger("MAIN", LOG_LEVEL_INFO, "Closing DB...");
	close_dbp();
	logger("MAIN", LOG_LEVEL_INFO, "Cleaning ResourceTree...");
	free_all_resource(rt->cb);
	free(rt);
	logger("MAIN", LOG_LEVEL_INFO, "Done");
	exit(0);
}