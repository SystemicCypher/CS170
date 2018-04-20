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
void runcommandwithpipe(char* command1, char* command2, char** args1, char** args2, int pfd[]);
void runcommand(char* command, char** args);

void runcommandwithpipe(char* command1, char* command2, char** args1, char** args2, int pfd[]) {
  pid_t pid = fork();

  if(pid) { // parent
    dup2(pfd[0], 0);
    close(pfd[1]);
    runcommand(command2, args2);
  } else { //child
    dup2(pfd[1], 1);
    close(pfd[0]);
    execvp(command1, args1);
  }
}

void runcommand(char* command, char** args) {
  int flag = 0;
  int first = 0;

  char* command1 = NULL;
  char* command2 = NULL;
  char* args1[MAX_TOKEN_COUNT];
  char* args2[MAX_TOKEN_COUNT];
  bzero(args1, MAX_TOKEN_COUNT);
  bzero(args2, MAX_TOKEN_COUNT);

  for(int i = 0; args[i] != 0; i++) {
    if(strcmp(args[i], "|") == 0) {
      if(first == 0) {
        for(int j = 0; j < i; j++) {
          args1[j] = args[j];
        }
        for(int k = i + 1; args[k] != 0; k++) {
          args2[k - i - 1] = args[k];
        }
        command1 = args1[0];
        command2 = args2[0];
        flag = 1;
        first = 1;
      }
    }
  }
  if(flag) {
    int pipefd[2];
    pipe(pipefd);
    pid_t pipeid = fork();
    if(pipeid) { // parent
      waitpid(pipeid, NULL, 0);
    }
    else { // child
      runcommandwithpipe(command1, command2, args1, args2, pipefd);
      exit(0);
    }
  }
  else {
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
  	i--;
        }
        else if(strcmp(args[i], ">") == 0) {
  	if((newfd = open(args[i+1], O_CREAT|O_WRONLY, 0644)) < 0) {
  	  perror(args[i+1]);
  	  exit(1);
  	}
  	close(1);
  	dup2(newfd, 1);
  	close(newfd);
  	for(int j = i; args[j] != 0; j++)
  	  args[j] = args[j+2];
  	i--;
        }
      }
      execvp(command, args);
    }
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
