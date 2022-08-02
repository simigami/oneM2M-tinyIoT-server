#include "onem2m.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int Store_CSE(CSE *cse_object)
{
    char* DATABASE = "CSE.db";

    DB* dbp;    // db handle
    DBC* dbcp;
    FILE* error_file_pointer;
    DBT key, data;  // storving key and real data
    int ret;        // template value

    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_csi, key_ty;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_csi, data_ty;  // storving key and real data

    char* program_name = "my_prog";

    // if input == NULL
    if (cse_object->ri == NULL) cse_object->ri = "";
    if (cse_object->rn == NULL) cse_object->rn = "";
    if (cse_object->pi == NULL) cse_object->pi = "NULL";
    if (cse_object->ty == '\0') cse_object->ty = -1;
    if (cse_object->ct == NULL) cse_object->ct = "";
    if (cse_object->lt == NULL) cse_object->lt = "";
    if (cse_object->csi == NULL) cse_object->csi = "";

    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        printf("File ERROR\n");
        exit(1);
    }

    dbp->set_errfile(dbp, error_file_pointer);
    dbp->set_errpfx(dbp, program_name);

    /*Set duplicate*/
    ret = dbp->set_flags(dbp, DB_DUP);
    if (ret != 0) {
        dbp->err(dbp, ret, "Attempt to set DUPSORT flag failed.");
        printf("Flag Set ERROR\n");
        dbp->close(dbp, 0);
        return(ret);
    }

    /*DB Open*/
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        printf("DB Open ERROR\n");
        exit(1);
    }

    /*
  * The DB handle for a Btree database supporting duplicate data
  * items is the argument; acquire a cursor for the database.
  */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        printf("Cursor ERROR");
        exit(1);
    }
 
    /* keyand data must initialize */
    memset(&key_rn, 0, sizeof(DBT));
    memset(&key_ri, 0, sizeof(DBT));
    memset(&key_pi, 0, sizeof(DBT));
    memset(&key_ty, 0, sizeof(DBT));
    memset(&key_ct, 0, sizeof(DBT));
    memset(&key_lt, 0, sizeof(DBT));
    memset(&key_csi, 0, sizeof(DBT));

    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_pi, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));
    memset(&data_ct, 0, sizeof(DBT));
    memset(&data_lt, 0, sizeof(DBT));
    memset(&data_csi, 0, sizeof(DBT));

    /* initialize the data to be the first of two duplicate records. */
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

    data_ct.data = cse_object->ct;
    data_ct.size = strlen(cse_object->ct) + 1;
    key_ct.data = "ct";
    key_ct.size = strlen("ct") + 1;

    data_lt.data = cse_object->lt;
    data_lt.size = strlen(cse_object->lt) + 1;
    key_lt.data = "lt";
    key_lt.size = strlen("lt") + 1;

    data_csi.data = cse_object->csi;
    data_csi.size = strlen(cse_object->csi) + 1;
    key_csi.data = "csi";
    key_csi.size = strlen("csi") + 1;

    data_ty.data = &cse_object->ty;
    data_ty.size = sizeof(cse_object->ty);
    key_ty.data = "ty";
    key_ty.size = strlen("ty") + 1;

    /* CSE -> only one & first */
    if ((ret = dbcp->put(dbcp, &key_ri, &data_ri, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_rn, &data_rn, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_pi, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_ty, &data_ty, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    if ((ret = dbcp->put(dbcp, &key_ct, &data_ct, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_lt, &data_lt, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_csi, &data_csi, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    dbcp->close(dbcp);
    dbp->close(dbp, 0); //DB close
    return 1;
}

int Store_AE(AE* ae_object) {
    char* DATABASE = "AE.db";
    DB* dbp;    // db handle
    DBC* dbcp;
    FILE* error_file_pointer;
    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_ty, key_et, key_api, key_rr,key_aei;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_ty, data_et, data_api, data_rr, data_aei;  // storving key and real data
    int ret;        // template value

    char* program_name = "my_prog";

    // if input == NULL
    if (ae_object->ri == NULL) ae_object->ri = "";
    if (ae_object->rn == NULL) ae_object->rn = "";
    if (ae_object->pi == NULL) ae_object->pi = "";
    if (ae_object->ty == '\0') ae_object->ty = -1;
    if (ae_object->ct == NULL) ae_object->ct = "";
    if (ae_object->lt == NULL) ae_object->lt = "";
    if (ae_object->et == NULL) ae_object->et = "";

    if (ae_object->rr == '\0') ae_object->rr = true;
    if (ae_object->api == NULL) ae_object->api = "";
    if (ae_object->aei == NULL) ae_object->aei = "";

    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        fprintf(stderr,"File ERROR\n");
        exit(1);
    }

    dbp->set_errfile(dbp, error_file_pointer);
    dbp->set_errpfx(dbp, program_name);

    /*Set duplicate*/
    ret = dbp->set_flags(dbp, DB_DUP);
    if (ret != 0) {
        dbp->err(dbp, ret, "Attempt to set DUPSORT flag failed.");
        fprintf(stderr,"Flag Set ERROR\n");
        dbp->close(dbp, 0);
        return(ret);
    }

    /*DB Open*/
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);

    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        fprintf(stderr,"DB Open ERROR\n");
        exit(1);
    }
    
    /*
* The DB handle for a Btree database supporting duplicate data
* items is the argument; acquire a cursor for the database.
*/
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        fprintf(stderr,"Cursor ERROR\n");
        exit(1);
    }

    /* keyand data must initialize */
    memset(&key_rn, 0, sizeof(DBT));
    memset(&key_ri, 0, sizeof(DBT));
    memset(&key_pi, 0, sizeof(DBT));
    memset(&key_ty, 0, sizeof(DBT));
    memset(&key_ct, 0, sizeof(DBT));
    memset(&key_lt, 0, sizeof(DBT));
    memset(&key_et, 0, sizeof(DBT));
    memset(&key_api, 0, sizeof(DBT));
    memset(&key_rr, 0, sizeof(DBT));
    memset(&key_aei, 0, sizeof(DBT));

    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_pi, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));
    memset(&data_ct, 0, sizeof(DBT));
    memset(&data_lt, 0, sizeof(DBT));
    memset(&data_et, 0, sizeof(DBT));
    memset(&data_api, 0, sizeof(DBT));
    memset(&data_rr, 0, sizeof(DBT));
    memset(&data_aei, 0, sizeof(DBT));

    // Store key & data
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

    data_ct.data = ae_object->ct;
    data_ct.size = strlen(ae_object->ct) + 1;
    key_ct.data = "ct";
    key_ct.size = strlen("ct") + 1;

    data_lt.data = ae_object->lt;
    data_lt.size = strlen(ae_object->lt) + 1;
    key_lt.data = "lt";
    key_lt.size = strlen("lt") + 1;

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


    if ((ret = dbcp->put(dbcp, &key_ri, &data_ri, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_rn, &data_rn, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_pi, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_ty, &data_ty, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_ct, &data_ct, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_lt, &data_lt, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_et, &data_et, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_api, &data_api, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_rr, &data_rr, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");
    if ((ret = dbcp->put(dbcp, &key_aei, &data_aei, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");


    dbcp->close(dbcp);
    dbp->close(dbp, 0); //DB close

    return 1;
}

int Store_CNT(CNT *cnt_object)
{
    char* DATABASE = "CNT.db";

    DB* dbp;    // db handle
    DBC* dbcp;
    FILE* error_file_pointer;
    DBT key, data;  // storving key and real data
    int ret;        // template value

    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_ty, key_et, key_cni, key_cbs, key_st;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_ty, data_et, data_cni, data_cbs, data_st;  // storving key and real data

    char* program_name = "my_prog";

    // if input == NULL
    if (cnt_object->ri == NULL) cnt_object->ri = "";
    if (cnt_object->rn == NULL) cnt_object->rn = "";
    if (cnt_object->pi == NULL) cnt_object->pi = "";
    if (cnt_object->ty == '\0') cnt_object->ty = -1;
    if (cnt_object->ct == NULL) cnt_object->ct = "";
    if (cnt_object->lt == NULL) cnt_object->lt = "";
    if (cnt_object->et == NULL) cnt_object->et = "";

    if (cnt_object->cni == '\0') cnt_object->cni = -1;
    if (cnt_object->cbs == '\0') cnt_object->cbs = -1;
    if (cnt_object->st == '\0') cnt_object->st = -1;

    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        fprintf(stderr,"File ERROR\n");
        exit(1);
    }

    dbp->set_errfile(dbp, error_file_pointer);
    dbp->set_errpfx(dbp, program_name);

    /*Set duplicate*/
    ret = dbp->set_flags(dbp, DB_DUP);
    if (ret != 0) {
        dbp->err(dbp, ret, "Attempt to set DUPSORT flag failed.");
        fprintf(stderr,"Flag Set ERROR\n");
        dbp->close(dbp, 0);
        return(ret);
    }

    /*DB Open*/
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        fprintf(stderr,"DB Open ERROR\n");
        exit(1);
    }

    /*
  * The DB handle for a Btree database supporting duplicate data
  * items is the argument; acquire a cursor for the database.
  */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        fprintf(stderr,"Cursor ERROR");
        exit(1);
    }
 
    /* keyand data must initialize */
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


    /* initialize the data to be the first of two duplicate records. */
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

    data_ct.data = cnt_object->ct;
    data_ct.size = strlen(cnt_object->ct) + 1;
    key_ct.data = "ct";
    key_ct.size = strlen("ct") + 1;

    data_lt.data = cnt_object->lt;
    data_lt.size = strlen(cnt_object->lt) + 1;
    key_lt.data = "lt";
    key_lt.size = strlen("lt") + 1;

    data_et.data = cnt_object->et;
    data_et.size = strlen(cnt_object->et) + 1;
    key_et.data = "et";
    key_et.size = strlen("et") + 1;

    data_ty.data = &cnt_object->ty;
    data_ty.size = sizeof(cnt_object->ty);
    key_ty.data = "ty";
    key_ty.size = strlen("ty") + 1;

    data_st.data = &cnt_object->st;
    data_st.size = sizeof(cnt_object->st);
    key_st.data = "st";
    key_st.size = strlen("st") + 1;

    data_cni.data = &cnt_object->cni;
    data_cni.size = sizeof(cnt_object->cni);
    key_cni.data = "cni";
    key_cni.size = strlen("cni") + 1;

    data_cbs.data = &cnt_object->cbs;
    data_cbs.size = sizeof(cnt_object->cbs);
    key_cbs.data = "cbs";
    key_cbs.size = strlen("cbs") + 1;

    /* CNT -> only one & first */
    if ((ret = dbcp->put(dbcp, &key_ri, &data_ri, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_rn, &data_rn, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_pi, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_ty, &data_ty, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    if ((ret = dbcp->put(dbcp, &key_ct, &data_ct, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_lt, &data_lt, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_cni, &data_cni, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_st, &data_st, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_et, &data_et, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_cbs, &data_cbs, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    dbcp->close(dbcp);
    dbp->close(dbp, 0); //DB close
    return 1;
}

int Store_CIN(CIN *cin_object)
{
    char* DATABASE = "CIN.db";

    DB* dbp;    // db handle
    DBC* dbcp;
    FILE* error_file_pointer;
    DBT key, data;  // storving key and real data
    int ret;        // template value

    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_ty, key_et, key_st, key_cs, key_con;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_ty, data_et, data_st, data_cs, data_con;  // storving key and real data

    char* program_name = "my_prog";

    // if input == NULL
    if (cin_object->ri == NULL) cin_object->ri = "";
    if (cin_object->rn == NULL) cin_object->rn = "";
    if (cin_object->pi == NULL) cin_object->pi = "";
    if (cin_object->ty == '\0') cin_object->ty = -1;
    if (cin_object->ct == NULL) cin_object->ct = "";
    if (cin_object->lt == NULL) cin_object->lt = "";
    if (cin_object->et == NULL) cin_object->et = "";

    if (cin_object->con == NULL) cin_object->con = "";
    if (cin_object->cs == '\0') cin_object->cs = -1;
    if (cin_object->st == '\0') cin_object->st = -1;
    
    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        printf("File ERROR\n");
        exit(1);
    }

    dbp->set_errfile(dbp, error_file_pointer);
    dbp->set_errpfx(dbp, program_name);

    /*Set duplicate*/
    ret = dbp->set_flags(dbp, DB_DUP);
    if (ret != 0) {
        dbp->err(dbp, ret, "Attempt to set DUPSORT flag failed.");
        printf("Flag Set ERROR\n");
        dbp->close(dbp, 0);
        return(ret);
    }

    /*DB Open*/
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        printf("DB Open ERROR\n");
        exit(1);
    }

    /*
  * The DB handle for a Btree database supporting duplicate data
  * items is the argument; acquire a cursor for the database.
  */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        printf("Cursor ERROR");
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
    memset(&key_cs, 0, sizeof(DBT));
    memset(&key_con, 0, sizeof(DBT));
    memset(&key_st, 0, sizeof(DBT));

    memset(&data_ct, 0, sizeof(DBT));
    memset(&data_lt, 0, sizeof(DBT));
    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_pi, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));
    memset(&data_et, 0, sizeof(DBT));
    memset(&data_cs, 0, sizeof(DBT));
    memset(&data_con, 0, sizeof(DBT));
    memset(&data_st, 0, sizeof(DBT));


    /* initialize the data to be the first of two duplicate records. */
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

    data_ct.data = cin_object->ct;
    data_ct.size = strlen(cin_object->ct) + 1;
    key_ct.data = "ct";
    key_ct.size = strlen("ct") + 1;

    data_lt.data = cin_object->lt;
    data_lt.size = strlen(cin_object->lt) + 1;
    key_lt.data = "lt";
    key_lt.size = strlen("lt") + 1;

    data_et.data = cin_object->et;
    data_et.size = strlen(cin_object->et) + 1;
    key_et.data = "et";
    key_et.size = strlen("et") + 1;

    data_con.data = cin_object->con;
    data_con.size = strlen(cin_object->con) + 1;
    key_con.data = "con";
    key_con.size = strlen("con") + 1;

    data_ty.data = &cin_object->ty;
    data_ty.size = sizeof(cin_object->ty);
    key_ty.data = "ty";
    key_ty.size = strlen("ty") + 1;

    data_st.data = &cin_object->st;
    data_st.size = sizeof(cin_object->st);
    key_st.data = "st";
    key_st.size = strlen("st") + 1;

    data_cs.data = &cin_object->cs;
    data_cs.size = sizeof(cin_object->cs);
    key_cs.data = "cs";
    key_cs.size = strlen("cs") + 1;

    /* CIN -> only one & first */
    if ((ret = dbcp->put(dbcp, &key_ri, &data_ri, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_rn, &data_rn, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_pi, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_ty, &data_ty, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    if ((ret = dbcp->put(dbcp, &key_ct, &data_ct, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_lt, &data_lt, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_st, &data_st, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_et, &data_et, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    if ((ret = dbcp->put(dbcp, &key_cs, &data_cs, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_con, &data_con, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    dbcp->close(dbcp);
    dbp->close(dbp, 0); //DB close
    return 1;
}

CSE* Get_CSE() {
    fprintf(stderr, "[Get CSE]\n");

    //store CSE Object
    CSE* new_cse = (CSE*)malloc(sizeof(CSE));

    char* database = "CSE.db";
    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int close_db, close_dbc, ret;

    close_db = close_dbc = 0;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }
    close_db = 1;

    /* Turn on additional error output. */
    dbp->set_errfile(dbp, stderr);
    dbp->set_errpfx(dbp, database);

    /* Open the database. */
    if ((ret = dbp->open(dbp, NULL, database, NULL,
        DB_UNKNOWN, DB_RDONLY, 0)) != 0) {
        dbp->err(dbp, ret, "%s: DB->open", database);
        goto err;
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        goto err;
    }
    close_dbc = 1;

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    //store CSE
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {

        if (strncmp(key.data, "ri", key.size) == 0) {
            new_cse->ri = malloc(data.size);
            strcpy(new_cse->ri, data.data);
        }
        if (strncmp(key.data, "pi", key.size) == 0) {
            new_cse->pi = malloc(data.size);
            strcpy(new_cse->pi, data.data);
        }
        if (strncmp(key.data, "rn", key.size) == 0) {
            new_cse->rn = malloc(data.size);
            strcpy(new_cse->rn, data.data);
        }
        if (strncmp(key.data, "ty", key.size) == 0) {
            new_cse->ty = *(int*)data.data;
        }
        if (strncmp(key.data, "csi", key.size) == 0) {
            new_cse->csi = malloc(data.size);
            strcpy(new_cse->csi, data.data);
        }
        if (strncmp(key.data, "lt", key.size) == 0) {
            new_cse->lt = malloc(data.size);
            strcpy(new_cse->lt, data.data);
        }
        if (strncmp(key.data, "ct", key.size) == 0) {
            new_cse->ct = malloc(data.size);
            strcpy(new_cse->ct, data.data);
        }
    }

    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        printf("Cursor ERROR\n");
        exit(0);
    }

err:    if (close_dbc && (ret = dbcp->close(dbcp)) != 0)
dbp->err(dbp, ret, "DBcursor->close");
if (close_db && (ret = dbp->close(dbp, 0)) != 0)
fprintf(stderr,
    "%s: DB->close: %s\n", database, db_strerror(ret));
return new_cse;
}

AE* Get_AE(char* ri) {
    //fprintf(stderr,"[Get AE] ri = %s\n", ri);

    //store AE
    AE* new_ae = (AE*)malloc(sizeof(AE));

    char* database = "AE.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }
	
    /* Open the database. */
    ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", database);
        exit(1);
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int idx = 0;
    int flag = 0;
    // žî¹øÂ° AEÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            idx++;
            if (strncmp(data.data, ri, data.size) == 0) {
                flag = 1;
                new_ae->ri = malloc(data.size);
                strcpy(new_ae->ri, data.data);
                break;
            }
        }
    }
    if (flag == 0) {
        fprintf(stderr,"Not Found\n");
        return NULL;
        //exit(1);
    }

    int cnt_rn = 0;
    int cnt_pi = 0;
    int cnt_ty = 0;
    int cnt_et = 0;
    int cnt_lt = 0;
    int cnt_ct = 0;
    int cnt_api = 0;
    int cnt_aei = 0;
    int cnt_rr = 0;
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "rn", key.size) == 0) {
            cnt_rn++;
            if (cnt_rn == idx) {
                new_ae->rn = malloc(data.size);
                strcpy(new_ae->rn, data.data);
            }
        }
        if (strncmp(key.data, "pi", key.size) == 0) {
            cnt_pi++;
            if (cnt_pi == idx) {
                new_ae->pi = malloc(data.size);
                strcpy(new_ae->pi, data.data);
            }
        }
        if (strncmp(key.data, "api", key.size) == 0) {
            cnt_api++;
            if (cnt_api == idx) {
                new_ae->api = malloc(data.size);
                strcpy(new_ae->api, data.data);
            }
        }
        if (strncmp(key.data, "aei", key.size) == 0) {
            cnt_aei++;
            if (cnt_aei == idx) {
                new_ae->aei = malloc(data.size);
                strcpy(new_ae->aei, data.data);
            }
        }
        if (strncmp(key.data, "et", key.size) == 0) {
            cnt_et++;
            if (cnt_et == idx) {
                new_ae->et = malloc(data.size);
                strcpy(new_ae->et, data.data);
            }
        }
        if (strncmp(key.data, "lt", key.size) == 0) {
            cnt_lt++;
            if (cnt_lt == idx) {
                new_ae->lt = malloc(data.size);
                strcpy(new_ae->lt, data.data);
            }
        }
        if (strncmp(key.data, "ct", key.size) == 0) {
            cnt_ct++;
            if (cnt_ct == idx) {
                new_ae->ct = malloc(data.size);
                strcpy(new_ae->ct, data.data);
            }
        }
        if (strncmp(key.data, "ty", key.size) == 0) {
            cnt_ty++;
            if (cnt_ty == idx) {
                new_ae->ty = *(int*)data.data;
            }
        }
        if (strncmp(key.data, "rr", key.size) == 0) {
            cnt_rr++;
            if (cnt_rr == idx) {
                new_ae->rr = *(bool*)data.data;
            }
        }
    }
    //fprintf(stderr,"[%d]\n",idx);

    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr,"Cursor ERROR\n");
        exit(0);
    }

    return new_ae;
}

CNT* Get_CNT(char* ri) {
    //fprintf(stderr,"[Get CNT] ri = %s\n", ri);

    //store CNT
    CNT* new_cnt = (CNT*)malloc(sizeof(CNT));

    char* database = "CNT.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int close_db, close_dbc, ret;

    close_db = close_dbc = 0;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }
    close_db = 1;

    /* Turn on additional error output. */
    dbp->set_errfile(dbp, stderr);
    dbp->set_errpfx(dbp, database);

    /* Open the database. */
    if ((ret = dbp->open(dbp, NULL, database, NULL,
        DB_UNKNOWN, DB_RDONLY, 0)) != 0) {
        dbp->err(dbp, ret, "%s: DB->open", database);
        goto err;
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        goto err;
    }
    close_dbc = 1;

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int idx = 0;
    // žî¹øÂ° CNTÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        goto err;
    }
    close_dbc = 1;
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            idx++;
            if (strncmp(data.data, ri, data.size) == 0) {
                new_cnt->ri = malloc(data.size);
                strcpy(new_cnt->ri, data.data);
                break;
            }
        }
    }

    int cnt_rn = 0;
    int cnt_pi = 0;
    int cnt_ty = 0;
    int cnt_et = 0;
    int cnt_lt = 0;
    int cnt_ct = 0;
    int cnt_st = 0;
    int cnt_cni = 0;
    int cnt_cbs = 0;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "rn", key.size) == 0) {
            cnt_rn++;
            if (cnt_rn == idx) {
                new_cnt->rn = malloc(data.size);
                strcpy(new_cnt->rn, data.data);
            }
        }
        if (strncmp(key.data, "pi", key.size) == 0) {
            cnt_pi++;
            if (cnt_pi == idx) {
                new_cnt->pi = malloc(data.size);
                strcpy(new_cnt->pi, data.data);
            }
        }
        if (strncmp(key.data, "et", key.size) == 0) {
            cnt_et++;
            if (cnt_et == idx) {
                new_cnt->et = malloc(data.size);
                strcpy(new_cnt->et, data.data);
            }
        }
        if (strncmp(key.data, "lt", key.size) == 0) {
            cnt_lt++;
            if (cnt_lt == idx) {
                new_cnt->lt = malloc(data.size);
                strcpy(new_cnt->lt, data.data);
            }
        }
        if (strncmp(key.data, "ct", key.size) == 0) {
            cnt_ct++;
            if (cnt_ct == idx) {
                new_cnt->ct = malloc(data.size);
                strcpy(new_cnt->ct, data.data);
            }
        }
        if (strncmp(key.data, "ty", key.size) == 0) {
            cnt_ty++;
            if (cnt_ty == idx) {
                new_cnt->ty = *(int*)data.data;
            }
        }
        if (strncmp(key.data, "st", key.size) == 0) {
            cnt_st++;
            if (cnt_st == idx) {
                new_cnt->st = *(int*)data.data;
            }
        }
        if (strncmp(key.data, "cni", key.size) == 0) {
            cnt_cni++;
            if (cnt_cni == idx) {
                new_cnt->cni = *(int*)data.data;
            }
        }
        if (strncmp(key.data, "cbs", key.size) == 0) {
            cnt_cbs++;
            if (cnt_cbs == idx) {
                new_cnt->cbs = *(int*)data.data;
            }
        }
    }
    //fprintf(stderr,"[%d]\n",idx);

    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr,"Cursor ERROR\n");
        exit(0);
    }

