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
    while ((nread = fread(buf, 1, sizeof buf, file)) > 0)
      fwrite(buf, 1, nread, stdout);

    err = ferror(file);
    fclose(file);
  }
  return err;
}

void route() {
	char *j_payload;
	
	if(payload_size > 0) {
		if(!(j_payload = json_payload())) {
			HTTP_500;
			return;
		}
	}

	Operation op = Parse_Operation();
	ObjectType ty;
	
	if(op == o_CREATE) {
		ty = Parse_ObjectType();
		
		switch(ty) {
		
		case t_AE : 
			Node* pnode = Find_Node(rt);
			if(pnode) {
				AE* ae = JSON_to_AE(j_payload);
				Set_AE(ae);
				int result = Store_AE(ae);
				AE* gae = Get_AE(ae->ri);
				char *resjson = AE_to_json(gae);
				Node* node = Create_Node(ae->ri, ae->rn, t_AE);
				Add_child(pnode,node);
				HTTP_201;
				printf("%s",resjson);
			} else {
				HTTP_500;
				printf("Invalid URI\n");
			}
			
			break;	
					
		case t_CNT :
			HTTP_201;

			break;
			
		case t_CIN :
			HTTP_201;

			break;
			
		case t_CSE :
			/*No Definition such request*/
			break;
			
		default : 
			HTTP_500;
		}
	}
	else if(op == o_RETRIEVE) {
		Node* node = Find_Node(rt);
		if(node) {
			ty = node->ty;
		
			switch(ty) {
		
			case t_AE : 
				AE* gae = Get_AE(node->ri);
				char *resjson = AE_to_json(gae);
				HTTP_200;	
				printf("%s",resjson);			
				break;	
			/*			
			case t_CNT :
				HTTP_200;
				Retrieve_CNT(j_payload);			
				break;
				
			case t_CIN :
				HTTP_200;
				Retrieve_CIN(j_payload);			
				break;
				
			case t_CSE :
				break;
				
			default : 
				HTTP_500;
				*/
			} 
		} else {
			HTTP_500;
			printf("Invalid URI\n");
		}
	}
	else if(op == o_UPDATE) {
		/*
		switch(ty) {
		
		case t_AE : 
			HTTP_200;
			Update_AE(j_payload);				
			break;
						
		case t_CNT :
			HTTP_200;
			Update_CNT(j_payload);		
			break;
			
		case t_CIN :
			HTTP_200;
			Update_CIN(j_payload);			
			break;
						
		case t_CSE :
			break;
			
		default : 
			HTTP_500;
		}
		*/
	}
	else if(op == o_DELETE) {
		/*
		switch(ty) {
		
		case t_AE : 
			HTTP_200;
			Delete_AE();				
			break;	
					
		case t_CNT :
			HTTP_200;
			Delete_CNT();			
			break;
			
		case t_CIN :
			HTTP_200;		
			Delete_CIN();
			break;
			
		case t_CSE :
			break;
			
		default : 
			HTTP_500;
		}
		*/
	}
}
