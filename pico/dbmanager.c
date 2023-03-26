#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sqlite/sqlite3.h"
#include "onem2m.h"
#include "dbmanager.h"
#include "logger.h"
#include "jsonparser.h"
#include "util.h"


sqlite3* db;


/* DB init */
int init_dbp(){
    sqlite3_stmt* res;
    char *sql = NULL, *err_msg = NULL;

    // Open (or create) DB File
    int rc = sqlite3_open("data.db", &db);
    
    if( rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot open database: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    // Setup Tables
    sql = calloc(1, 1024);
    
    strcpy(sql, "CREATE TABLE IF NOT EXISTS general ( \
        rn VARCHAR(60), ri VARCHAR(40), pi VARCHAR(40), ct VARCHAR(30), et VARCHAR(30), lt VARCHAR(30), \
        uri VARCHAR(255), acpi VARCHAR(255), lbl VARCHAR(255), ty INT );");
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot create table: %s", err_msg);
        sqlite3_close(db);
        free(sql);
        return 0;
    }

    strcpy(sql, "CREATE TABLE IF NOT EXISTS ae ( \
        ri VARCHAR(40), api VARCHAR(45), aei VARCHAR(200), rr VARCHAR(10), poa VARCHAR(255), apn VARCHAR(100), srv VARCHAR(45));");
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot create table: %s", err_msg);
        sqlite3_close(db);
        free(sql);
        return 0;
    }

    strcpy(sql, "CREATE TABLE IF NOT EXISTS cnt ( \
        ri VARCHAR(40), cr VARCHAR(40), mni INT, mbs INT, mia INT, cni INT, cbs INT, li VARCHAR(45), oref VARCHAR(45), disr VARCHAR(45));");
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot create table: %s", err_msg);
        sqlite3_close(db);
        free(sql);
        return 0;
    }

    strcpy(sql, "CREATE TABLE IF NOT EXISTS cin ( \
        ri VARCHAR(40), cs INT, cr VARCHAR(45), cnf VARCHAR(45), oref VARCHAR(45), con TEXT);");
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot create table: %s", err_msg);
        sqlite3_close(db);
        free(sql);
        return 0;
    }

    strcpy(sql, "CREATE TABLE IF NOT EXISTS acp ( ri VARCHAR(40), pvacop VARCHAR(60), pvacor VARCHAR(60), pvsacop VARCHAR(60), pvsacor VARCHAR(60));");
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot create table: %s", err_msg);
        sqlite3_close(db);
        free(sql);
        return 0;
    }

    strcpy(sql, "CREATE TABLE IF NOT EXISTS sub ( \
        ri VARCHAR(40), enc VARCHAR(45), exc VARCHAR(45), nu VARCHAR(200), gpi VARCHAR(45), nfu VARCAHR(45), bn VARCHAR(45), rl VARCHAR(45), \
        psn VARCHAR(45), pn VARCAHR(45), nsp VARCHAR(45), ln VARCHAR(45), nct VARCHAR(45), net VARCHAR(45), cr VARCHAR(45), su VARCHAR(45));");
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot create table: %s", err_msg);
        sqlite3_close(db);
        free(sql);
        return 0;
    }

    strcpy(sql, "CREATE TABLE IF NOT EXISTS grp ( \
        ri VARCHAR(40), cr VARCHAR(45), mt INT, cnm INT, mnm INT, mid TEXT, macp TEXT, mtv INT, csy INT, gn VARCAHR(30));");
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot create table: %s", err_msg);
        sqlite3_close(db);
        free(sql);
        return 0;
    }

    strcpy(sql, "CREATE TABLE IF NOT EXISTS cb ( \
        ri VARCHAR(40), cst VARCHAR(45), csi VARCHAR(45), srt VARCHAR(100), poa VARCHAR(200), nl VARCHAR(45), ncp VARCHAR(45), srv VARCHAR(45));");
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot create table: %s", err_msg);
        sqlite3_close(db);
        free(sql);
        return 0;
    }


    free(sql);
    return 1;
}




int close_dbp(){
    if(db)
        sqlite3_close(db);
    db = NULL;
    return 1;
}

int db_store_cse(CSE *cse_object){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_cse");

    int rc = 0;
    char sql[512] = {0}, err_msg = NULL;

     // if input == NULL
    if (cse_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }
    if (cse_object->rn == NULL) cse_object->rn = " ";
    if (cse_object->pi == NULL) cse_object->pi = " ";
    if (cse_object->ty == '\0') cse_object->ty = 0;
    if (cse_object->ct == NULL) cse_object->ct = " ";
    if (cse_object->lt == NULL) cse_object->lt = " ";
    if (cse_object->csi == NULL) cse_object->csi = " ";


    sprintf(sql, "INSERT INTO general (rn, ri, pi, ct, lt, ty) VALUES ('%s', '%s', '%s', '%s', '%s', %d);",
                cse_object->rn, cse_object->ri, cse_object->pi, cse_object->ct,
                cse_object->lt, cse_object->ty);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        return 0;
    }

    sprintf(sql, "INSERT INTO cb (ri, csi) VALUES('%s', '%s');",
                cse_object->ri, cse_object->csi);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        return 0;
    }
    
    return 1;
}