err:    if (close_dbc && (ret = dbcp->close(dbcp)) != 0)
dbp->err(dbp, ret, "DBcursor->close");
if (close_db && (ret = dbp->close(dbp, 0)) != 0)
fprintf(stderr,
    "%s: DB->close: %s\n", database, db_strerror(ret));
return new_cnt;
}

CIN* Get_CIN(char* ri) {
    //fprintf(stderr,"[Get CIN] ri = %s\n", ri);

    //store CIN
    CIN* new_cin = (CIN*)malloc(sizeof(CIN));

    char* database = "CIN.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int close_db, close_dbc, ret;

    close_db = close_dbc = 0;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }
    close_db = 1;

    /* Turn on additional error output. */
    dbp->set_errfile(dbp, stderr);
    dbp->set_errpfx(dbp, database);

    /* Open the database. */
    if ((ret = dbp->open(dbp, NULL, database, NULL,
        DB_UNKNOWN, DB_RDONLY, 0)) != 0) {
        dbp->err(dbp, ret, "%s: DB->open", database);
        goto err;
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        goto err;
    }
    close_dbc = 1;

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int idx = 0;
    // žî¹øÂ° CINÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        goto err;
    }
    close_dbc = 1;
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            idx++;
            if (strncmp(data.data, ri, data.size) == 0) {
                new_cin->ri = malloc(data.size);
                strcpy(new_cin->ri, data.data);
                break;
            }
        }
    }

    int cin_rn = 0;
    int cin_pi = 0;
    int cin_ty = 0;
    int cin_et = 0;
    int cin_lt = 0;
    int cin_ct = 0;
    int cin_st = 0;
    int cin_cs = 0;
    int cin_con = 0;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "rn", key.size) == 0) {
            cin_rn++;
            if (cin_rn == idx) {
                new_cin->rn = malloc(data.size);
                strcpy(new_cin->rn, data.data);
            }
        }
        if (strncmp(key.data, "pi", key.size) == 0) {
            cin_pi++;
            if (cin_pi == idx) {
                new_cin->pi = malloc(data.size);
                strcpy(new_cin->pi, data.data);
            }
        }
        if (strncmp(key.data, "et", key.size) == 0) {
            cin_et++;
            if (cin_et == idx) {
                new_cin->et = malloc(data.size);
                strcpy(new_cin->et, data.data);
            }
        }
        if (strncmp(key.data, "lt", key.size) == 0) {
            cin_lt++;
            if (cin_lt == idx) {
                new_cin->lt = malloc(data.size);
                strcpy(new_cin->lt, data.data);
            }
        }
        if (strncmp(key.data, "ct", key.size) == 0) {
            cin_ct++;
            if (cin_ct == idx) {
                new_cin->ct = malloc(data.size);
                strcpy(new_cin->ct, data.data);
            }
        }
        if (strncmp(key.data, "con", key.size) == 0) {
            cin_con++;
            if (cin_con == idx) {
                new_cin->con = malloc(data.size);
                strcpy(new_cin->con, data.data);
            }
        }

        if (strncmp(key.data, "ty", key.size) == 0) {
            cin_ty++;
            if (cin_ty == idx) {
                new_cin->ty = *(int*)data.data;
            }
        }
        if (strncmp(key.data, "st", key.size) == 0) {
            cin_st++;
            if (cin_st == idx) {
                new_cin->st = *(int*)data.data;
            }
        }
        if (strncmp(key.data, "cs", key.size) == 0) {
            cin_cs++;
            if (cin_cs == idx) {
                new_cin->cs = *(int*)data.data;
            }
        }
    }
    //fprintf(stderr,"[%d]\n",idx);

    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr,"Cursor ERROR\n");
        exit(0);
    }

