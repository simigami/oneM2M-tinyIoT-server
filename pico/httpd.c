#include "httpd.h"
#include "config.h"
#include "onem2m.h"
#include "util.h"

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
#define QUEUE_SIZE 64

pthread_mutex_t mutex_lock;
int listenfd;
int clients[MAX_CONNECTIONS];
static void start_server(const char *);
static void respond(int);
extern void route(oneM2MPrimitive *o2pt);

static char *buf[MAX_CONNECTIONS];

// Client request
char *method, // "GET" or "POST"
    *uri,     // "/index.html" things before '?'
    *qs,      // "a=1&b=2" things after  '?'
    *prot,    // "HTTP/1.1"
    *payload; // for POST

int payload_size;

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
        clients[i] = -1;
    }   

    logger("HTTP", LOG_LEVEL_INFO, "Server started %shttp://127.0.0.1:%s%s", "\033[92m", PORT, "\033[0m");  
    start_server(PORT); 
    // Ignore SIGCHLD to avoid zombie threads
    signal(SIGCHLD, SIG_IGN); 
    // ACCEPT connections
    while (1) {
        clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);
        if (clients[slot] < 0) {
            perror("accept() error");
            //exit(1);
            clients[slot] = -1;
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
char *request_header(header_t *h, const char *name) {
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
    bool flag = false;
    char buffer[BUF_SIZE] = {0};
    buf[slot] = malloc(BUF_SIZE*sizeof(char));
    rcvd = recv(clients[slot], buf[slot], BUF_SIZE, 0); 

    memset(reqhdr, 0, sizeof(reqhdr));

    if (rcvd < 0){ // receive error
        logger("HTTP", LOG_LEVEL_ERROR, "recv() error");
        return;
    } else if (rcvd == 0) { // receive socket closed
        logger("HTTP", LOG_LEVEL_ERROR, "Client disconnected upexpectedly");
        return;
    } else { // message received
        pthread_mutex_lock(&mutex_lock);  
        buf[slot][rcvd] = '\0';
        logger("HTTP", LOG_LEVEL_DEBUG, "\n\n%s\n",buf[slot]);
        memcpy(buffer, buf[slot], rcvd);

        method = strtok(buffer, " \t\r\n");
        uri = strtok(NULL, " \t");
        prot = strtok(NULL, " \t\r\n");   
        if(!uri) {
            logger("HTTP", LOG_LEVEL_ERROR, "URI is NULL");
            return;
        } 
        uri_unescape(uri);
    
        logger("HTTP", LOG_LEVEL_DEBUG, "\x1b[36m[%s] %s\x1b[0m",method, uri); 
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
            fprintf(stderr, "[H] %s: %s\n", key, val); // print request headers 

            t = val + 1 + strlen(val);
            if (t[0] == '\r' && t[1] == '\n')
                break;
        }
        t = strtok(NULL, "\r\n");
        t2 = request_header(request_headers(), "Content-Length"); // and the related header if there is
        payload = t;
        payload_size = t2 ? atol(t2) : 0;
        if(payload_size > 0 && (payload == NULL)) {
            flag = true;
            payload = (char *)calloc(MAX_PAYLOAD_SIZE, sizeof(char));
            recv(clients[slot], payload, MAX_PAYLOAD_SIZE, 0);
        }
        if(payload) normalize_payload();
        // bind clientfd to stdout, making it easier to write
        
        //dup2(clientfd, STDOUT_FILENO);
        // call router
        handle_http_request(slot);    
        // tidy up
        fflush(stdout);
        shutdown(STDOUT_FILENO, SHUT_WR);
        //close(STDOUT_FILENO);
    }
    if(flag) free(payload);
    free(buf[slot]);
    pthread_mutex_unlock(&mutex_lock);
}

