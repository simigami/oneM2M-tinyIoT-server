#include "onem2m.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

char *tree;

int Validate_OneM2M_Standard() {
	if(request_header("X-M2M-RI") && request_header("X-M2M-Origin")) {
		return 1;
	} else {
		return 0;
	}
}

Node* Validate_URI(RT *rt) { 
	Node *node = rt->root;
	
	uri = strtok(uri, "/");
	
	int viewer = 0;
	if(!strcmp("viewer",uri)) {
		viewer = 1;
		uri = strtok(NULL, "/");
	}
	
	while(uri != NULL && node) {
		if(!strcmp("latest",uri)) {
			while(node->siblingRight) node = node->siblingRight;
			break;
		}
		while(node) {
			if(!strcmp(node->rn,uri)) break;
			node = node->siblingRight;
		}
		
		uri = strtok(NULL, "/");
		
		if(uri == NULL) break;
		
		if(node) node = node->child;
	}
	
	if(viewer && node) {
		TreeViewerAPI(node);
		return NULL;
	} else if(!node) {
		HTTP_400;
		printf("Invalid URI\n");
		return NULL;
	}
	
	return node;
}

void TreeViewerAPI(Node *node) {
	char *viewer_data = (char *)calloc(10000,sizeof(char));
	Tree_data(node, &viewer_data);
	HTTP_200;
	printf("%s",viewer_data);
	free(viewer_data);
	viewer_data = NULL;
}

void Tree_data(Node *node, char **viewer_data) {
	char *json = Node_to_json(node);
	strcat(*viewer_data, json);
	strcat(*viewer_data, "\n");
	
	node = node->child;
	
	while(node) {
		Tree_data(node, viewer_data);
		node = node->siblingRight;
	}
}

char *Parse_Request_JSON() {
	char *json_payload = malloc(sizeof(char)*payload_size);
	int index = 0;
	
	for(int i=0; i<payload_size; i++) {
		if(payload[i] != 0 && payload[i] != 32 && payload[i] != 10) {
			json_payload[index++] =  payload[i];
		}
	}
	
	json_payload[index] = '\0';
	
	return json_payload;
}

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

Node* Create_Node(char *ri, char *rn, char *pi, ObjectType ty){
	Node* node = (Node*)malloc(sizeof(Node));
	
	node->rn = (char*)malloc(sizeof(rn));
	node->ri = (char*)malloc(sizeof(ri));
	node->pi = (char*)malloc(sizeof(pi));
	
	strcpy(node->rn, rn);
	strcpy(node->ri, ri);
	strcpy(node->pi, pi);
	
	node->parent = NULL;
	node->child = NULL;
	node->siblingLeft = NULL;
	node->siblingRight = NULL;
	node->ty = ty;
	
	return node;
}

int Add_child(Node *parent, Node *child) {
	Node *node = parent->child;
	child->parent = parent;
	
	if(node) {
		while(node->siblingRight) { 
			node = node->siblingRight;
		}
		
		node->siblingRight = child;
		child->siblingLeft = node;
	}
	else {
		parent->child = child;
	}
	
	return 1;
}

void Delete_Node(Node *node, int flag) {
	Node *left = node->siblingLeft;
	Node *right = node->siblingRight;
	
	if(!left) node->parent->child = right;
	
	if(flag == 1) {
		if(left) left->siblingRight = node->siblingRight;
	} else {
		if(node->siblingRight) Delete_Node(node->siblingRight, 0);
	}
	
	if(node->child) Delete_Node(node->child, 0);
	
	switch(node->ty) {
	case t_AE : Delete_AE(node->ri); break;
	case t_CNT : Delete_CNT(node->ri); break;
	}
	
	Free_Node(node);
	node = NULL;
}

void Free_Node(Node *node) {
	free(node->ri);
	free(node->rn);
	free(node->pi);
	free(node);
}

char *Get_LocalTime() {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	char year[5], mon[3], day[3], hour[3], minute[3], sec[3]; 
	
	sprintf(year,"%d", tm.tm_year+1900);
	sprintf(mon,"%02d",tm.tm_mon+1);
	sprintf(day,"%02d",tm.tm_mday);
	sprintf(hour,"%02d",tm.tm_hour);
	sprintf(minute,"%02d",tm.tm_min);
	sprintf(sec,"%02d",tm.tm_sec);
	
	char* now = (char*)calloc(16,sizeof(char));
	
	strcat(now,year);
	strcat(now,mon);
	strcat(now,day);
	strcat(now,"T");
	strcat(now,hour);
	strcat(now,minute);
	strcat(now,sec);
	
	return now;
}

