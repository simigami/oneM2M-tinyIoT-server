#include "onem2m.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

char *tree;

Node* Validate_URI(RT *rt) { 
	Node *node = rt->root;
	
	uri = strtok(uri, "/");
	
	int view = 0;
	
	while(uri != NULL && node) {
	/*
		if(!strcmp(ptr,"viewer")) {
			ptr = strtok(NULL, "/");
			view = 1;
			continue;
		}
	*/
		while(node) {
			if(!strcmp(node->rn,uri)) break;
			node = node->siblingRight;
		}
		
		uri = strtok(NULL, "/");
		
		if(uri == NULL) break;
		
		node = node->child;
	}
	
	/*
	if(view) {
		tree = (char *)calloc(1000,sizeof(char));
		tree_data(node);
		HTTP_200;
		printf("%s\n",tree);
	}*/
	
	return node;
}

void tree_data(Node *node) {
	fprintf(stderr,"%s\n",node->rn);
	
	node = node->child;
	
	while(node) {
		tree_data(node);
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
		while(node->siblingRight) node = node->siblingRight;
		
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
	//case t_CNT : Delete_CNT(node->ri); break;
	}
	
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

void Set_AE(AE* ae, char *pi) {
	char *now = Get_LocalTime();
	
	ae->ri = (char*)malloc(sizeof(now));
	ae->et = (char*)malloc(sizeof(now));
	ae->ct = (char*)malloc(sizeof(now));
	ae->lt = (char*)malloc(sizeof(now));
	ae->aei = (char*)malloc(sizeof(now));
	ae->pi = (char*)malloc(sizeof(now));
	strcpy(ae->ri, now);
	strcpy(ae->et, now);
	strcpy(ae->ct, now);
	strcpy(ae->lt, now);
	strcpy(ae->aei,now);
	strcpy(ae->pi, pi);
	
	ae->ty = t_AE;
	
	free(now);
}

void Set_CNT(CNT* cnt, char *pi) {
	char *now = Get_LocalTime();
	
	cnt->ri = (char*)malloc(sizeof(now));
	cnt->et = (char*)malloc(sizeof(now));
	cnt->ct = (char*)malloc(sizeof(now));
	cnt->lt = (char*)malloc(sizeof(now));
	cnt->pi = (char*)malloc(sizeof(now));
	strcpy(cnt->ri, now);
	strcpy(cnt->et, now);
	strcpy(cnt->ct, now);
	strcpy(cnt->lt, now);
	strcpy(cnt->pi, pi);
	
	cnt->ty = t_CNT;
	cnt->st = 0;
	cnt->cni = 0;
	cnt->cbs = 0;
	
	free(now);
}