err:    if (close_dbc && (ret = dbcp->close(dbcp)) != 0)
dbp->err(dbp, ret, "DBcursor->close");
if (close_db && (ret = dbp->close(dbp, 0)) != 0)
fprintf(stderr,
    "%s: DB->close: %s\n", database, db_strerror(ret));
return new_cin;
}

AE* Delete_AE(char* ri) {
    fprintf(stderr,"[Delete AE] ri = %s\n", ri);

    //store AE
    AE* new_ae = (AE*)malloc(sizeof(AE));

    char* database = "AE.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }

    /* Open the database. */
    ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", database);
        exit(1);
    }


    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int idx = 0;
    int flag = 0;
    // žî¹øÂ° AEÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }

    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            idx++;
            if (strncmp(data.data, ri, data.size) == 0) {
                flag = 1;
                new_ae->ri = malloc(data.size);
                strcpy(new_ae->ri, data.data);
                dbcp0->del(dbcp0, 0);
                break;
            }
        }
    }
    if (flag == 0) {
        fprintf(stderr,"Not Found\n");
        exit(1);
    }

    int cnt_rn = 0;
    int cnt_pi = 0;
    int cnt_ty = 0;
    int cnt_et = 0;
    int cnt_lt = 0;
    int cnt_ct = 0;
    int cnt_api = 0;
    int cnt_aei = 0;
    int cnt_rr = 0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "rn", key.size) == 0) {
            cnt_rn++;
            if (cnt_rn == idx) {
                new_ae->rn = malloc(data.size);
                strcpy(new_ae->rn, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "pi", key.size) == 0) {
            cnt_pi++;
            if (cnt_pi == idx) {
                new_ae->pi = malloc(data.size);
                strcpy(new_ae->pi, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "api", key.size) == 0) {
            cnt_api++;
            if (cnt_api == idx) {
                new_ae->api = malloc(data.size);
                strcpy(new_ae->api, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "aei", key.size) == 0) {
            cnt_aei++;
            if (cnt_aei == idx) {
                new_ae->aei = malloc(data.size);
                strcpy(new_ae->aei, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "et", key.size) == 0) {
            cnt_et++;
            if (cnt_et == idx) {
                new_ae->et = malloc(data.size);
                strcpy(new_ae->et, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "lt", key.size) == 0) {
            cnt_lt++;
            if (cnt_lt == idx) {
                new_ae->lt = malloc(data.size);
                strcpy(new_ae->lt, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "ct", key.size) == 0) {
            cnt_ct++;
            if (cnt_ct == idx) {
                new_ae->ct = malloc(data.size);
                strcpy(new_ae->ct, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "ty", key.size) == 0) {
            cnt_ty++;
            if (cnt_ty == idx) {
                new_ae->ty = *(int*)data.data;
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "rr", key.size) == 0) {
            cnt_rr++;
            if (cnt_rr == idx) {
                new_ae->rr = *(bool*)data.data;
                dbcp->del(dbcp, 0);
            }
        }
    }


    /* Cursors must be closed */
    if (dbcp0 != NULL)
        dbcp0->close(dbcp0);
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    

    return new_ae;
}

CNT* Delete_CNT(char* ri) {
    //fprintf(stderr,"[Delete CNT] ri = %s\n", ri);

    //store CNT
    CNT* new_cnt = (CNT*)malloc(sizeof(CNT));

    char* database = "CNT.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }

    /* Open the database. */
    ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", database);
        exit(1);
    }


    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int idx = 0;
    int flag = 0;
    // žî¹øÂ° CNTÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }

    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            idx++;
            if (strncmp(data.data, ri, data.size) == 0) {
                flag = 1;
                new_cnt->ri = malloc(data.size);
                strcpy(new_cnt->ri, data.data);
                dbcp0->del(dbcp0, 0);
                break;
            }
        }
    }
    if (flag == 0) {
        printf("Not Found\n");
        return NULL;
        exit(1);
    }

    // ÇØŽç index¿¡ ŒøŒ­ÀÇ °ª Ã£ŸÆ Áö¿ò
    int cnt_rn = 0;
    int cnt_pi = 0;
    int cnt_ty = 0;
    int cnt_et = 0;
    int cnt_lt = 0;
    int cnt_ct = 0;
    int cnt_st = 0;
    int cnt_cni = 0;
    int cnt_cbs = 0;

    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "rn", key.size) == 0) {
            cnt_rn++;
            if (cnt_rn == idx) {
                new_cnt->rn = malloc(data.size);
                strcpy(new_cnt->rn, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "pi", key.size) == 0) {
            cnt_pi++;
            if (cnt_pi == idx) {
                new_cnt->pi = malloc(data.size);
                strcpy(new_cnt->pi, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "et", key.size) == 0) {
            cnt_et++;
            if (cnt_et == idx) {
                new_cnt->et = malloc(data.size);
                strcpy(new_cnt->et, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "lt", key.size) == 0) {
            cnt_lt++;
            if (cnt_lt == idx) {
                new_cnt->lt = malloc(data.size);
                strcpy(new_cnt->lt, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "ct", key.size) == 0) {
            cnt_ct++;
            if (cnt_ct == idx) {
                new_cnt->ct = malloc(data.size);
                strcpy(new_cnt->ct, data.data);
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "ty", key.size) == 0) {
            cnt_ty++;
            if (cnt_ty == idx) {
                new_cnt->ty = *(int*)data.data;
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "st", key.size) == 0) {
            cnt_st++;
            if (cnt_st == idx) {
                new_cnt->st = *(int*)data.data;
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "cbs", key.size) == 0) {
            cnt_cbs++;
            if (cnt_cbs == idx) {
                new_cnt->cbs = *(int*)data.data;
                dbcp->del(dbcp, 0);
            }
        }
        if (strncmp(key.data, "cni", key.size) == 0) {
            cnt_cni++;
            if (cnt_cni == idx) {
                new_cnt->cni = *(int*)data.data;
                dbcp->del(dbcp, 0);
            }
        }

    }


    /* Cursors must be closed */
    if (dbcp0 != NULL)
        dbcp0->close(dbcp0);
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    

    return new_cnt;
}

Node* Get_All_AE() {
    fprintf(stderr, "[Get All AE]\n");

    char* database = "AE.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }

    ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", database);
        exit(1);
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int cnt = 0;
    // žî¹øÂ° AEÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            cnt++; // ¿ÀºêÁ§Æ® °³Œö
        }
    }
    //fprintf(stderr, "<%d>\n",cnt);

    if (cnt == 0) {
        fprintf(stderr, "Data not exist\n");
        return NULL;
        exit(1);
    }

    // cnt °³ŒöžžÅ­ µ¿ÀûÇÒŽç
    Node* head = Create_Node("","","",0);
    Node* node_ri;
    Node* node_pi;
    Node* node_rn;
    Node* node_ty;

    node_ri = node_pi = node_rn = node_ty = head;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "pi", key.size) == 0) {
            node_pi->pi = malloc(data.size);
            strcpy(node_pi->pi, data.data);
            node_pi->siblingRight = Create_Node("","","",0);
            node_pi->siblingRight->siblingLeft = node_pi;
            node_pi = node_pi->siblingRight;
        }
        if (strncmp(key.data, "ri", key.size) == 0) {
            node_ri->ri = malloc(data.size);
            strcpy(node_ri->ri, data.data);
            node_ri = node_ri->siblingRight;

        }
        if (strncmp(key.data, "rn", key.size) == 0) {
            node_rn->rn = malloc(data.size);
            strcpy(node_rn->rn, data.data);
            node_rn = node_rn->siblingRight;
        }
        if (strncmp(key.data, "ty", key.size) == 0) {
            node_ty->ty = *(int*)data.data;
            node_ty = node_ty->siblingRight;
        }
    }

    node_pi->siblingLeft->siblingRight = NULL;
    free(node_pi);
    node_ri = node_pi = node_rn = node_ty = NULL;

    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        exit(0);
    }

    return head;
}