int db_store_ae(AE *ae_object){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_ae");
  //  AE *ae_object = (AE *) rtnode->obj;
    char* blankspace = " ";
    char *err_msg = NULL;
    int rc = 0;
    char attrs[128]={0}, vals[512]={0};
    char buf[128] = {0};
    char sql[1024] = {0};

        // db handle
    char rr[6] ="";

    // if input == NULL
    if (ae_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }

    if(ae_object->rr == false) strcpy(rr, "false");
    else strcpy(rr, "true");

  
    strcat(attrs, "ri,");
    sprintf(buf, "'%s',", ae_object->ri);
    strcat(vals, buf);

    if(ae_object->rn){
        strcat(attrs, "rn,");
        sprintf(buf, "'%s',", ae_object->rn);
        strcat(vals, buf);
    }
    if(ae_object->pi){
        strcat(attrs, "pi,");
        sprintf(buf, "'%s',", ae_object->pi);
        strcat(vals, buf);
    }
    if(ae_object->ct){
        strcat(attrs, "ct,");
        sprintf(buf, "'%s',", ae_object->ct);
        strcat(vals, buf);
    }
    if(ae_object->lt){
        strcat(attrs, "lt,");
        sprintf(buf, "'%s',", ae_object->lt);
        strcat(vals, buf);
    }
    if(ae_object->et){
        strcat(attrs, "et,");
        sprintf(buf, "'%s',", ae_object->et);
        strcat(vals, buf);
    }
    if(ae_object->lbl){
        strcat(attrs, "lbl,");
        sprintf(buf, "'%s',", ae_object->lbl);
        strcat(vals, buf);
    }
    if(ae_object->acpi){
        strcat(attrs, "acpi,");
        sprintf(buf, "'%s',", ae_object->acpi);
        strcat(vals, buf);
    }
    strcat(attrs, "ty");
    sprintf(buf, "%d", ae_object->ty);
    strcat(vals, buf);


    sprintf(sql, "INSERT INTO general (%s) VALUES(%s);", attrs, vals);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        return 0;
    }

    attrs[0] = '\0';
    vals[0] = '\0';

    strcat(attrs, "ri,");
    sprintf(buf, "'%s',", ae_object->ri);
    strcat(vals, buf);

    if(ae_object->api){
        strcat(attrs, "api,");
        sprintf(buf, "'%s',", ae_object->api);
        strcat(vals, buf);
    }
    if(ae_object->aei){
        strcat(attrs, "aei,");
        sprintf(buf, "'%s',", ae_object->aei);
        strcat(vals, buf);
    }
    if(ae_object->srv){
        strcat(attrs, "srv,");
        sprintf(buf, "'%s',", ae_object->srv);
        strcat(vals, buf);
    }
    
    strcat(attrs, "rr");
    sprintf(buf, "'%s'", rr);
    strcat(vals, buf);

    sprintf(sql, "INSERT INTO ae (%s) VALUES(%s);", attrs, vals);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        return 0;
    }
    
    return 1;

}

int db_store_cnt(CNT *cnt_object){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_cnt");
    char sql[1024] = {0}, *err_msg = NULL;
    int rc = 0;
    char attrs[128]={0}, vals[512]={0}, buf[256]={0};

    if (cnt_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }

    strcat(attrs, "ri,");
    sprintf(buf, "'%s',", cnt_object->ri);
    strcat(vals, buf);
    if(cnt_object->rn){
        strcat(attrs, "rn,");
        sprintf(buf, "'%s',", cnt_object->rn);
        strcat(vals, buf);
    }
    
    if(cnt_object->lbl){
        strcat(attrs, "lbl,");
        sprintf(buf, "'%s',", cnt_object->lbl);
        strcat(vals, buf);
    }
    if(cnt_object->pi){
        strcat(attrs, "pi,");
        sprintf(buf, "'%s',", cnt_object->pi);
        strcat(vals, buf);
    }
    if(cnt_object->ct){
        strcat(attrs, "ct,");
        sprintf(buf, "'%s',", cnt_object->ct);
        strcat(vals, buf);
    }
    if(cnt_object->lt){
        strcat(attrs, "lt,");
        sprintf(buf, "'%s',", cnt_object->lt);
        strcat(vals, buf);
    }
    if(cnt_object->et){
        strcat(attrs, "et,");
        sprintf(buf, "'%s',", cnt_object->et);
        strcat(vals, buf);
    }
    if(cnt_object->acpi){
        strcat(attrs, "acpi,");
        sprintf(buf, "'%s',", cnt_object->acpi);
        strcat(vals, buf);
    }
    strcat(attrs, "ty");
    sprintf(buf, "%d", RT_CNT);
    strcat(vals, buf);

    sprintf(sql, "INSERT INTO general (%s) VALUES(%s);", attrs, vals);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %d", sql, err_msg);
        return 0;
    }

    attrs[0] = '\0';
    vals[0] = '\0';

    strcat(attrs, "ri,");
    sprintf(buf, "'%s',", cnt_object->ri);
    strcat(vals, buf);
    
    if(cnt_object->mni >= 0){
        strcat(attrs, "mni,");
        sprintf(buf, "%d,", cnt_object->mni);
        strcat(vals, buf);
    }
    if(cnt_object->mbs >= 0){
        strcat(attrs, "mbs,");
        sprintf(buf, "%d,", cnt_object->mbs);
        strcat(vals, buf);
    }
    if(cnt_object->cni >= 0){
        strcat(attrs, "cni,");
        sprintf(buf, "%d,", cnt_object->cni);
        strcat(vals, buf);
    }
    if(cnt_object->cbs >= 0){
        strcat(attrs, "cbs,");
        sprintf(buf, "%d,", cnt_object->cbs);
        strcat(vals, buf);
    }
    attrs[strlen(attrs)-1] = '\0';
    vals[strlen(vals)-1] = '\0';
    
    sprintf(sql, "INSERT INTO cnt (%s) VALUES(%s);", attrs, vals);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        return 0;
    }

    return 1;
}

