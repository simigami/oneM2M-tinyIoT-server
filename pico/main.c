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
	ROUTE_START()

	POST("/") {
		HTTP_201;
		if (request_header("X-M2M-Origin") && request_header("X-M2M-RI")) {
			if (payload_size > 0) { 
				char json_payload[payload_size];
				char exjson[100] = "api : \"tiny_project\"\nrr : true\nrn : \"ae_test\"";
				int index = 0;
				for(int i=0; i<payload_size; i++) {
					if(payload[i] != 0 && payload[i] != 32 && payload[i] != 10) {
						json_payload[index++] = payload[i];
					}
				}
				json_payload[index] = '\0';
				AE temp = Create_AE(exjson);
				printf("%s %d", temp.api, temp.rr);
			}
		}
		else
			printf("request is not OneM2M Standard.");
	}

	GET("/") {
		HTTP_200;
		printf("CSE Discovery");
	}

	ROUTE_END()
}
