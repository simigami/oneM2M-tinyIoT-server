#include "httpd.h"
#include "config.h"
#include "onem2m.h"

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
#define BUF_SIZE 32768
#define QUEUE_SIZE 64

// Client request
char *method, // "GET" or "POST"
    *uri,            // "/index.html" things before '?'
    *qs,             // "a=1&b=2" things after  '?'
    *prot,           // "HTTP/1.1"
    *payload;        // for POST

int payload_size;

pthread_mutex_t mutex_lock;
int listenfd;
int clients[MAX_CONNECTIONS];
static void start_server(const char *);
static void respond(int);

static char *buf[MAX_CONNECTIONS];

void *respond_thread(void *ps) {
    int slot = *((int*)ps);
    respond(slot);
    close(clients[slot]);
    clients[slot] = -1;
    return NULL;
}

void serve_forever(const char *PORT) {
    pthread_mutex_init(&mutex_lock, NULL);
    struct sockaddr_in clientaddr;
    socklen_t addrlen = 0;

    int slot = 0;
    int slots[MAX_CONNECTIONS];
    for(int i=0; i<MAX_CONNECTIONS; i++) {
        slots[i] = i;
    }   
    logger("HTTP", LOG_LEVEL_INFO, "Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", PORT, "\033[0m");  
    // create shared memory for client slot array
    //clients = mmap(NULL, sizeof(*clients) * MAX_CONNECTIONS,
                   //PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);  
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
        if (clients[slot] < 0) {
            perror("accept() error");
            exit(1);
        } else {
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, respond_thread, (void*)&slots[slot]);
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
    //   socket and 
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
    int flag = 0;

    buf[slot] = malloc(BUF_SIZE*sizeof(char));
    rcvd = recv(clients[slot], buf[slot], BUF_SIZE, 0); 
    if (rcvd < 0){ // receive error
        logger("HTTP", LOG_LEVEL_ERROR, "recv() error");
        return;
    } else if (rcvd == 0) { // receive socket closed
        logger("HTTP", LOG_LEVEL_ERROR, "Client disconnected upexpectedly");
        return;
    }
    else // message received
    {
        pthread_mutex_lock(&mutex_lock);
        memset(reqhdr,0,sizeof(reqhdr));  
        buf[slot][rcvd] = '\0';
        logger("HTTP", LOG_LEVEL_DEBUG, "\n\n%s\n",buf[slot]);    
        method = strtok(buf[slot], " \t\r\n");
        uri = strtok(NULL, " \t");
        prot = strtok(NULL, " \t\r\n");   
        if(!uri) {
            logger("HTTP", LOG_LEVEL_ERROR, "URI is NULL");
            return;
        } 
        uri_unescape(uri);

        logger("HTTP", LOG_LEVEL_DEBUG, "\x1b[36m + [%s] %s\x1b[0m",method, uri); 
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
            if (t[1] == '\r' && t[2] == '\n') break;
      }
      t = strtok(NULL, "\r\n");
      t2 = request_header("Content-Length"); // and the related header if there is
      payload = t;
      payload_size = t2 ? atol(t2) : 0;
      if(payload_size > 0 && !payload) {
        flag = 1;
        payload = (char *)malloc(MAX_PAYLOAD_SIZE*sizeof(char));
        recv(clients[slot], payload, MAX_PAYLOAD_SIZE, 0);
      }
      if(payload) normalize_payload();
      // bind clientfd to stdout, making it easier to write
      int clientfd = clients[slot];
      dup2(clientfd, STDOUT_FILENO);
      close(clientfd);  
      // call router
      handle_http_request();    
      // tidy up
      fflush(stdout);
      shutdown(STDOUT_FILENO, SHUT_WR);
      //close(STDOUT_FILENO);
    }
    free(buf[slot]);
    if(flag == 1) free(payload);
    pthread_mutex_unlock(&mutex_lock);
}

Operation http_parse_operation(){
	Operation op;

	if(strcmp(method, "POST") == 0) op = OP_CREATE;
	else if(strcmp(method, "GET") == 0) op = OP_RETRIEVE;
	else if (strcmp(method, "PUT") == 0) op = OP_UPDATE;
	else if (strcmp(method, "DELETE") == 0) op = OP_DELETE;

	return op;
}

void set_response_header(char *key, char *value, char *response_headers) {
    if(!value) return;
    char header[128];

    sprintf(header, "%s: %s\n", key, value);
    strcat(response_headers, header);

    return;
}

void normalize_payload() {
	int index = 0;

	for(int i=0; i<payload_size; i++) {
		if(is_json_valid_char(payload[i])) {
			payload[index++] =  payload[i];
		}
	}

	payload[index] = '\0';
}

void http_respond_to_client(oneM2MPrimitive *o2pt, int status) {
    char content_length[16];
    char rsc[6];
    char response_headers[1024] = {'\0'};

    sprintf(content_length, "%ld", strlen(o2pt->pc));
    sprintf(rsc, "%d", o2pt->rsc);
    set_response_header("Content-Length", content_length, response_headers);
    set_response_header("X-M2M-RSC", rsc, response_headers);
    set_response_header("X-M2M-RVI", o2pt->rvi, response_headers);
    set_response_header("X-M2M-RI", o2pt->rqi, response_headers);

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
    printf("%s",o2pt->pc); 
    fprintf(stderr,"%s\n",o2pt->pc);
    fprintf(stderr,"\n\n\033[34m==========================================================\033[0m\n");
}