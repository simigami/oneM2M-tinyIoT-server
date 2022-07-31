#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "onem2m.h"

#define CHUNK_SIZE 1024 // read 1024 bytes at a time

// Public directory settings
#define PUBLIC_DIR "./public"
#define INDEX_HTML "/index.html"
#define NOT_FOUND_HTML "/404.html"

RT *rt;

int main(int c, char **v) {
	init();
 	char *port = c == 1 ? "3000" : v[1];
	serve_forever(port);
  
	return 0;
}

int file_exists(const char *file_name) {
	struct stat buffer;
	int exists;

	exists = (stat(file_name, &buffer) == 0);

	return exists;
}

int read_file(const char *file_name) {
	char buf[CHUNK_SIZE];
	FILE *file;
	size_t nread;
	int err = 1;

	file = fopen(file_name, "r");

	if (file) {
		while ((nread = fread(buf, 1, sizeof buf, file)) > 0) {
			fwrite(buf, 1, nread, stdout);
		}

		err = ferror(file);
		fclose(file);
	}
	
	return err;
}

void route() {
	Node* pnode = Validate_URI(rt);
	if(!pnode) return;
	
	char *json_payload;
	
	if(payload_size > 0) {
		if(!(json_payload = Parse_Request_JSON())) {
			HTTP_500;
			return;
		}
	}

	Operation op = Parse_Operation();
	
	switch(op) {
	
	case o_CREATE:	
		Create_Object(json_payload, pnode); break;
	
	case o_RETRIEVE:
		Retrieve_Object(pnode);	break;
		
	case o_UPDATE: 
		//Update_Object(); break;
		
	case o_DELETE:
		Delete_Object(pnode); break;
	
	default:
		HTTP_500;
	}
}

void init() {
	CSE *cse;
	
	if(access("./CSE.db", 0) == -1) {
		cse = (CSE*)malloc(sizeof(CSE));
		Set_CSE(cse);
		Store_CSE(cse);
	} else {
		cse = Get_CSE("5-20220801T070241");
	}
	rt = (RT *)malloc(sizeof(rt));
 	rt->root = Create_Node(cse->ri, cse->rn, cse->pi, t_CSE);
 	Free_CSE(cse);
 	cse = NULL;
 	Restruct_ResourceTree();
}

void Create_Object(char *json_payload, Node *pnode) {
	ObjectType ty = Parse_ObjectType();

	switch(ty) {
		
	case t_AE :
		Create_AE(json_payload, pnode);
		break;	
					
	case t_CNT :
		Create_CNT(json_payload, pnode);
		break;
			
	case t_CIN :
		Create_CIN(json_payload, pnode);
		break;
	case t_CSE :
		/*No Definition such request*/
	}	
}

void Retrieve_Object(Node *pnode) {

	switch(pnode->ty) {
		
	case t_CSE :
		Retrieve_CSE(pnode);
		break;
	
	case t_AE : 
		Retrieve_AE(pnode);			
		break;	
			
	case t_CNT :
		Retrieve_CNT(pnode);			
		break;
			
	case t_CIN :
		Retrieve_CIN(pnode);			
		break;
	}	
}

void Create_AE(char *json_payload, Node *pnode) {
	AE* ae = JSON_to_AE(json_payload);
	Set_AE(ae,pnode->ri);
	
	int result = Store_AE(ae);
	if(result != 1) { 
		HTTP_500;
		printf("DB Store Fail\n");
		Free_AE(ae);
		ae = NULL;
		return;
	}
	
	Node* node = Create_Node(ae->ri, ae->rn, ae->pi, ae->ty);
	Add_child(pnode,node);
	
	char *resjson = AE_to_json(ae);
	HTTP_201;
	printf("%s",resjson);
	free(resjson);
	Free_AE(ae);
	resjson = NULL;
	ae = NULL;
}

void Create_CNT(char *json_payload, Node *pnode) {
	CNT* cnt = JSON_to_CNT(json_payload);
	Set_CNT(cnt,pnode->ri);
	
	int result = Store_CNT(cnt);
	if(result != 1) { 
		HTTP_500;
		printf("DB Store Fail\n");
		Free_CNT(cnt);
		cnt = NULL;
		return;
	}
	
	Node* node = Create_Node(cnt->ri, cnt->rn, cnt->pi, cnt->ty);
	Add_child(pnode,node);
	
	char *resjson = CNT_to_json(cnt);
	HTTP_201;
	printf("%s",resjson);
	free(resjson);
	Free_CNT(cnt);
	resjson = NULL;
	cnt = NULL;
}

