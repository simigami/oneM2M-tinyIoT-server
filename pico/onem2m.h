#ifndef __ONEM2M_H__
#define __ONEM2M_H__

#include <stdbool.h>
#include "cJSON.h"
#include "onem2mTypes.h"

//enum
typedef enum {
	PROT_HTTP = 1,
	PROT_MQTT
}Protocol;

typedef enum {
	OP_NONE = 0,
	OP_CREATE,
	OP_RETRIEVE,
	OP_UPDATE,
	OP_DELETE,
	OP_VIEWER = 1000,
	OP_OPTIONS
}Operation;

typedef enum {
	NOTIFICATION_EVENT_1 = 1,
	NOTIFICATION_EVENT_2 = 2,
	NOTIFICATION_EVENT_3 = 4,
	NOTIFICATION_EVENT_4 = 8
}NET;

typedef enum {
	ACOP_CREATE = 1,
	ACOP_RETRIEVE = 2,
	ACOP_UPDATE = 4,
	ACOP_DELETE = 8,
	ACOP_NOTIFY = 16,
	ACOP_DISCOVERY = 32
}ACOP;

//oneM2M Resource
typedef struct {
	char *ct;
	char *lt;
	char *rn;
	char *ri;
	char *pi;
	char *csi;
	ResourceType ty;
} CSE;

typedef struct {
	char *et;
	char *ct;
	char *lt;
	char *rn;
	char *ri;
	char *pi;
	char *api;
	char *aei;
	char *lbl;
	char *srv;
	char *acpi;
	ResourceType ty;
	bool rr;
} AE;

typedef struct {
	char *et;
	char *ct;
	char *lt;
	char *rn;
	char *ri;
	char *pi;
	char *lbl;
	char *acpi;
	ResourceType ty;
	int st;
	int cni;
	int cbs;
	int mni;
	int mbs;
} CNT;

typedef struct {
	char *et;
	char *ct;
	char *lt;
	char *rn;
	char *ri;
	char *pi;
	char *con;	
	ResourceType ty;
	int st;
	int cs;
} CIN;

typedef struct {
	char *et;
	char *ct;
	char *lt;
	char *rn;
	char *ri;
	char *pi;
	char *nu;
	char *net;
	char *sur;
	ResourceType ty;
	int nct;
} SUB;

typedef struct {
	char *rn;
	char *pi;
	char *ri;
	char *ct;
	char *lt;
	char *et;
	char *pv_acor;
	char *pv_acop;
	char *pvs_acor;
	char *pvs_acop;
	ResourceType ty;
} ACP;

typedef struct {
	char *rn;
	char *pi;
	char *ri;
	char *ct;
	char *lt;
	char *et;
	char *acpi;
	char *macp;
	
	ResourceType mt;
	int mnm;
	int cnm;

	char **mid;
	bool mtv;
	ConsistencyStrategy csy;
} GRP;

//Resource Tree
typedef struct RTNode {
	struct RTNode *parent;
	struct RTNode *child;
	struct RTNode *sibling_left;
	struct RTNode *sibling_right;

	char *uri;
	ResourceType ty;
	void *obj;
}RTNode;

typedef struct {  
	RTNode *cb;
}ResourceTree;

typedef struct {
	char *to;
	char *fr;
	char *rqi;
	char *rvi;
	char *pc;
	Operation op;
	Protocol prot;
	cJSON *cjson_pc;
	int rsc;
	int ty;
	char *origin;
	char *req_type;
	bool isFopt;
	char *fopt;
}oneM2MPrimitive;

//onem2m resource
int create_onem2m_resource(oneM2MPrimitive *o2pt, RTNode* target_rtnode);
int handle_onem2m_request(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_object_filtercriteria(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int update_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int delete_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
void notify_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *node, char *response_payload, NET net);
int fopt_onem2m_resource(oneM2MPrimitive *o2pt, RTNode *parent_rtnode);

int create_ae(oneM2MPrimitive *o2pt, RTNode *parent_rtnode);
int create_cnt(oneM2MPrimitive *o2pt, RTNode *parent_rtnode);
int create_cin(oneM2MPrimitive *o2pt, RTNode *parent_rtnode);
int create_sub(oneM2MPrimitive *o2pt, RTNode *parent_rtnode);
int create_acp(oneM2MPrimitive *o2pt, RTNode *parent_rtnode);
int create_grp(oneM2MPrimitive *o2pt, RTNode *parent_rtnode);

int retrieve_cse(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_ae(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_cnt(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_cin(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_cin_latest(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_cin_by_ri(char *ri);
int retrieve_sub(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_acp(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int retrieve_grp(oneM2MPrimitive *o2pt, RTNode *target_rtnode);

int update_cse(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int update_ae(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int update_cnt(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int update_sub(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int update_acp(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int update_grp(oneM2MPrimitive *o2pt, RTNode *target_rtnode);


void init_cse(CSE* cse);
void init_ae(AE* ae, char *pi, char *origin);
void init_cnt(CNT* cnt, char *pi);
void init_cin(CIN* cin, char *pi);
void init_sub(SUB* sub, char *pi, char *uri);
void init_acp(ACP* acp, char *pi);
void init_grp(GRP* grp, char *pi);
int set_ae_update(cJSON *m2m_ae, AE* after);
int set_cnt_update(cJSON *m2m_cnt, CNT* after);
int set_sub_update(cJSON *m2m_sub, SUB* after);
int set_acp_update(cJSON *m2m_acp, ACP* after);
int set_grp_update(cJSON *m2m_grp, GRP* after);
void set_rtnode_update(RTNode* rtnode, void *after);

void free_cse(CSE* cse);
void free_ae(AE* ae);
void free_cnt(CNT* cnt);
void free_cin(CIN* cin);
void free_sub(SUB* sub);
void free_acp(ACP *acp);
void free_grp(GRP *grp);

//resource tree
RTNode* create_rtnode(void *resource, ResourceType ty);
int delete_rtnode_and_db_data(RTNode *node, int flag);
void free_rtnode(RTNode *node);
void free_rtnode_list(RTNode *node);

RTNode* restruct_resource_tree(RTNode *node, RTNode *list);
RTNode* latest_cin_list(RTNode *cinList, int num); // use in viewer API
RTNode* find_latest_oldest(RTNode* node, int flag);
void set_node_uri(RTNode* node);

//etc
int update_cnt_cin(RTNode *cnt_rtnode, RTNode *cin_rtnode, int sign);

#define MAX_TREE_VIEWER_SIZE 65536
#define EXPIRE_TIME -3600*24*365*2
#define ALL_ACOP ACOP_CREATE + ACOP_RETRIEVE + ACOP_UPDATE + ACOP_DELETE + ACOP_NOTIFY + ACOP_DISCOVERY

#endif