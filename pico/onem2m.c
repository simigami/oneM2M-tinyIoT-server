#include "onem2m.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int Validate_OneM2M_Standard() {
	int ret = 1;

	if(!request_header("X-M2M-RI")) {
		fprintf(stderr,"Request has no \"X-M2M-RI\" Header\n");
		ret = 0;
	} 
	if(!request_header("X-M2M-Origin")) {
		fprintf(stderr,"Request has no \"X-M2M-Origin\" Header\n");
		ret = 0;		
	}
	
	return ret;
}

Node* Parse_URI(RT *rt) {
	fprintf(stderr,"Parse_URI \x1b[33m%s\x1b[0m...",uri); 
	Node *node = NULL;
	
	uri = strtok(uri, "/");
	
	int viewer = 0, test = 0;
	
	if(uri != NULL) {
		if(!strcmp("test", uri)) test = 1;
		else if(!strcmp("viewer", uri)) viewer = 1;
		
		if(test || viewer) uri = strtok(NULL, "/");
	}
	
	if(uri != NULL && !strcmp("TinyIoT", uri)) node = rt->root;
	
	while(uri != NULL && node) {
		char *cin_ri;
		if((cin_ri = strstr(uri, "4-20")) != NULL) {
			fprintf(stderr,"OK\n\x1b[43mRetrieve CIN By Ri\x1b[0m\n");
			Retrieve_CIN_Ri(cin_ri);
			return NULL;
		}
	
		if(!strcmp("la", uri) || !strcmp("latest", uri)) {
			while(node->siblingRight) node = node->siblingRight;
			if(node->ty != t_CIN) node = NULL;
			break;
		} else if(!strcmp("ol", uri) || !strcmp("oldest", uri)) {
			while(node) {
				if(node->ty == t_CIN) break;
				node = node->siblingRight;
			}
			break;
		}
		while(node) {
			if(!strcmp(node->rn,uri)) break;
			node = node->siblingRight;
		}
		
		uri = strtok(NULL, "/");
		
		if(uri == NULL) break;
		
		if(!strcmp(uri,"cinperiod")) {
			fprintf(stderr,"OK\n\x1b[43mRetrieve CIN in Period\x1b[0m\n");
			CIN_in_period(node);
			return NULL;
		}
		
		if(node) node = node->child;
	}
	
	if(node) {
		if(viewer) {
			fprintf(stderr,"OK\n\x1b[43mTree Viewer API\x1b[0m\n");
			TreeViewerAPI(node);
			return NULL;
		} else if(test) {
			fprintf(stderr,"OK\n\x1b[43mObject Test API\x1b[0m\n");
			ObjectTestAPI(node);
			return NULL;
		}
	} else if(!node) {
		fprintf(stderr,"Invalid\n");
		HTTP_400;
		printf("Invalid URI\n");
		return NULL;
	}
	
	fprintf(stderr,"OK\n");
	
	return node;
}

Operation Parse_Operation(){
	Operation op;

	if(strcmp(method, "POST") == 0) op = o_CREATE;
	else if(strcmp(method, "GET") == 0) op = o_RETRIEVE;
	else if (strcmp(method, "PUT") == 0) op = o_UPDATE;
	else if (strcmp(method, "DELETE") == 0) op = o_DELETE;

	return op;	
}

void Retrieve_CIN_Ri(char *ri) {
	CIN* gcin = Get_CIN(ri);
	
	if(gcin) {
		char *resjson = CIN_to_json(gcin);
		HTTP_200_CORS;
		printf("%s",resjson);
		free(resjson);
		Free_CIN(gcin);
		resjson = NULL;
		gcin = NULL;
	} else {
		fprintf(stderr,"There is no such CIN ri = %s\n",ri);
		HTTP_400;
		printf("Invalid URI\n");
	}
}

