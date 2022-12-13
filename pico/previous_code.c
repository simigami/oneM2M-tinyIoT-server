// create_node
Node *create_node(char *rn, char *ri, char *pi, char *nu, char *sur, char *acpi, char *pv_acor, char *pv_acop, char *pvs_acor, char *pvs_acop) {}

// parse_uri
Node* parse_uri(Node *cb, char *uri_array) {
	fprintf(stderr,"parse_uri \x1b[33m%s\x1b[0m...",uri_array);
	//char uri_array[MAX_URI_SIZE];
	char *uri_parse = uri_array;
	Node *node = NULL;

	//strcpy(uri_array, uri);
	
	uri_parse = strtok(uri_parse, "/");
	
	int viewer = 0, test = 0, la_ol = 0;
	
	if(uri_parse != NULL) {
		if(!strcmp("test", uri_parse)) test = 1;
		else if(!strcmp("viewer", uri_parse)) viewer = 1;
		
		if(test || viewer) uri_parse = strtok(NULL, "/");
	}
	
	if(uri_parse != NULL && !strcmp("TinyIoT", uri_parse)) node = cb;
	
	while(uri_parse != NULL && node) {
		char *cin_ri;
		if((cin_ri = strstr(uri_parse, "4-20")) != NULL) {
			fprintf(stderr,"OK\n\x1b[43mRetrieve CIN By Ri\x1b[0m\n");
			retrieve_cin_by_ri(cin_ri);
			return NULL;
		}
	
		if(!strcmp("la", uri_parse) || !strcmp("latest", uri_parse)) {
			while(node->sibling_right) {
				if(node->ty == TY_CIN && node->sibling_right->ty != TY_CIN) break;
				node = node->sibling_right;
			}
			la_ol = 1;
		} else if(!strcmp("ol", uri_parse) || !strcmp("oldest", uri_parse)) {
			while(node) {
				if(node->ty == TY_CIN) break;
				node = node->sibling_right;
			}
			la_ol = 1;
		}
		
		if(!la_ol)
		{
			while(node) {
				if(!strcmp(node->rn,uri_parse)) break;
				node = node->sibling_right;
			}
		}

		la_ol = 0;
		
		uri_parse = strtok(NULL, "/");
		
		if(uri_parse == NULL) break;
		
		if(!strcmp(uri_parse, "cinperiod")) {
			fprintf(stderr,"OK\n\x1b[43mRetrieve CIN in Period\x1b[0m\n");
			cin_in_period(node);
			return NULL;
		}
		
		if(node) node = node->child;
	}
	
	if(node) {
		if(viewer) {
			fprintf(stderr,"OK\n\x1b[43mTree Viewer API\x1b[0m\n");
			tree_viewer_api(node);
			return NULL;
		} else if(test) {
			fprintf(stderr,"OK\n\x1b[43mObject Test API\x1b[0m\n");
			object_test_api(node);
			return NULL;
		}
	} else if(!node) {
		fprintf(stderr,"Invalid\n");
		HTTP_400;
		printf("{\"m2m:dbg\": \"invalid object\"}");
		return NULL;
	}
	
	fprintf(stderr,"OK\n");

	return node;
}

//tree_viewer_data
void tree_viewer_data(Node *node, char **viewer_data, int cin_num) {
	if(node->ty == TY_CIN) {
		Node *cinLatest = Get_CIN_Pi(node->pi);
		
		Node *p = cinLatest;
		
		cinLatest = latest_cin_list(cinLatest, cin_num);
		
		while(cinLatest) {
			char *json = node_to_json(cinLatest);
			strcat(*viewer_data, ",");
			strcat(*viewer_data, json);
			Node *right = cinLatest->sibling_right;
			free_node(cinLatest);
			cinLatest = right;
		}
		return;
	}
	
	char *json = node_to_json(node);
	strcat(*viewer_data, ",");
	strcat(*viewer_data, json);
	
	node = node->child;
	
	while(node) {
		tree_viewer_data(node, viewer_data, cin_num);
		if(node->ty == TY_CIN) {
			while(node->sibling_right && node->sibling_right->ty != TY_SUB) {
				node = node->sibling_right;
			}
		}
		node = node->sibling_right;
	}
}

// httpd open source basic functions
int file_exists(const char *file_name) {
	struct stat buffer;
	int exists;

	exists = (stat(file_name, &buffer) == 0);

	return exists;
}
// httpd open source basic functions
int read_file(const char *file_name) {
	char buf[CHUNK_SIZE];
	FILE *file;
	size_t nread;
	int err = 1;

	file = fopen(file_name, "r");

	if (file) {
		while ((nread = fread(buf, 1, sizeof buf, file)) > 0) {
			fwrite(buf, 1, nread, stdout);
		}

		err = ferror(file);
		fclose(file);
	}
	
	return err;
}

/*
if (fork() == 0) {
	close(listenfd);
	respond(slot);
	close(clients[slot]);
	clients[slot] = -1;
	exit(0);
} else {
	close(clients[slot]);
}
*/