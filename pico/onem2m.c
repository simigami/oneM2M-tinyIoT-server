#include "httpd.h"
#include "onem2m.h"
#include "cJSON.h"
#include <db.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

Operation Parse_Operation(){
	Operation op;

	if(strcmp(method, "POST") == 0) op = o_CREATE;
	else if(strcmp(method, "GET") == 0) op = o_RETRIEVE;
	else if (strcmp(method, "PUT") == 0) op = o_UPDATE;
	else if (strcmp(method, "DELETE") == 0) op = o_DELETE;

	return op;	
}

ObjectType Parse_ObjectType() {
	ObjectType ty;
	char *ct = request_header("Content-Type");
	int tail = strlen(ct) - 1;
	
	switch(ct[tail]) {
	case '2' : ty = t_AE; break;
	case '3' : ty = t_CNT; break;
	case '4' : ty = t_CIN; break;
	case '5' : ty = t_CSE; break;
	default : ty = 0;
	}
	
	return ty;
}

ObjectType Parse_ObjectType_By_URI() {
	ObjectType ty;
	int cnt = 0;
	
	int uri_len = strlen(uri);
	
	for(int i=0; i<uri_len; i++) {
		if(uri[i] == '/') cnt++;
	}
	
	if(uri[uri_len-1] == '/') cnt = cnt - 1;
	
	switch(cnt) {
	case 0 : ty = t_CSE; break;
	case 1 : ty = t_AE; break;
	case 2 : ty = t_CNT; break;
	case 3 : ty = t_CIN; break;
	default : ty = 0;
	}
	
	return ty;
}

