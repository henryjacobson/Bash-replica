#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "los.h"

/*
typedef struct los {
        char** list;
        int size;
        int cap;
} los;
*/


los* new_los()
{
	los* ret = malloc(sizeof(los));
	ret->list = malloc(4 * sizeof(char*));
	ret->size = 0;
	ret->cap = 4;
	return ret;
}

los* copy_los(los* list)
{
	if(!list) return 0;
	los* ret = new_los();
	for(int i = 0; i < list->size; i++){
		char* cpy = malloc(strlen(list->list[i]) * sizeof(char));
		strcpy(cpy, list->list[i]);
		push_los(ret, cpy);
		free(cpy);
	}
	return ret;
}

void push_los(los* list, char* str)
{
	int size = list->size;

	if(size >= list->cap) {
		list->cap *= 2;
		list->list = (char**) realloc(
				list->list,
				list->cap * sizeof(char*));
	}

	list->size += 1;

	char* cpy = malloc(strlen(str) * sizeof(char));
	strcpy(cpy, str);
	list->list[size] = cpy;
}

void reverse_los(los* list)
{
	int size = list->size;

	for(int i = 0; i < size / 2; i++){
		char* tmp = list->list[size - (i + 1)];
		list->list[size - (i + 1)] = list->list[i];
		list->list[i] = tmp;
	}
}

void print_los(los* list)
{
	char** strs = list->list;
	int size = list->size;

	for(int i = 0; i < size; i++){
		printf("%s\n", strs[i]);
	}
}

void trim_los(los* list)
{
	for(int i = 0; i < list->size - 1; i++) {
		char* cpy = malloc(strlen(list->list[i + 1]) * sizeof(char));
		strcpy(cpy, list->list[i + 1]);
		list->list[i] = cpy;
	}
	list->size -= 2;
	list->list[list->size] = 0;
}

void free_los(los* list)
{
	for(int i = 0; i < list->size; i++){
		free(list->list[i]);
	}
	free(list->list);
	free(list);
}

char*
next_alphas(char* line, int start)
{
	int i = start;
	for(; isalnum(line[i]) ||
			line[i] == '/' || 
			line[i] == '.' ||
			line[i] == '_'; i++);
	char* ret = alloca((i - start + 1) * sizeof(char));
	strncpy(ret, line + start, i - start);
	ret[i - start] = '\0';
	return ret;
}

los*
separate(char* line)
{
	los* list = new_los();

	char* token = alloca(100);
	char single[2] = " ";
	int i = 0;
	int n = strlen(line);
	
	while(i < n){
		if(isspace(line[i])) {
			i++;
			continue;
		}

		if(isalnum(line[i]) ||
				line[i] == '.' ||
				line[i] == '/') {
			token = next_alphas(line, i);
			char* cpy = alloca(strlen(token) * sizeof(char) + 1);
			strcpy(cpy, token);
			push_los(list, cpy);
			i = i + strlen(cpy);
			continue;
		}

		if(line[i] == ';') {
			push_los(list, ";");
			i++;
			continue;
		}

		if(line[i] == '&' && line [i + 1] == '&'){
			push_los(list, "&&");
			i += 2;
			continue;
		}

		if(line[i] == '&'){
                        push_los(list, "&");
                        i ++;
                        continue;
                }


		if(line[i] == '|' && line [i + 1] == '|'){
                        push_los(list, "||");
                        i += 2;
                        continue;
                }

		if(line[i] == '|'){
                        push_los(list, "|");
                        i ++;
                        continue;
                }

		if(line[i] == '('){
                        push_los(list, "(");
                        i ++;
                        continue;
                }

		if(line[i] == ')'){
                        push_los(list, ")");
                        i ++;
                        continue;
                }

		if(line[i] == '-'){
			char* cpy = malloc(2 * sizeof(char));
			strncpy(cpy, line + i, 2);
			cpy[2] = '\0';
			push_los(list, cpy);
			free(cpy);
			token = "";
			i += 2;
			continue;
		}

		strncpy(token, line + i, 1);
		token[1] = '\0';
		char* cpy = alloca(2 * sizeof(char));
                strcpy(cpy, token);
                push_los(list, cpy);
                //free(cpy);
		token = "";
		i++;
	}
	return list;
}