Node* Get_All_CNT() {
    fprintf(stderr, "[Get All CNT]\n");

    char* database = "CNT.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }

    ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", database);
        exit(1);
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int cnt = 0;
    // žî¹øÂ° CNTÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            cnt++; // ¿ÀºêÁ§Æ® °³Œö
        }
    }
    //fprintf(stderr, "<%d>\n",cnt);

    if (cnt == 0) {
        fprintf(stderr, "Data not exist\n");
        return NULL;
        exit(1);
    }

    // cnt °³ŒöžžÅ­ µ¿ÀûÇÒŽç
    Node* head = Create_Node("","","",0);
    Node* node_ri;
    Node* node_pi;
    Node* node_rn;
    Node* node_ty;

    node_ri = node_pi = node_rn = node_ty = head;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "pi", key.size) == 0) {
            node_pi->pi = malloc(data.size);
            strcpy(node_pi->pi, data.data);
            node_pi->siblingRight = Create_Node("","","",0);
            node_pi->siblingRight->siblingLeft = node_pi;
            node_pi = node_pi->siblingRight;
        }
        if (strncmp(key.data, "ri", key.size) == 0) {
            node_ri->ri = malloc(data.size);
            strcpy(node_ri->ri, data.data);
            node_ri = node_ri->siblingRight;

        }
        if (strncmp(key.data, "rn", key.size) == 0) {
            node_rn->rn = malloc(data.size);
            strcpy(node_rn->rn, data.data);
            node_rn = node_rn->siblingRight;
        }
        if (strncmp(key.data, "ty", key.size) == 0) {
            node_ty->ty = *(int*)data.data;
            node_ty = node_ty->siblingRight;
        }
    }

    node_pi->siblingLeft->siblingRight = NULL;
    free(node_pi);
    node_ri = node_pi = node_rn = node_ty = NULL;

    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        exit(0);
    }

    return head;
}

