#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sqlite/sqlite3.h"
#include "onem2m.h"
#include "dbmanager.h"
#include "logger.h"
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
        ri VARCHAR(40), pi VARCHAR(40), rr VARCHAR(10), poa VARCHAR(255), apn VARCHAR(100));");
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
        ri VARCHAR(40), pi VARCHAR(40), cs INT, cr VARCHAR(45), cnf VARCHAR(45), oref VARCHAR(45), con TEXT);");
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
        ri VARCHAR(40), pi VARCHAR(40), enc VARCHAR(45), exc VARCHAR(45), nu VARCHAR(200), gpi VARCHAR(45), nfu VARCAHR(45), bn VARCHAR(45), rl VARCHAR(45), \
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
    char *sql = NULL, err_msg = NULL;
    int rc = 0;

        // db handle
    char rr[6] ="";

    // if input == NULL
    if (ae_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }
    if (ae_object->rn == NULL) ae_object->rn = blankspace;
    if (ae_object->pi == NULL) ae_object->pi = blankspace;
    if (ae_object->ty == '\0') ae_object->ty = 0;
    if (ae_object->ct == NULL) ae_object->ct = blankspace;
    if (ae_object->lt == NULL) ae_object->lt = blankspace;
    if (ae_object->et == NULL) ae_object->et = blankspace;
    if (ae_object->api == NULL) ae_object->api = blankspace;
    if (ae_object->aei == NULL) ae_object->aei = blankspace;
    if (ae_object->lbl == NULL) ae_object->lbl = blankspace;
    if (ae_object->srv == NULL) ae_object->srv = blankspace;
    if (ae_object->acpi == NULL) ae_object->acpi = blankspace;
    if(ae_object->rr == false) strcpy(rr, "false");
    else strcpy(rr, "true");

    sql = malloc(sizeof(char) * 1024);

    sprintf(sql, "INSERT INTO general (rn, ri, pi, ct, et, lt, uri, acpi, lbl, ty) VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d);",
                ae_object->rn, ae_object->ri, ae_object->pi, ae_object->ct, ae_object->et, 
                ae_object->lt, "", ae_object->acpi, ae_object->lbl, ae_object->ty);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    sprintf(sql, "INSERT INTO ae (ri, pi, rr) VALUES('%s', '%s', '%s');",
                ae_object->ri, ae_object->pi, rr);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    if (ae_object->rn == blankspace) ae_object->rn = NULL;
    if (ae_object->pi == blankspace) ae_object->pi = NULL;
    if (ae_object->ty == '\0') ae_object->ty = 0;
    if (ae_object->ct == blankspace) ae_object->ct = NULL;
    if (ae_object->lt == blankspace) ae_object->lt = NULL;
    if (ae_object->et == blankspace) ae_object->et = NULL;
    if (ae_object->api == blankspace) ae_object->api = NULL;
    if (ae_object->aei == blankspace) ae_object->aei = NULL;
    if (ae_object->lbl == blankspace) ae_object->lbl = NULL;
    if (ae_object->srv == blankspace) ae_object->srv = NULL;
    if (ae_object->acpi == blankspace) ae_object->acpi = NULL;
    
    free(sql);
    return 1;

}

