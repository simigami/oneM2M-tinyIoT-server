#include "httpd.h"
#include "onem2m.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

AE Create_AE(char *parsed_json) {	
	AE created_ae;
	int json_len = strlen(parsed_json);
	int index;
	char shortname[4];
	char value[100];

	for(int i=0; i<json_len; i++){
		index = 0;
		
		while(parsed_json[i] != ' ') {
			shortname[index++] = parsed_json[i++];
		}
		shortname[index] = '\0';
		
		index = 0;
		i+=3;
		
		while(parsed_json[i] != '\n' && parsed_json[i] != '\0') {
			if(parsed_json[i] != '\"')
			    value[index++] = parsed_json[i];
			i++;
		}
		value[index] = '\0';
		
		if(strcmp(shortname, "api") == 0) {
		    created_ae.api = malloc(((int)strlen(value)+1)*sizeof(char));
		    strcpy(created_ae.api,value);
		}

		if(strcmp(shortname, "rr") == 0) {
		    created_ae.rr = malloc(sizeof(bool));
		    if(strcmp(value,"true") == 0) {
			created_ae.rr = true;	
		    }
		    else {
			created_ae.rr = false;
		    }
		}

		if(strcmp)
	}

	return created_ae;
}


