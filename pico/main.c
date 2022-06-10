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
		j_payload = json_payload();
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
		}
	}
}
