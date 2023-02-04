#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <db.h>
#include "onem2m.h"
#include "berkeleyDB.h"
#include "logger.h"


/*DB CREATE*/
DB* DB_CREATE_(DB *dbp){
    int ret;

    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        fprintf(stderr, "File ERROR\n");
        return NULL;
    }
    return dbp;
}

/*DB Open*/
DB* DB_OPEN_(DB *dbp,char* DATABASE){
    int ret;

    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        fprintf(stderr, "DB Open ERROR\n");
        return NULL;
    }
    return dbp;
}

/*DB Get Cursor*/
DBC* DB_GET_CURSOR(DB *dbp, DBC *dbcp){
    int ret;
    
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        fprintf(stderr, "Cursor ERROR");
        return NULL;
    }
    return dbcp;
}

/*DB Display*/
int db_display(char* database)
{
    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int close_db, close_dbc, ret;

    close_db = close_dbc = 0;

    /* Open the database. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr,
            "%s: db_create: %s\n", database, db_strerror(ret));
        return -1;
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
        if (strncmp(key.data, "ty", key.size) == 0 ||
            strncmp(key.data, "st", key.size) == 0 ||
            strncmp(key.data, "cni", key.size) == 0 ||
            strncmp(key.data, "cbs", key.size) == 0 ||
            strncmp(key.data, "cs", key.size) == 0
            ){
            printf("%.*s : %d\n", (int)key.size, (char*)key.data, *(int*)data.data);
        }
        else if (strncmp(key.data, "rr", key.size) == 0) {
            printf("%.*s : ", (int)key.size, (char*)key.data);
            if (*(bool*)data.data == true)
                printf("true\n");
            else
                printf("false\n");
        }

        else {
            printf("%.*s : %.*s\n",
                (int)key.size, (char*)key.data,
                (int)data.size, (char*)data.data);
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        printf("Cursor ERROR\n");
        return -1;
    }


err:    if (close_dbc && (ret = dbcp->close(dbcp)) != 0)
dbp->err(dbp, ret, "DBcursor->close");
if (close_db && (ret = dbp->close(dbp, 0)) != 0)
fprintf(stderr,
    "%s: DB->close: %s\n", database, db_strerror(ret));
return -1;
}


int db_store_cse(CSE *cse_object) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_store_cse");
    char* DATABASE = "RESOURCE.db";

    DB* dbp;    // db handle
    DBC* dbcp;
    int ret;        // template value

    DBT key_ri;
    DBT data;  // storving key and real data


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

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);
    
    /* key and data must initialize */
    memset(&key_ri, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    /* initialize the data to be the first of two duplicate records. */
    key_ri.data = cse_object->ri;
    key_ri.size = strlen(cse_object->ri) + 1;

    /* List data excluding 'ri' as strings using delimiters. */
    char str[DB_STR_MAX]= "\0";
    sprintf(str, "%s;%s;%d;%s;%s;%s",
            cse_object->rn,cse_object->pi,cse_object->ty,cse_object->ct,cse_object->lt,cse_object->csi);

    data.data = str;
    data.size = strlen(str) + 1;

    /* input DB */
    if ((ret = dbcp->put(dbcp, &key_ri, &data, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    /* DB close */
    dbcp->close(dbcp);
    dbp->close(dbp, 0); 

    return 1;
}

int db_store_ae(AE *ae_object) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_store_ae");
    char* DATABASE = "RESOURCE.db";
    char* blankspace = " ";

    DB* dbp;    // db handle
    DBC* dbcp;
    int ret;        // template value

    DBT key_ri;
    DBT data;  // storving key and real data
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
    if(ae_object->rr == false) strcpy(rr,"false");
    else strcpy(rr,"true");

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);
    
    /* key and data must initialize */
    memset(&key_ri, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    /* initialize the data to be the first of two duplicate records. */
    key_ri.data = ae_object->ri;
    key_ri.size = strlen(ae_object->ri) + 1;

    /* List data excluding 'ri' as strings using delimiters. */
    char str[DB_STR_MAX]= "\0";
    sprintf(str, "%s;%s;%d;%s;%s;%s;%s;%s;%s;%s;%s",
            ae_object->rn,ae_object->pi,ae_object->ty,ae_object->ct,ae_object->lt,
            ae_object->et,ae_object->api,rr,ae_object->aei,ae_object->lbl,ae_object->srv);

    data.data = str;
    data.size = strlen(str) + 1;

    /* input DB */
    if ((ret = dbcp->put(dbcp, &key_ri, &data, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    /* DB close */
    dbcp->close(dbcp);
    dbp->close(dbp, 0);

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

    
    return 1;
}

int db_store_cnt(CNT *cnt_object) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_store_cnt");
    char* DATABASE = "RESOURCE.db";
    char* blankspace = " ";

    DB* dbp;    // db handle
    DBC* dbcp;
    int ret;        // template value

    DBT key_ri;
    DBT data;  // storving key and real data
    
    // if input == NULL
    if (cnt_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }
    if (cnt_object->rn == NULL) cnt_object->rn = blankspace;
    if (cnt_object->pi == NULL) cnt_object->pi = blankspace;
    if (cnt_object->ty == '\0') cnt_object->ty = 0;
    if (cnt_object->ct == NULL) cnt_object->ct = blankspace;
    if (cnt_object->lt == NULL) cnt_object->lt = blankspace;
    if (cnt_object->et == NULL) cnt_object->et = blankspace;
    
    if (cnt_object->acpi == NULL) cnt_object->acpi = blankspace;
    if (cnt_object->lbl == NULL) cnt_object->lbl = blankspace;
    if (cnt_object->cni == '\0') cnt_object->cni = 0;
    if (cnt_object->cbs == '\0') cnt_object->cbs = 0;
    if (cnt_object->st == '\0') cnt_object->st = 0;

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);
    
    /* key and data must initialize */
    memset(&key_ri, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    /* initialize the data to be the first of two duplicate records. */
    key_ri.data = cnt_object->ri;
    key_ri.size = strlen(cnt_object->ri) + 1;

    /* List data excluding 'ri' as strings using delimiters. */
    char str[DB_STR_MAX]= "\0";
    sprintf(str, "%s;%s;%d;%s;%s;%s;%s;%s;%d;%d;%d",
            cnt_object->rn,cnt_object->pi,cnt_object->ty,cnt_object->ct,cnt_object->lt,cnt_object->et,
            cnt_object->lbl,cnt_object->acpi,cnt_object->cbs,cnt_object->cni,cnt_object->st);

    data.data = str;
    data.size = strlen(str) + 1;

    /* input DB */
    if ((ret = dbcp->put(dbcp, &key_ri, &data, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    /* DB close */
    dbcp->close(dbcp);
    dbp->close(dbp, 0); 

    if (cnt_object->rn == blankspace) cnt_object->rn = NULL;
    if (cnt_object->pi == blankspace) cnt_object->pi = NULL;
    if (cnt_object->ty == 0) cnt_object->ty = '\0';
    if (cnt_object->ct == blankspace) cnt_object->ct = NULL;
    if (cnt_object->lt == blankspace) cnt_object->lt = NULL;
    if (cnt_object->et == blankspace) cnt_object->et = NULL;
    
    if (cnt_object->acpi == blankspace) cnt_object->acpi = NULL;
    if (cnt_object->lbl == blankspace) cnt_object->lbl = NULL;
    if (cnt_object->cni == 0) cnt_object->cni = '\0';
    if (cnt_object->cbs == 0) cnt_object->cbs = '\0';
    if (cnt_object->st == 0) cnt_object->st = '\0';

    
    return 1;
}

int db_store_cin(CIN *cin_object) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_store_cin");
    char* DATABASE = "RESOURCE.db";

    DB* dbp;    // db handle
    DBC* dbcp;
    int ret;        // template value

    DBT key_ri;
    DBT data;  // storving key and real data
    
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

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);
    
    /* key and data must initialize */
    memset(&key_ri, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    /* initialize the data to be the first of two duplicate records. */
    key_ri.data = cin_object->ri;
    key_ri.size = strlen(cin_object->ri) + 1;

    /* List data excluding 'ri' as strings using delimiters. */
    char str[DB_STR_MAX]= "\0";
    sprintf(str, "%s;%s;%d;%s;%s;%s;%s;%d;%d",
            cin_object->rn,cin_object->pi,cin_object->ty,cin_object->ct,cin_object->lt,cin_object->et,
            cin_object->con,cin_object->cs,cin_object->st);

    data.data = str;
    data.size = strlen(str) + 1;

    /* input DB */
    if ((ret = dbcp->put(dbcp, &key_ri, &data, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    /* DB close */
    dbcp->close(dbcp);
    dbp->close(dbp, 0); 
    
    return 1;
}


int db_store_sub(Sub *sub_object) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_store_sub");
    char* DATABASE = "SUB.db";

    DB* dbp;    // db handle
    DBC* dbcp;
    FILE* error_file_pointer;
    DBT key, data;  // storving key and real data
    int ret;        // template value

    DBT key_pi;
    DBT data_rn, data_net, data_nu, data_ri, data_ct,
     data_et, data_lt, data_ty, data_nct, data_sur;  // storving key and real data

    char* program_name = "my_prog";

    // if input == NULL
    if (sub_object->pi == NULL) {
        fprintf(stderr, "The key is NULL\n");
        return 0;
    }
    if (sub_object->rn == NULL) sub_object->rn = "";
    if (sub_object->ri == NULL) sub_object->ri = "";
    if (sub_object->nu == NULL) sub_object->nu = "";
    if (sub_object->net == NULL) sub_object->net = "0";
    if (sub_object->ct == NULL) sub_object->ct = "";
    if (sub_object->et == NULL) sub_object->et = "";
    if (sub_object->lt == NULL) sub_object->lt = "";
    if (sub_object->ty == '\0') sub_object->ty = 23;
    if (sub_object->nct == '\0') sub_object->nct = 1;
    if (sub_object->sur == NULL) sub_object->sur = "";    


    ret = db_create(&dbp, NULL, 0);
    if (ret) {
        fprintf(stderr, "db_create : %s\n", db_strerror(ret));
        fprintf(stderr, "File ERROR\n");
        return 0;
    }

    dbp->set_errfile(dbp, error_file_pointer);
    dbp->set_errpfx(dbp, program_name);

    /*Set duplicate*/
    ret = dbp->set_flags(dbp, DB_DUP);
    if (ret != 0) {
        dbp->err(dbp, ret, "Attempt to set DUPSORT flag failed.");
        fprintf(stderr, "Flag Set ERROR\n");
        dbp->close(dbp, 0);
        return(ret);
    }

    /*DB Open*/
    ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret) {
        dbp->err(dbp, ret, "%s", DATABASE);
        fprintf(stderr, "DB Open ERROR\n");
        return 0;
    }

    /*
  * The DB handle for a Btree database supporting duplicate data
  * items is the argument; acquire a cursor for the database.
  */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        fprintf(stderr, "Cursor ERROR");
        return 0;
    }

    /* keyand data must initialize */
    memset(&key_pi, 0, sizeof(DBT));

    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_nu, 0, sizeof(DBT));
    memset(&data_net, 0, sizeof(DBT));
    memset(&data_ct, 0, sizeof(DBT));
    memset(&data_et, 0, sizeof(DBT));
    memset(&data_lt, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));
    memset(&data_nct, 0, sizeof(DBT));
    memset(&data_sur, 0, sizeof(DBT));

    /* initialize the data to be the first of two duplicate records. */
    key_pi.data = sub_object->pi;
    key_pi.size = strlen(sub_object->pi) + 1;

    data_rn.data = sub_object->rn;
    data_rn.size = strlen(sub_object->rn) + 1;

    data_ri.data = sub_object->ri;
    data_ri.size = strlen(sub_object->ri) + 1;

    data_nu.data = sub_object->nu;
    data_nu.size = strlen(sub_object->nu) + 1;

    data_net.data = sub_object->net;
    data_net.size = strlen(sub_object->net) + 1;

    data_ct.data = sub_object->ct;
    data_ct.size = strlen(sub_object->ct) + 1;

    data_et.data = sub_object->et;
    data_et.size = strlen(sub_object->et) + 1;

    data_lt.data = sub_object->lt;
    data_lt.size = strlen(sub_object->lt) + 1;

    data_ty.data = &sub_object->ty;
    data_ty.size = sizeof(sub_object->ty) + 1;

    data_nct.data = &sub_object->nct;
    data_nct.size = sizeof(sub_object->nct) + 1;

    data_sur.data = sub_object->sur;
    data_sur.size = strlen(sub_object->sur) + 1;


    /* input DB */
    if ((ret = dbcp->put(dbcp, &key_pi, &data_ri, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_rn, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_nu, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_net, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_sur, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");


    if ((ret = dbcp->put(dbcp, &key_pi, &data_ct, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_et, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_lt, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_ty, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");
    if ((ret = dbcp->put(dbcp, &key_pi, &data_nct, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");



    /* DB close */
    dbcp->close(dbcp);
    dbp->close(dbp, 0); 
    
    if(!strcmp(sub_object->net, "0")) {
        sub_object->net = NULL;
    }
    return 1;
}

int db_store_acp(ACP *acp_object) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_store_acp");
    char* DATABASE = "ACP.db";

    DB* dbp;    // db handle
    DBC* dbcp;
    int ret;        // template value

    DBT key_ri;
    DBT data;  // storving key and real data


    // if input == NULL
    if (acp_object->ri == NULL) {
        fprintf(stderr, "ri is NULL\n");
        return -1;
    }
    if (acp_object->rn == NULL) acp_object->rn = " ";
    if (acp_object->pi == NULL) acp_object->pi = " ";
    if (acp_object->ty == '\0') acp_object->ty = 0;
    if (acp_object->ct == NULL) acp_object->ct = " ";
    if (acp_object->lt == NULL) acp_object->lt = " ";
    if (acp_object->et == NULL) acp_object->et = " ";

   if (acp_object->pv_acor == NULL) acp_object->pv_acor = " ";       
   if (acp_object->pv_acop == NULL) acp_object->pv_acop = " "; 
   if (acp_object->pvs_acop == NULL) acp_object->pvs_acor = " "; 
   if (acp_object->pvs_acop == NULL) acp_object->pvs_acop = " "; 
  

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* keyand data must initialize */
    memset(&key_ri, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    /* initialize the data to be the first of two duplicate records. */
    key_ri.data = acp_object->ri;
    key_ri.size = strlen(acp_object->ri) + 1;

    /* List data excluding 'ri' as strings using delimiters. */
    char str[DB_STR_MAX]= "\0";
    //strcat(str,acp_object->rn);
    sprintf(str, "%s;%s;%d;%s;%s;%s;%s;%s;%s;%s",
            acp_object->rn,acp_object->pi,acp_object->ty,acp_object->ct,acp_object->lt,
            acp_object->et,acp_object->pv_acor,acp_object->pv_acop,acp_object->pvs_acor,acp_object->pvs_acop);
            
    data.data = str;
    data.size = strlen(str) + 1;

    /* input DB */
    if ((ret = dbcp->put(dbcp, &key_ri, &data, DB_KEYLAST)) != 0)
        dbp->err(dbp, ret, "db->cursor");

    /* DB close */
    dbcp->close(dbcp);
    dbp->close(dbp, 0); 
    
    return 1;
}

CSE* db_get_cse(char* ri) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_get_cse");
    char* DATABASE = "RESOURCE.db";

    //struct to return
    CSE* new_cse= NULL;

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    int cnt = 0;
    int flag = 0;
    int idx = 0;
    
    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        cnt++;
        if (strncmp(key.data, ri, key.size) == 0) {
            new_cse= calloc(1,sizeof(CSE));
            flag=1;
            // ri = key
            new_cse->ri = malloc((key.size + 1)*sizeof(char));
            strcpy(new_cse->ri, key.data);

            char *ptr = strtok((char*)data.data,DB_SEP);  //split first string
            while (ptr != NULL) { // Split to end of next string

                switch (idx) {
                case 0:
                    if(strcmp(ptr," ")==0) new_cse->rn=NULL; //data is NULL
                    else{
                        new_cse->rn = malloc((strlen(ptr) + 1) *sizeof(char));
                        strcpy(new_cse->rn, ptr);
                    }
                    idx++;
                    break;
                case 1:
                    if(strcmp(ptr," ")==0) new_cse->pi=NULL; //data is NULL
                    else{
                        new_cse->pi = malloc((strlen(ptr)+1)*sizeof(char));
                        strcpy(new_cse->pi, ptr);
                    }
                    idx++;
                    break;
                case 2:
                    if(strcmp(ptr,"0")==0) new_cse->ty=0;
                    else {new_cse->ty = atoi(ptr);}

                    idx++;
                    break;
                case 3:
                    if(strcmp(ptr," ")==0) new_cse->ct=NULL; //data is NULL
                    else{
                        new_cse->ct = malloc((strlen(ptr)+1)*sizeof(char));
                        strcpy(new_cse->ct, ptr);
                    }
                    idx++;
                    break;
                case 4:
                    if(strcmp(ptr," ")==0) new_cse->lt=NULL;
                    else{
                    new_cse->lt = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cse->lt, ptr);
                    }
                    idx++;
                    break;                
                case 5:
                if(strcmp(ptr," ")==0) new_cse->csi=NULL;
                else{
                    new_cse->csi = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cse->csi, ptr);
                }
                    idx++;
                    break;             
                default:
                    idx=-1;
                }
                
                ptr = strtok(NULL, DB_SEP); //The delimiter is ,
            }
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }
    if (cnt == 0 || flag==0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "Data does not exist");
        return NULL;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    
    return new_cse;
}

AE* db_get_ae(char* ri) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_get_ae");
    char* DATABASE = "RESOURCE.db";

    //struct to return
    AE* new_ae = calloc(1,sizeof(AE));

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    int cnt = 0;
    int flag = 0;
    int idx = 0;
    
    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        cnt++;
        if (strncmp(key.data, ri, key.size) == 0) {
            flag=1;
            // ri = key
            new_ae->ri = malloc((key.size+1)*sizeof(char));
            strcpy(new_ae->ri, key.data);

            char *ptr = strtok((char*)data.data,DB_SEP);  //split first string
            while (ptr != NULL) { // Split to end of next string
                switch (idx) {
                case 0:
                if(strcmp(ptr," ")==0) new_ae->rn=NULL; //data is NULL
                else{
                    new_ae->rn = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->rn, ptr);
                }
                    idx++;
                    break;
                case 1:
                if(strcmp(ptr," ")==0) new_ae->pi=NULL; //data is NULL
                    else{
                    new_ae->pi = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->pi, ptr);
                    }
                    idx++;
                    break;
                case 2:
                if(strcmp(ptr,"0")==0) new_ae->ty=0;
                else new_ae->ty = atoi(ptr);

                    idx++;
                    break;
                case 3:
                if(strcmp(ptr," ")==0) new_ae->ct=NULL; //data is NULL
                else{
                    new_ae->ct = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->ct, ptr);
                }
                    idx++;
                    break;
                case 4:
                if(strcmp(ptr," ")==0) new_ae->lt=NULL; //data is NULL
                else{                
                    new_ae->lt = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->lt, ptr);
                }
                    idx++;
                    break;                
                case 5:
                if(strcmp(ptr," ")==0) new_ae->et=NULL; //data is NULL
                else{                
                    new_ae->et = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->et, ptr);
                }
                    idx++;
                    break;      
                case 6:
                if(strcmp(ptr," ")==0) new_ae->api=NULL; //data is NULL
                else{                
                    new_ae->api = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->api, ptr);
                }
                    idx++;
                    break;      
                case 7:
                    if(strcmp(ptr,"true")==0)
                        new_ae->rr = true;
                    else
                        new_ae->rr = false;

                    idx++;
                    break;      
                case 8:
                if(strcmp(ptr," ")==0) new_ae->aei=NULL; //data is NULL
                else{                
                    new_ae->aei = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->aei, ptr);
                }
                    idx++;
                    break; 
                case 9:
                if(strcmp(ptr," ")==0) new_ae->lbl=NULL; //data is NULL
                else{                
                    new_ae->lbl = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->lbl, ptr);
                }
                    idx++;
                    break;
                case 10:
                if(strcmp(ptr," ")==0) new_ae->srv=NULL; //data is NULL
                else{                
                    new_ae->srv = malloc((strlen(ptr) + 1) * sizeof(char));
                    strcpy(new_ae->srv, ptr);
                }            
                    idx++;
                    break;                                    
                default:
                    idx=-1;
                }
                
                ptr = strtok(NULL, DB_SEP); //The delimiter is ,
            }
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }
    if (cnt == 0 || flag==0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "Data does not exist");
        return NULL;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    
    return new_ae;
}

CNT* db_get_cnt(char* ri) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_get_cnt");
    char* DATABASE = "RESOURCE.db";

    //struct to return
    CNT* new_cnt = calloc(1,sizeof(CNT));

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    int cnt = 0;
    int flag = 0;
    int idx = 0;
    
    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        cnt++;
        if (strncmp(key.data, ri, key.size) == 0) {
            flag=1;
            // ri = key
            new_cnt->ri = malloc((key.size+1)*sizeof(char));
            strcpy(new_cnt->ri, key.data);

            char *ptr = strtok((char*)data.data,DB_SEP);  //split first string
            while (ptr != NULL) { // Split to end of next string
                switch (idx) {
                case 0:
                if(strcmp(ptr," ")==0) new_cnt->rn=NULL; //data is NULL
                    else{
                    new_cnt->rn = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cnt->rn, ptr);
                    }
                    idx++;
                    break;
                case 1:
                if(strcmp(ptr," ")==0) new_cnt->pi=NULL; //data is NULL
                    else{
                    new_cnt->pi = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cnt->pi, ptr);
                    }
                    idx++;
                    break;
                case 2:
                if(strcmp(ptr,"0")==0) new_cnt->ty=0;
                else new_cnt->ty = atoi(ptr);

                    idx++;
                    break;
                case 3:
                if(strcmp(ptr," ")==0) new_cnt->ct=NULL; //data is NULL
                    else{
                    new_cnt->ct = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cnt->ct, ptr);
                    }
                    idx++;
                    break;
                case 4:
                if(strcmp(ptr," ")==0) new_cnt->lt=NULL; //data is NULL
                    else{
                    new_cnt->lt = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cnt->lt, ptr);
                    }
                    idx++;
                    break;                
                case 5:
                if(strcmp(ptr," ")==0) new_cnt->et=NULL; //data is NULL
                    else{
                    new_cnt->et = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cnt->et, ptr);
                    }
                    idx++;
                    break;      
                case 6:
                if(strcmp(ptr," ")==0) new_cnt->lbl=NULL; //data is NULL
                    else{
                    new_cnt->lbl = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cnt->lbl, ptr);
                    }
                    idx++;
                    break;   
                case 7:
                if(strcmp(ptr," ")==0) new_cnt->acpi=NULL; //data is NULL
                    else{
                    new_cnt->acpi = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_cnt->acpi, ptr);
                    }
                    idx++;
                    break;                                           
                case 8:
                if(strcmp(ptr,"0")==0) new_cnt->cbs=0;
                else new_cnt->cbs = atoi(ptr);

                    idx++;
                    break;     
                case 9:
                if(strcmp(ptr,"0")==0) new_cnt->cni=0;
                else new_cnt->cni = atoi(ptr);

                    idx++;
                    break;    
                case 10:
                if(strcmp(ptr,"0")==0) new_cnt->st=0;
                else new_cnt->st = atoi(ptr);

                    idx++;
                    break;                                                                    
                default:
                    idx=-1;
                }
                
                ptr = strtok(NULL, DB_SEP); //The delimiter is ;
            }
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }
    if (cnt == 0 || flag==0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "Data does not exist");
        return NULL;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    

    

    return new_cnt;
}

