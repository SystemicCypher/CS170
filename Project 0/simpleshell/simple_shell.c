#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

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

// Function prototypes
void runcommandwithpipe(char* command1, char* command2, char** args1, char** args2, int pfd[]);
void runcommand(char* command, char** args);

// Helper function for a command that has pipes within them
void runcommandwithpipe(char* command1, char* command2, char** args1, char** args2, int pfd[]) {
  // Forks the parent will run the second command and the child will run the first command
  // We want the first command to run first so that the output will be given to the input of the second command
  pid_t pid = fork();

  if(pid) { // parent
    // Wait for child to finish running its command and then queues the second command to runcommand after correctly assigning file descriptors
    waitpid(pid, NULL, 0);
    dup2(pfd[0], 0);
    close(pfd[0]);
    close(pfd[1]);
    runcommand(command2, args2);
  } else { //child
    // Correctly assigns file descriptors and queues its command to runcommand. After run command finishs, exits so the parent can run.
    dup2(pfd[1], 1);
    close(pfd[0]);
    close(pfd[1]);
    runcommand(command1, args1);
    exit(0);
  }
}

void runcommand(char* command, char** args) {
  // Flag for whether there is a pipe and first to find the first pipe
  int flag = 0;
  int first = 0;

  // Initialize command1, command2, args1, args2 if we need to split the commands into two commands in the case when there is a pipe present in the command
  char* command1 = NULL;
  char* command2 = NULL;
  char* args1[MAX_TOKEN_COUNT];
  char* args2[MAX_TOKEN_COUNT];
  bzero(args1, MAX_TOKEN_COUNT);
  bzero(args2, MAX_TOKEN_COUNT);

  // Finds a pipe if it exists and splits command and args into command1, command2, args1, args2. Sets flag to 1 and first to 1 if pipe is found
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
    // If pipe exists we run pipe on a file descriptor to get file descriptors for input and output
    // Fork so the child runs the helper function for commands with pipes and the parent will wait for the child to finish
    pid_t pipeid = fork();

    int pipefd[2];
    pipe(pipefd);
    if(pipeid) { //parent
      waitpid(pipeid, NULL, 0);
    }
    else { // child
      runcommandwithpipe(command1, command2, args1, args2, pipefd);
      exit(0);
    }
  }
  else {
    // If pipe does not exist in the command, we check if there is input/output redirection
    // If we find input/output redirection, we change the appropriate file descriptors and correctly change the arguments list
    // Then call execvp on the command
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

// Signal handler function to handle when ctrl-z is pressed. If pressed twice, the process will stop.
void handler(int sig) {
  static int count = 0;
  printf("Enter CTRL-Z again to exit\n");
  count++;
  if(count == 1)
    signal(SIGTSTP, SIG_DFL);
}

int main(){
  // Changes the default SIGTSTP behavior to our custom handler
  signal(SIGTSTP, handler);

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