int db_store_cin(CIN *cin_object) {
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_cin");
    
    char *sql = NULL, *err_msg = NULL;
    char attrs[256] = {0}, vals[1024] = {0};
    char buf[256] = {0};
    int rc = 0;
    
    // if input == NULL
    if (cin_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }


    strcat(attrs, "ri,");
    sprintf(buf, "'%s',", cin_object->ri);
    strcat(vals, buf);

    if(cin_object->rn){
        strcat(attrs, "rn,");
        sprintf(buf, "'%s',", cin_object->rn);
        strcat(vals, buf);
    }
    if(cin_object->pi){
        strcat(attrs, "pi,");
        sprintf(buf, "'%s',", cin_object->pi);
        strcat(vals, buf);
    }
    if(cin_object->ct){
        strcat(attrs, "ct,");
        sprintf(buf, "'%s',", cin_object->ct);
        strcat(vals, buf);
    }
    if(cin_object->lt){
        strcat(attrs, "lt,");
        sprintf(buf, "'%s',", cin_object->lt);
        strcat(vals, buf);
    }
    if(cin_object->et){
        strcat(attrs, "et,");
        sprintf(buf, "'%s',", cin_object->et);
        strcat(vals, buf);
    }
    strcat(attrs, "ty");
    sprintf(buf, "%d", cin_object->ty);
    strcat(vals, buf);

    
    sql = malloc(sizeof(char) * 1024);

    sprintf(sql, "INSERT INTO general (%s) VALUES(%s);", attrs, vals);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        return 0;
    }
    attrs[0] = '\0';
    vals[0] = '\0';

    strcat(attrs, "ri,");
    sprintf(buf, "'%s',", cin_object->ri);
    strcat(vals, buf);

    strcat(attrs, "cs,");
    sprintf(buf, "%d,", cin_object->cs);
    strcat(vals, buf);

    if(cin_object->con){
        strcat(attrs, "con,");
        sprintf(buf, "'%s',", cin_object->con);
        strcat(vals, buf);
    }

    attrs[strlen(attrs)-1] = '\0';
    vals[strlen(vals) - 1] = '\0';

    sprintf(sql, "INSERT INTO cin (%s) VALUES(%s);", attrs, vals);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg); 
        return 0;
    }

    return 1;
}

int db_store_grp(GRP *grp_object){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_grp");
    char sql[1024] = {0}, *err_msg = NULL;
    char attrs[128] = {0}, vals[512] = {0};
    char buf[256] = {0};
    int rc = 0;

    if(grp_object->ri == NULL){
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }

    strcat(attrs, "ri,");
    sprintf(buf, "'%s',", grp_object->ri);
    strcat(vals, buf);

    if(grp_object->rn){
        strcat(attrs, "rn,");
        sprintf(buf, "'%s',", grp_object->rn);
        strcat(vals, buf);
    }
    if(grp_object->pi){
        strcat(attrs, "pi,");
        sprintf(buf, "'%s',", grp_object->pi);
        strcat(vals, buf);
    }
    if(grp_object->ct){
        strcat(attrs, "ct,");
        sprintf(buf, "'%s',", grp_object->ct);
        strcat(vals, buf);
    }
    if(grp_object->lt){
        strcat(attrs, "lt,");
        sprintf(buf, "'%s',", grp_object->lt);
        strcat(vals, buf);
    }
    if(grp_object->et){
        strcat(attrs, "et,");
        sprintf(buf, "'%s',", grp_object->et);
        strcat(vals, buf);
    }
    strcat(attrs, "ty");
    sprintf(buf, "%d", RT_GRP);
    strcat(vals, buf);


    sprintf(sql, "INSERT INTO general (%s) VALUES(%s);",attrs, vals);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    attrs[0] = '\0';
    vals[0] = '\0';

    strcat(attrs, "ri,");
    sprintf(buf, "'%s',", grp_object->ri);
    strcat(vals, buf);

    if(grp_object->mt){
        strcat(attrs, "mt,");
        sprintf(buf, "%d,", grp_object->mt);
        strcat(vals, buf);
    }
    if(grp_object->cnm){
        strcat(attrs, "cnm,");
        sprintf(buf, "%d,", grp_object->cnm);
        strcat(vals, buf);
    }
    if(grp_object->mnm){
        strcat(attrs, "mnm,");
        sprintf(buf, "%d,", grp_object->mnm);
        strcat(vals, buf);
    }
    if(grp_object->mtv){
        strcat(attrs, "mtv,");
        sprintf(buf, "%d,", grp_object->mtv);
        strcat(vals, buf);
    }
    if(grp_object->csy){
        strcat(attrs, "csy,");
        sprintf(buf, "%d,", grp_object->csy);
        strcat(vals, buf);
    }

    strcat(attrs, "mid,");
    // change mid to str
    char strbuf[128] = {0};
    if(grp_object->mid && grp_object->cnm > 0) {
        sprintf(strbuf, "'[%s", grp_object->mid[0]);
        strcat(vals, strbuf);
        for(int i = 1 ; i < grp_object->cnm; i++){
            if(grp_object->mid[i]){
                sprintf(strbuf, ",%s", grp_object->mid[i]);
                strcat(vals, strbuf);
            }else
                break;
        }
        strcat(vals, "]',");
    }else{
        strcat(vals, "'[]',");
    }

    attrs[strlen(attrs) - 1] = '\0';
    vals[strlen(vals) - 1] = '\0';
    

    sprintf(sql, "INSERT INTO grp (%s) VALUES(%s);", attrs, vals);
    logger("DB-t", LOG_LEVEL_DEBUG, "sql : %s", sql);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    return 1;
}