int db_store_cnt(RTNode *rtnode){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_cnt");
    char* blankspace = " ";
    char *sql = NULL, err_msg = NULL;
    int rc = 0;

    if(rtnode->ty != RT_CNT){
        return -1;
    }
    CNT * cnt_object = (CNT*) rtnode->obj;
    // if input == NULL
    if (cnt_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }
    if (cnt_object->rn == NULL) cnt_object->rn = blankspace;
    if (cnt_object->pi == NULL) cnt_object->pi = blankspace;
    if (cnt_object->ct == NULL) cnt_object->ct = blankspace;
    if (cnt_object->lt == NULL) cnt_object->lt = blankspace;
    if (cnt_object->et == NULL) cnt_object->et = blankspace;
    if (cnt_object->acpi == NULL) cnt_object->acpi = blankspace;
    if (cnt_object->lbl == NULL) cnt_object->lbl = blankspace;

    sql = malloc(sizeof(char) * 1024);

    sprintf(sql, "INSERT INTO general (rn, ri, pi, ct, et, lt, uri, acpi, lbl, ty) VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d);",
                cnt_object->rn, cnt_object->ri, cnt_object->pi, cnt_object->ct, cnt_object->et, 
                cnt_object->lt, rtnode->uri, cnt_object->acpi, cnt_object->lbl, cnt_object->ty);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    sprintf(sql, "INSERT INTO cnt (ri, mni, mbs, cni, cbs) VALUES('%s', %d, %d, %d, %d);",
                cnt_object->ri, cnt_object->mni, cnt_object->mbs, cnt_object->cni, cnt_object->cbs);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    if (cnt_object->rn == blankspace) cnt_object->rn = NULL;
    if (cnt_object->pi == blankspace) cnt_object->pi = NULL;
    if (cnt_object->ct == blankspace) cnt_object->ct = NULL;
    if (cnt_object->lt == blankspace) cnt_object->lt = NULL;
    if (cnt_object->et == blankspace) cnt_object->et = NULL;
    if (cnt_object->acpi == blankspace) cnt_object->acpi = NULL;
    if (cnt_object->lbl == blankspace) cnt_object->lbl = NULL;
    
    free(sql);
    return 1;
}

int db_store_cin(RTNode *rtnode) {
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_cin");
    if(rtnode->ty != RT_CIN){
        return -1;
    }
    CIN * cin_object = (CIN*) rtnode->obj;
    char *sql = NULL, err_msg = NULL;
    int rc = 0;
    
    // if input == NULL
    if (cin_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }
    if (cin_object->rn == NULL) cin_object->rn = " ";
    if (cin_object->pi == NULL) cin_object->pi = "NULL";
    if (cin_object->ty == '\0') cin_object->ty = 0;
    if (cin_object->ct == NULL) cin_object->ct = " ";
    if (cin_object->lt == NULL) cin_object->lt = " ";
    if (cin_object->et == NULL) cin_object->et = " ";

    if (cin_object->con == NULL) cin_object->con = " ";
    if (cin_object->cs == '\0') cin_object->cs = 0;
    if (cin_object->st == '\0') cin_object->st = 0;

    
    sql = malloc(sizeof(char) * 1024);

    sprintf(sql, "INSERT INTO general (rn, ri, pi, ct, et, lt, uri, ty) VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d);",
              cin_object->rn, cin_object->ri, cin_object->pi, cin_object->ct, cin_object->et, cin_object->lt, rtnode->uri, cin_object->ty );
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    sprintf(sql, "INSERT INTO cin (ri, pi, cs, con) VALUES('%s', '%s', %d, '%s');",
               cin_object->ri, cin_object->pi, cin_object->cs, cin_object->con);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }

    free(sql);
    return 1;
}

int db_store_grp(RTNode *rtnode){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_grp");
    char *bs = " ";
    char *sql = NULL, err_msg = NULL;
    char mid[1024] = {0};
    int rc = 0;

    if(rtnode->ty != RT_GRP){
        return -1;
    }
    GRP* grp_object = (GRP *) rtnode->obj;
    
    if(grp_object->rn == NULL) grp_object->rn = bs;
    if(grp_object->acpi == NULL) grp_object->acpi = bs;


    sql = malloc(sizeof(char) * 1024);

    sprintf(sql, "INSERT INTO general (rn, ri, pi, ct, et, lt, uri, ty) VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d);",
                grp_object->rn, grp_object->ri, grp_object->pi, grp_object->ct, grp_object->et, grp_object->lt, rtnode->uri, RT_GRP);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }
    // change mid to str
    char strbuf[128] = {0};
    if(grp_object->mid) {
        sprintf(strbuf, "%s", grp_object->mid[0]);
        strcat(mid, strbuf);
        for(int i = 1 ; i < grp_object->cnm; i++){
            if(grp_object->mid[i]){
                sprintf(strbuf, ",%s", grp_object->mid[i]);
                strcat(mid, strbuf);
            }else
                break;
        }
    }
    

    sprintf(sql, "INSERT INTO grp (ri, mt, cnm, mnm, mid, mtv, csy) VALUES('%s', %d, %d, %d, '%s', '%s', '%s', %d);",
               grp_object->ri, grp_object->mt, grp_object->cnm, grp_object->mnm, mid, grp_object->mtv, grp_object->csy);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed Insert SQL: %s, msg : %s", sql, err_msg);
        free(sql);
        return 0;
    }
    

    if(grp_object->rn == bs) grp_object->rn = NULL;
    if(grp_object->acpi == bs) grp_object->acpi = NULL;

    return 1;
}

