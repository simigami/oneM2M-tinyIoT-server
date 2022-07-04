#include "onem2m.h"
#include <stdlib.h>
#include <stdbool.h>

CSE* Get_sample_CSE(char *ri) {
    CSE* cse = (CSE*)malloc(sizeof(CSE));
    
    cse->ct = (char*)malloc(16*sizeof(char));
    cse->lt = (char*)malloc(16*sizeof(char));
    cse->rn = (char*)malloc(32*sizeof(char));
    cse->ri = (char*)malloc(32*sizeof(char));
    cse->pi = (char*)malloc(32*sizeof(char));
    cse->csi = (char*)malloc(16*sizeof(char));

    strcpy(cse->ct, "20191210T093452");
    strcpy(cse->lt, "20191210T093452");
    strcpy(cse->ri, "5-20191210093452845");
    strcpy(cse->pi, "NULL");
    strcpy(cse->csi, "/Tiny_Project2");
    strcpy(cse->ri, ri);
    cse->ty = 5;
    
    return cse;
}

AE* Get_sample_AE(char *ri) {
    AE* ae = (AE*)malloc(sizeof(AE));
    
    ae->pi = (char*)malloc(32*sizeof(char));
    ae->ri = (char*)malloc(32*sizeof(char));
    ae->ct = (char*)malloc(16*sizeof(char));
    ae->lt = (char*)malloc(16*sizeof(char));
    ae->et = (char*)malloc(16*sizeof(char));
    ae->api = (char*)malloc(32*sizeof(char));
    ae->aei = (char*)malloc(32*sizeof(char));
    
    strcpy(ae->ct, "20220513T083900");
    strcpy(ae->lt, "20220513T083900");
    strcpy(ae->et, "20240513T083900");
    strcpy(ae->api, "tinyProject");
    strcpy(ae->aei, "TAE");
    strcpy(ae->ri ,ri);
    ae->rr = true;
    ae->ty = 2;
    
    return ae;
}

CNT* Get_sample_CNT(char *ri) {
    CNT* cnt = (CNT*)malloc(sizeof(CNT));
    
    cnt->pi = (char*)malloc(32*sizeof(char));
    cnt->ri = (char*)malloc(32*sizeof(char));
    cnt->ct = (char*)malloc(16*sizeof(char));
    cnt->lt = (char*)malloc(16*sizeof(char));
    cnt->et = (char*)malloc(16*sizeof(char));
    cnt->rn = (char*)malloc(32*sizeof(char));
    
    strcpy(cnt->pi, "TAE");
    strcpy(cnt->ri, ri);
    strcpy(cnt->ct, "202205T093154");
    strcpy(cnt->rn, "status");
    strcpy(cnt->lt, "20220513T093154");
    strcpy(cnt->et, "20220513T093154");
    cnt->ty = 3;
    cnt->st = 0;
    cnt->cni = 0;
    cnt->cbs = 0;
    
    return cnt;
}

CIN* Get_sample_CIN(char *ri) {
    CIN* cin = (CIN*)malloc(sizeof(CIN));
    
    cin->pi = (char*)malloc(32*sizeof(char));
    cin->ri = (char*)malloc(32*sizeof(char));
    cin->ct = (char*)malloc(16*sizeof(char));
    cin->lt = (char*)malloc(16*sizeof(char));
    cin->et = (char*)malloc(16*sizeof(char));
    cin->rn = (char*)malloc(32*sizeof(char));
    cin->con = (char*)malloc(16*sizeof(char));
    cin->csi = (char*)malloc(16*sizeof(char));
    
    strcpy(cin->pi, "3-20220513091700249586");
    strcpy(cin->ri, ri); 
    strcpy(cin->ct, "202205T093154");
    strcpy(cin->rn, "4-20220513093154147745");
    strcpy(cin->lt, "20220513T093154");
    strcpy(cin->et, "20220513T093154");
    strcpy(cin->con, "ON");
    stycpy(cin->csi, "csitest");
    cin->ty = 4;
    cin->st = 1;
    
    return cin;
}