int db_store_sub(RTNode* rtnode){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_cnt");
    char* blankspace = " ";
    char *sql = NULL, *err_msg = NULL;
    int rc = 0;

    SUB *sub_object = (SUB *)rtnode->obj;
    
    // if input == NULL
    if (sub_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }
    if (sub_object->rn == NULL) sub_object->rn = blankspace;
    if (sub_object->ri == NULL) sub_object->ri = blankspace;
    if (sub_object->nu == NULL) sub_object->nu = blankspace;
    if (sub_object->net == NULL) sub_object->net = blankspace;
    if (sub_object->ct == NULL) sub_object->ct = blankspace;
    if (sub_object->et == NULL) sub_object->et = blankspace;
    if (sub_object->lt == NULL) sub_object->lt = blankspace;
    if (sub_object->ty == '\0') sub_object->ty = 23;
    if (sub_object->sur == NULL) sub_object->sur = blankspace;    

    sql = malloc(sizeof(char) * 1024);

    sprintf(sql, "INSERT INTO general (rn, ri, pi, ct, et, lt, uri, ty) VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d);",
              sub_object->rn, sub_object->ri, sub_object->pi, sub_object->ct, sub_object->et, sub_object->lt, rtnode->uri, sub_object->ty );
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    sprintf(sql, "INSERT INTO sub (ri, pi, nu, nct, net, sur) VALUES('%s', '%s', '%s', %d, '%s', '%s');",
               sub_object->ri, sub_object->pi, sub_object->nu, sub_object->nct, sub_object->net, sub_object->sur);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }
    
    if (sub_object->rn == blankspace) sub_object->rn = NULL;
    if (sub_object->ri == blankspace) sub_object->ri = NULL;
    if (sub_object->nu == blankspace) sub_object->nu = NULL;
    if (sub_object->net == blankspace) sub_object->net = NULL;
    if (sub_object->ct == blankspace) sub_object->ct = NULL;
    if (sub_object->et == blankspace) sub_object->et = NULL;
    if (sub_object->lt == blankspace) sub_object->lt = NULL;
    if (sub_object->ty == '\0') sub_object->ty = 23;
    if (sub_object->sur == blankspace) sub_object->sur = NULL; 

    free(sql);
    return 1;
}

int db_store_acp(ACP *acp_object) {
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_acp");
    char* blankspace = " ";
    char *sql = NULL, *err_msg = NULL;
    int rc = 0;

    // if input == NULL
    if (acp_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }
    if (acp_object->rn == NULL) acp_object->rn = blankspace;
    if (acp_object->pi == NULL) acp_object->pi = blankspace;
    if (acp_object->ct == NULL) acp_object->ct = blankspace;
    if (acp_object->lt == NULL) acp_object->lt = blankspace;
    if (acp_object->et == NULL) acp_object->et = blankspace;
    if (acp_object->pv_acor == NULL) acp_object->pv_acor = blankspace;       
    if (acp_object->pv_acop == NULL) acp_object->pv_acop = blankspace; 
    if (acp_object->pvs_acop == NULL) acp_object->pvs_acor = blankspace; 
    if (acp_object->pvs_acop == NULL) acp_object->pvs_acop = blankspace; 

   sprintf(sql, "INSERT INTO general (rn, ri, pi, ct, et, lt, uri, ty) VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d);",
              acp_object->rn, acp_object->ri, acp_object->pi, acp_object->ct, acp_object->et, acp_object->lt, "", acp_object->ty );
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    sprintf(sql, "INSERT INTO acp (ri, pvacop, pvacor, pvsacop, pvsacor) VALUES('%s', '%s', '%s', '%s', '%s');",
               acp_object->ri, acp_object->pv_acop, acp_object->pv_acor, acp_object->pvs_acop, acp_object->pvs_acor);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    if (acp_object->rn == blankspace) acp_object->rn = NULL;
    if (acp_object->pi == blankspace) acp_object->pi = NULL;
    if (acp_object->ct == blankspace) acp_object->ct = NULL;
    if (acp_object->lt == blankspace) acp_object->lt = NULL;
    if (acp_object->et == blankspace) acp_object->et = NULL;
    if (acp_object->pv_acor == blankspace) acp_object->pv_acor = NULL;       
    if (acp_object->pv_acop == blankspace) acp_object->pv_acop = NULL; 
    if (acp_object->pvs_acop == blankspace) acp_object->pvs_acor = NULL; 
    if (acp_object->pvs_acop == blankspace) acp_object->pvs_acop = NULL; 
    return 1;
}

