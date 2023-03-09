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
    if(pjson)
        fc->sts = get_number_from_cjson(pjson);

    pjson = cJSON_GetObjectItem(fcjson, "stb");
    if(pjson)
        fc->stb = get_number_from_cjson(pjson);

    pjson = cJSON_GetObjectItem(fcjson, "exb");
    if(pjson && pjson->valuestring)
        fc->exb = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "exa");
    if(pjson && pjson->valuestring)
        fc->exa = strdup(pjson->valuestring);

    pjson = cJSON_GetObjectItem(fcjson, "arp");
    if(pjson && pjson->valuestring){
        fc->arp = strdup(pjson->valuestring);
    }

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
            fc->ty[i] = get_number_from_cjson(cJSON_GetArrayItem(pjson, i));
        }
        fc->tycnt = ty_len;
    }else if(pjson){
        fc->tycnt = 1;
        fc->ty = calloc(sizeof(ResourceType), 1);
        fc->ty[0] = get_number_from_cjson(pjson);
    }

    

    pjson = cJSON_GetObjectItem(fcjson, "chty");
    if(cJSON_IsArray(pjson)){
        int ty_len = cJSON_GetArraySize(pjson);
        fc->chty = calloc(sizeof(ResourceType), ty_len);
        for(int i = 0 ; i < ty_len ; i++){
            fc->chty[i] = get_number_from_cjson(cJSON_GetArrayItem(pjson, i));
        }
        fc->chtycnt = ty_len;
    }else if(pjson){
        fc->chtycnt = 1;
        fc->chty = calloc(sizeof(ResourceType), 1);
        fc->chty[0] = get_number_from_cjson(pjson);
    }

    pjson = cJSON_GetObjectItem(fcjson, "pty");
    if(cJSON_IsArray(pjson)){
        int ty_len = cJSON_GetArraySize(pjson);
        fc->pty = calloc(sizeof(ResourceType), ty_len);
        for(int i = 0 ; i < ty_len ; i++){
            fc->pty[i] = get_number_from_cjson(cJSON_GetArrayItem(pjson, i));
        }
        fc->ptycnt = ty_len;
    }else if(pjson){
        fc->ptycnt = 1;
        fc->pty = calloc(sizeof(ResourceType), 1);
        fc->pty[0] = get_number_from_cjson(pjson);
    }

    pjson = cJSON_GetObjectItem(fcjson, "sza");
    if(pjson){
        fc->sza = get_number_from_cjson(pjson);
    }
 

    pjson = cJSON_GetObjectItem(fcjson, "szb");
    if(pjson){
        fc->szb = get_number_from_cjson(pjson);

    }

    //TODO - remaining attrs

    pjson = cJSON_GetObjectItem(fcjson, "fu");
    if(pjson){
        fc->fu = get_number_from_cjson(pjson);
    }else{
        fc->fu = FU_CONDITIONAL_OPERATION;
    }

    pjson = cJSON_GetObjectItem(fcjson, "lim");
    if(pjson){
        fc->lim = get_number_from_cjson(pjson);
    }else{
        fc->lim = __INT_MAX__;
    }

    pjson = cJSON_GetObjectItem(fcjson, "fo");
    if(pjson){
        fc->fo = get_number_from_cjson(pjson);
    }else
        fc->fo = FO_AND;

    pjson = cJSON_GetObjectItem(fcjson, "lvl");
    if(pjson){
        fc->lvl = get_number_from_cjson(pjson);
    }else{
        fc->lvl = __INT_MAX__;
    }

    pjson = cJSON_GetObjectItem(fcjson, "ofst");
    if(pjson){
        fc->ofst = get_number_from_cjson(pjson);
    }else{
        fc->ofst = 0;
    }

    
    if(!isFCAttrValid(fc)){ // TODO - If rcn == 11(discovery result references) fu must be 1
        free_fc(fc);
        return NULL;
    }
    return fc;
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
    if(fc->chty)
        free(fc->chty);
    if(fc->pty)
        free(fc->pty);

    free(fc);
    fc = NULL;
}



bool FC_isAptCrb(char *fcCrb, RTNode *rtnode){
    if(!rtnode || !fcCrb) return false;

    char *ct = get_ct_rtnode(rtnode);
    if(strcmp(fcCrb, ct) > 0) return true;

    return false;
}

bool FC_isAptCra(char* fcCra, RTNode *rtnode){
    if(!rtnode || !fcCra) return false;

    char* ct = get_ct_rtnode(rtnode);

    if(strcmp(fcCra, ct) <= 0) return true;

    return false;
}

bool FC_isAptMs(char *fcMs, RTNode *rtnode){
    if(!rtnode || !fcMs) return false;

    char *lt = get_lt_rtnode(rtnode);
    if(strcmp(fcMs, lt) <= 0) return true;

    return false;
}

bool FC_isAptUs(char *fcUs, RTNode *rtnode){
    if(!rtnode || !fcUs) return false;

    char *lt = get_lt_rtnode(rtnode);
    if(strcmp(fcUs, lt) > 0) return true;

    return false;
}

bool FC_isAptStb(int fcStb, RTNode *rtnode){
    if(!rtnode) return false;

    int st = get_st_rtnode(rtnode);
    if(st == -1) return false;
    if(fcStb <= st) return true;

    return false;
}

bool FC_isAptSts(int fcSts, RTNode *rtnode){
    if(!rtnode) return false;

    int st = get_st_rtnode(rtnode);
    if(st == -1) return false;
    if(st < fcSts) return true;

    return false;
}

bool FC_isAptExa(char *fcExa, RTNode *rtnode){
    if(!rtnode || !fcExa) return false;

    char *et = get_et_rtnode(rtnode);
    if(strcmp(fcExa, et) >= 0) return false;

    return true;
}

bool FC_isAptExb(char *fcExb, RTNode *rtnode){
    if(!rtnode || !fcExb) return false;

    char *et = get_et_rtnode(rtnode);
    if(strcmp(fcExb, et) < 0) return false;

    return true;
}

bool FC_isAptTy(int *fcTy, int tycnt, int ty){
 
    for(int i = 0 ; i < tycnt; i++){
        if(ty == fcTy[i]) return true;
    }

    return false;
}

bool FC_isAptChty(int *fcChty, int tycnt, int ty){
 
    for(int i = 0 ; i < tycnt; i++){
        if(ty == fcChty[i]) return true;
    }

    return false;
}

bool FC_isAptPty(int *fcPty, int tycnt, int ty){
    
    for(int i = 0 ; i < tycnt; i++){
        if(ty == fcPty[i]) return true;
    }
 
    return false;
}

bool FC_isAptSza(int fcSza, RTNode *rtnode){
    int cs = get_cs_rtnode(rtnode);
    if(cs == -1) return false;
    if(fcSza <= cs) return true;

    return false;
}

bool FC_isAptSzb(int fcSzb, RTNode *rtnode){
    int cs = get_cs_rtnode(rtnode);
    if(cs == -1) return false;
    if(cs < fcSzb) return true;

    return false;
}