CIN* db_get_cin(char* ri) {
    //fprintf(stderr,"[Get CIN] %s...", ri);
    char* DATABASE = "RESOURCE.db";

    //struct to return
    CIN* new_cin = NULL;

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    int cin = 0;
    int idx = 0;
    
    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        cin++;
        if (strncmp(key.data, ri, key.size) == 0) {
            new_cin = calloc(1,sizeof(CIN));
            // ri = key
            new_cin->ri = malloc((key.size+1)*sizeof(char));
            strcpy(new_cin->ri, key.data);

            char *ptr = strtok((char*)data.data, DB_SEP);  //split first string
            while (ptr != NULL) { // Split to end of next string
                switch (idx) {
                case 0:
                if(strcmp(ptr," ")==0) new_cin->rn=NULL; //data is NULL
                    else{
                    new_cin->rn = malloc(strlen(ptr)*sizeof(char));
                    strcpy(new_cin->rn, ptr);
                    }
                    idx++;
                    break;
                case 1:
                if(strcmp(ptr," ")==0) new_cin->pi=NULL; //data is NULL
                    else{
                    new_cin->pi = malloc(strlen(ptr)*sizeof(char));
                    strcpy(new_cin->pi, ptr);
                    }
                    idx++;
                    break;
                case 2:
                if(strcmp(ptr,"0")==0) new_cin->ty=0;
                    else
                    new_cin->ty = atoi(ptr);

                    idx++;
                    break;
                case 3:
                if(strcmp(ptr," ")==0) new_cin->ct=NULL; //data is NULL
                    else{
                    new_cin->ct = malloc(strlen(ptr)*sizeof(char));
                    strcpy(new_cin->ct, ptr);
                    }
                    idx++;
                    break;
                case 4:
                    new_cin->lt = malloc(strlen(ptr)*sizeof(char));
                    strcpy(new_cin->lt, ptr);

                    idx++;
                    break;                
                case 5:
                if(strcmp(ptr," ")==0) new_cin->et=NULL; //data is NULL
                    else{
                    new_cin->et = malloc(strlen(ptr)*sizeof(char));
                    strcpy(new_cin->et, ptr);
                    }
                    idx++;
                    break;      
                case 6:
                if(strcmp(ptr," ")==0) new_cin->con=NULL; //data is NULL
                    else{
                    new_cin->con = malloc(strlen(ptr)*sizeof(char));
                    strcpy(new_cin->con, ptr);
                    }
                    idx++;
                    break;       
                case 7:
                if(strcmp(ptr,"0")==0) new_cin->cs=0;
                    else
                    new_cin->cs = atoi(ptr);

                    idx++;
                    break;            
                case 8:
                if(strcmp(ptr,"0")==0) new_cin->st=0;
                    else
                    new_cin->st = atoi(ptr);

                    idx++;
                    break;                                                                              
                default:
                    idx=-1;
                }
                
                ptr = strtok(NULL, DB_SEP); //The delimiter is ;
            }
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        exit(0);
    }
    if (cin == 0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "Data does not exist");
        return NULL;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    //
    return new_cin;
}