int db_store_sub(RTNode* rtnode){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_cnt");
    char* blankspace = " ";
    char *sql = NULL, err_msg = NULL;
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

int db_store_acp(RTNode *rtnode) {
    logger("DB", LOG_LEVEL_DEBUG, "Call db_store_acp");
    char* blankspace = " ";
    char *sql = NULL, err_msg = NULL;
    int rc = 0;

    ACP *acp_object = (ACP *) rtnode->obj;

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
              acp_object->rn, acp_object->ri, acp_object->pi, acp_object->ct, acp_object->et, acp_object->lt, rtnode->uri, acp_object->ty );
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
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    
    cols = sqlite3_column_count(res);
    rc = sqlite3_step(res);
    logger("DB", LOG_LEVEL_DEBUG, "cols : %d, rc : %d", cols, rc);
    if(rc == SQLITE_ROW){
        new_cse = calloc(1, sizeof(CSE));
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if( (bytes = sqlite3_column_bytes(res, col)) == 0)
                continue;

            logger("DB", LOG_LEVEL_DEBUG, "colname : %s, bytes : %d", colname, bytes);
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
    }
    

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
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, ae WHERE general.ri='%s' and ae.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    new_ae = calloc(1, sizeof(AE));
    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW){
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                new_ae->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                new_ae->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                new_ae->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                new_ae->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                new_ae->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                new_ae->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "acpi") ){
                new_ae->acpi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "aei") ){
                new_ae->aei = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "api") ){
                new_ae->api = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lbl") ){
                new_ae->lbl = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "rr") ){
                if( strcmp(sqlite3_column_text(res, col), "true") )
                    new_ae->rr = true;
                else
                    new_ae->rr = false;
            }else if( !strcmp(colname, "srv") ){
                new_ae->srv = strdup(sqlite3_column_text(res, col));
            }
        }
    }
    new_ae->ty = RT_AE;

    sqlite3_finalize(res);
    return new_ae;   
}

CNT *db_get_cnt(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_cnt");

    //struct to return
    int rc = 0;
    int cols = 0;
    CNT* new_cnt = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, cnt WHERE general.ri='%s' and cnt.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    new_cnt = calloc(1, sizeof(CNT));
    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW){
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                new_cnt->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                new_cnt->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                new_cnt->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                new_cnt->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                new_cnt->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                new_cnt->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "acpi") ){
                new_cnt->acpi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lbl") ){
                new_cnt->lbl = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "cbs") ){
                new_cnt->cbs = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "cni") ){
                new_cnt->cni = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "mbs") ){
                new_cnt->mbs = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "mni") ){
                new_cnt->mni = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "st") ){
                new_cnt->st = sqlite3_column_int(res, col);
            }
        }
    }
    new_cnt->ty = RT_CNT;

    sqlite3_finalize(res);
    return new_cnt;   
}

CIN *db_get_cin(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_cin");

    //struct to return
    int rc = 0;
    int cols = 0;
    CIN* new_cin = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, cin WHERE general.ri='%s' and cin.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    new_cin = calloc(1, sizeof(CIN));
    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW){
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                new_cin->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                new_cin->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                new_cin->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                new_cin->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                new_cin->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                new_cin->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "con") ){
                new_cin->con = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "cs") ){
                new_cin->cs = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "st") ){
                new_cin->st = sqlite3_column_int(res, col);
            }
        }
    }
    new_cin->ty = RT_CIN;

    sqlite3_finalize(res);
    return new_cin;   
}

