#ifndef __ONEM2M_H__
#define __ONEM2M_H__

#include <stdbool.h>

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
	OP_OPTIONS,
	OP_LATEST,
	OP_OLDEST
}Operation;

typedef enum {
	TY_NONE = 0,
	TY_ACP,
	TY_AE,
	TY_CNT,
	TY_CIN,
	TY_CSE,
	TY_SUB = 23
}ObjectType;

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
	int ty;
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
	int ty;
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
	int ty;
	int st;
	int cni;
	int cbs;
} CNT;

typedef struct {
	char *et;
	char *ct;
	char *lt;
	char *rn;
	char *ri;
	char *pi;
	char *con;	
	int ty;
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
	int ty;
	int nct;
} Sub;

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
	int ty;
} ACP;

//Resource Tree
typedef struct RTNode {
	struct RTNode *parent;
	struct RTNode *child;
	struct RTNode *sibling_left;
	struct RTNode *sibling_right;
	
	char *rn;
	char *ri;
	char *pi;
	char *nu;
	char *sur;
	char *acpi;
	char *pv_acor;
	char *pv_acop;
	char *pvs_acor;
	char *pvs_acop;
	char *uri;
	ObjectType ty;

	int net;
}RTNode;

typedef struct {  
	RTNode *cb;
}ResourceTree;

typedef struct {
	char *to;
	char *fr;
	char *rqi;
	char *rsc;
	char *rvi;
	char *pc;
	Operation op;
	Protocol prot;
	int ty;
}oneM2MPrimitive;

//http request
RTNode* parse_uri(oneM2MPrimitive *o2pt, RTNode *cb);
ObjectType http_parse_object_type();
ObjectType parse_object_type_in_request_body();

//onem2m resource
void create_object(oneM2MPrimitive *o2pt, RTNode* pnode);
void retrieve_object(oneM2MPrimitive *o2pt, RTNode *pnode);
void retrieve_object_filtercriteria(oneM2MPrimitive *o2pt, RTNode *pnode);
void update_object(oneM2MPrimitive *o2pt, RTNode *pnode);
void delete_object(oneM2MPrimitive *o2pt, RTNode *pnode);
void notify_object(oneM2MPrimitive *o2pt, RTNode *node, char *response_payload, NET net);

void create_ae(RTNode *pnode);
void create_cnt(RTNode *pnode);
void create_cin(RTNode *pnode);
void create_sub(RTNode *pnode);
void create_acp(RTNode *pnode);

void retrieve_cse(oneM2MPrimitive *o2pt, RTNode *pnode);
void retrieve_ae(oneM2MPrimitive *o2pt, RTNode *pnode);
void retrieve_cnt(oneM2MPrimitive *o2pt, RTNode *pnode);
void retrieve_cin(oneM2MPrimitive *o2pt, RTNode *pnode);
void retrieve_cin_latest(oneM2MPrimitive *o2pt, RTNode *pnode);
void retrieve_cin_by_ri(char *ri);
void retrieve_sub(oneM2MPrimitive *o2pt, RTNode *pnode);
void retrieve_acp(oneM2MPrimitive *o2pt, RTNode *pnode);

void update_cse(RTNode *pnode);
void update_ae(RTNode *pnode);
void update_cnt(RTNode *pnode);
void update_sub(RTNode *pnode);
void update_acp(RTNode *pnode);

void init_cse(CSE* cse);
void init_ae(AE* ae, char *pi);
void init_cnt(CNT* cnt, char *pi);
void init_cin(CIN* cin, char *pi);
void init_sub(Sub* sub, char *pi);
void init_acp(ACP* acp, char *pi);
void set_ae_update(AE* after);
void set_cnt_update(CNT* after);
void set_sub_update(Sub* after);
void set_acp_update(ACP* after);
void set_node_update(RTNode* node, void *after);

void free_cse(CSE* cse);
void free_ae(AE* ae);
void free_cnt(CNT* cnt);
void free_cin(CIN* cin);
void free_sub(Sub* sub);
void free_acp(ACP *acp);

//resource tree
RTNode* create_node(void *obj, ObjectType ty);
RTNode* create_cse_node(CSE *cse);
RTNode* create_ae_node(AE *ae);
RTNode* create_cnt_node(CNT *cnt);
RTNode* create_cin_node(CIN *cin);
RTNode* create_sub_node(Sub *sub);
RTNode* create_acp_node(ACP *acp);
int add_child_resource_tree(RTNode *parent, RTNode *child);
RTNode *find_rtnode_by_uri(RTNode *cse, char *node_uri);
void delete_node_and_db_data(RTNode *node, int flag);
void free_rtnode(RTNode *node);
void free_rtnode_list(RTNode *node);

void tree_viewer_api(RTNode *node);
void tree_viewer_data(RTNode *node, char **viewer_data, int cin_size);
void restruct_resource_tree();
RTNode* restruct_resource_tree_child(RTNode *node, RTNode *list);
RTNode* latest_cin_list(RTNode *cinList, int num); // use in viewer API
RTNode* find_latest_oldest(RTNode* node, Operation *op);
void set_node_uri(RTNode* node);

//json
void remove_invalid_char_json(char* json);
int is_json_valid_char(char c);
bool is_rn_valid_char(char c);

//http etc
struct url_data { size_t size; char* data;};
size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data);
int send_http_packet(char *target, char *post_data);

//exception
void no_mandatory_error();
void child_type_error();
int check_privilege(RTNode *node, ACOP acop, Operation op, ObjectType target_ty);
int check_request_body_empty();
int check_resource_name_duplicate(RTNode *node);
int check_same_resource_name_exists(RTNode *pnode);
int check_resource_aei_duplicate(RTNode *node);
int check_same_resource_aei_exists(RTNode *node);
int check_resource_type_equal(ObjectType ty1, ObjectType ty2);
int result_parse_uri(RTNode *node, oneM2MPrimitive *o2pt);
int check_payload_size(oneM2MPrimitive *o2pt);
int check_json_format();
int check_resource_name_invalid(ObjectType ty);

//etc
void init_server();
char* get_local_time(int diff);
char* resource_identifier(ObjectType ty, char *ct);
void cin_in_period(RTNode *pnode);
void object_test_api(RTNode *node);
char* json_label_value(char *json_payload);
int net_to_bit(char *net);
int get_acop(RTNode *node);
int get_acop_origin(char *origin, RTNode *acp, int flag);
int get_value_querystring_int(char *key);
void log_runtime(double start);
void set_o2pt_pc(oneM2MPrimitive *o2pt, char *pc);
void set_o2pt_rsc(oneM2MPrimitive *o2pt, char *rsc);
void handle_http_request();
void respond_to_client(int status, oneM2MPrimitive *o2pt);

#define MAX_TREE_VIEWER_SIZE 65536
#define EXPIRE_TIME -3600*24*365*2
#define ALL_ACOP ACOP_CREATE + ACOP_RETRIEVE + ACOP_UPDATE + ACOP_DELETE + ACOP_NOTIFY + ACOP_DISCOVERY

#endif