Sub* db_get_sub(char* ri) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_get_sub");
    char* database = "SUB.db";

    //store AE
    Sub* new_sub = (Sub*)calloc(1, sizeof(Sub));

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
    int flag = 0;
    int struct_size = 10;

    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        cnt++;
        if (strncmp(data.data, ri, data.size) == 0) {
            flag=1;
            break;
        }
    }
    if (cnt == 0 || flag==0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "Data does not exist");
        return NULL;
        //exit(1);
    }
    
    new_sub->pi = malloc(data.size);
    strcpy(new_sub->pi, key.data);

    int idx = -1;
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(data.data, ri, data.size) == 0) {
            idx=0;
        }
        switch (idx) {
            case 0:
                new_sub->ri = malloc(data.size);
                strcpy(new_sub->ri, data.data);

                idx++;
                break;
            case 1:
                new_sub->rn = malloc(data.size);
                strcpy(new_sub->rn, data.data);

                idx++;
                break;
            case 2:
                new_sub->nu = malloc(data.size);
                strcpy(new_sub->nu, data.data);

                idx++;
                break;
            case 3:
                new_sub->net = malloc(data.size);
                strcpy(new_sub->net, data.data);

                idx++;
                break;
            case 4:
                new_sub->sur = malloc(data.size);
                strcpy(new_sub->sur, data.data);

                idx++;
                break;
            case 5:
                new_sub->ct = malloc(data.size);
                strcpy(new_sub->ct, data.data);

                idx++;
                break;
            case 6:
                new_sub->et = malloc(data.size);
                strcpy(new_sub->et, data.data);

                idx++;
                break;
            case 7:
                new_sub->lt = malloc(data.size);
                strcpy(new_sub->lt, data.data);

                idx++;
                break;
            case 8:
                new_sub->ty = *(int*)data.data;

                idx++;
                break;
            case 9:
                new_sub->nct = *(int*)data.data;

                idx++;
                break;
            default:
                idx=-1;
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
    
    if(!strcmp(new_sub->net, "0")) {
        new_sub->net = NULL;
    }
    return new_sub;
}

