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
	rt = (RT *)malloc(sizeof(rt));
 	rt->root = Create_Node("1234", "TEST_CSE", t_CSE);
 	Restruct_ResourceTree();
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
	if(!pnode) {
		HTTP_500;
		printf("Invalid URI\n");
		return;
	}
	
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
		Create_Object(json_payload, pnode);
		break;
	
	case o_RETRIEVE:
		Retrieve_Object(pnode);
		break;
		
	case o_UPDATE: 
		//Update_Object();
		break;
		
	case o_DELETE:
		Delete_Object(pnode);
		break;
	
	default:
		HTTP_500;
	}
}

void Create_Object(char *json_payload, Node *pnode) {
	ObjectType ty = Parse_ObjectType();
		
	switch(ty) {
		
	case t_AE : 
		Create_AE(json_payload, pnode);
		break;	
					
	case t_CNT :
		//Create_CNT(json_payload, pnode);
		break;
			
	case t_CIN :
		//Create_CIN(json_payload, pnode);
		break;
	case t_CSE :
		/*No Definition such request*/
	}	
}

void Retrieve_Object(Node *pnode) {

	switch(pnode->ty) {
		
	case t_CSE :
		//Retrieve_CSE(pnode);
		break;
	
	case t_AE : 
		Retrieve_AE(pnode);			
		break;	
			
	case t_CNT :
		//Retrieve_CNT(pnode);			
		break;
			
	case t_CIN :
		//Retrieve_CIN(pnode);			
		break;
	}	
}

void Delete_Object(Node* pnode) {
	Delete_Node(pnode,1);
	HTTP_200;
	printf("Deleted");
}

void Create_AE(char *json_payload, Node *pnode) {
	AE* ae = JSON_to_AE(json_payload);
	Set_AE(ae,pnode->ri);
	
	int result = Store_AE(ae);
	if(result != 1) HTTP_500;

	char *resjson = AE_to_json(ae);
	
	Node* node = Create_Node(ae->ri, ae->rn, ae->ty);
	Add_child(pnode,node);
	
	HTTP_201;
	printf("%s",resjson);
}

void Retrieve_AE(Node *pnode){
	AE* gae = Get_AE(pnode->ri);
	char *resjson = AE_to_json(gae);
	HTTP_201;
	printf("%s",resjson);
}

void Restruct_ResourceTree(){
	
}

void Restruct_childs(Node *node, Node *list) {
	Node *cursor = list;
	
	while(cursor) {
		if(!strcmp(node->ri, cursor->ri)) {
			Node *left = cursor->siblingLeft;
			Node *right = cursor->siblingRight;
			if(!left) {
				list = list->siblingLeft;
			} else {
				left->siblingRight = right;
				if(right) right->siblingLeft = left;
			}
			
			cursor->siblingLeft = cursor->siblingRight = NULL;
			Add_child(node, cursor);
		}
		cursor = cursor->siblingRight;
	}
	
	Node *child = node->child;
	
	while(child) {
		Restruct_childs(child, list);
		child = child->siblingRight;
	}
}