SUB *db_get_sub(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_sub");

    //struct to return
    int rc = 0;
    int cols = 0;
    SUB* new_sub = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, sub WHERE general.ri='%s' and sub.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    new_sub = calloc(1, sizeof(SUB));
    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW){
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                new_sub->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                new_sub->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                new_sub->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                new_sub->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                new_sub->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                new_sub->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "net") ){
                new_sub->net = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "nu") ){
                new_sub->nu = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "sur") ){
                new_sub->sur = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "nct") ){
                new_sub->nct = sqlite3_column_int(res, col);
            }
        }
    }
    new_sub->ty = RT_SUB;

    sqlite3_finalize(res);
    return new_sub;   
}

GRP *db_get_grp(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_grp");

    //struct to return
    int rc = 0;
    int cols = 0;
    GRP* new_grp = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, grp WHERE general.ri='%s' and grp.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    new_grp = calloc(1, sizeof(GRP));
    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW){
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                new_grp->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                new_grp->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                new_grp->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                new_grp->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                new_grp->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                new_grp->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "mid") ){
                new_grp->mid = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "macp") ){
                new_grp->macp = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "mt") ){
                new_grp->mt = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "cnm") ){
                new_grp->cnm = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "mnm") ){
                new_grp->mnm = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "mtv") ){
                new_grp->mtv = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "csy") ){
                new_grp->csy = sqlite3_column_int(res, col);
            }
        }
    }

    sqlite3_finalize(res);
    return new_grp;   
}

ACP *db_get_acp(char *ri){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_acp");

    //struct to return
    int rc = 0;
    int cols = 0;
    ACP* new_acp = NULL;
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, acp WHERE general.ri='%s' and acp.ri='%s';", ri, ri);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    new_acp = calloc(1, sizeof(ACP));
    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW){
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                new_acp->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                new_acp->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                new_acp->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                new_acp->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                new_acp->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                new_acp->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pvacop") ){
                new_acp->pv_acop = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pvacor") ){
                new_acp->pv_acor = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pvsacop") ){
                new_acp->pvs_acop = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pvsacor") ){
                new_acp->pvs_acor = strdup(sqlite3_column_text(res, col));
            }
        }
    }

    new_acp->ty = RT_ACP;
    sqlite3_finalize(res);
    return new_acp;   
}

