#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "los.h"
#include "parse.h"

int
execute(exec_ast* ast);

void
assert_ok(int rv, char* call)
{
	if(rv == -1) {
		fprintf(stderr, "Failed call: %s\n", call);
		perror("Error:");
		exit(1);
	}
}

int
execute_rin(exec_ast* ast)
{
	int cpid;

	if((cpid = fork())) {
		// in parent
		int status;

		// wait on child
		waitpid(cpid, &status, 0);

		// return child exit code
		return WEXITSTATUS(status);
	} else {
		// in child

		// open file
		int fd = open(ast->file,
			       	O_CREAT | O_RDONLY,
				0644);
		assert_ok(fd, "open");

		// replace stdin with file
		close(0);
		dup(fd);
		close(fd);

		// execute command with input
		// redirected
		int ev;
		ev = execute(ast->exec1);
		exit(ev);
	}
}

int
execute_rout(exec_ast* ast)
{
	int cpid;

	if((cpid = fork())) {
		// in parent
		int status;

		// wait for child
		waitpid(cpid, &status, 0);

		// return child exit code
		return WEXITSTATUS(status);
	} else {
		// in child
		
		// open file
		int fd = open(ast->file,
			       	O_CREAT | O_WRONLY | O_TRUNC,
				0644);
		assert_ok(fd, "open");
		
		// replace stdout with file
		close(1);
		dup(fd);
		close(fd);

		// execute command with output
		// redirected
		int ev;
		ev = execute(ast->exec1);
		exit(ev);
	}
}

int execute_pipe(exec_ast* ast)
{
	int cpid1, cpid2, rv;

	if((cpid1 = fork())) {
		// in parent
		int status;

		// wait on child
		waitpid(cpid1, &status, 0);

		// return chuld exit code
		return WEXITSTATUS(status);
	} else {
		int pipes[2];
		rv = pipe(pipes);
		assert_ok(rv, "pipe");
		// pipes[0] reads
		// pipes[1] writes

		if((cpid2 = fork())) {
			// in child-parent
			
			// close unecessary pipe end
			close(pipes[1]);

			// replace stdin with
			// read end of pipe
			close(0);
			dup(pipes[0]);
			close(pipes[0]);

			// execute command with
			// input redirected
			int cStatus;
			cStatus = execute(ast->exec2);
			exit(cStatus);
		} 
		
		else {
			// in child-child

			// close unnecessary pipe end
			close(pipes[0]);

			// replace stdout with
			// write end of pipe
			close(1);
			dup(pipes[1]);
			close(pipes[1]);
			
			// execute command with
			// output redirected
			exit(execute(ast->exec1));
		}
	}
}

int 
execute_bg(exec_ast* ast)
{
	int cpid;

	if((cpid = fork())) {
		// in parent
		exec_ast* other = ast->exec2;

		// if cmd is empty do nothing
		if (!strcmp(other->type, "=") &&
				other->cmd->size == 0)
			return 0;

		// execute other command
		// without waiting
		return execute(other);
	} 
	
	else {
		// in child
		int ev;

		// execute command
		ev = execute(ast->exec1);
		exit(ev);
	}	
}

int
execute_semi(exec_ast* ast)
{
	int first, second;

	// execute first command
	first = execute(ast->exec1);

	// if first command isn't exit,
	// execute second command
	if(first != -1) second =  execute(ast->exec2);

	// prioritize non-zero return value
	// or second return value
	if(!second) return first;
	return second;
}

int
execute_and(exec_ast* ast)
{
	// execute first command
	if(!execute(ast->exec1)) {
		// if first is successful,
		// execute second command
		return execute(ast->exec2);
	}
	return 0;
}

int
execute_or(exec_ast* ast)
{
	// execute second command
	if(execute(ast->exec1)) {
		// if first command is unseccessful,
		// execute second command
		return execute(ast->exec2);
	}

	return 0;
}

int
execute(exec_ast* ast)
{
	// if command has an operator,
	// use appropriate helper-function

	if(!strcmp(ast->type, "<")) {
		return execute_rin(ast);
	}

	if(!strcmp(ast->type, ">")) {
		return execute_rout(ast);
	}

	if(!strcmp(ast->type, "|")) {
		return execute_pipe(ast);
	}

	if(!strcmp(ast->type, "&")) {
		return execute_bg(ast);
	}

	if(!strcmp(ast->type, ";")) {
		return execute_semi(ast);
	}

	if(!strcmp(ast->type, "&&")) {
		return execute_and(ast);
	}

	if(!strcmp(ast->type, "||")) {
		return execute_or(ast);
	}

	if (!strcmp(ast->cmd->list[0], "cd")) {
		chdir(ast->cmd->list[1]);
		return 0;
	}

	if (!strcmp(ast->cmd->list[0], "exit")) {
		return -1;
	}

	if (ast->cmd->size == 0) {
		return 0;
	}

	int cpid;

	if ((cpid = fork())) {
	        // in parent

	        int status;
		// wait on child
	        waitpid(cpid, &status, 0);

		// return child exit code
		return WEXITSTATUS(status);
	} 
	
	else {
		// in child, execute command
		execvp(ast->cmd->list[0], ast->cmd->list);
		assert_ok(-1, "execvp");
	}
}

int
main(int argc, char* argv[])
{
	char* line = malloc(256 * sizeof(char));
	FILE* file;

	// if a file is given, open the file
	if (argc == 2) {
		file = fopen(argv[1], "r");
	} 
	
	// else just use stdin
	else {
		file = stdin;
	}

	while(1){
		// print prompt for stdin use
	        if(file == stdin) printf("nush$ ");

		// read line from file or stdin
	        char* rv = fgets(line, 256, file);

		// if nothing is read, break loop
		if (!rv) break;
		fflush(file);

		// tokenize line of text
		los* cmd = separate(line);

		// parse tokens to ast
		exec_ast* ast = parse(cmd);

		// execute command tree
		int rv2 = execute(ast);
		if(rv2 == -1) break;

		free_los(cmd);
		free_exec_ast(ast);
	}

	free(line);
	return 0;
}
