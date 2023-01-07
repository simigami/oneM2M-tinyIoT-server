#include "httpd.h"
#include "config.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define MAX_CONNECTIONS 1024
#define BUF_SIZE 65535
#define QUEUE_SIZE 1000000

pthread_mutex_t mutex_lock;
int listenfd;
int *clients;
static void start_server(const char *);
static void respond(int);

static char *buf[MAX_CONNECTIONS];
extern char *response_json;
char *response_headers;

// Client request
char *method, // "GET" or "POST"
    *uri,     // "/index.html" things before '?'
    *qs,      // "a=1&b=2" things after  '?'
    *prot,    // "HTTP/1.1"
    *payload; // for POST

int payload_size;

void *respond_thread(void *ps) {
  int *slot = (int*)ps;
	respond(*slot);
	close(clients[*slot]);
	clients[*slot] = -1;
	return NULL;
}

void serve_forever(const char *PORT) {
  pthread_mutex_init(&mutex_lock, NULL);
  struct sockaddr_in clientaddr;
  socklen_t addrlen;
  
  int slot = 0; 

  fprintf(stderr,"Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", PORT, "\033[0m");

  // create shared memory for client slot array
  clients = mmap(NULL, sizeof(*clients) * MAX_CONNECTIONS,
                 PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  // Setting all elements to -1: signifies there is no client connected
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++)
    clients[i] = -1;
  start_server(PORT);

  // Ignore SIGCHLD to avoid zombie threads
  signal(SIGCHLD, SIG_IGN);

  // ACCEPT connections
  while (1) {
    clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

    int flag = fcntl(clients[slot], F_GETFL, O_NONBLOCK);

    if (clients[slot] < 0) {
      perror("accept() error");
      exit(1);
    } else {
      pthread_t thread_id;
      int s = slot;
      int *ps = &s; 
      pthread_create(&thread_id, NULL, respond_thread, (void*)ps);
      if(MONO_THREAD) pthread_join(thread_id, NULL);
    }
    while (clients[slot] != -1)
      slot = (slot + 1) % MAX_CONNECTIONS;
  }
}

// start server
void start_server(const char *port) {
  struct addrinfo hints, *res, *p;

  // getaddrinfo for host
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, port, &hints, &res) != 0) {
    perror("getaddrinfo() error");
    exit(1);
  }
  // socket and 
  for (p = res; p != NULL; p = p->ai_next) {
    int option = 1;
    listenfd = socket(p->ai_family, p->ai_socktype, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (listenfd == -1)
      continue;
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
      break;
  }
  if (p == NULL) {
    perror("socket() or bind()");
    exit(1);
  }

  freeaddrinfo(res);

  // listen for incoming connections
  if (listen(listenfd, QUEUE_SIZE) != 0) {
    perror("listen() error");
    exit(1);
  }
}

// get request header by name
char *request_header(const char *name) {
  header_t *h = reqhdr;
  while (h->name) {
    if (strcmp(h->name, name) == 0)
      return h->value;
    h++;
  }
  return NULL;
}

// get all request headers
header_t *request_headers(void) { return reqhdr; }

// Handle escape characters (%xx)
static void uri_unescape(char *uri) {
  char chr = 0;
  char *src = uri;
  char *dst = uri;
  
  // Skip initial non encoded character
  while (*src && !isspace((int)(*src)) && (*src != '%')) 
    src++;

  // Replace encoded characters with corresponding code.
  dst = src;
  while (*src && !isspace((int)(*src))) {
    if (*src == '+')
      chr = ' ';
    else if ((*src == '%') && src[1] && src[2]) {
      src++;
      chr = ((*src & 0x0F) + 9 * (*src > '9')) * 16;
      src++;
      chr += ((*src & 0x0F) + 9 * (*src > '9'));
    } else
      chr = *src;
    *dst++ = chr;
    src++;
  }

  *dst = '\0';
}