Operation http_parse_operation(){
	Operation op;

	if(strcmp(method, "POST") == 0) op = OP_CREATE;
	else if(strcmp(method, "GET") == 0) op = OP_RETRIEVE;
	else if (strcmp(method, "PUT") == 0) op = OP_UPDATE;
	else if (strcmp(method, "DELETE") == 0) op = OP_DELETE;
    else if (strcmp(method, "OPTIONS") == 0) op = OP_OPTIONS;

	return op;
}

void handle_http_request(int slotno) {
	oneM2MPrimitive *o2pt = (oneM2MPrimitive *)calloc(1, sizeof(oneM2MPrimitive));
    cJSON *fcjson = NULL;
	char *header = NULL;

    

	if(payload) {
		o2pt->pc = (char *)malloc((payload_size + 1) * sizeof(char));
		strcpy(o2pt->pc, payload);
		o2pt->cjson_pc = cJSON_Parse(o2pt->pc);
	} 

	if((header = request_header(request_headers(), "X-M2M-Origin"))) {
		o2pt->fr = (char *)malloc((strlen(header) + 1) * sizeof(char));
		strcpy(o2pt->fr, header);
	} 

	if((header = request_header(request_headers(), "X-M2M-RI"))) {
		o2pt->rqi = (char *)malloc((strlen(header) + 1) * sizeof(char));
		strcpy(o2pt->rqi, header);
	} 

	if((header = request_header(request_headers(), "X-M2M-RVI"))) {
		o2pt->rvi = (char *)malloc((strlen(header) + 1) * sizeof(char));
		strcpy(o2pt->rvi, header);
	} 

	if(uri) {
        if(strncmp(uri, "/~/", 3) == 0) {
            o2pt->isForwarding = true;
            o2pt->to = (char *)malloc((strlen(uri) + 1) * sizeof(char));
            strcpy(o2pt->to, uri+3);
        }else{
            o2pt->to = (char *)malloc((strlen(uri) + 1) * sizeof(char));
            strcpy(o2pt->to, uri+1);
        }
        logger("HTTP", LOG_LEVEL_DEBUG, "to: %s", o2pt->to);
	} 
 
    o2pt->op = http_parse_operation();    

	if(o2pt->op == OP_CREATE) o2pt->ty = http_parse_object_type();
    else if(o2pt->op == OP_OPTIONS){
        o2pt->rsc = RSC_OK;
        http_respond_to_client(o2pt, slotno);
        free_o2pt(o2pt);
        return;
    }
    // else if(o2pt->op == OP_FORWARDING){
    //     o2pt->pc = strndup(buf[slotno], BUF_SIZE);
    // }
    o2pt->errFlag = false;

    

    if(qs && strlen(qs) > 0){
        fcjson = qs_to_json(qs);
        if(!(o2pt->fc = parseFilterCriteria(fcjson))){
            if(o2pt->pc)
                free(o2pt->pc);
            o2pt->pc = strdup("{\"m2m:dbg\": \"Invalid FilterCriteria\"}");
            o2pt->rsc = RSC_BAD_REQUEST;
            http_respond_to_client(o2pt, slotno);
            cJSON_Delete(fcjson);
            free_o2pt(o2pt);
            return;
        }

        if(o2pt->fc->fu == FU_DISCOVERY){
            o2pt->op = OP_DISCOVERY;
        }
        cJSON_Delete(fcjson);
    }
    pthread_mutex_trylock(&mutex_lock);
	route(o2pt);
    pthread_mutex_unlock(&mutex_lock);
    
    http_respond_to_client(o2pt, slotno);
    free_o2pt(o2pt);
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

void http_respond_to_client(oneM2MPrimitive *o2pt, int slotno) {
    int status_code = rsc_to_http_status(o2pt->rsc);
    char content_length[64];
    char rsc[64];
    char cnst[32], ot[32];
    char response_headers[2048] = {'\0'};
    char buf[BUF_SIZE] = {'\0'};
    char *status;

    sprintf(content_length, "%ld", o2pt->pc ? strlen(o2pt->pc) : 0);
    sprintf(rsc, "%d", o2pt->rsc);
    set_response_header("Content-Length", content_length, response_headers);
    set_response_header("X-M2M-RSC", rsc, response_headers);
    set_response_header("X-M2M-RVI", o2pt->rvi, response_headers);
    set_response_header("X-M2M-RI", o2pt->rqi, response_headers);

    if(o2pt->cnot > 0){
        sprintf(cnst, "%d", o2pt->cnst);
        set_response_header("X-M2M-CNST", cnst, response_headers);
        sprintf(ot, "%d", o2pt->cnot);
        set_response_header("X-M2M-CNOT", ot, response_headers);
    }

   

    switch(status_code) {
        case 200: status = "200 OK"; break;
        case 201: status = "201 Created"; break;
        case 209: status = "209 Conflict"; break;
        case 400: status = "400 Bad Request"; break;
        case 403: status = "403 Forbidden"; break;
        case 404: status = "404 Not found"; break;
        case 406: status = "406 Not Acceptable"; break;
        case 413: status = "413 Payload Too Large"; break;
        case 500: status = "500 Internal Server Error"; break;
    }
    sprintf(buf, "%s %s\n%s%s\n", HTTP_PROTOCOL_VERSION, status, DEFAULT_RESPONSE_HEADERS, response_headers);
    if(o2pt->pc) strcat(buf, o2pt->pc);
    write(clients[slotno], buf, strlen(buf)); 
    logger("HTTP", LOG_LEVEL_DEBUG, "\n\n%s\n",buf);
}

void http_notify(oneM2MPrimitive *o2pt, char *noti_json, NotiTarget *nt) {

    struct sockaddr_in serv_addr;
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        logger("HTTP", LOG_LEVEL_ERROR, "socket error");
        return;
    }

    http_send_get_request(nt->host, nt->port, nt->target, DEFAULT_REQUEST_HEADERS, "", noti_json);
}

