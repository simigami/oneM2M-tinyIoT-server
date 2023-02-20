#ifndef __UTIL_H__
#define __UTIL_H__

void init_server();

void set_o2pt_pc(oneM2MPrimitive *o2pt, char *pc, ...);
void set_o2pt_rsc(oneM2MPrimitive *o2pt, int rsc);
void log_runtime(double start);

RTNode* parse_uri(oneM2MPrimitive *o2pt, RTNode *cb);

//Resource Tree
void restruct_resource_tree();
int add_child_resource_tree(RTNode *parent, RTNode *child);
RTNode *find_rtnode_by_uri(RTNode *cse, char *node_uri);

//error
void no_mandatory_error(oneM2MPrimitive *o2pt);
void child_type_error(oneM2MPrimitive *o2pt);
int check_privilege(oneM2MPrimitive *o2pt, RTNode *target_rtnode, ACOP acop);
int check_payload_empty(oneM2MPrimitive *o2pt);
int check_rn_duplicate(oneM2MPrimitive *o2pt, RTNode *rtnode);
int check_aei_duplicate(oneM2MPrimitive *o2pt, RTNode *rtnode);
int check_resource_type_equal(oneM2MPrimitive *o2pt);
int check_resource_type_invalid(oneM2MPrimitive *o2pt);
int result_parse_uri(oneM2MPrimitive *o2pt, RTNode *target_rtnode);
int check_payload_size(oneM2MPrimitive *o2pt);
int check_payload_format(oneM2MPrimitive *o2pt);
int check_rn_invalid(oneM2MPrimitive *o2pt, ResourceType ty);
void api_prefix_invalid(oneM2MPrimitive *o2pt);
void too_large_content_size_error(oneM2MPrimitive *o2pt);
void mni_mbs_invalid(oneM2MPrimitive *o2pt, char *attribute);
void db_store_fail(oneM2MPrimitive *o2pt);

//etc
char* get_local_time(int diff);
char* resource_identifier(ResourceType ty, char *ct);
void delete_cin_under_cnt_mni_mbs(CNT *cnt);
void respond_to_client(oneM2MPrimitive *o2pt, int status);
void cin_in_period(RTNode *pnode);
void object_test_api(RTNode *node);
char* json_label_value(char *json_payload);
int net_to_bit(char *net);
int get_acop(RTNode *node);
int get_acop_origin(char *origin, RTNode *acp, int flag);
int get_value_querystring_int(char *key);
void remove_invalid_char_json(char* json);
int is_json_valid_char(char c);
bool is_rn_valid_char(char c);

ResourceType http_parse_object_type();
ResourceType parse_object_type_cjson(cJSON *cjson);

struct url_data { size_t size; char* data;};
size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data);
int send_http_packet(char *target, char *post_data);

#endif