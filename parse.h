#ifndef PARSE_H
#define PARSE_H

#include "los.h"

typedef struct exec_ast {
	char* type;
	// Type is either:
	//   one of: <, >, |, &, &&, ||, ;
	//   = for simple exec
	struct exec_ast* exec1;
	struct exec_ast* exec2;
	char* file;
	los* cmd;
} exec_ast;

exec_ast* make_exec_ast(char* type, exec_ast* exec1, exec_ast* exec2, char* file, los* cmd);

void free_exec_ast(exec_ast* ast);

exec_ast* parse(los* line);

#endif