int Store_CSE(CSE* cse_object, char* database) {
    char* DATABASE = database;
    DB* dbp;    // db handle
    DBC* cursorp;
    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_csi, key_ty;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_csi, data_ty;  // storving key and real data
    int ret;        // template value

    /*DB open*/
    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        exit(1);
    }
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        exit(1);
    }

    // key and data must initialize
    memset(&key_ct, 0, sizeof(DBT));
    memset(&key_lt, 0, sizeof(DBT));
    memset(&key_rn, 0, sizeof(DBT));
    memset(&key_ri, 0, sizeof(DBT));
    memset(&key_pi, 0, sizeof(DBT));
    memset(&key_csi, 0, sizeof(DBT));
    memset(&key_ty, 0, sizeof(DBT));

    memset(&data_ct, 0, sizeof(DBT));
    memset(&data_lt, 0, sizeof(DBT));
    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_pi, 0, sizeof(DBT));
    memset(&data_csi, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));

    // Store key & data
    data_ct.data = cse_object->ct;
    data_ct.size = strlen(cse_object->ct) + 1;
    key_ct.data = "ct";
    key_ct.size = strlen("ct") + 1;

    data_lt.data = cse_object->lt;
    data_lt.size = strlen(cse_object->lt) + 1;
    key_lt.data = "lt";
    key_lt.size = strlen("lt") + 1;

    data_rn.data = cse_object->rn;
    data_rn.size = strlen(cse_object->rn) + 1;
    key_rn.data = "rn";
    key_rn.size = strlen("rn") + 1;

    data_ri.data = cse_object->ri;
    data_ri.size = strlen(cse_object->ri) + 1;
    key_ri.data = "ri";
    key_ri.size = strlen("ri") + 1;

    data_pi.data = cse_object->pi;
    data_pi.size = strlen(cse_object->pi) + 1;
    key_pi.data = "pi";
    key_pi.size = strlen("pi") + 1;

    data_csi.data = cse_object->csi;
    data_csi.size = strlen(cse_object->csi) + 1;
    key_csi.data = "csi";
    key_csi.size = strlen("csi") + 1;

    data_ty.data = &cse_object->ty;
    data_ty.size = sizeof(cse_object->ty);
    key_ty.data = "ty";
    key_ty.size = strlen("ty") + 1;


    // Put key & data
    ret = dbp->put(dbp, NULL, &key_ct, &data_ct, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_lt, &data_lt, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_rn, &data_rn, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_ri, &data_ri, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_pi, &data_pi, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_csi, &data_csi, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_ty, &data_ty, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    dbp->close(dbp, 0); //DB close

    return 1;
}
int Store_AE(AE* ae_object, char* database) {
    char* DATABASE = database;
    DB* dbp;    // db handle
    DBC* cursorp;
    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_ty, key_et, key_api, key_rr, key_aei;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_ty, data_et, data_api, data_rr, data_aei;  // storving key and real data
    int ret;        // template value

    /*DB open*/
    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        exit(1);
    }
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        exit(1);
    }


    // key and data must initialize
    memset(&key_ct, 0, sizeof(DBT));
    memset(&key_lt, 0, sizeof(DBT));
    memset(&key_rn, 0, sizeof(DBT));
    memset(&key_ri, 0, sizeof(DBT));
    memset(&key_pi, 0, sizeof(DBT));
    memset(&key_ty, 0, sizeof(DBT));
    memset(&key_et, 0, sizeof(DBT));
    memset(&key_api, 0, sizeof(DBT));
    memset(&key_rr, 0, sizeof(DBT));
    memset(&key_aei, 0, sizeof(DBT));


    memset(&data_ct, 0, sizeof(DBT));
    memset(&data_lt, 0, sizeof(DBT));
    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_pi, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));
    memset(&data_et, 0, sizeof(DBT));
    memset(&data_api, 0, sizeof(DBT));
    memset(&data_rr, 0, sizeof(DBT));
    memset(&data_aei, 0, sizeof(DBT));


    // Store key & data
    data_ct.data = ae_object->ct;
    data_ct.size = strlen(ae_object->ct) + 1;
    key_ct.data = "ct";
    key_ct.size = strlen("ct") + 1;

    data_lt.data = ae_object->lt;
    data_lt.size = strlen(ae_object->lt) + 1;
    key_lt.data = "lt";
    key_lt.size = strlen("lt") + 1;

    data_rn.data = ae_object->rn;
    data_rn.size = strlen(ae_object->rn) + 1;
    key_rn.data = "rn";
    key_rn.size = strlen("rn") + 1;

    data_ri.data = ae_object->ri;
    data_ri.size = strlen(ae_object->ri) + 1;
    key_ri.data = "ri";
    key_ri.size = strlen("ri") + 1;

    data_pi.data = ae_object->pi;
    data_pi.size = strlen(ae_object->pi) + 1;
    key_pi.data = "pi";
    key_pi.size = strlen("pi") + 1;

    data_ty.data = &ae_object->ty;
    data_ty.size = sizeof(ae_object->ty);
    key_ty.data = "ty";
    key_ty.size = strlen("ty") + 1;

    data_et.data = ae_object->et;
    data_et.size = strlen(ae_object->et) + 1;
    key_et.data = "et";
    key_et.size = strlen("et") + 1;

    data_api.data = ae_object->api;
    data_api.size = strlen(ae_object->api) + 1;
    key_api.data = "api";
    key_api.size = strlen("api") + 1;

    data_rr.data = &ae_object->rr;
    data_rr.size = sizeof(ae_object->rr);
    key_rr.data = "rr";
    key_rr.size = strlen("rr") + 1;

    data_aei.data = ae_object->aei;
    data_aei.size = strlen(ae_object->aei) + 1;
    key_aei.data = "aei";
    key_aei.size = strlen("aei") + 1;

    // Put key & data
    ret = dbp->put(dbp, NULL, &key_ct, &data_ct, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_lt, &data_lt, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_rn, &data_rn, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_ri, &data_ri, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_pi, &data_pi, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_ty, &data_ty, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    ret = dbp->put(dbp, NULL, &key_et, &data_et, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    ret = dbp->put(dbp, NULL, &key_api, &data_api, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_rr, &data_rr, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_aei, &data_aei, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    dbp->close(dbp, 0); //DB close

    return 1;
}
int Store_CNT(CNT* cnt_object, char* database) {
    char* DATABASE = database;
    DB* dbp;    // db handle
    DBC* cursorp;
    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_ty, key_et, key_cni, key_cbs, key_st;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_ty, data_et, data_cni, data_cbs, data_st;  // storving key and real data
    int ret;        // template value

    /*DB open*/
    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        exit(1);
    }
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        exit(1);
    }


    // key and data must initialize
    memset(&key_ct, 0, sizeof(DBT));
    memset(&key_lt, 0, sizeof(DBT));
    memset(&key_rn, 0, sizeof(DBT));
    memset(&key_ri, 0, sizeof(DBT));
    memset(&key_pi, 0, sizeof(DBT));
    memset(&key_ty, 0, sizeof(DBT));
    memset(&key_et, 0, sizeof(DBT));
    memset(&key_cni, 0, sizeof(DBT));
    memset(&key_cbs, 0, sizeof(DBT));
    memset(&key_st, 0, sizeof(DBT));


    memset(&data_ct, 0, sizeof(DBT));
    memset(&data_lt, 0, sizeof(DBT));
    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_pi, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));
    memset(&data_et, 0, sizeof(DBT));
    memset(&data_cni, 0, sizeof(DBT));
    memset(&data_cbs, 0, sizeof(DBT));
    memset(&data_st, 0, sizeof(DBT));


    // Store key & data
    data_ct.data = cnt_object->ct;
    data_ct.size = strlen(cnt_object->ct) + 1;
    key_ct.data = "ct";
    key_ct.size = strlen("ct") + 1;

    data_lt.data = cnt_object->lt;
    data_lt.size = strlen(cnt_object->lt) + 1;
    key_lt.data = "lt";
    key_lt.size = strlen("lt") + 1;

    data_rn.data = cnt_object->rn;
    data_rn.size = strlen(cnt_object->rn) + 1;
    key_rn.data = "rn";
    key_rn.size = strlen("rn") + 1;

    data_ri.data = cnt_object->ri;
    data_ri.size = strlen(cnt_object->ri) + 1;
    key_ri.data = "ri";
    key_ri.size = strlen("ri") + 1;

    data_pi.data = cnt_object->pi;
    data_pi.size = strlen(cnt_object->pi) + 1;
    key_pi.data = "pi";
    key_pi.size = strlen("pi") + 1;

    data_ty.data = &cnt_object->ty;
    data_ty.size = sizeof(cnt_object->ty);
    key_ty.data = "ty";
    key_ty.size = strlen("ty") + 1;

    data_et.data = cnt_object->et;
    data_et.size = strlen(cnt_object->et) + 1;
    key_et.data = "et";
    key_et.size = strlen("et") + 1;

    data_cni.data = &cnt_object->cni;
    data_cni.size = sizeof(cnt_object->cni);
    key_cni.data = "cni";
    key_cni.size = strlen("cni") + 1;

    data_cbs.data = &cnt_object->cbs;
    data_cbs.size = sizeof(cnt_object->cbs);
    key_cbs.data = "cbs";
    key_cbs.size = strlen("cbs") + 1;

    data_st.data = &cnt_object->st;
    data_st.size = sizeof(cnt_object->st);
    key_st.data = "st";
    key_st.size = strlen("st") + 1;

    // Put key & data
    ret = dbp->put(dbp, NULL, &key_ct, &data_ct, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_lt, &data_lt, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_rn, &data_rn, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_ri, &data_ri, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_pi, &data_pi, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_ty, &data_ty, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    ret = dbp->put(dbp, NULL, &key_et, &data_et, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    ret = dbp->put(dbp, NULL, &key_cni, &data_cni, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_cbs, &data_cbs, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_st, &data_st, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    dbp->close(dbp, 0); //DB close

    return 1;
}
int Store_CIN(CIN* cin_object, char* database) {
    char* DATABASE = database;
    DB* dbp;    // db handle
    DBC* cursorp;
    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_ty, key_et, key_st, key_csi, key_con;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_ty, data_et, data_st, data_csi, data_con;  // storving key and real data
    int ret;        // template value

    /*DB open*/
    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        exit(1);
    }
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        exit(1);
    }


    // key and data must initialize
    memset(&key_ct, 0, sizeof(DBT));
    memset(&key_lt, 0, sizeof(DBT));
    memset(&key_rn, 0, sizeof(DBT));
    memset(&key_ri, 0, sizeof(DBT));
    memset(&key_pi, 0, sizeof(DBT));
    memset(&key_ty, 0, sizeof(DBT));
    memset(&key_et, 0, sizeof(DBT));
    memset(&key_csi, 0, sizeof(DBT));
    memset(&key_con, 0, sizeof(DBT));
    memset(&key_st, 0, sizeof(DBT));


    memset(&data_ct, 0, sizeof(DBT));
    memset(&data_lt, 0, sizeof(DBT));
    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_pi, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));
    memset(&data_et, 0, sizeof(DBT));
    memset(&data_csi, 0, sizeof(DBT));
    memset(&data_con, 0, sizeof(DBT));
    memset(&data_st, 0, sizeof(DBT));


    // Store key & data
    data_ct.data = cin_object->ct;
    data_ct.size = strlen(cin_object->ct) + 1;
    key_ct.data = "ct";
    key_ct.size = strlen("ct") + 1;

    data_lt.data = cin_object->lt;
    data_lt.size = strlen(cin_object->lt) + 1;
    key_lt.data = "lt";
    key_lt.size = strlen("lt") + 1;

    data_rn.data = cin_object->rn;
    data_rn.size = strlen(cin_object->rn) + 1;
    key_rn.data = "rn";
    key_rn.size = strlen("rn") + 1;

    data_ri.data = cin_object->ri;
    data_ri.size = strlen(cin_object->ri) + 1;
    key_ri.data = "ri";
    key_ri.size = strlen("ri") + 1;

    data_pi.data = cin_object->pi;
    data_pi.size = strlen(cin_object->pi) + 1;
    key_pi.data = "pi";
    key_pi.size = strlen("pi") + 1;

    data_ty.data = &cin_object->ty;
    data_ty.size = sizeof(cin_object->ty);
    key_ty.data = "ty";
    key_ty.size = strlen("ty") + 1;

    data_et.data = cin_object->et;
    data_et.size = strlen(cin_object->et) + 1;
    key_et.data = "et";
    key_et.size = strlen("et") + 1;

    data_csi.data = &cin_object->csi;
    data_csi.size = sizeof(cin_object->csi);
    key_csi.data = "csi";
    key_csi.size = strlen("csi") + 1;

    data_con.data = cin_object->con;
    data_con.size = strlen(cin_object->con) + 1;
    key_con.data = "con";
    key_con.size = strlen("con") + 1;

    data_st.data = &cin_object->st;
    data_st.size = sizeof(cin_object->st);
    key_st.data = "st";
    key_st.size = strlen("st") + 1;

    // Put key & data
    ret = dbp->put(dbp, NULL, &key_ct, &data_ct, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_lt, &data_lt, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_rn, &data_rn, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_ri, &data_ri, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_pi, &data_pi, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_ty, &data_ty, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    ret = dbp->put(dbp, NULL, &key_et, &data_et, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    ret = dbp->put(dbp, NULL, &key_csi, &data_csi, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_con, &data_con, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }
    ret = dbp->put(dbp, NULL, &key_st, &data_st, 0);
    if (ret) {
        dbp->err(dbp, ret, "DB->put");
        exit(1);
    }

    dbp->close(dbp, 0); //DB close

    return 1;
}

CSE* Get_sample_CSE(char *rn) {
    CSE* cse = (CSE*)malloc(sizeof(CSE));
    
    cse->ct = (char*)malloc(16*sizeof(char));
    cse->lt = (char*)malloc(16*sizeof(char));
    cse->rn = (char*)malloc(32*sizeof(char));
    cse->ri = (char*)malloc(32*sizeof(char));
    cse->pi = (char*)malloc(32*sizeof(char));
    cse->csi = (char*)malloc(16*sizeof(char));

    strcpy(cse->ct, "20191210T093452");
    strcpy(cse->lt, "20191210T093452");
    strcpy(cse->rn, rn);
    strcpy(cse->pi, "NULL");
    strcpy(cse->csi, "/Tiny_Project2");
    strcpy(cse->ri, "235235");
    cse->ty = 5;
    
    return cse;
}

AE* Get_sample_AE(char *rn) {
    AE* ae = (AE*)malloc(sizeof(AE));
    
    ae->pi = (char*)malloc(32*sizeof(char));
    ae->ri = (char*)malloc(32*sizeof(char));
    ae->ct = (char*)malloc(16*sizeof(char));
    ae->lt = (char*)malloc(16*sizeof(char));
    ae->et = (char*)malloc(16*sizeof(char));
    ae->api = (char*)malloc(32*sizeof(char));
    ae->aei = (char*)malloc(32*sizeof(char));
    ae->rn = (char*)malloc(32*sizeof(char));
    
    strcpy(ae->rn, rn);
    strcpy(ae->ct, "20220513T083900");
    strcpy(ae->lt, "20220513T083900");
    strcpy(ae->et, "20240513T083900");
    strcpy(ae->api, "tinyProject");
    strcpy(ae->aei, "TAE");
    strcpy(ae->ri , "9345");
    ae->rr = true;
    ae->ty = 2;
    
    return ae;
}

CNT* Get_sample_CNT(char *rn) {
    CNT* cnt = (CNT*)malloc(sizeof(CNT));
    
    cnt->pi = (char*)malloc(32*sizeof(char));
    cnt->ri = (char*)malloc(32*sizeof(char));
    cnt->ct = (char*)malloc(16*sizeof(char));
    cnt->lt = (char*)malloc(16*sizeof(char));
    cnt->et = (char*)malloc(16*sizeof(char));
    cnt->rn = (char*)malloc(32*sizeof(char));
    
    strcpy(cnt->pi, "TAE");
    strcpy(cnt->ri, "123465");
    strcpy(cnt->ct, "202205T093154");
    strcpy(cnt->rn, rn);
    strcpy(cnt->lt, "20220513T093154");
    strcpy(cnt->et, "20220513T093154");
    cnt->ty = 3;
    cnt->st = 0;
    cnt->cni = 0;
    cnt->cbs = 0;
    
    return cnt;
}

CIN* Get_sample_CIN(char *rn) {
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
    strcpy(cin->ri, "123456"); 
    strcpy(cin->ct, "202205T093154");
    strcpy(cin->rn, rn);
    strcpy(cin->lt, "20220513T093154");
    strcpy(cin->et, "20220513T093154");
    strcpy(cin->con, "ON");
    strcpy(cin->csi, "csitest");
    cin->ty = 4;
    cin->st = 1;
    
    return cin;
}

Node* Create_Node(CSE *cse, AE *ae, CNT *cnt, CIN *cin){
	Node* node = (Node*)malloc(sizeof(Node));
	
	node->rn = (char*)malloc(sizeof(char)*32);
	if(cse) strcpy(node->rn,cse->rn);
	if(ae) strcpy(node->rn,ae->rn);
	if(cnt) strcpy(node->rn,cnt->rn);
	if(cin) strcpy(node->rn,cin->rn);
	
	node->cse = cse;
	node->ae = ae;
	node->cnt = cnt;
	node->cin = cin;
	node->child = NULL;
	node->sibling = NULL;
	
	return node;
}

Node* Find_Node(RT *rt) {
	Node *node = rt->root;
	
	char *ptr = (char*)malloc(sizeof(uri));
	strcpy(ptr,uri);
	
	ptr = strtok(ptr, "/");
	
	fprintf(stderr,"uri : %s\n",uri);
	
	if(rt->root->child) fprintf(stderr,"good\n");
	
	while(ptr != NULL && node) {
		node = node->child;
		
		fprintf(stderr,"%s %s\n",node->rn, ptr);
		
		while(node) {
			if(!strcmp(node->rn,ptr)) break;
			node = node->sibling;
		}
		ptr = strtok(NULL, "/");
	}
	
	free(ptr);
	
	return node;
}

int Add_child(Node *parent, Node *child) {
	Node *node = parent->child;
	
	if(node) {
		while(node->sibling) node = node->sibling;
		
		node->sibling = child;
	}
	else {
		parent->child = child;
	}
	
	return 1;
}