CSE *db_get_cse(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_cse");
    int rc = 0;
    int cols = 0;
    sqlite3_stmt *res;
    CSE* new_cse= NULL;
    char *colname;
    int bytes = 0;

    rc = sqlite3_prepare_v2(db, "SELECT * FROM general, cb WHERE general.ri = cb.ri;", -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select, %d", rc);
        return 0;
    }

    if(sqlite3_step(res) != SQLITE_ROW){
        return 0;
    }
    cols = sqlite3_column_count(res);
    new_cse = calloc(1, sizeof(CSE));
    for(int col = 0 ; col < cols; col++){
        colname = sqlite3_column_name(res, col);
        if( (bytes = sqlite3_column_bytes(res, col)) == 0)
            continue;

        if(!strcmp(colname, "rn") ){
            new_cse->rn = calloc(sizeof(char), bytes + 2);
            strncpy(new_cse->rn, sqlite3_column_text(res, col), bytes);
        }else if( !strcmp(colname, "ri") ){
            new_cse->ri = calloc(sizeof(char), bytes + 2);
            strncpy(new_cse->ri, sqlite3_column_text(res, col), bytes);
        }else if( !strcmp(colname, "pi") ){
            new_cse->pi = calloc(sizeof(char), bytes + 2);
            strncpy(new_cse->pi, sqlite3_column_text(res, col), bytes);
        }else if( !strcmp(colname, "ct") ){
            new_cse->ct = calloc(sizeof(char), bytes + 2);
            strncpy(new_cse->ct, sqlite3_column_text(res, col), bytes);
        }else if( !strcmp(colname, "lt") ){
            new_cse->lt = calloc(sizeof(char), bytes + 2);
            strncpy(new_cse->lt, sqlite3_column_text(res, col), bytes);
        }else if( !strcmp(colname, "csi") ){
            new_cse->csi = calloc(sizeof(char), bytes + 2);
            strncpy(new_cse->csi, sqlite3_column_text(res, col), bytes);
        }
    }
    new_cse->ty = RT_CSE;
    

    sqlite3_finalize(res);
    return new_cse;
}

AE *db_get_ae(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_ae");

    //struct to return
    int rc = 0;
    int cols = 0;
    AE* new_ae = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    cJSON *json = NULL, *root = NULL;
    int bytes = 0, coltype = 0;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, ae WHERE general.ri='%s' and ae.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    if(sqlite3_step(res) != SQLITE_ROW){
        return 0;
    }
    cols = sqlite3_column_count(res);
    json = cJSON_CreateObject();
    root = cJSON_CreateObject();
    
    for(int col = 0 ; col < cols; col++){
        
        colname = sqlite3_column_name(res, col);
        bytes = sqlite3_column_bytes(res, col);
        coltype = sqlite3_column_type(res, col);

        if(!strcmp(colname, "rr")){
            if(!strncmp(sqlite3_column_text(res, col), "true", 4)){
                cJSON_AddItemToObject(json, colname, cJSON_CreateBool(true));
            }else{
                cJSON_AddItemToObject(json, colname, cJSON_CreateBool(false));
            }
            continue;
        }

        if(bytes == 0) continue;
        switch(coltype){
            case SQLITE_TEXT:
                memset(buf,0, 256);
                strncpy(buf, sqlite3_column_text(res, col), bytes);
                cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                break;
            case SQLITE_INTEGER:
                cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                break;
        }
    }
    cJSON_AddItemToObject(root, "m2m:ae", json);
    new_ae = cjson_to_ae(root);
    cJSON_Delete(root);

    sqlite3_finalize(res);
    return new_ae;   
}

CNT *db_get_cnt(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_cnt");

    //struct to return
    int rc = 0;
    int cols = 0;
    CNT* new_cnt = NULL;
    cJSON *json = NULL, *root = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    int bytes = 0, coltype = 0;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, cnt WHERE general.ri='%s' and cnt.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    if(sqlite3_step(res) != SQLITE_ROW){
        return 0;
    }
    cols = sqlite3_column_count(res);


    json = cJSON_CreateObject();
    root = cJSON_CreateObject();
    
    for(int col = 0 ; col < cols; col++){
        
        colname = sqlite3_column_name(res, col);
        bytes = sqlite3_column_bytes(res, col);
        coltype = sqlite3_column_type(res, col);

        if(bytes == 0) continue;
        switch(coltype){
            case SQLITE_TEXT:
                memset(buf,0, 256);
                strncpy(buf, sqlite3_column_text(res, col), bytes);
                cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                break;
            case SQLITE_INTEGER:
                cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                break;
        }
    }
    cJSON_AddItemToObject(root, "m2m:cnt", json);
    new_cnt = cjson_to_cnt(root);
    cJSON_Delete(root);
    

    sqlite3_finalize(res);
    return new_cnt;   
}

CIN *db_get_cin(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_cin");

    //struct to return
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    CIN* new_cin = NULL;
    cJSON *json, *root;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, cin WHERE general.ri='%s' and cin.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    if(sqlite3_step(res) != SQLITE_ROW){
        return 0;
    }
    cols = sqlite3_column_count(res);
    json = cJSON_CreateObject();
    root = cJSON_CreateObject();
    
    for(int col = 0 ; col < cols; col++){
        
        colname = sqlite3_column_name(res, col);
        bytes = sqlite3_column_bytes(res, col);
        coltype = sqlite3_column_type(res, col);

        if(bytes == 0) continue;
        switch(coltype){
            case SQLITE_TEXT:
                memset(buf,0, 256);
                strncpy(buf, sqlite3_column_text(res, col), bytes);
                cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                break;
            case SQLITE_INTEGER:
                cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                break;
        }
    }
    cJSON_AddItemToObject(root, "m2m:cin", json);
    new_cin = cjson_to_cin(root);
    cJSON_Delete(json);


    sqlite3_finalize(res);
    return new_cin;   
}