Node* Get_All_CIN() {
    fprintf(stderr, "[Get All CIN]\n");

    char* database = "CIN.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }

    ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", database);
        exit(1);
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int cnt = 0;
    // žî¹øÂ° CINÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            cnt++; // ¿ÀºêÁ§Æ® °³Œö
        }
    }
    //fprintf(stderr, "<%d>\n",cnt);

    if (cnt == 0) {
        fprintf(stderr, "Data not exist\n");
        return NULL;
        exit(1);
    }

    // cnt °³ŒöžžÅ­ µ¿ÀûÇÒŽç
    Node* head = Create_Node("","","",0);
    Node* node_ri;
    Node* node_pi;
    Node* node_rn;
    Node* node_ty;

    node_ri = node_pi = node_rn = node_ty = head;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "pi", key.size) == 0) {
            node_pi->pi = malloc(data.size);
            strcpy(node_pi->pi, data.data);
            node_pi->siblingRight = Create_Node("","","",0);
            node_pi->siblingRight->siblingLeft = node_pi;
            node_pi = node_pi->siblingRight;
        }
        if (strncmp(key.data, "ri", key.size) == 0) {
            node_ri->ri = malloc(data.size);
            strcpy(node_ri->ri, data.data);
            node_ri = node_ri->siblingRight;

        }
        if (strncmp(key.data, "rn", key.size) == 0) {
            node_rn->rn = malloc(data.size);
            strcpy(node_rn->rn, data.data);
            node_rn = node_rn->siblingRight;
        }
        if (strncmp(key.data, "ty", key.size) == 0) {
            node_ty->ty = *(int*)data.data;
            node_ty = node_ty->siblingRight;
        }
    }

    node_pi->siblingLeft->siblingRight = NULL;
    free(node_pi);
    node_ri = node_pi = node_rn = node_ty = NULL;

    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        exit(0);
    }

    return head;
}

