#include <stdio.h>
#include <stdbool.h>

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
AE* Create_AE(char *parsed_json);