SUB *db_get_sub(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_sub");

    //struct to return
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    SUB* new_sub = NULL;
    sqlite3_stmt *res;
    cJSON *json = NULL, *root = NULL;
    char *colname = NULL;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, sub WHERE general.ri='%s' and sub.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    if(sqlite3_step(res) != SQLITE_ROW){
        return 0;
    }
    cols = sqlite3_column_count(res);
    json = cJSON_CreateObject();
    root = cJSON_CreateObject();
    
    for(int col = 0 ; col < cols; col++){
        
        colname = sqlite3_column_name(res, col);
        bytes = sqlite3_column_bytes(res, col);
        coltype = sqlite3_column_type(res, col);

        if(bytes == 0) continue;
        switch(coltype){
            case SQLITE_TEXT:
                memset(buf, 0, 256);
                strncpy(buf, sqlite3_column_text(res, col), bytes);
                cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                break;
            case SQLITE_INTEGER:
                cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                break;
        }
    }
    cJSON_AddItemToObject(root, "m2m:sub", json);
    new_sub = cjson_to_sub(root);
    cJSON_Delete(root);


    sqlite3_finalize(res);
    return new_sub;   
}

GRP *db_get_grp(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_grp");

    //struct to return
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    GRP* new_grp = NULL;
    cJSON *root = NULL, *json = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM 'general', 'grp' WHERE general.ri='%s' and grp.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return NULL;
    }

    if(sqlite3_step(res) != SQLITE_ROW){
        logger("DB", LOG_LEVEL_ERROR, "Not Found");
        return NULL;
    }

    new_grp = calloc(1, sizeof(GRP));
    cols = sqlite3_column_count(res);
    json = cJSON_CreateObject();
    root = cJSON_CreateObject();
    
    for(int col = 0 ; col < cols; col++){
        
        colname = sqlite3_column_name(res, col);
        bytes = sqlite3_column_bytes(res, col);
        coltype = sqlite3_column_type(res, col);

        if(bytes == 0) continue;
        switch(coltype){
            case SQLITE_TEXT:
                memset(buf, 0, 256);
                strncpy(buf, sqlite3_column_text(res, col), bytes);
                if(buf[0] == '['){
                    cJSON_AddItemToObject(json, colname, string_to_cjson_list_item(buf));
                }else{
                    cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                }
                break;
            case SQLITE_INTEGER:
                cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                break;
        }
    }
    cJSON_AddItemToObject(root, "m2m:grp", json);
    cjson_to_grp(root, new_grp);
    cJSON_Delete(root);


    sqlite3_finalize(res);
    return new_grp;   
}

ACP *db_get_acp(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_acp");

    //struct to return
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    ACP* new_acp = NULL;
    sqlite3_stmt *res;
    cJSON *json = NULL, *root = NULL;
    char *colname = NULL;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, acp WHERE general.ri='%s' and acp.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    if(sqlite3_step(res) != SQLITE_ROW){
        return 0;
    }
    cols = sqlite3_column_count(res);

    json = cJSON_CreateObject();
    root = cJSON_CreateObject();
    
    for(int col = 0 ; col < cols; col++){
        
        colname = sqlite3_column_name(res, col);
        bytes = sqlite3_column_bytes(res, col);
        coltype = sqlite3_column_type(res, col);

        if(bytes == 0) continue;
        switch(coltype){
            case SQLITE_TEXT:
                memset(buf,0, 256);
                strncpy(buf, sqlite3_column_text(res, col), bytes);
                cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                break;
            case SQLITE_INTEGER:
                cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                break;
        }
    }
    cJSON_AddItemToObject(root, "m2m:acp", json);
    new_acp = cjson_to_acp(root);
    cJSON_Delete(root);

    sqlite3_finalize(res);
    return new_acp;   
}

int db_delete_onem2m_resource(RTNode *rtnode) {
    logger("DB", LOG_LEVEL_DEBUG, "Delete [RI] %s",get_ri_rtnode(rtnode));
    char sql[1024] = {0};
    char *err_msg;
    char *tableName = NULL;
    int rc; 

    sprintf(sql, "DELETE FROM general WHERE ri='%s';", get_ri_rtnode(rtnode));
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource from general/ msg : %s", err_msg);
        sqlite3_close(db);
        return 0;
    }

    switch(rtnode->ty){
        case RT_AE:
            tableName = "ae";
            break;
        case RT_CNT:
            tableName = "cnt";
            break;
        case RT_CIN:
            tableName = "cin";
            break;
        case RT_ACP:
            tableName = "acp";
            break;
        case RT_GRP:
            tableName = "grp";
            break;
        case RT_SUB:
            tableName = "sub";
            break;
    }

    sprintf(sql, "DELETE FROM %s WHERE ri='%s';", tableName, get_ri_rtnode(rtnode));
    logger("dbt", LOG_LEVEL_DEBUG, "sql : %s", sql);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource from %s/ msg : %s", tableName, err_msg);
        sqlite3_close(db);
        return 0;
    }


    return 1;
}

