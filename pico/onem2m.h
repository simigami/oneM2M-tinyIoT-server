#include <stdio.h>
#include <stdbool.h>

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
	char ct[16];
	char lt[16];
	char *rn;
	char *ri;
	char *pi;
	char *csi;
	int ty;
} CSE;

typedef struct {
	char et[16];
	char ct[16];
	char lt[16];
	char *rn;
	char *ri;
	char *pi;
	char *api;
	char *aei;
	int ty;
	bool rr;
} AE;

typedef struct {
	char et[16];
	char ct[16];
	char lt[16];
	char *rn;
	char *ri;
	char *pi;
	int ty;
	int st;
	int cni;
	int cbs;
} CNT;

typedef struct {
	char et[16];
	char ct[16];
	char lt[16];
	char *rn;
	char *ri;
	char *pi;
	char *csi;
	char *con;	
	int ty;
	int st;
} CIN;

// OneM2M Resource function
Operation Parse_Operation();
ObjectType Parse_ObjectType();
ObjectType Parse_ObjectType_By_URI();
AE* Create_AE(char *json_payload);
CNT* Create_CNT(char *json_payload);
CIN* Create_CIN(char *json_payload);
void Retrieve_CSE(CSE* cse_object);
void Retrieve_AE(AE* ae_object);
void Retrieve_CNT(CNT* ae_object);
void Retrieve_CIN(CIN* ae_object);
CSE* Update_CSE(char *json_payload);
AE* Update_AE(char *json_payload);
CNT* Update_CNT(char *json_payload);
CIN* Update_CIN(char *json_payload);