void CIN_in_period(Node *pnode) {
	int period = 0;
	char key[8] = "period=";
	
	qs = strtok(qs, "&");
	
	while(qs != NULL) {
		int flag = 1;
		if(strlen(qs) >= 8) {
			for(int i=0; i<7; i++) {
				if(qs[i] != key[i]) flag = 0;
			}
		}
		if(flag) {
			period = atoi(qs+7);
			break;
		}
		qs = strtok(NULL, "&");
	}
	
	char *start = Get_LocalTime(period);
	char *end = Get_LocalTime(0);
	Node *cinList = Get_CIN_Period(start, end);
	
	fprintf(stderr,"period : %d seconds\n",period);
	
	Node *cin = cinList;
	
	HTTP_200_CORS;
	while(cin) {
		if(!strcmp(cin->pi, pnode->ri)) {
			CIN* gcin = Get_CIN(cin->ri);
			char *resjson = CIN_to_json(gcin);
			printf("%s\n",resjson);
			free(resjson);
			Free_CIN(gcin);
			resjson = NULL;
			gcin = NULL;
		}
		cin = cin->siblingRight;
	}
	
	while(cinList) {
		Node *r = cinList->siblingRight;
		Free_Node(cinList);
		cinList = r;
	}
}

void TreeViewerAPI(Node *node) {
	char *viewer_data = (char *)calloc(10000,sizeof(char));
	strcat(viewer_data,"[");
	
	Node *p = node;
	while(p = p->parent) {
		char *json = Node_to_json(p);
		strcat(viewer_data,",");
		strcat(viewer_data,json);
	}
	
	int cinSize = 1;
	
	char *la = strstr(qs,"la=");
	if(la) cinSize = atoi(la+3);
	
	fprintf(stderr,"Latest CIN Size : %d\n", cinSize);
	
	Tree_data(node, &viewer_data, cinSize);
	strcat(viewer_data,"]");
	char res[10000] = "";
	int index = 0;
	
	for(int i=0; i<10000; i++) {
		if(i == 1) continue;
		if(viewer_data[i] != 0 && viewer_data[i] != 32 && viewer_data[i] != 10 && viewer_data[i] != 9) {
			res[index++] = viewer_data[i];
		}
	}
	
	fprintf(stderr,"TreeViewerAPI Content-Size : %ld\n",strlen(res));

	HTTP_200_CORS;
	printf("%s",res);
	free(viewer_data);
	viewer_data = NULL;
}

void Tree_data(Node *node, char **viewer_data, int cin_num) {
	if(node->ty == t_CIN) {
		Node *cinLatest = Get_CIN_Pi(node->pi);
		
		Node *p = cinLatest;
		
		cinLatest = LatestCINs(cinLatest, cin_num);
		
		while(cinLatest) {
			char *json = Node_to_json(cinLatest);
			strcat(*viewer_data, ",");
			strcat(*viewer_data, json);
			Node *right = cinLatest->siblingRight;
			Free_Node(cinLatest);
			cinLatest = right;
		}
		return;
	}
	
	char *json = Node_to_json(node);
	strcat(*viewer_data, ",");
	strcat(*viewer_data, json);
	
	node = node->child;
	
	while(node) {
		Tree_data(node, viewer_data, cin_num);
		if(node->ty == t_CIN) break;
		node = node->siblingRight;
	}
}

Node *LatestCINs(Node* cinList, int num) {
	Node *head, *tail;
	head = tail = cinList;
	int cnt = 1;
	
	while(tail->siblingRight) {
		tail = tail->siblingRight;
		cnt++;
	}
	
	for(int i=0; i < cnt-num; i++) {
		head = head->siblingRight;
		Free_Node(head->siblingLeft);
		head->siblingLeft = NULL;
	}
	
	return head;
}

