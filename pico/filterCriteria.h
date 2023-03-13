#ifndef __FILTER_CRITERIA__
#define __FILTER_CRITERIA__ 1
 
#include "onem2mTypes.h"
#include "cJSON.h"


typedef enum {
    FU_DISCOVERY                    = 1,
    FU_CONDITIONAL_OPERATION       = 2, // DEFAULT
    FU_IPE_ON_DEMAND_DISCOVERY     = 3,
    FU_DISCOVERY_BASED_OPERATION   = 4
} FilterUsage;


typedef enum{
    FO_AND = 1,    // DEFAULT
    FO_OR  = 2,
    FO_XOR = 3
} FilterOperation;


typedef struct {
    /*Created Before*/
    char* crb;
    /*Created After*/
    char* cra;
    /*Modified Since*/
    char* ms;
    /*Unmodified Since*/
    char* us;
    /*stateTag Smaller*/
    int sts;
    /*stateTag Bigger*/
    int stb;
    /*Expire Before*/
    char* exb;
    /*Expire After*/
    char* exa;
    
    /*Label*/
    cJSON* lbl;
    /*Parent Label*/
    cJSON *palb;
    /*Child Label*/
    cJSON *clbl;

    /*Labels Query*/
    char* lbq;
    /*Resource Type*/
    int *ty;
    int tycnt;
    /*Child Resource Type*/
    int *chty;
    int chtycnt;
    /*Parent Resource Type*/
    int *pty;
    int ptycnt;
    /*Size Above*/
    int sza;
    /*Size Below*/
    int szb;

    /*Content Type*/
    char* cty;
    /*Attribute*/
    char* atr;
    /*Child Attribute*/
    char* catr;
    /*Parent Attribute*/
    char* patr;

    FilterUsage fu;
    /*Limit*/
    int lim;
    /*sememtics FIlter*/
    char* smf;
    FilterOperation fo;

    /*Content Filter Syntax*/
    char* cfs;
    /*Content Filter Query*/
    char* cfq;

    /*Level*/
    int lvl;
    /*Offset*/
    int ofst;
    /*apply Relative Path*/
    char* arp;
    /*GeoQuery*/
    char* gq;
    /*Operations*/
    char* ops;
} FilterCriteria;

bool isValidFcAttr(char* attr);

FilterCriteria *parseFilterCriteria(cJSON *fcjson);

void free_fc(FilterCriteria *fc);


#endif