int db_delete_sub(char* ri) {
    logger("DB", LOG_LEVEL_DEBUG, "Call db_delete_sub");
    
    char sql[1024] = {0};
    char *err_msg;
    int rc; 

    sprintf(sql, "DELETE FROM sub WHERE ri='%s';", ri);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource %s", err_msg);
        sqlite3_close(db);
        return 0;
    }

    sprintf(sql, "DELETE FROM general WHERE ri='%s';", ri);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource %s", err_msg);
        sqlite3_close(db);
        return 0;
    }

    return 1;
}

int db_delete_acp(char* ri) {
    logger("DB", LOG_LEVEL_DEBUG, "Call db_delete_acp");
    
    char sql[1024] = {0};
    char *err_msg;
    int rc; 

    sprintf(sql, "DELETE FROM acp WHERE ri='%s';", ri);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource %s", err_msg);
        sqlite3_close(db);
        return 0;
    }

    sprintf(sql, "DELETE FROM general WHERE ri='%s';", ri);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource %s", err_msg);
        sqlite3_close(db);
        return 0;
    }

    return 1;
}

int db_delete_grp(char* ri) {
    logger("DB", LOG_LEVEL_DEBUG, "Call db_delete_grp");
    
    char sql[1024] = {0};
    char *err_msg;
    int rc; 

    sprintf(sql, "DELETE FROM grp WHERE ri='%s';", ri);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource %s", err_msg);
        sqlite3_close(db);
        return 0;
    }

    sprintf(sql, "DELETE FROM general WHERE ri='%s';", ri);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource %s", err_msg);
        sqlite3_close(db);
        return 0;
    }

    return 1;
}