void Set_CSE(CSE* cse) {
	char *now = Get_LocalTime();
	char ri[18] = "5-";
	char rn[8] = "TinyIoT";
	strcat(ri, now);
	
	cse->ri = (char*)malloc(sizeof(ri));
	cse->rn = (char*)malloc(sizeof(rn));
	cse->ct = (char*)malloc(sizeof(now));
	cse->lt = (char*)malloc(sizeof(now));
	cse->csi = (char*)malloc(sizeof(now));
	cse->pi = (char*)malloc(sizeof("NULL"));
	
	strcpy(cse->ri, ri);
	strcpy(cse->rn, rn);
	strcpy(cse->ct, now);
	strcpy(cse->lt, now);
	strcpy(cse->csi,now);
	strcpy(cse->pi,"NULL");
	
	cse->ty = t_CSE;
	
	free(now);
}

void Set_AE(AE* ae, char *pi) {
	char *now = Get_LocalTime();
	char ri[18] = "2-";
	char tmp[100];
	strcat(ri, now);
	
	strcpy(tmp,ae->api);
	ae->api = (char*)malloc(sizeof(ae->api));
	strcpy(ae->api,tmp);
	
	strcpy(tmp,ae->rn);
	ae->rn = (char*)malloc(sizeof(ae->rn));
	strcpy(ae->rn,tmp);
	
	ae->ri = (char*)malloc(sizeof(ri));
	ae->pi = (char*)malloc(sizeof(pi));
	ae->et = (char*)malloc(sizeof(now));
	ae->ct = (char*)malloc(sizeof(now));
	ae->lt = (char*)malloc(sizeof(now));
	ae->aei = (char*)malloc(sizeof(now));
	
	strcpy(ae->ri, ri);
	strcpy(ae->pi, pi);
	strcpy(ae->et, now);
	strcpy(ae->ct, now);
	strcpy(ae->lt, now);
	strcpy(ae->aei,now);
	
	ae->ty = t_AE;
	
	free(now);
}

void Set_CNT(CNT* cnt, char *pi) {
	char *now = Get_LocalTime();
	char ri[18] = "3-";
	char tmp[100];
	strcat(ri, now);
	
	strcpy(tmp,cnt->rn);
	cnt->rn = (char*)malloc(sizeof(cnt->rn));
	strcpy(cnt->rn,tmp);
	
	cnt->ri = (char*)malloc(sizeof(ri));
	cnt->pi = (char*)malloc(sizeof(pi));
	cnt->et = (char*)malloc(sizeof(now));
	cnt->ct = (char*)malloc(sizeof(now));
	cnt->lt = (char*)malloc(sizeof(now));
	strcpy(cnt->ri, ri);
	strcpy(cnt->pi, pi);
	strcpy(cnt->et, now);
	strcpy(cnt->ct, now);
	strcpy(cnt->lt, now);
	
	cnt->ty = t_CNT;
	cnt->st = 0;
	cnt->cni = 0;
	cnt->cbs = 0;
	
	free(now);
}

void Set_CIN(CIN* cin, char *pi) {
	char *now = Get_LocalTime();
	char ri[18] = "4-";
	char tmp[100];
	strcat(ri, now);
	
	strcpy(tmp,cin->con);
	cin->con = (char*)malloc(sizeof(cin->con));
	strcpy(cin->con,tmp);
	
	cin->rn = (char*)malloc(sizeof(ri));
	cin->ri = (char*)malloc(sizeof(ri));
	cin->pi = (char*)malloc(sizeof(pi));
	cin->et = (char*)malloc(sizeof(now));
	cin->ct = (char*)malloc(sizeof(now));
	cin->lt = (char*)malloc(sizeof(now));
	strcpy(cin->rn, ri);
	strcpy(cin->ri, ri);
	strcpy(cin->pi, pi);
	strcpy(cin->et, now);
	strcpy(cin->ct, now);
	strcpy(cin->lt, now);
	
	cin->ty = t_CIN;
	cin->st = 0;
	
	free(now);
}

void Free_CSE(CSE *cse) {
	free(cse->ct);
	free(cse->lt);
	free(cse->rn);
	free(cse->ri);
	free(cse->csi);
	free(cse->pi);
	free(cse);
}

void Free_AE(AE *ae) {
	free(ae->et);
	free(ae->ct);
	free(ae->lt);
	free(ae->rn);
	free(ae->ri);
	free(ae->pi);
	free(ae->api);
	free(ae->aei);
	free(ae);
}

void Free_CNT(CNT *cnt) {
	free(cnt->et);
	free(cnt->ct);
	free(cnt->lt);
	free(cnt->rn);
	free(cnt->ri);
	free(cnt->pi);
	free(cnt);
}

void Free_CIN(CIN* cin) {
	free(cin->et);
	free(cin->ct);
	free(cin->lt);
	free(cin->rn);
	free(cin->ri);
	free(cin->pi);
	free(cin->con);
	free(cin);
}