ACP* db_get_acp(char* ri) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_get_acp");
    char* DATABASE = "ACP.db";

    //struct to return
    ACP* new_acp = calloc(1,sizeof(ACP));

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    int cnt = 0;
    int flag = 0;
    int idx = 0;
    
    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        cnt++;
        if (strncmp(key.data, ri, key.size) == 0) {
            flag=1;
            // ri = key
            new_acp->ri =malloc((key.size+1)*sizeof(char));
            strcpy(new_acp->ri, key.data);

            char *ptr = strtok((char*)data.data, DB_SEP);  //split first string
            while (ptr != NULL) { // Split to end of next string
                switch (idx) {
                case 0:if(strcmp(ptr," ")==0) new_acp->rn=NULL; //data is NULL
                else{
                    new_acp->rn = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->rn, ptr);
                    }
                    idx++;
                    break;
                case 1:
                if(strcmp(ptr," ")==0) new_acp->pi=NULL; //data is NULL
                else{
                    new_acp->pi = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->pi, ptr);
                    }
                    idx++;
                    break;
                case 2:
                if(strcmp(ptr,"0")==0) new_acp->ty=0;
                else new_acp->ty = atoi(ptr);

                    idx++;
                    break;
                case 3:
                if(strcmp(ptr," ")==0) new_acp->ct=NULL; //data is NULL
                else{
                    new_acp->ct = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->ct, ptr);
                    }
                    idx++;
                    break;
                case 4:
                if(strcmp(ptr," ")==0) new_acp->lt=NULL; //data is NULL
                else{
                    new_acp->lt = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->lt, ptr);
                    }
                    idx++;
                    break;                
                case 5:
                if(strcmp(ptr," ")==0) new_acp->et=NULL; //data is NULL
                else{
                    new_acp->et = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->et, ptr);
                }
                    idx++;
                    break;
                case 6:
                if(strcmp(ptr," ")==0) new_acp->pv_acor=NULL; //data is NULL
                else{
                    new_acp->pv_acor = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->pv_acor, ptr);
                }
                    idx++;
                    break;
                case 7:
                if(strcmp(ptr," ")==0) new_acp->pv_acop=NULL; //data is NULL
                else{
                    new_acp->pv_acop = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->pv_acop, ptr);
                }
                    idx++;
                    break;
                case 8:
                if(strcmp(ptr," ")==0) new_acp->pvs_acor=NULL; //data is NULL
                else{
                    new_acp->pvs_acor = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->pvs_acor, ptr);
                }
                    idx++;
                    break;
                case 9:
                if(strcmp(ptr," ")==0) new_acp->pvs_acop=NULL; //data is NULL
                else{
                    new_acp->pvs_acop = malloc((strlen(ptr)+1)*sizeof(char));
                    strcpy(new_acp->pvs_acop, ptr);
                }
                    idx++;
                    break;                
                default:
                    idx=-1;
                }
                
                ptr = strtok(NULL, DB_SEP); //The delimiter is ;
            }
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }
    if (cnt == 0 || flag==0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "Data does not exist");
        return NULL;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    

    return new_acp;
}

