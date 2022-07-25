#include "onem2m.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

char *tree;

Node* Validate_URI(RT *rt) { 
	Node *node = rt->root;
	
	char *ptr = (char*)malloc(sizeof(uri));
	char *ptr2 = ptr;
	strcpy(ptr,uri);
	
	ptr = strtok(ptr, "/");
	
	int view = 0;
	
	while(ptr != NULL && node) {
		if(!strcmp(ptr,"viewer")) {
			ptr = strtok(NULL, "/");
			view = 1;
			continue;
		}

		while(node) {
			if(!strcmp(node->rn,ptr)) break;
			node = node->siblingRight;
		}
		node = node->child;
		ptr = strtok(NULL, "/");
	}
	
	free(ptr2);
	
	if(view) {
		tree = (char *)calloc(1000,sizeof(char));
		tree_data(node);
		HTTP_200;
		printf("%s\n",tree);
	}
	
	return node;
}

void tree_data(Node *node) {
	strcat(tree,node->rn);
	strcat(tree,"\n");
	
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
	}
	
	free(node->ri);
	free(node->rn);
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
	
	char* now = (char*)malloc(sizeof(char)*16);
	
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
	
	cnt->ri = (char*)malloc(sizeof(now));
	cnt->et = (char*)malloc(sizeof(now));
	cnt->ct = (char*)malloc(sizeof(now));
	cnt->lt = (char*)malloc(sizeof(now));
	cnt->cni = (char*)malloc(sizeof(now));
	ae->pi = (char*)malloc(sizeof(now));
	strcpy(ae->ri, now);
	strcpy(ae->et, now);
	strcpy(ae->ct, now);
	strcpy(ae->lt, now);
	strcpy(ae->aei,now);
	strcpy(ae->pi, pi);
	
	ae->ty = 2;
	
	free(now);
}

void Set_CNT(CNT* cnt, char *pi) {
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
	
	ae->ty = 2;
	
	free(now);
}

CSE* Get_sample_CSE(char *ri) {
    CSE* cse = (CSE*)malloc(sizeof(CSE));
    
    cse->ct = (char*)malloc(16*sizeof(char));
    cse->lt = (char*)malloc(16*sizeof(char));
    cse->rn = (char*)malloc(32*sizeof(char));
    cse->ri = (char*)malloc(32*sizeof(char));
    cse->pi = (char*)malloc(32*sizeof(char));
    cse->csi = (char*)malloc(16*sizeof(char));

    strcpy(cse->ct, "20191210T093452");
    strcpy(cse->lt, "20191210T093452");
    strcpy(cse->rn, "sample_CSE");
    strcpy(cse->pi, "NULL");
    strcpy(cse->csi, "/Tiny_Project2");
    strcpy(cse->ri, ri);
    cse->ty = 5;
    
    return cse;
}

AE* Get_sample_AE(char *ri) {
    AE* ae = (AE*)malloc(sizeof(AE));
    
    ae->pi = (char*)malloc(32*sizeof(char));
    ae->ri = (char*)malloc(32*sizeof(char));
    ae->ct = (char*)malloc(16*sizeof(char));
    ae->lt = (char*)malloc(16*sizeof(char));
    ae->et = (char*)malloc(16*sizeof(char));
    ae->api = (char*)malloc(32*sizeof(char));
    ae->aei = (char*)malloc(32*sizeof(char));
    ae->rn = (char*)malloc(32*sizeof(char));
    
    strcpy(ae->rn, "sample_AE");
    strcpy(ae->ct, "20220513T083900");
    strcpy(ae->lt, "20220513T083900");
    strcpy(ae->et, "20240513T083900");
    strcpy(ae->api, "tinyProject");
    strcpy(ae->aei, "TAE");
    strcpy(ae->ri , ri);
    ae->rr = true;
    ae->ty = 2;
    
    return ae;
}

CNT* Get_sample_CNT(char *ri) {
    CNT* cnt = (CNT*)malloc(sizeof(CNT));
    
    cnt->pi = (char*)malloc(32*sizeof(char));
    cnt->ri = (char*)malloc(32*sizeof(char));
    cnt->ct = (char*)malloc(16*sizeof(char));
    cnt->lt = (char*)malloc(16*sizeof(char));
    cnt->et = (char*)malloc(16*sizeof(char));
    cnt->rn = (char*)malloc(32*sizeof(char));
    
    strcpy(cnt->pi, "TAE");
    strcpy(cnt->ri, ri);
    strcpy(cnt->ct, "202205T093154");
    strcpy(cnt->rn, "sample_CNT");
    strcpy(cnt->lt, "20220513T093154");
    strcpy(cnt->et, "20220513T093154");
    cnt->ty = 3;
    cnt->st = 0;
    cnt->cni = 0;
    cnt->cbs = 0;
    
    return cnt;
}

CIN* Get_sample_CIN(char *ri) {
    CIN* cin = (CIN*)malloc(sizeof(CIN));
    
    cin->pi = (char*)malloc(32*sizeof(char));
    cin->ri = (char*)malloc(32*sizeof(char));
    cin->ct = (char*)malloc(16*sizeof(char));
    cin->lt = (char*)malloc(16*sizeof(char));
    cin->et = (char*)malloc(16*sizeof(char));
    cin->rn = (char*)malloc(32*sizeof(char));
    cin->con = (char*)malloc(16*sizeof(char));
    cin->csi = (char*)malloc(16*sizeof(char));
    
    strcpy(cin->pi, "3-20220513091700249586");
    strcpy(cin->ri, ri); 
    strcpy(cin->ct, "202205T093154");
    strcpy(cin->rn, "sample_CIN");
    strcpy(cin->lt, "20220513T093154");
    strcpy(cin->et, "20220513T093154");
    strcpy(cin->con, "ON");
    strcpy(cin->csi, "csitest");
    cin->ty = 4;
    cin->st = 1;
    
    return cin;
}