void http_send_get_request(char *host, char *port, char *uri, char *header, char *qs, char *data){
    struct sockaddr_in serv_addr;
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        logger("HTTP", LOG_LEVEL_ERROR, "socket error");
        return;
    }
    char buffer[BUF_SIZE];
    sprintf(buffer, "GET %s%s %s\r\n%s\r\n%s\r\n", uri, qs, HTTP_PROTOCOL_VERSION, header, data);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host);
    serv_addr.sin_port = htons(atoi(port));

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        logger("HTTP", LOG_LEVEL_ERROR, "connect error");
        return;
    }
    logger("http", LOG_LEVEL_DEBUG, "%s", buffer);

    send(sock, buffer, sizeof(buffer), 0);
    close(sock);
}

void http_forwarding(oneM2MPrimitive *o2pt, char *host, char *port, CSR* csr){
    struct sockaddr_in serv_addr;
    char *method = NULL;
    switch (o2pt->op){
        case OP_CREATE:
            method = "POST";
            break;
        case OP_UPDATE:
            method = "PUT";
            break;
        case OP_DELETE:
            method = "DELETE";
            break;
        case OP_RETRIEVE:
            method = "GET";
            break;
    }

    int rcvd;
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        logger("HTTP", LOG_LEVEL_ERROR, "socket error");
        return;
    }
    char *response = (char *)malloc(sizeof(char) * BUF_SIZE);
    char buffer[BUF_SIZE];

    char *headers[2048] = {0};
    set_response_header("X-M2M-Origin", CSE_BASE_NAME, headers);
    set_response_header("X-M2M-RVI", o2pt->rvi, headers);
    set_response_header("X-M2M-RI", o2pt->rqi, headers);
    sprintf(buffer, "application/json;ty=%d", o2pt->ty);
    set_response_header("Content-Type", buffer, headers);
    sprintf(buffer, "%d", strlen(o2pt->pc));
    set_response_header("Content-Length", buffer, headers);

    char *new_to = malloc(sizeof(char) * strlen(o2pt->to));
    sprintf(new_to, "/%s", csr->csi);
    if(strlen(o2pt->to + strlen(CSE_BASE_NAME)+1 + strlen(csr->rn)) > 0){
        strcat(new_to, o2pt->to + strlen(CSE_BASE_NAME)+1+ strlen(csr->rn));
    }

    sprintf(buffer, "%s\t%s\t%s\r\n%s\r\n", 
    method, new_to, HTTP_PROTOCOL_VERSION, headers);

    if(o2pt->pc){
        strcat(buffer, o2pt->pc);
    }

    free(new_to);
    new_to = NULL;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host);
    serv_addr.sin_port = htons(atoi(port));
    logger("http", LOG_LEVEL_DEBUG, "forwarding to %s, %s", host, port);

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        logger("HTTP", LOG_LEVEL_ERROR, "connect error");
        o2pt->rsc = RSC_TARGET_NOT_REACHABLE;
        free(o2pt->pc);
        o2pt->pc = NULL;
        return;
    }

    // setting timeout
    struct timeval tv;
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));


    logger("http", LOG_LEVEL_DEBUG, "%d, %s", strlen(buffer), buffer);
    write(sock, buffer, strlen(buffer));

    memset(buffer, 0, BUF_SIZE);
    rcvd = recv(sock, buffer, BUF_SIZE, 0); 

    if (rcvd < 0){ // receive error
        logger("HTTP", LOG_LEVEL_ERROR, "recv() error");
        o2pt->rsc = RSC_TARGET_NOT_REACHABLE;
        return;
    } else if (rcvd == 0) { // receive socket closed
        logger("HTTP", LOG_LEVEL_ERROR, "Client disconnected upexpectedly");
        o2pt->rsc = RSC_TARGET_NOT_REACHABLE;
        return;
    } else { // message received
        char *protocol = strtok(buffer, " \t");
        char *statuscode = strtok(NULL, " \t");
        char *statusword = strtok(NULL, " \t\r\n");   

        logger("HTTP", LOG_LEVEL_DEBUG, "protocol: %s, statuscode: %s, statusword: %s", protocol, statuscode, statusword);
        header_t *h = malloc(sizeof(header_t) * 20);
        header_t *ptr = h;
        char *t, *t2;
        while (ptr < h + 16) {
            char *key, *val;  
            key = strtok(NULL, "\r\n: \t");
            if (!key)
                break;    
            val = strtok(NULL, "\r\n");
            while (*val && *val == ' ')
                val++;    
            ptr->name = key;
            ptr->value = val;
            ptr++;
            //fprintf(stderr, "[H] %s: %s\n", key, val); // print request headers 

            t = val + 1 + strlen(val);
            if (t[1] == '\r' && t[2] == '\n')
                break;
        }
        logger("HTTP", LOG_LEVEL_DEBUG, "curt: %s", t);
        t = strtok(NULL, "\r\n");
        char *body = t;
        
        logger("HTTP", LOG_LEVEL_DEBUG, "body: %s", body);
        t2 = request_header(h, "Content-Length");
        size_t body_size = t2 ? atol(t2) : 0;
        logger("HTTP", LOG_LEVEL_DEBUG, "body_size: %d", body_size);
        if(body_size > 0 && (body_size == NULL)) {
            body = (char *)calloc(MAX_PAYLOAD_SIZE, sizeof(char));
            recv(sock, body, MAX_PAYLOAD_SIZE, 0);
        }
        if(body) normalize_payload();

        char *rsc = request_header(h, "X-M2M-RSC");
        o2pt->rsc = atoi(rsc);
        if(o2pt->pc) {
            free(o2pt->pc);
            o2pt->pc = NULL;
        }
        if(body)
            o2pt->pc = strdup(body);
        else

        if(h) free(h);
    }
    close(sock);
    return;
}