int db_delete_onem2m_resource(char* ri) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_delete_onem2m_resource");
    char* DATABASE = "RESOURCE.db";
    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;
    int flag = 0;

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, ri, key.size) == 0) {
            flag = 1;
            dbcp->del(dbcp, 0);
            break;
        }
    }
    if (flag == 0) {
        fprintf(stderr, "Not Found\n");
        return -1;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    
    
    /* Delete Success */
    return 1;
}

int db_delete_sub(char* ri) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_delete_sub");
    char* database = "SUB.db";

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
        return 0;
        exit(1);
    }

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        return 0;
        exit(1);
    }

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int cnt = 0;
    int flag = 0;
    int struct_size = 10;

    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        return 0;
    }
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        cnt++;
        if (strncmp(data.data, ri, data.size) == 0) {
            flag=1;
            break;
        }
    }
    if (cnt == 0 || flag==0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "Data does not exist");
        return 0;
    }

    int idx = -1;
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(data.data, ri, data.size) == 0) {
            idx=0;
        }
        if(idx!=-1 && idx < struct_size){
            dbcp->del(dbcp, 0);
            idx++;
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return 0;
    }

    /* Cursors must be closed */
    if (dbcp0 != NULL)
        dbcp0->close(dbcp0);
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    
    
    /* Delete Success */
    return 1;
}