Node* Mapping(char uri[]) {
    Node* head = (Node*)malloc(sizeof(Node));
    Node* node;
    node = head;

    int flag = 1;
    char* cse_ri;
    char* ae_ri;
    char* cnt_ri;
    char* cin_ri;

    char* ptr = strtok(uri, "/");
    while (ptr != NULL) {
        //printf("%s\n", ptr);

        // 1: CSE, 2: AE, 3: CNT or CIN, 4: CNT or CIN, default: ERROR
        switch (flag) {
        //CSE
        case 1:
            //printf("CSE ");
            //¹ÝÈ¯ÇÒ ³ëµå¿¡ Ãß°¡ÇÏŽÂ °úÁ€
            node->rn = malloc(sizeof(ptr) + 1);
            strcpy(node->rn, ptr);

            //rn¿¡ ÇØŽçÇÏŽÂ ri Ã£±â
            cse_ri = malloc(sizeof(Find_ri("CSE.db", ptr))); 
            strcpy(cse_ri, Find_ri("CSE.db", ptr));
            node->ri = malloc(sizeof(cse_ri) + 1);
            strcpy(node->ri, cse_ri);

            node->siblingRight = (Node*)malloc(sizeof(Node));
            node->siblingRight->siblingLeft = node;
            node = node->siblingRight;

            //Mapping.db¿¡ ÀúÀå
            Store_map(ptr, cse_ri); 
            //printf("[%s : %s]\n", ptr, cse_ri);

            flag++;
            break;

        //AE
        case 2:
            //printf("AE ");

            //¹ÝÈ¯ÇÒ ³ëµå¿¡ Ãß°¡ÇÏŽÂ °úÁ€
            node->rn = malloc(sizeof(ptr) + 1);
            strcpy(node->rn, ptr);

            //rn¿¡ ÇØŽçÇÏŽÂ ri Ã£±â
            ae_ri = malloc(sizeof(Find_ri("AE.db", ptr)));
            strcpy(ae_ri, Find_ri("AE.db", ptr));
            node->ri = malloc(sizeof(ae_ri) + 1);
            strcpy(node->ri, ae_ri);

            node->siblingRight = (Node*)malloc(sizeof(Node));
            node->siblingRight->siblingLeft = node;
            node = node->siblingRight;

            //Mapping.db¿¡ ÀúÀå
            Store_map(ptr, ae_ri);

            //printf("[%s : %s]\n", ptr, ae_ri);
            flag++;
            break;

        //CNT
        case 3:
            //printf("CNT ");

            //¹ÝÈ¯ÇÒ ³ëµå¿¡ Ãß°¡ÇÏŽÂ °úÁ€
            node->rn = malloc(sizeof(ptr) + 1);
            strcpy(node->rn, ptr);

            //rn¿¡ ÇØŽçÇÏŽÂ ri Ã£±â
            cnt_ri = malloc(sizeof(Find_ri("CNT.db", ptr)));
            strcpy(cnt_ri, Find_ri("CNT.db", ptr));
            node->ri = malloc(sizeof(cnt_ri) + 1);
            strcpy(node->ri, cnt_ri);

            node->siblingRight = (Node*)malloc(sizeof(Node));
            node->siblingRight->siblingLeft = node;
            node = node->siblingRight;

            //Mapping.db¿¡ ÀúÀå
            Store_map(ptr, cnt_ri);

            //printf("[%s : %s]\n", ptr, cnt_ri);
            flag++;
            break;

        //CNT or CIN
        case 4:
            //printf("CNT or CIN ");

            //4¹øÂ° ÀÌÈÄ rnÀÌ CNTÀÌžé
            if (Find_ri("CIN.db", ptr) == NULL) {
                //¹ÝÈ¯ÇÒ ³ëµå¿¡ Ãß°¡ÇÏŽÂ °úÁ€
                node->rn = malloc(sizeof(ptr) + 1);
                strcpy(node->rn, ptr);

                //rn¿¡ ÇØŽçÇÏŽÂ ri Ã£±â
                cnt_ri = malloc(sizeof(Find_ri("CNT.db", ptr)));
                strcpy(cnt_ri, Find_ri("CNT.db", ptr));
                node->ri = malloc(sizeof(cnt_ri) + 1);
                strcpy(node->ri, cnt_ri);

                node->siblingRight = (Node*)malloc(sizeof(Node));
                node->siblingRight->siblingLeft = node;
                node = node->siblingRight;

                //Mapping.db¿¡ ÀúÀå
                Store_map(ptr, cnt_ri);

                //printf("[%s : %s]\n", ptr, cnt_ri);
                break;
            }
            //4¹øÂ° ÀÌÈÄ rnÀÌ CINÀÌžé
            else {
                //¹ÝÈ¯ÇÒ ³ëµå¿¡ Ãß°¡ÇÏŽÂ °úÁ€
                node->rn = malloc(sizeof(ptr) + 1);
                strcpy(node->rn, ptr);

                //rn¿¡ ÇØŽçÇÏŽÂ ri Ã£±â
                cin_ri = malloc(sizeof(Find_ri("CIN.db", ptr)));
                strcpy(cin_ri, Find_ri("CIN.db", ptr));
                node->ri = malloc(sizeof(cin_ri) + 1);
                strcpy(node->ri, cin_ri);

                node->siblingRight = (Node*)malloc(sizeof(Node));
                node->siblingRight->siblingLeft = node;
                node = node->siblingRight;

                //Mapping.db¿¡ ÀúÀå
                Store_map(ptr, cin_ri);

                //printf("[%s : %s]\n", ptr, cin_ri);
                break;
            }
        default:
            fprintf(stderr, "Flag ERROR\n");
            exit(0);
        }
        ptr = strtok(NULL, "/");
    }

    node->siblingLeft->siblingRight = NULL;
    free(node);

    return head;
}

