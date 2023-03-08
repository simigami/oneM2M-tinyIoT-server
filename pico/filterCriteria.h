#ifndef __FILTER_CRITERIA__
#define __FILTER_CRITERIA__ 1
 
#include "onem2mTypes.h"
#include "cJSON.h"

typedef enum {
    FU_DISCOVERY_CRITERIA          = 1,
    FU_CONDITIONAL_RETRIEVAL       = 2, // DEFAULT
    FU_IPE_ON_DEMAND_DISCOVERY     = 3,
    FU_DISCOVERY_BASED_OPERATION   = 4
} FilterUsage;


typedef enum{
    FO_AND = 1,    // DEFAULT
    FO_OR  = 2,
    FO_XOR = 3
} FilterOperation;


typedef struct {
    char* crb;
    char* cra;
    char* ms;
    char* us;
    char* sts;
    char* stb;
    char* exb;
    char* exa;
    char* lbl;
    char* lbq;
    int *ty;
    size_t tycnt;
    int chty;
    int pty;
    int sza;
    int szb;

    char* cty;
    char* atr;
    char* catr;
    char* patr;

    FilterUsage fu;
    int lim;
    char* smf;
    FilterOperation fo;

    char* cfs;
    char* cfq;

    int lvl;
    int ofst;
    char* arp;
    char* gq;
    char* ops;
} FilterCriteria;

bool isValidFcAttr(char* attr);

FilterCriteria *parseFilterCriteria(cJSON *fcjson);

void free_fc(FilterCriteria *fc);

/* check resource is apt to filter criteria */
bool FC_isaptTy(int *fcTy, int tycnt, int ty);
bool FC_isAptCra(char* fcCra, void *obj, ResourceType ty);


#endif