int db_delete_acp(char* ri) {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_delete_acp");
    char* DATABASE = "ACP.db";
    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;
    int flag = 0;

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, ri, key.size) == 0) {
            flag = 1;
            dbcp->del(dbcp, 0);
            break;
        }
    }
    if (flag == 0) {
        fprintf(stderr, "Not Found\n");
        return -1;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);
    
    /* Delete Success */
    
    return 1;
}

RTNode* db_get_all_cse() {
    fprintf(stderr,"\x1b[92m[Get All CSE]\x1b[0m\n");
    char* DATABASE = "RESOURCE.db";
    const char* TYPE = "5-";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int cse = 0;
    DBC* dbcp0;
    dbcp0 = DB_GET_CURSOR(dbp,dbcp0);
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, TYPE , 2) == 0) 
            cse++;
    }
    //fprintf(stderr, "<%d>\n",cse);

    if (cse == 0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "CSE does not exist");
        return NULL;
    }

    RTNode* head = NULL, *rtnode = NULL;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, TYPE , 2) == 0){
            CSE* cse = db_get_cse((char*)key.data);
            if(!head) {
                head = create_rtnode(ae,TY_CSE);
                rtnode = head;
            } else {
                rtnode->sibling_right = create_rtnode(cse,TY_CSE);
                rtnode = rtnode->sibling_right;
            }   
            free(cse);
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);    
    return head;
}

