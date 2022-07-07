#include <stdio.h>
#include <stdbool.h>
#include <db.h>

typedef enum {
	o_CREATE = 1,
	o_RETRIEVE,
	o_UPDATE,
	o_DELETE
}Operation;

typedef enum {
	t_AE = 2,
	t_CNT,
	t_CIN,
	t_CSE
}ObjectType;

// OneM2M Resource struct
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
	char *csi;
	char *con;	
	int ty;
	int st;
} CIN;

typedef struct Node{
	struct Node *child;
	struct Node *sibling;
	
	char *rn;
	
	CSE *cse;
	AE *ae;
	CNT *cnt;
	CIN *cin;
}Node;

typedef struct {  
	Node *root;
}RT;

// OneM2M Resource function
Operation Parse_Operation();
ObjectType Parse_ObjectType();
ObjectType Parse_ObjectType_By_URI();

AE* Create_AE(char *json_payload);
CNT* Create_CNT(char *json_payload);
CIN* Create_CIN(char *json_payload);

CSE* Retrieve_CSE();
AE* Retrieve_AE();
CNT* Retrieve_CNT();
CIN* Retrieve_CIN();

CSE* Update_CSE(char *json_payload);
AE* Update_AE(char *json_payload);
CNT* Update_CNT(char *json_payload);

CSE* Delete_CSE();
AE* Delete_AE();
CNT* Delete_CNT();
CIN* Delete_CIN();

CSE* JSON_to_CSE(char *json_payload);
AE* JSON_to_AE(char *json_payload);
CNT* JSON_to_CNT(char *json_payload);
CIN* JSON_to_CIN(char *json_payload);

char* CSE_to_json(CSE* cse_object);
char* AE_to_json(AE* ae_object);
char* CNT_to_json(CNT* cnt_object);
char* CIN_to_json(CIN* cin_object);

int Store_CSE(CSE* cse_object);
int Store_AE(AE* ae_object);
int Store_CNT(CNT* cnt_object);
int Store_CIN(CIN* cin_object);

CSE* Get_CSE(char *ri);
AE* Get_AE(char *ri);
CNT* Get_CNT(char *ri);
CIN* Get_CIN(char *ri);

CSE* Get_sample_CSE();
AE* Get_sample_AE();
CNT* Get_sample_CNT();
CIN* Get_sample_CIN();

Node* Create_Node(CSE *cse, AE *ae, CNT *cnt, CIN *cin);
Node* Find_Node(RT *rt);
int Add_child(Node *parent, Node *child);

int display(char* database);
void print_AE(AE* gae);
