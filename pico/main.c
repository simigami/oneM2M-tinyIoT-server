#include "httpd.h"
#include "onem2m.h"
#include <sys/stat.h>

#define CHUNK_SIZE 1024 // read 1024 bytes at a time

// Public directory settings
#define PUBLIC_DIR "./public"
#define INDEX_HTML "/index.html"
#define NOT_FOUND_HTML "/404.html"

int main(int c, char **v) {
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

	Operation op;

	op = Parse_Operation();	
	
	if(op == o_CREATE) {
		ObjectType ty;
		ty = Parse_ObjectType();
		
		switch(ty) {
		
		case t_AE : 
			HTTP_201;
			AE* ae = Create_AE(j_payload);			
			Print_AE_json(ae);	
			break;	
					
		case t_CNT :
			HTTP_201;
			/*
			CNT* cnt = Create_CNT(j_payload);
			Print_CNT_json(cnt);
			*/
			break;
			
		case t_CIN :
			HTTP_201;
			/*
			CIN* cin = Create_CIN(j_payload);
			Print_CNT_json(cin);
			*/
			break;
			
		case t_CSE :
			/*No Definition such request*/
			break;
			
		default : 
			HTTP_500;
		}
	}
	else if(op == o_RETRIEVE) {
		/*
		switch(ty) {
		
		case t_AE : 
			HTTP_200;
			Retrieve_AE(j_payload);				
			break;	
					
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
		}
		*/
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
			Delete_AE(j_payload);				
			break;	
					
		case t_CNT :
			HTTP_200;
			Delete_CNT(j_payload);			
			break;
			
		case t_CIN :
			HTTP_200;		
			Delete_CIN(j_payload);
			break;
			
		case t_CSE :
			break;
			
		default : 
			HTTP_500;
		}
		*/
	}
}