RTNode* db_get_all_ae() {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_get_all_ae");
    char* DATABASE = "RESOURCE.db";
    const const char* TYPE = "C";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int cnt = 0;
    DBC* dbcp0;
    dbcp0 = DB_GET_CURSOR(dbp,dbcp0);
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, TYPE , 1) == 0) 
            cnt++;
    }
    //fprintf(stderr, "<%d>\n",cnt);

    if (cnt == 0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "AE does not exist");
        return NULL;
    }

    RTNode* head = NULL, *rtnode = NULL;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, TYPE , 1) == 0){
            AE* ae = db_get_ae((char*)key.data);

            if(!head) {
                head = create_rtnode(ae,TY_AE);
                rtnode = head;
            } else {
                rtnode->sibling_right = create_rtnode(ae,TY_AE);
                rtnode = rtnode->sibling_right;
            }      
            free(ae);
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);    
    return head;
}

RTNode* db_get_all_cnt() {
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call db_get_all_cnt");
    char* DATABASE = "RESOURCE.db";
    const char* TYPE = "3-";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int cnt = 0;
    DBC* dbcp0;
    dbcp0 = DB_GET_CURSOR(dbp,dbcp0);
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, TYPE , 2) == 0) 
            cnt++;
    }
    //fprintf(stderr, "<%d>\n",cnt);

    if (cnt == 0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "CNT does not exist");
        return NULL;
    }

    RTNode* head = NULL, *rtnode = NULL;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, TYPE , 2) == 0){
            CNT* cnt_ = db_get_cnt((char*)key.data);
            if(!head) {
                head = create_rtnode(cnt_,TY_CNT);
                rtnode = head;
            } else {
                rtnode->sibling_right = create_rtnode(cnt_,TY_CNT);
                rtnode = rtnode->sibling_right;
            }     
            free(cnt_);
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);    
    return head;
}

RTNode* db_get_all_sub(){
    logger("ONEM2M", LOG_LEVEL_DEBUG, "Call get_all_sub");
    char* database = "SUB.db";

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
    int idx = 0;
    int cnt_sub = 0;

    DBC* dbcp0;
    if ((ret = dbp->cursor(dbp, NULL, &dbcp0, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        exit(1);
    }
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        cnt++;
    }
    if (cnt == 0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "SUB does not exist");
        return NULL;
    }

    int struct_size = 10;
    cnt = cnt / struct_size;
    char* tmp;


    RTNode* head = (RTNode*)calloc(cnt, sizeof(RTNode));
    RTNode* node;
    node = head;
    //node_ri = node_pi = node_rn = node_nu = node_sub_bit = head;
    
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        switch (idx) {
            case 0:
                node->ty = TY_SUB;
                node->ri = malloc(data.size);
                strcpy(node->ri, data.data);

                node->sibling_right = (RTNode*)calloc(1,sizeof(RTNode));
                node->sibling_right->sibling_left = node;

                idx++;
                break;
            case 1:
                node->rn = malloc(data.size);
                strcpy(node->rn, data.data);

                idx++;
                break;
            case 2:
                node->nu = malloc(data.size);
                strcpy(node->nu, data.data);

                idx++;
                break;
            case 3:
                //tmp = malloc(data.size);
                //strcpy(tmp, data.data);
                node->net = net_to_bit(data.data);

                //node->net = malloc(data.size);
                //strcpy(node->net, data.data);

                idx++;
                break;
            case 4:
                node->sur = malloc(data.size);
                strcpy(node->sur, data.data);

                idx++;
                break;
            case 5:
                node->pi = malloc(key.size);
                strcpy(node->pi, key.data);

                node = node->sibling_right;
                idx++;
                break;
            default:
                idx++;
                if (idx == struct_size) idx = 0;
        }
    }

    node->sibling_left->sibling_right = NULL;
    free(node);
    node = NULL;

    /* DB close */
    dbcp->close(dbcp0);
    dbcp->close(dbcp);
    dbp->close(dbp, 0);
    fprintf(stderr,"\n");
    return head;
}