RTNode* db_get_all_cse() {
    fprintf(stderr,"\x1b[92m[Get All CSE]\x1b[0m\n");
    int rc = 0;
    int cols = 0;
    
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, cb WHERE general.ri = cb.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;
    rc = sqlite3_step(res);
    cols = sqlite3_column_count(res);
    while(rc == SQLITE_ROW){
        CSE* cse = calloc(1, sizeof(CSE));
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                cse->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                cse->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                cse->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                cse->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                cse->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "csi") ){
                cse->csi = strdup(sqlite3_column_text(res, col));
            }
        }
        cse->ty = RT_CSE;
        if(!head) {
            head = create_rtnode(cse,RT_CSE);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(cse,RT_CSE);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        rc = sqlite3_step(res);
    }

    // if (dbp != NULL)
    //     dbp->close(dbp, 0);   
    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_ae_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_ae_rtnode");
    int rc = 0;
    int cols = 0;
    int coltype = 0, bytes = 0;
    cJSON *json, *root;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};
    char buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, ae WHERE general.ri = ae.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;

    rc = sqlite3_step(res);
    cols = sqlite3_column_count(res);
    while(rc == SQLITE_ROW){
        json = cJSON_CreateObject();
        root = cJSON_CreateObject();
        
        AE* ae = NULL;
        for(int col = 0 ; col < cols; col++){
            
            colname = sqlite3_column_name(res, col);
            bytes = sqlite3_column_bytes(res, col);
            coltype = sqlite3_column_type(res, col);
            if(!strcmp(colname, "rr")){
                if(!strncmp(sqlite3_column_text(res, col), "true", 4)){
                    cJSON_AddItemToObject(json, colname, cJSON_CreateBool(true));
                }else{
                    cJSON_AddItemToObject(json, colname, cJSON_CreateBool(false));
                }
                continue;
            }

            if(bytes == 0) continue;
            switch(coltype){
                case SQLITE_TEXT:
                    memset(buf, 0, 256);
                    strncpy(buf, sqlite3_column_text(res, col), bytes);
                    cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                    break;
                case SQLITE_INTEGER:
                    cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                    break;
            }
        }
        cJSON_AddItemToObject(root, "m2m:ae", json);
        ae = cjson_to_ae(root);
        if(!head) {
            head = create_rtnode(ae,RT_AE);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(ae, RT_AE);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }  
        cJSON_Delete(root); 
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_cnt_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_cnt_rtnode");
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    cJSON *json, *root;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0}, buf[256]={0};

    sprintf(sql, "SELECT * FROM general, cnt WHERE general.ri = cnt.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;
    rc = sqlite3_step(res);
    cols = sqlite3_column_count(res);
    while(rc == SQLITE_ROW){
        json = cJSON_CreateObject();
        root = cJSON_CreateObject();
        
        CNT* cnt = NULL;
        for(int col = 0 ; col < cols; col++){
            
            colname = sqlite3_column_name(res, col);
            bytes = sqlite3_column_bytes(res, col);
            coltype = sqlite3_column_type(res, col);

            if(bytes == 0) continue;
            switch(coltype){
                case SQLITE_TEXT:
                    memset(buf, 0, 256);
                    strncpy(buf, sqlite3_column_text(res, col), bytes);
                    cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                    break;
                case SQLITE_INTEGER:
                    cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                    break;
            }
        }
        cJSON_AddItemToObject(root, "m2m:cnt", json);
        cnt = cjson_to_cnt(root);
        if(!head) {
            head = create_rtnode(cnt,RT_CNT);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(cnt, RT_CNT);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        cJSON_Delete(root);
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_sub_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_sub_rtnode");
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    cJSON *json, *root;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, sub WHERE general.ri = sub.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;
    rc = sqlite3_step(res);
    cols = sqlite3_column_count(res);

    while(rc == SQLITE_ROW){
        json = cJSON_CreateObject();
        root = cJSON_CreateObject();
        
        SUB* sub = NULL;
        for(int col = 0 ; col < cols; col++){
            
            colname = sqlite3_column_name(res, col);
            bytes = sqlite3_column_bytes(res, col);
            coltype = sqlite3_column_type(res, col);

            if(bytes == 0) continue;
            switch(coltype){
                case SQLITE_TEXT:
                    memset(buf, 0, 256);
                    strncpy(buf, sqlite3_column_text(res, col), bytes);
                    cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                    break;
                case SQLITE_INTEGER:
                    cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                    break;
            }
        }
        cJSON_AddItemToObject(root, "m2m:sub", json);
        sub = cjson_to_sub(root);
        if(!head) {
            head = create_rtnode(sub,RT_SUB);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(sub, RT_SUB);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        cJSON_Delete(root);
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_acp_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_acp_rtnode");
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    cJSON *json = NULL, *root = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, acp WHERE general.ri = acp.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;
    rc = sqlite3_step(res);
    cols = sqlite3_column_count(res);

    while(rc == SQLITE_ROW){
        json = cJSON_CreateObject();
        root = cJSON_CreateObject();
        ACP* acp = NULL;
        
        for(int col = 0 ; col < cols; col++){
            
            colname = sqlite3_column_name(res, col);
            bytes = sqlite3_column_bytes(res, col);
            coltype = sqlite3_column_type(res, col);

            if(bytes == 0) continue;
            switch(coltype){
                case SQLITE_TEXT:
                    memset(buf, 0, 256);
                    strncpy(buf, sqlite3_column_text(res, col), bytes);
                    cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                    break;
                case SQLITE_INTEGER:
                    cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                    break;
            }
        }
        cJSON_AddItemToObject(root, "m2m:acp", json);
        acp = cjson_to_acp(root);
        if(!head) {
            head = create_rtnode(acp,RT_SUB);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(acp, RT_SUB);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        cJSON_Delete(root);
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_grp_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_grp_rtnode");
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    cJSON *json = NULL, *root = NULL;
    sqlite3_stmt *res;
    GRP* grp = NULL;
    char *colname = NULL;
    char sql[1024] = {0}, buf[256] = {0};

    sprintf(sql, "SELECT * FROM general, grp WHERE general.ri = grp.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;
    rc = sqlite3_step(res);

    while(rc == SQLITE_ROW){
        cols = sqlite3_column_count(res);
        grp = (GRP*) calloc(1, sizeof(GRP));
        json = cJSON_CreateObject();
        root = cJSON_CreateObject();
        
        for(int col = 0 ; col < cols; col++){
            
            colname = sqlite3_column_name(res, col);
            bytes = sqlite3_column_bytes(res, col);
            coltype = sqlite3_column_type(res, col);

            if(bytes == 0) continue;
            switch(coltype){
                case SQLITE_TEXT:
                    memset(buf, 0, 256);
                    strncpy(buf, sqlite3_column_text(res, col), bytes);
                    if(buf[0] == '['){
                        cJSON_AddItemToObject(json, colname, string_to_cjson_list_item(buf));
                    }else{
                        cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                    }
                    break;
                case SQLITE_INTEGER:
                    cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                    break;
            }
        }
        cJSON_AddItemToObject(root, "m2m:grp", json);
        cjson_to_grp(root, grp);
        
        if(!head) {
            head = create_rtnode(grp,RT_GRP);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(grp, RT_GRP);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        cJSON_Delete(root);
        root = NULL;
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode* db_get_cin_rtnode_list(RTNode *parent_rtnode) {

    logger("DB", LOG_LEVEL_DEBUG, "call db_get_cin_rtnode_list");
    char buf[256] = {0};
    char* pi = get_ri_rtnode(parent_rtnode);
    int rc = 0;
    int cols = 0, bytes = 0, coltype = 0;
    cJSON *json, *root;
    sqlite3_stmt *res = NULL;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM 'general', 'cin' WHERE general.pi='%s' AND general.ri=cin.ri;", pi);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select, %d", rc);
        return 0;
    }
    
        
    RTNode* head = NULL, *rtnode = NULL;
    rc = sqlite3_step(res);
    cols = sqlite3_column_count(res);
    while(rc == SQLITE_ROW){
        CIN* cin = NULL;
        json = cJSON_CreateObject();
        root = cJSON_CreateObject();
        
        for(int col = 0 ; col < cols; col++){
            
            colname = sqlite3_column_name(res, col);
            bytes = sqlite3_column_bytes(res, col);
            coltype = sqlite3_column_type(res, col);

            if(bytes == 0) continue;
            switch(coltype){
                case SQLITE_TEXT:
                    memset(buf,0, 256);
                    strncpy(buf, sqlite3_column_text(res, col), bytes);
                    cJSON_AddItemToObject(json, colname, cJSON_CreateString(buf));
                    break;
                case SQLITE_INTEGER:
                    cJSON_AddItemToObject(json, colname, cJSON_CreateNumber(sqlite3_column_int(res, col)));
                    break;
            }
        }
        cJSON_AddItemToObject(root, "m2m:cin", json);
        logger("DB", LOG_LEVEL_DEBUG, "%s", cJSON_PrintUnformatted(root));
        cin = cjson_to_cin(root);
        if(!head) {
            head = create_rtnode(cin,RT_CIN);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(cin, RT_CIN);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        cJSON_Delete(root);
        json = NULL;
        root = NULL;
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}