void ObjectTestAPI(Node *node) {
	HTTP_200_CORS;
	printf("%d",node->cinSize);
	return;
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

ObjectType Parse_ObjectType() {
	ObjectType ty;
	char *ct = request_header("Content-Type");
	if(!ct) return 0;
	ct = strstr(ct, "ty=");
	int objType = atoi(ct+3);
	
	switch(objType) {
	case 2 : ty = t_AE; break;
	case 3 : ty = t_CNT; break;
	case 4 : ty = t_CIN; break;
	case 5 : ty = t_CSE; break;
	}
	
	return ty;
}

ObjectType Parse_ObjectType_Body(char *json_payload) {
	ObjectType ty;
	
	char *cse, *ae, *cnt;
	
	cse = strstr(json_payload, "m2m:cse");
	ae = strstr(json_payload, "m2m:ae");
	cnt = strstr(json_payload, "m2m:cnt");
	
	if(cse) ty = t_CSE;
	else if(ae) ty = t_AE;
	else if(cnt) ty = t_CNT;
	
	return ty;
}

Node* Create_Node(char *ri, char *rn, char *pi, ObjectType ty){
	Node* node = (Node*)malloc(sizeof(Node));
	
	if(strcmp(rn,"") && strcmp(rn,"TinyIoT")) {
		fprintf(stderr,"\nCreate Tree Node\n[rn] %s\n[ri] %s...", rn, ri);
	}
	
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
	node->cinSize = 0;
	
	if(strcmp(rn,"") && strcmp(rn,"TinyIoT")) fprintf(stderr,"OK\n");
	
	return node;
}

int Add_child(Node *parent, Node *child) {
	Node *node = parent->child;
	child->parent = parent;
	
	if(child->ty == t_CIN) parent->cinSize++;
	
	if(child->ty != t_CIN) fprintf(stderr,"\nAdd Child\n[P] %s\n[C] %s...",parent->rn, child->rn);
	
	if(!node) {
		parent->child = child;
	} else if(node) {
		if(child->ty < node->ty) {
			parent->child = child;
			child->siblingRight = node;
			node->siblingLeft = child;
		} else {
			while(node->siblingRight && node->siblingRight->ty <= child->ty) { 
				if(node->ty == t_CIN && child->ty == t_CIN) {
					Free_Node(node->siblingRight);
					node->siblingRight = NULL;
					break;
				}
				node = node->siblingRight;
			}
			
			if(node->siblingRight) {
				node->siblingRight->siblingLeft = child;
				child->siblingRight = node->siblingRight;
			}
			node->siblingRight = child;
			child->siblingLeft = node;
		}
	}
	
	if(child->ty != t_CIN) fprintf(stderr,"OK\n");
	
	return 1;
}

void Delete_Node_Object(Node *node, int flag) {
	Node *left = node->siblingLeft;
	Node *right = node->siblingRight;
	
	if(!left) node->parent->child = right;
	
	if(flag == 1) {
		if(left) left->siblingRight = node->siblingRight;
	} else {
		if(node->siblingRight) Delete_Node_Object(node->siblingRight, 0);
	}
	
	if(node->child) Delete_Node_Object(node->child, 0);
	
	switch(node->ty) {
	case t_AE : Delete_AE(node->ri); break;
	case t_CNT : Delete_CNT(node->ri); break;
	}
	
	fprintf(stderr,"Free_Node : %s...",node->rn);
	Free_Node(node);
	fprintf(stderr,"OK\n");
	node = NULL;
}

void Free_Node(Node *node) {
	free(node->ri);
	free(node->rn);
	free(node->pi);
	free(node);
}

char *Get_LocalTime(int diff) {
	time_t t = time(NULL) - diff;
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
	char *now = Get_LocalTime(0);
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
	char *now = Get_LocalTime(0);
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

void Set_AE_Update(AE* before, AE* after) {
	strcpy(after->ct, before->ct);
	strcpy(after->ri, before->ri);
	strcpy(after->aei, before->aei);
	strcpy(after->pi, before->pi);
}

void Set_CNT(CNT* cnt, char *pi) {
	char *now = Get_LocalTime(0);
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
	char *now = Get_LocalTime(0);
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