RTNode* db_get_all_acp() {
    fprintf(stderr,"\x1b[92m[Get All ACP]\x1b[0m\n");
    char* DATABASE = "ACP.db";
    const char* TYPE = "1-";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int acp = 0;
    DBC* dbcp0;
    dbcp0 = DB_GET_CURSOR(dbp,dbcp0);
    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, TYPE , 2) == 0) 
            acp++;
    }
    //fprintf(stderr, "<%d>\n",acp);

    if (acp == 0) {
        logger("ONEM2M", LOG_LEVEL_DEBUG, "ACP does not exist");
        return NULL;
    }

    RTNode* head = calloc(1,sizeof(RTNode));
    RTNode* node;
    node = head;

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        if (strncmp(key.data, TYPE , 2) == 0){
            ACP* acp = db_get_acp((char*)key.data);
            node->ri = malloc((strlen(acp->ri)+1) * sizeof(char));
            node->rn = malloc((strlen(acp->rn)+1) * sizeof(char));
            node->pi = malloc((strlen(acp->pi)+1) * sizeof(char));
            if(acp->pv_acor && acp->pv_acop) { 
                node->pv_acor = malloc((strlen(acp->pv_acor)+1) * sizeof(char));
                node->pv_acop = malloc((strlen(acp->pv_acop)+1) * sizeof(char));
                strcpy(node->pv_acor,acp->pv_acor);
                strcpy(node->pv_acop,acp->pv_acop);
            }
            if(acp->pvs_acor && acp->pvs_acop) {
                node->pvs_acor = malloc((strlen(acp->pvs_acor)+1) *sizeof(char));
                node->pvs_acop = malloc(strlen((acp->pvs_acop)+1) *sizeof(char));
                strcpy(node->pvs_acor,acp->pvs_acor);
                strcpy(node->pvs_acop,acp->pvs_acop);
            }

            strcpy(node->ri,acp->ri);
            strcpy(node->rn,acp->rn);
            strcpy(node->pi,acp->pi);
            node->ty = acp->ty;

            node->sibling_right=calloc(1,sizeof(RTNode));            
            node->sibling_right->sibling_left = node;
            node = node->sibling_right;
            free(acp);
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }

    node->sibling_left->sibling_right = NULL;
    free(node->ri);
    free(node->rn);
    free(node->pi);
    free(node);
    node = NULL;

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbp != NULL)
        dbp->close(dbp, 0);    
    fprintf(stderr,"\n");
    return head;
}

RTNode* db_get_cin_list_by_pi(char* pi) {
    char* DATABASE = "RESOURCE.db";
    const char* TYPE = "4-";

    DB* dbp;
    DBC* dbcp;
    DBT key, data;
    int ret;

    dbp = DB_CREATE_(dbp);
    dbp = DB_OPEN_(dbp,DATABASE);
    dbcp = DB_GET_CURSOR(dbp,dbcp);

    /* Initialize the key/data return pair. */
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    int cnt = 0;
    int idx = 0;
    int* arr = NULL;

    DBC* dbcp0;
    dbcp0 = DB_GET_CURSOR(dbp,dbcp0);

    while ((ret = dbcp0->get(dbcp0, &key, &data, DB_NEXT)) == 0) {
        // find CIN
        if (strncmp(key.data, TYPE, 2) == 0) {
            CIN *cin = db_get_cin((char*)key.data);
            //find pi
            if(strncmp(pi, cin->pi, strlen(pi)) == 0)
                cnt++;
            free(cin);
        }
    }
    //fprintf(stderr, "<%d>\n",cnt);

    if (cnt == 0) {
        //logger("ONEM2M", LOG_LEVEL_DEBUG, "Data does not exist");
        return NULL;
    }
    RTNode* head = calloc(1,sizeof(RTNode));
    RTNode* node;
    node = head;
    
    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
        //find CIN
        if (strncmp(key.data, TYPE , 2) == 0){
            CIN *cin = db_get_cin((char*)key.data);
            //find pi
            if(strncmp(pi, cin->pi, strlen(pi)) == 0){
                node->ri = malloc((strlen(cin->ri)+1)*sizeof(char));
                node->rn = malloc((strlen(cin->rn)+1)*sizeof(char));
                node->pi = malloc((strlen(cin->pi)+1)*sizeof(char));

                strcpy(node->ri,cin->ri);
                strcpy(node->rn,cin->rn);
                strcpy(node->pi,cin->pi);
                node->ty = cin->ty;

                node->sibling_right=calloc(1,sizeof(RTNode));            
                node->sibling_right->sibling_left = node;
                node = node->sibling_right;
            }
            free(cin);
        }
    }
    if (ret != DB_NOTFOUND) {
        dbp->err(dbp, ret, "DBcursor->get");
        fprintf(stderr, "Cursor ERROR\n");
        return NULL;
    }    

    if(node->sibling_left) node->sibling_left->sibling_right = NULL;
    free_rtnode(node); 
    node = NULL;

    /* Cursors must be closed */
    if (dbcp != NULL)
        dbcp->close(dbcp);
    if (dbcp != NULL)
        dbcp->close(dbcp0);          
    if (dbp != NULL)
        dbp->close(dbp, 0); 

    return head;
}