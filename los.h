#ifndef LOS_H
#define LOS_H

typedef struct los {
	char** list;
	int size;
	int cap;
} los;

los* new_los();
los* copy_los(los* list);
void push_los(los* list, char* str);
void reverse_los(los* list);
void print_los(los* list);
void trim_los(los* list);
void free_los(los* list);

char* next_alphas(char* line, int start);
los* separate(char* line);

#endif
