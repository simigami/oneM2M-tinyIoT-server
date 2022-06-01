#include <stdio.h>

// OneM2M Resource struct
typedef struct {
	char *rn;
	char *ri;
	char *ty;
	char *pi;
	char *ct;
	char *lt;
	char *csi;
} cse;

typedef struct {
	char *rn;
	char *ri;
	char *ty;
	char *pi;
	char *et;
	char *ct;
	char *lt;
	char *api;
	char *aei;
	char *rr;
} AE;

typedef struct {
	char *rn;
	char *ri;
	char *ty;
	char *pi;
	char *et;
	char *ct;
	char *lt;
	char *st;
	char *cni;
	char *cbs;
} Container;

typedef struct {
	char *rn;
	char *ri;
	char *ty;
	char *pi;
	char *et;
	char *ct;
	char *lt;
	char *st;
	char *csi;
	char *con;	
} contentInstance;

// OneM2M Resource function
AE create_AE(char *parsed_json);
