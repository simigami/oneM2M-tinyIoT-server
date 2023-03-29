#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include "onem2mTypes.h"
#include "util.h"
#include "cJSON.h"
#include "onem2m.h"
#include "jsonparser.h"
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
        if(fc->ty[i] < 0) return false;
    }

    if(fc->chty < 0) return false;
    if(fc->pty < 0) return false;

    return true;
}

bool isValidFcAttr(char* attr){
    char *fcAttr[32] = {
    "crb", "cra", "ms", "us", "sts", "stb", "exb", "exa", "lbl","clbl", "palb", "lbq", "ty", "chty", "pty", "sza", "szb", "cty", 
    "atr", "catr", "patr", "fu", "lim", "smf", "fo", "cfs", "cfq", "lvl", "ofst", "arp", "gq", "ops"};

    for(int i = 0 ; i < 32 ; i++){
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


    pjson = cJSON_GetObjectItem(fcjson, "lbl");
    if(pjson && pjson->valuestring){
        fc->lbl = cJSON_CreateArray();
        cJSON_AddItemToArray(fc->lbl, cJSON_CreateString(pjson->valuestring));
    }else{
        fc->lbl = cJSON_Duplicate(pjson, true);
    }

    pjson = cJSON_GetObjectItem(fcjson, "palb");
    if(pjson && pjson->valuestring){
        fc->palb = cJSON_CreateArray();
        cJSON_AddItemToArray(fc->palb, cJSON_CreateString(pjson->valuestring));
    }else{
        fc->palb = cJSON_Duplicate(pjson, true);
    }

    pjson = cJSON_GetObjectItem(fcjson, "clbl");
    if(pjson && pjson->valuestring){
        fc->clbl = cJSON_CreateArray();
        cJSON_AddItemToArray(fc->clbl, cJSON_CreateString(pjson->valuestring));
    }else{
        fc->clbl = cJSON_Duplicate(pjson, true);
    }

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

    pjson = cJSON_GetObjectItem(fcjson, "ops");
    if(pjson){
        fc->ops = get_number_from_cjson(pjson);
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
    if(fc->lbl)
        cJSON_Delete(fc->lbl);
    if(fc->palb)
        cJSON_Delete(fc->palb);
    if(fc->clbl)
        cJSON_Delete(fc->clbl);

    if(fc->ty)
        free(fc->ty);
    if(fc->chty)
        free(fc->chty);
    if(fc->pty)
        free(fc->pty);

    free(fc);
    fc = NULL;
}