#include "httpd.h"
#include "onem2m.h"
#include "cJSON.h"
#include <db.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

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

int Store_CSE(CSE *cse_object)
{
    char* DATABASE = "CSE_DUP_2.db";

    DB* dbp;    // db handle
    DBC* dbcp;
    FILE* error_file_pointer;
    DBT key, data;  // storving key and real data
    int ret;        // template value

    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_csi, key_ty;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_csi, data_ty;  // storving key and real data

    char* program_name = "my_prog";

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

    memset(&data_rn, 0, sizeof(DBT));
    memset(&data_ri, 0, sizeof(DBT));
    memset(&data_pi, 0, sizeof(DBT));
    memset(&data_ty, 0, sizeof(DBT));

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

    dbcp->close(dbcp);
    dbp->close(dbp, 0); //DB close
    return 1;
}

/* Store Success -> return 1 */
int Store_AE(AE* ae_object) {
    char* DATABASE = "AE_DUP_2.db";
    DB* dbp;    // db handle
    DBC* dbcp;
    FILE* error_file_pointer;
    DBT key_ct, key_lt, key_rn, key_ri, key_pi, key_ty, key_et, key_api, key_rr,key_aei;
    DBT data_ct, data_lt, data_rn, data_ri, data_pi, data_ty, data_et, data_api, data_rr, data_aei;  // storving key and real data
    int ret;        // template value

    char* program_name = "my_prog";

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

AE* Get_AE(char* ri) {
    //store AE
    AE* new_ae = (AE*)malloc(sizeof(AE));

    char* database = "AE_DUP_2.db";

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
    // 몇번째 AE인지 찾기 위한 커서
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
                new_ae->ri = malloc(data.size);
                strcpy(new_ae->ri, data.data);
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
    //printf("[%d]\n",idx);

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
return new_ae;
}

void Set_AE(AE* ae) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	char year[5], mon[3], day[3], hour[3], minute[3], sec[3]; 
	
	sprintf(year,"%d", tm.tm_year+1900);
	sprintf(mon,"%02d",tm.tm_mon+1);
	sprintf(day,"%02d",tm.tm_mday);
	sprintf(hour,"%02d",tm.tm_hour);
	sprintf(minute,"%02d",tm.tm_min);
	sprintf(sec,"%02d",tm.tm_sec);
	
	char now[16] = "";
	
	strcat(now,year);
	strcat(now,mon);
	strcat(now,day);
	strcat(now,"T");
	strcat(now,hour);
	strcat(now,minute);
	strcat(now,sec);
	
	ae->ri = (char*)malloc(sizeof(now));
	ae->et = (char*)malloc(sizeof(now));
	ae->ct = (char*)malloc(sizeof(now));
	ae->lt = (char*)malloc(sizeof(now));
	ae->aei = (char*)malloc(sizeof(now));
	ae->pi = (char*)malloc(sizeof(now));
	strcpy(ae->ri, now);
	strcpy(ae->et, now);
	strcpy(ae->ct, now);
	strcpy(ae->lt, now);
	strcpy(ae->aei,now);
	strcpy(ae->pi, now);
	
	ae->ty = 2;
}

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
    strcpy(cse->rn, "sample_CSE");
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
    ae->rn = (char*)malloc(32*sizeof(char));
    
    strcpy(ae->rn, "sample_AE");
    strcpy(ae->ct, "20220513T083900");
    strcpy(ae->lt, "20220513T083900");
    strcpy(ae->et, "20240513T083900");
    strcpy(ae->api, "tinyProject");
    strcpy(ae->aei, "TAE");
    strcpy(ae->ri , ri);
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
    strcpy(cnt->rn, "sample_CNT");
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
    strcpy(cin->rn, "sample_CIN");
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
	
	fprintf(stderr,"uri : %s\n",ptr);
	
	ptr = strtok(ptr, "/");
	
	while(ptr != NULL && node) {
		
		while(node) {
			if(!strcmp(node->rn,ptr)) break;
			node = node->sibling;
		}
		
		ptr = strtok(NULL, "/");
	}
	
	free(ptr);
	
	fprintf(stderr,"%s\n", node->rn);
	
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

AE* JSON_to_AE(char *json_payload) {
	AE *ae = (AE *)malloc(sizeof(AE));

	cJSON *root = NULL;
	cJSON *api = NULL;
	cJSON *rr = NULL;
	cJSON *rn = NULL;

	cJSON* json = cJSON_Parse(json_payload);
	if (json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	root = cJSON_GetObjectItem(json, "m2m:ae");

	// api
	api = cJSON_GetObjectItem(root, "api");
	if (!cJSON_IsString(api) && (api->valuestring == NULL))
	{
		goto end;
	}
	ae->api = cJSON_Print(api);
	ae->api = strtok(ae->api, "\"");

	// rr
	rr = cJSON_GetObjectItemCaseSensitive(root, "rr");
	if (!cJSON_IsTrue(rr) && !cJSON_IsFalse(rr))
	{
		goto end;
	}
	else if (cJSON_IsTrue(rr))
	{
		ae->rr = true;
	}
	else if (cJSON_IsFalse(rr))
	{
		ae->rr = false;
	}
	
	// rn
	rn = cJSON_GetObjectItem(root, "rn");
	if (!cJSON_IsString(rn) && (rn->valuestring == NULL))
	{
		goto end;
	}
	ae->rn = cJSON_Print(rn);
	ae->rn = strtok(ae->rn, "\"");

end:
	cJSON_Delete(json);

	return ae;
}

char* AE_to_json(AE *ae_object) {
	char *json = NULL;

	cJSON *root = NULL;
	cJSON *ae = NULL;

	/* Our "ae" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "m2m:ae", ae = cJSON_CreateObject());
	cJSON_AddStringToObject(ae, "rn", ae_object->rn);
	cJSON_AddNumberToObject(ae, "ty", ae_object->ty);
	cJSON_AddStringToObject(ae, "pi", ae_object->pi);
	cJSON_AddStringToObject(ae, "ri", ae_object->ri);
	cJSON_AddStringToObject(ae, "ct", ae_object->ct);
	cJSON_AddStringToObject(ae, "lt", ae_object->lt);
	cJSON_AddStringToObject(ae, "et", ae_object->et);
	cJSON_AddStringToObject(ae, "api", ae_object->api);
	cJSON_AddBoolToObject(ae, "rr", ae_object->rr);
	cJSON_AddStringToObject(ae, "aei", ae_object->aei);

	json = cJSON_Print(root);

	cJSON_Delete(root);

	return json;
}

int display(char* database)
{
    printf("[Display] %s \n", database); //DB name print

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
            printf("%.*s : %d\n", (int)key.size, (char*)key.data, *(int*)data.data);
        }
        //bool
        else if (strncmp(key.data, "rr", key.size) == 0) {
            printf("%.*s : ", (int)key.size, (char*)key.data);
            if (*(bool*)data.data == true)
                printf("true\n");
            else
                printf("false\n");
        }

        //string
        else {
            printf("%.*s : %.*s\n",
                (int)key.size, (char*)key.data,
                (int)data.size, (char*)data.data);
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
return (0);
}


