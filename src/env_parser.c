#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** split(const char* str, const char split){
	unsigned int len1 = 0;
	for(unsigned int i = 0; str[i] != split; i++, len1++);
	unsigned int len2 = 0;
	for(unsigned int i = len1 + 1; str[i]; i++, len2++);
	char** split_text = malloc(2 * sizeof(char*));
	if(!split_text){
		fprintf(stderr, "ERROR: Can't allocate memory\n");
		return NULL;
	}
	split_text[0] = malloc(len1 + 1);
	split_text[1] = malloc(len2 + 1);
	if(!split_text[0] || !split_text[1]){
		fprintf(stderr, "ERROR: Can't allocate memory\n");
		if(split_text[0])
			free(split_text[0]);
		if(split_text[1])
			free(split_text[1]);
		free(split_text);
		return NULL;
	}
	strncpy(split_text[0], str, len1);
	split_text[0][len1] = '\0';
	strncpy(split_text[1], str + len1 + 1, len2);
	split_text[1][len2] = '\0';
	return split_text;
}

char* dotenv_get(const char* key){
	FILE* file = fopen(".env", "r");
	if(!file){
		fprintf(stderr, "ERROR: Can't open .env file");
		return NULL;
	}
	char* line = NULL;
	size_t size;
	char* value = NULL;
	while(getline(&line, &size, file) != -1){
		line[strlen(line) - 1] = '\0';
		char** split_text = split(line, '=');
		if(strcmp(split_text[0], key))
			continue;
		free(line);
		value = strdup(split_text[1]);
		free(split_text[0]);
		free(split_text[1]);
		free(split_text);
		break;
	}
	return value;
}