char* Find_ri(char* database, char* rn) {

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }

    /* Open the database. */
    ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", database);
        exit(1);
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int idx = 0;
    int flag = 0;
    // žî¹øÂ° ¿ÀºêÁ§Æ®ÀÎÁö Ã£±â À§ÇÑ Ä¿Œ­
    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "rn", key.size) == 0) {
            idx++;
            if (strncmp(data.data, rn, data.size) == 0) {
                flag = 1;
                break;
            }
        }
    }
    if (flag == 0) {
        //printf("Not Found\n");
        return NULL;
        //exit(1);
    }
    //printf("<%d>\n", idx);

    char* ri = NULL;
    int cnt_ri = 0;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, "ri", key.size) == 0) {
            cnt_ri++;
            if (cnt_ri == idx) {
                ri = malloc(data.size);
                strcpy(ri, data.data);
            }
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        exit(0);
    }

    /* Cursors must be closed */
    if (dbcp0 != NULL)
        dbcp0->close(dbcp0);
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);

    return ri;
}

int Store_map(char* key_str, char* data_str) {
    char* database = "Mapping.db";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return 0;
    }

    /* Open the database. */
    ret = dbp->open(dbp, NULL, database, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", database);
        exit(1);
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    // Store key & data
    data.data = data_str;
    data.size = strlen(data_str) + 1;
    key.data = key_str;
    key.size = strlen(key_str) + 1;

    if ((ret = dbcp->put(dbcp, &key, &data, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "DB->cursor");

    //DB close
    dbcp->close(dbcp);
    dbp->close(dbp, 0);

    return 1;
}

int display(char* database)
{
    fprintf(stderr,"[Display] %s \n", database); //DB name print

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int close_db, close_dbc, ret;

    close_db = close_dbc = 0;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return (1);
    }
    close_db = 1;

    /* Turn on additional error output. */
    dbp->set_errfile(dbp, stderr);
    dbp->set_errpfx(dbp, database);

    /* Open the database. */
    if ((ret = dbp->open(dbp, NULL, database, NULL,
        DB_UNKNOWN, DB_RDONLY, 0)) != 0) {
        dbp->err(dbp, ret, "%s: DB->open", database);
        goto err;
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        goto err;
    }
    close_dbc = 1;

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    /* Walk through the database and print out the key/data pairs. */
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        //int
        if (strncmp(key.data, "ty", key.size) == 0 ||
            strncmp(key.data, "st", key.size) == 0 ||
            strncmp(key.data, "cni", key.size) == 0 ||
            strncmp(key.data, "cbs", key.size) == 0 ||
            strncmp(key.data, "cs", key.size) == 0
            ) {
            fprintf(stderr,"%.*s : %d\n", (int)key.size, (char*)key.data, *(int*)data.data);
        }
        //bool
        else if (strncmp(key.data, "rr", key.size) == 0) {
            fprintf(stderr,"%.*s : ", (int)key.size, (char*)key.data);
            if (*(bool*)data.data == true)
                fprintf(stderr,"true\n");
            else
                fprintf(stderr,"false\n");
        }

        //string
        else {
            fprintf(stderr,"%.*s : %.*s\n",
                (int)key.size, (char*)key.data,
                (int)data.size, (char*)data.data);
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr,"Cursor ERROR\n");
        exit(0);
    }


err:    if (close_dbc && (ret = dbcp->close(dbcp)) != 0)
dbp->err(dbp, ret, "DBcursor->close");
if (close_db && (ret = dbp->close(dbp, 0)) != 0)
fprintf(stderr,
    "%s: DB->close: %s\n", database, db_strerror(ret));
return (0);
}