int db_delete_onem2m_resource(char* ri) {
    logger("DB", LOG_LEVEL_DEBUG, "Delete [RI] %s",ri);
    char sql[1024] = {0};
    char *err_msg;
    int rc; 

    sprintf(sql, "DELETE FROM general WHERE ri='%s';", ri);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Cannot delete resource %s", err_msg);
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

    cols = sqlite3_column_count(res);
    logger("DB", LOG_LEVEL_DEBUG, "cols : %d", cols);
    rc = sqlite3_step(res);
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
    
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, ae WHERE general.ri = ae.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;

    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    while(rc == SQLITE_ROW){
        AE* ae = calloc(1, sizeof(AE));
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                ae->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                ae->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                ae->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                ae->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                ae->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                ae->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "acpi") ){
                ae->acpi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "aei") ){
                ae->aei = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "api") ){
                ae->api = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lbl") ){
                ae->lbl = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "rr") ){
                if(!strcmp(sqlite3_column_text(res, col), "true"))
                    ae->rr = true;
                else  
                    ae->rr = false;
            }else if( !strcmp(colname, "srv") ){
                ae->srv = strdup(sqlite3_column_text(res, col));
            }
        }
        ae->ty = RT_AE;
        if(!head) {
            head = create_rtnode(ae,RT_AE);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(ae, RT_AE);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_cnt_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_cnt_rtnode");
    int rc = 0;
    int cols = 0;
    
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, cnt WHERE general.ri = cnt.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;

    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    while(rc == SQLITE_ROW){
        CNT* cnt = calloc(1, sizeof(CNT));
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                cnt->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                cnt->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                cnt->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                cnt->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                cnt->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                cnt->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "acpi") ){
                cnt->acpi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lbl") ){
                cnt->lbl = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "cbs") ){
                cnt->cbs = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "cni") ){
                cnt->cni = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "mbs") ){
                cnt->mbs = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "mni") ){
                cnt->mni = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "st") ){
                cnt->st = sqlite3_column_int(res, col);
            }
            
        }
        cnt->ty = RT_CNT;
        if(!head) {
            head = create_rtnode(cnt,RT_CNT);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(cnt, RT_CNT);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_sub_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_sub_rtnode");
    int rc = 0;
    int cols = 0;
    
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, sub WHERE general.ri = sub.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;

    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    while(rc == SQLITE_ROW){
        SUB* sub = calloc(1, sizeof(SUB));
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                sub->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                sub->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                sub->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                sub->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                sub->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                sub->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "net") ){
                sub->net = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "nu") ){
                sub->nu = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "sur") ){
                sub->sur = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "nct") ){
                sub->nct = sqlite3_column_int(res, col);
            }
            
        }
        sub->ty = RT_SUB;
        if(!head) {
            head = create_rtnode(sub,RT_SUB);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(sub, RT_SUB);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_acp_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_acp_rtnode");
    int rc = 0;
    int cols = 0;
    
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, acp WHERE general.ri = acp.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;

    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    while(rc == SQLITE_ROW){
        ACP* acp = calloc(1, sizeof(ACP));
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                acp->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                acp->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                acp->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                acp->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                acp->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                acp->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pvacop") ){
                acp->pv_acop = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pvacor") ){
                acp->pv_acor = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pvsacop") ){
                acp->pvs_acop = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pvsacor") ){
                acp->pvs_acor = strdup(sqlite3_column_text(res, col));
            }
            
        }
        acp->ty = RT_SUB;
        if(!head) {
            head = create_rtnode(acp,RT_SUB);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(acp, RT_SUB);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode *db_get_all_grp_rtnode(){
    logger("DB", LOG_LEVEL_DEBUG, "Call db_get_all_grp_rtnode");
    int rc = 0;
    int cols = 0;
    
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, grp WHERE general.ri = grp.ri;");
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }

    RTNode* head = NULL, *rtnode = NULL;

    cols = sqlite3_column_count(res);

    rc = sqlite3_step(res);
    while(rc == SQLITE_ROW){
        GRP* grp = calloc(1, sizeof(GRP));
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                grp->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                grp->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                grp->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                grp->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                grp->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                grp->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "mid") ){
                grp->mid = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "macp") ){
                grp->macp = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "mt") ){
                grp->mt = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "cnm") ){
                grp->cnm = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "mnm") ){
                grp->mnm = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "mtv") ){
                grp->mtv = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "csy") ){
                grp->csy = sqlite3_column_int(res, col);
            }
            
        }
        
        if(!head) {
            head = create_rtnode(grp,RT_GRP);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(grp, RT_GRP);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}

RTNode* db_get_cin_rtnode_list(RTNode *parent_rtnode) {

    char buf[256] = {0};
    char* pi = get_ri_rtnode(parent_rtnode);
    int rc = 0;
    int cols = 0;
    
    sqlite3_stmt *res;
    char *colname = NULL;
    char sql[1024] = {0};

    sprintf(sql, "SELECT * FROM general, cin WHERE cin.pi='%s' AND general.ri=cin.ri ;",pi);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if(rc != SQLITE_OK){
        logger("DB", LOG_LEVEL_ERROR, "Failed select");
        return 0;
    }
    
        
    RTNode* head = NULL, *rtnode = NULL;

    rc = sqlite3_step(res);
    while(rc == SQLITE_ROW){
        CIN* cin = calloc(1, sizeof(CIN));
        for(int col = 0 ; col < cols; col++){
            colname = sqlite3_column_name(res, col);
            if(!strcmp(colname, "rn") ){
                cin->rn = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ri") ){
                cin->ri = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "pi") ){
                cin->pi = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "ct") ){
                cin->ct = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "lt") ){
                cin->lt = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "et") ){
                cin->et = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "con") ){
                cin->con = strdup(sqlite3_column_text(res, col));
            }else if( !strcmp(colname, "cs") ){
                cin->cs = sqlite3_column_int(res, col);
            }else if( !strcmp(colname, "st") ){
                cin->st = sqlite3_column_int(res, col);
            }
            
        }
        cin->ty = RT_CIN;
        if(!head) {
            head = create_rtnode(cin,RT_CIN);
            rtnode = head;
        } else {
            rtnode->sibling_right = create_rtnode(cin, RT_CIN);
            rtnode->sibling_right->sibling_left = rtnode;
            rtnode = rtnode->sibling_right;
        }   
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res); 
    return head;
}