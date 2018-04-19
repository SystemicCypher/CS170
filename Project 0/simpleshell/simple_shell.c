#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512

// Simple implementation for Shell command
// Assume all arguments are seperated by space
// Erros are ignored when executing fgets(), fork(), and waitpid().

/**
 * Sample session
 *  shell: echo hello
 *   hello
 *   shell: ls
 *   Makefile  simple  simple_shell.c
 *   shell: exit
**/

void runcommand(char* command, char** args) {
  pid_t pid = fork();
  if(pid) { // parent
    	waitpid(pid, NULL, 0);
  } else { // child
      int newfd;
      for(int i = 0; args[i] != 0; i++) {
        if(strcmp(args[i], "<") == 0) {
          if((newfd = open(args[i+1], O_CREAT|O_RDONLY, 0644)) < 0) {
            perror(args[i+1]);
            exit(1);
          }
          close(0);
          dup2(newfd, 0);
          close(newfd);
          for(int j = i; args[j] != 0; j++)
            args[j] = args[j+2];
        }
        if(strcmp(args[i], ">") == 0) {
          if((newfd = open(args[i+1], O_CREAT|O_WRONLY, 0644)) < 0) {
            perror(args[i+1]);
            exit(1);
          }
          close(1);
          dup2(newfd, 1);
          close(newfd);
          for(int j = i; args[j] != 0; j++)
            args[j] = args[j+2];
        }
      }
    	execvp(command, args);
  }
}

int main(){
    char line[MAX_LINE_LENGTH];
    //printf("shell: ");
    while(fgets(line, MAX_LINE_LENGTH, stdin)) {
    	// Build the command and arguments, using execv conventions.
    	line[strlen(line)-1] = '\0'; // get rid of the new line
    	char* command = NULL;
    	char* arguments[MAX_TOKEN_COUNT];
    	int argument_count = 0;

    	char* token = strtok(line, " ");
    	while(token) {
      		if(!command) command = token;
      		arguments[argument_count] = token;
	      	argument_count++;
      		token = strtok(NULL, " ");
    	}
    	arguments[argument_count] = NULL;
	if(argument_count>0){
		if (strcmp(arguments[0], "exit") == 0)
            		exit(0);
    		runcommand(command, arguments);
	}
        //printf("shell: ");
    }
    return 0;
}