// client connection
void respond(int slot) {
  int rcvd;
  
  buf[slot] = malloc(BUF_SIZE);
  rcvd = recv(clients[slot], buf[slot], BUF_SIZE, 0);

  if (rcvd < 0){ // receive error
    fprintf(stderr, ("recv() error\n"));
    return;
  }
  else if (rcvd == 0) { // receive socket closed
    fprintf(stderr, "Client disconnected upexpectedly.\n");
    return;
  }
  else // message received
  {
    pthread_mutex_lock(&mutex_lock);

    buf[slot][rcvd] = '\0';
    fprintf(stderr,"\n\033[34m======================Buffer received======================\033[0m\n\n");
    fprintf(stderr,"%s",buf[slot]);
    fprintf(stderr,"\n\033[34m==========================================================\033[0m\n");

    method = strtok(buf[slot], " \t\r\n");
    uri = strtok(NULL, " \t");
    prot = strtok(NULL, " \t\r\n");

    if(!uri) {
      fprintf(stderr,"uri is NULL\n");
      return;
    }

    uri_unescape(uri);
    
    fprintf(stderr, "\n\x1b[36m + [%s] %s\x1b[0m\n", method, uri);

    qs = strchr(uri, '?');

    if (qs)
      *qs++ = '\0'; // split URI
    else
      qs = uri - 1; // use an empty string

    header_t *h = reqhdr;
    char *t, *t2;
    while (h < reqhdr + 16) {
      char *key, *val;

      key = strtok(NULL, "\r\n: \t");
      if (!key)
        break;

      val = strtok(NULL, "\r\n");
      while (*val && *val == ' ')
        val++;

      h->name = key;
      h->value = val;
      h++;
      //fprintf(stderr, "[H] %s: %s\n", key, val); // print request headers 
      
      t = val + 1 + strlen(val);
      if (t[1] == '\r' && t[2] == '\n')
        break;
    }
    t = strtok(NULL, "\r\n");
    t2 = request_header("Content-Length"); // and the related header if there is
    payload = t;
    payload_size = t2 ? atol(t2) : (rcvd - (t - buf[slot]));
    if(payload) normalization_payload();
    // bind clientfd to stdout, making it easier to write
    int clientfd = clients[slot];
    dup2(clientfd, STDOUT_FILENO);
    close(clientfd);

    // call router
    route();

    pthread_mutex_unlock(&mutex_lock);

    // tidy up
    fflush(stdout);
    shutdown(STDOUT_FILENO, SHUT_WR);
    //close(STDOUT_FILENO);
  }
  free(buf[slot]);
}

void set_response_header(char *key, char *value) {
  char header[1024];

  sprintf(header, "%s: %s\n", key, value);
  strcat(response_headers, header);

  return;
}

void respond_to_client(int status, char *json, char *rsc) {
	if(json) {
		if(response_json) free(response_json);
		response_json = (char *)malloc((strlen(json) + 1) * sizeof(char));
		strcpy(response_json, json);
	}

  if(!response_json) {
    fprintf(stderr,"response_json is NULL\n");
    return;
  }

	char content_length[16];

	sprintf(content_length, "%ld", strlen(response_json));
	set_response_header("Content-Length", content_length);
  set_response_header("X-M2M-RSC", rsc);

  fprintf(stderr,"\n\033[34m========================Buffer sent========================\033[0m\n\n");
	switch(status) {
		case 200: HTTP_200; LOG_HTTP_200; break;
		case 201: HTTP_201; LOG_HTTP_201; break;
		case 209: HTTP_209; LOG_HTTP_209; break;
		case 400: HTTP_400; LOG_HTTP_400; break;
		case 403: HTTP_403; LOG_HTTP_403; break;
		case 404: HTTP_404; LOG_HTTP_404; break;
		case 406: HTTP_406; LOG_HTTP_406; break;
		case 413: HTTP_413; LOG_HTTP_413; break;
		case 500: HTTP_500; LOG_HTTP_500; break;
	}
	printf("%s",response_json);
  fprintf(stderr,"%s\n",response_json);
  fprintf(stderr,"\n\n\033[34m==========================================================\033[0m\n");
  //free(response_json);
}

void normalization_payload() {
	int index = 0;

	for(int i=0; i<payload_size; i++) {
		if(is_json_valid_char(payload[i])) {
			payload[index++] =  payload[i];
		}
	}

	payload[index] = '\0';
}