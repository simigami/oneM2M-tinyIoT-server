#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include "filterCriteria.h"
#include "onem2mTypes.h"
#include "util.h"
#include "cJSON.h"
#include "onem2m.h"
#include "logger.h"

bool isFCAttrValid(FilterCriteria *fc){
    if(fc->sts < 0) return false;
    if(fc->stb < 0) return false;
    
    if(fc->sza < 0) return false;
    if(fc->szb < 0) return false;

    if(fc->lim < 0) return false;

    if(fc->lvl < 0) return false;
    if(fc->ofst < 0) return false;

    for(int i = 0 ; i < fc->tycnt ; i++){
        logger("test", LOG_LEVEL_DEBUG, "%d", fc->ty[i]);
        if(fc->ty[i] < 0) return false;
    }

    if(fc->chty < 0) return false;
    if(fc->pty < 0) return false;

    return true;
}

bool isValidFcAttr(char* attr){
    char *fcAttr[30] = {
    "crb", "cra", "ms", "us", "sts", "stb", "exb", "exa", "lbl", "lbq", "ty", "chty", "pty", "sza", "szb", "cty", 
    "atr", "catr", "patr", "fu", "lim", "smf", "fo", "cfs", "cfq", "lvl", "ofst", "arp", "gq", "ops"};

    for(int i = 0 ; i < 30 ; i++){
        if(!strcmp(attr, fcAttr[i])) return true;
    }
    return false;
}

FilterCriteria *parseFilterCriteria(cJSON *fcjson){
    cJSON *pjson = NULL;
    FilterCriteria *fc = NULL;
    logger("FC", LOG_LEVEL_DEBUG, "parse Filter Criteria");
    if(!fcjson) return NULL;
    pjson = fcjson->child;
    while(pjson != NULL){
        if(!isValidFcAttr(pjson->string)){
            //cJSON_Delete(fcjson);
            return NULL;
        }
        pjson = pjson->next;
    }
    fc = (FilterCriteria*) calloc(1, sizeof(FilterCriteria));

    pjson = cJSON_GetObjectItem(fcjson, "crb");
    if(pjson && pjson->valuestring)
        fc->crb = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "cra");
    if(pjson && pjson->valuestring)
        fc->cra = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "ms");
    if(pjson && pjson->valuestring)
        fc->ms = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "us");
    if(pjson && pjson->valuestring)
        fc->us = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "sts");
    if(pjson && pjson->valuestring)
        fc->sts = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "stb");
    if(pjson && pjson->valuestring)
        fc->stb = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "exb");
    if(pjson && pjson->valuestring)
        fc->exb = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "exa");
    if(pjson && pjson->valuestring)
        fc->exa = strdup(pjson->valuestring);

    //TODO - LABEL
    // pjson = cJSON_GetObjectItem(fcjson, "lbl");
    // if(pjson && pjson->valuestring)
    //     fc->crb = strdup(pjson->valuestring);

    //TODO - LABEL Query
    // pjson = cJSON_GetObjectItem(fcjson, "lbq");
    // if(pjson && pjson->valuestring)
    //     fc->crb = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "ty");
    
    if(cJSON_IsArray(pjson)){
        int ty_len = cJSON_GetArraySize(pjson);
        fc->ty = calloc(sizeof(ResourceType), ty_len);
        for(int i = 0 ; i < ty_len ; i++){
            fc->ty[i] = getNumberofJsonValue(cJSON_GetArrayItem(pjson, i));
        }
        fc->tycnt = ty_len;
    }else{
        fc->tycnt = 1;
        fc->ty = calloc(sizeof(ResourceType), 1);
        fc->ty[0] = getNumberofJsonValue(pjson);
    }

    

    pjson = cJSON_GetObjectItem(fcjson, "chty");
    if(pjson){
        fc->chty = getNumberofJsonValue(pjson);
    }

    pjson = cJSON_GetObjectItem(fcjson, "pty");
    if(pjson){
        fc->pty = getNumberofJsonValue(pjson);
    }

    pjson = cJSON_GetObjectItem(fcjson, "sza");
    if(pjson){
        fc->sza = getNumberofJsonValue(pjson);
    }
 

    pjson = cJSON_GetObjectItem(fcjson, "szb");
    if(pjson){
        fc->szb = getNumberofJsonValue(pjson);

    }

    //TODO - remaining attrs

    pjson = cJSON_GetObjectItem(fcjson, "fu");
    if(pjson){
        fc->fu = getNumberofJsonValue(pjson);
    }

    pjson = cJSON_GetObjectItem(fcjson, "lim");
    if(pjson){
        fc->lim = getNumberofJsonValue(pjson);
    }

    
    if(!isFCAttrValid(fc)){
        free_fc(fc);
        return NULL;
    }
    return fc;
}

bool isResourceAptFC(RTNode *rtnode, FilterCriteria *fc){
    int flag = 0;
    if(fc->tycnt){
        if(!FC_isaptTy(fc->ty, fc->tycnt, rtnode->ty)) 
            return false;
    }

    if(fc->cra){
        if(!FC_isAptCra(fc->cra, rtnode->ct)) 
            return false;
    }

    return true;
}

void free_fc(FilterCriteria *fc){
    if(!fc) return;
    if(fc->arp)
        free(fc->arp);
    if(fc->atr)
        free(fc->atr);
    if(fc->catr)
        free(fc->catr);
    if(fc->cfq)
        free(fc->cfq);
    if(fc->cfs)
        free(fc->cfs);
    if(fc->cra)
        free(fc->cra);
    if(fc->crb)
        free(fc->crb);
    if(fc->exa)
        free(fc->exa);
    if(fc->exb)
        free(fc->exb);
    if(fc->gq)
        free(fc->gq);
    if(fc->sts)
        free(fc->sts);
    if(fc->smf)
        free(fc->smf);
    if(fc->patr)
        free(fc->patr);
    if(fc->ms)
        free(fc->ms);
    if(fc->lbq)
        free(fc->lbq);

    if(fc->ty)
        free(fc->ty);

    free(fc);
    fc = NULL;
}

bool FC_isaptTy(int *fcTy, int tycnt, int ty){
    int flag = 0;
    
    for(int i = 0 ; i < tycnt; i++){
        if(ty != fcTy[i]) flag++;
    }
    if(flag == tycnt) 
        return false;

    return true;
}

bool FC_isAptCra(char* fcCra, char* cra){
    if(strcmp(fcCra, cra) < 0) return false;

    return true;
}