void Create_CIN(char *json_payload, Node *pnode) {
	CIN* cin = JSON_to_CIN(json_payload);
	Set_CIN(cin,pnode->ri);
	
	int result = Store_CIN(cin);
	if(result != 1) { 
		HTTP_500;
		printf("DB Store Fail\n");
		Free_CIN(cin);
		cin = NULL;
		return;
	}
	
	Node* node = Create_Node(cin->ri, cin->rn, cin->pi, cin->ty);
	Add_child(pnode,node);
	
	char *resjson = CIN_to_json(cin);
	HTTP_201;
	printf("%s",resjson);
	free(resjson);
	Free_CIN(cin);
	resjson = NULL;
	cin = NULL;
}

void Retrieve_CSE(Node *pnode){
	CSE* gcse = Get_CSE(pnode->ri);
	char *resjson = CSE_to_json(gcse);
	HTTP_200;
	printf("%s",resjson);
	free(resjson);
	Free_CSE(gcse);
	resjson = NULL;
	gcse = NULL;
}

void Retrieve_AE(Node *pnode){
	AE* gae = Get_AE(pnode->ri);
	char *resjson = AE_to_json(gae);
	HTTP_200;
	printf("%s",resjson);
	free(resjson);
	Free_AE(gae);
	resjson = NULL;
	gae = NULL;
}

void Retrieve_CNT(Node *pnode){
	CNT* gcnt = Get_CNT(pnode->ri);
	char *resjson = CNT_to_json(gcnt);
	HTTP_200;
	printf("%s",resjson);
	free(resjson);
	Free_CNT(gcnt);
	resjson = NULL;
	gcnt = NULL;
}

void Retrieve_CIN(Node *pnode){
	CIN* gcin = Get_CIN(pnode->ri);
	char *resjson = CIN_to_json(gcin);
	HTTP_200;
	printf("%s",resjson);
	free(resjson);
	Free_CIN(gcin);
	resjson = NULL;
	gcin = NULL;
}

void Delete_Object(Node* pnode) {
	Delete_Node(pnode,1);
	pnode = NULL;
	HTTP_200;
	printf("Deleted");
}

void Restruct_ResourceTree(){
	Node *node_list = Create_Node("","","",0);
	Node *tail = node_list;
	
	if(access("./AE.db", 0) != -1) {
		Node* ae_list = Get_All_AE();
		tail->siblingRight = ae_list;
		ae_list->siblingLeft = tail;
		while(tail->siblingRight) tail = tail->siblingRight;
	} else {
		fprintf(stderr,"AE.db is not exist\n");
	}
	
	if(access("./CNT.db", 0) != -1) {
		Node* cnt_list = Get_All_CNT();
		tail->siblingRight = cnt_list;
		cnt_list->siblingLeft = tail;
		while(tail->siblingRight) tail = tail->siblingRight;
	} else {
		fprintf(stderr,"CNT.db is not exist\n");
	}
	
	if(access("./CIN.db", 0) != -1) {
		Node* cin_list = Get_All_CIN();
		tail->siblingRight = cin_list;
		cin_list->siblingLeft = tail;
		while(tail->siblingRight) tail = tail->siblingRight;
	} else {
		fprintf(stderr,"CIN.db is not exist\n");
	}
	
	Node *temp = node_list;
	node_list = node_list->siblingRight;
	if(node_list) node_list->siblingLeft = NULL;
	Free_Node(temp);
	
	if(node_list) Restruct_childs(rt->root, node_list);
}

Node* Restruct_childs(Node *pnode, Node *list) {
	Node *node = list;
	
	while(node) {
		Node *right = node->siblingRight;

		if(!strcmp(pnode->ri, node->pi)) {
			Node *left = node->siblingLeft;
			
			if(!left) {
				list = right;
			} else {
				left->siblingRight = right;
			}
			
			if(right) right->siblingLeft = left;
			node->siblingLeft = node->siblingRight = NULL;
			fprintf(stderr,"%s`s child : %s\n",pnode->rn, node->rn);
			Add_child(pnode, node);
		}
		node = right;
	}
	Node *child = pnode->child;
	
	while(child) {
		list = Restruct_childs(child, list);
		child = child->siblingRight;
	}
	
	return list;
}
