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
} Container;

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
} contentInstance;

// OneM2M Resource function
Operation Parse_Operation(char *method);
ObjectType Parse_ObjectType(char *content_type);
AE* Create_AE(char *parsed_json);
