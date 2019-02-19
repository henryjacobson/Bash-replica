#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "parse.h"
#include "los.h"

/*
typed exec_ast {
	char* type;
	// Type is either:
	//   one of: <, >, |, &, &&, ||, ;
	//   = for simple exec
	exec_ast* exec1;
	exec_ast* exec2;
	char* file;
	los* cmd;
} exec_ast;
*/

exec_ast* make_exec_ast(char* type, exec_ast* exec1, exec_ast* exec2, char* file, los* cmd)
{
	exec_ast* ast = malloc(sizeof(exec_ast));
	ast->type = type;
	ast->exec1 = exec1;
	ast->exec2 = exec2;
	if(file) {
		char* cpy = malloc(strlen(file) * sizeof(char));
		strcpy(cpy, file);
		ast->file = cpy;
	} else {
		ast->file = 0;
	}
	ast->cmd = copy_los(cmd);
}

void free_exec_ast(exec_ast* ast)
{
	if(ast->file) free(ast->file);
	free(ast);
}

int contains(los* cmd, char* str){
	int parens = 0;
	for(int i = 0; i < cmd->size; i++){
		if(!strcmp(cmd->list[i], "("))
			parens++;
		if(!strcmp(cmd->list[i], ")"))
			parens--;
		if(!strcmp(cmd->list[i], str) && 
				parens == 0) 
			return 1;
	}
	return 0;
}

int find_first(los* cmd, char* str){
	int parens = 0;
        for(int i = 0; i < cmd->size; i++){
		if(!strcmp(cmd->list[i], "("))
			parens++;
		if(!strcmp(cmd->list[i], ")"))
			parens--;
                if(!strcmp(cmd->list[i], str) &&
				parens == 0) {
                        return i;
                }
        }
        return 0;
}

los* slice(los* src, int st, int fin){
	los* ret = new_los();
	if(st == fin) {
		return ret;
	}
	for(int i = st; i < fin; i++){
		char* cpy = malloc(strlen(src->list[i]) * sizeof(char));
		strcpy(cpy, src->list[i]);
		push_los(ret, cpy);
		free(cpy);
	}
	return ret;
}

exec_ast* parse(los* cmd)
{
	if(contains(cmd, ";")) {
		int i = find_first(cmd, ";");
		los* xs = slice(cmd, 0, i);
		los* ys = slice(cmd, i + 1, cmd->size);
		exec_ast* ast = make_exec_ast(
				";",
				parse(xs),
				parse(ys),
				0, 0);
		free_los(xs);
		free_los(ys);
		return ast;
	}
	
	if(contains(cmd, "||")) {
		int i = find_first(cmd, "||");
		los* xs = slice(cmd, 0, i);
		los* ys = slice(cmd, i + 1, cmd->size);
		exec_ast* ast = make_exec_ast(
				"||",
				parse(xs),
				parse(ys),
				0, 0);
		free_los(xs);
		free_los(ys);
		return ast;
	}
	
	if(contains(cmd, "&&")) {
		int i = find_first(cmd, "&&");
		los* xs = slice(cmd, 0, i);
		los* ys = slice(cmd, i + 1, cmd->size);
		exec_ast* ast = make_exec_ast(
				"&&",
				parse(xs),
				parse(ys),
				0, 0);
		free_los(xs);
		free_los(ys);
		return ast;
	}
	
	if(contains(cmd, "&")) {
		int i = find_first(cmd, "&");
		los* xs = slice(cmd, 0, i);
		los* ys = slice(cmd, i + 1, cmd->size);
		exec_ast* ast = make_exec_ast(
				"&",
				parse(xs),
				parse(ys),
				0, 0);
		free_los(xs);
		return ast;
	}
	
	if(contains(cmd, "|")) {
		int i = find_first(cmd, "|");
		los* xs = slice(cmd, 0, i);
		los* ys = slice(cmd, i + 1, cmd->size);
		exec_ast* ast = make_exec_ast(
				"|",
				parse(xs),
				parse(ys),
				0, 0);
		free_los(xs);
		free_los(ys);
		return ast;
	}
	
	if(contains(cmd, ">")) {
		int i = find_first(cmd, ">");
		los* xs = slice(cmd, 0, i);
		los* ys = slice(cmd, i + 1, cmd->size);
		exec_ast* ast = make_exec_ast(
				">",
				parse(xs), 0,
				ys->list[0], 0);
		free_los(xs);
		free_los(ys);
		return ast;
	}
	
	if(contains(cmd, "<")) {
		int i = find_first(cmd, "<");
		los* xs = slice(cmd, 0, i);
		los* ys = slice(cmd, i + 1, cmd->size);
		exec_ast* ast = make_exec_ast(
				"<",
				parse(xs), 0,
				ys->list[0], 0);
		free_los(xs);
		free_los(ys);
		return ast;
	}

	if(cmd->size != 0 && 
			!strcmp(cmd->list[0], "(") &&
			!strcmp(cmd->list[cmd->size - 1], ")")) {
		trim_los(cmd);
		return parse(cmd);
	}

	exec_ast* ast = make_exec_ast("=", 0, 0, 0, cmd);
	return ast;
}

