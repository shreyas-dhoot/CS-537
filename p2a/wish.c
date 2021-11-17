#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>


char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    //printf("Concatenated string %s", result);
    return result;
}

char *paths[100];

char error_message[30] = "An error has occurred\n";

void addPaths(char *line) {
	//char *command = strsep(&line, " ");
	//printf("Command %s\n", command);
	char *token  = strsep(&line, " ");
	int counter = 0;
	if(token == NULL) {
		paths[0] = NULL;
		//write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}

	while (token != NULL) {
	//	printf("Token %s\n", token);
		if(token[strlen(token)-1] != '/'){
			paths[counter] = concat(token, "/");
		}
		else {
			paths[counter] = strdup(token);
		}
		//printf("Token %s\n", paths[counter]);
		token = strsep(&line, " ");
		counter++;
	}
	paths[counter] = NULL;
}

char *getCommand(char *command) {
	int counter = 0;
	char *fullCommand = NULL;
	while(paths[counter] != NULL) {
		//printf("%s %s\n", paths[counter], command);
		fullCommand = concat(paths[counter], command);
		//printf("Full Command Options %s\n", fullCommand);
		if(access(fullCommand, X_OK) == 0) {
			//printf("Working path present\n");
			return fullCommand;
		}
		counter++;
	}
	return NULL;
}

void checkPaths() {
	int counter = 0;
	while(paths[counter] != NULL) {
		//printf("Path %d : %s\n", counter, paths[counter]);
		counter++;
	}
}

void runLoop(char *line_buf, int i) {
	//printf("%s", line_buf);
	pid_t  pid = fork();
		
	if(pid < 0) {
		printf("Error occured\n");
		exit(1);
	}
	else if(pid == 0) {
		char *command = strsep(&line_buf, " ");
		if (strcmp(command, "$loop") == 0) {
			// Only 4 digit numbers
			command = malloc(sizeof(char) * 5);
			sprintf(command, "%d", i);
		}
		//printf("Command %s\n", command);
		char *token  = strsep(&line_buf, " ");
		int counter = 0;
		char *args[10];
		args[counter] = command;
		counter++;
		char *fileName = NULL;
		if(token == NULL) {
			//printf("Token is NULL\n");
		}
		while (token != NULL) {
			//printf("Token %s\n", token);
			if (strcmp(token, ">") == 0) {
				fileName = strsep(&line_buf, " ");
				break;
			}
			if (strcmp(token, "$loop") == 0) {
				// Only 4 digit numbers
				token = malloc(sizeof(char) * 5);
				sprintf(token, "%d", i);
			}
			args[counter] = token;
			token = strsep(&line_buf, " ");
			counter++;
		}
		//printf("Output file name %s\n", fileName);
		if (fileName != NULL) {
			close(STDOUT_FILENO);
			open(fileName, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
		}
		args[counter] = NULL;
		char *workingPath = getCommand(command);
		if(workingPath == NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		else {
			//printf("Working Path %s", workingPath);
			execv(workingPath, args);
		}
		exit(0);
	}
	else {
		wait(NULL);
	}
}

int main(int argc, char *argv[]) {
	char *inputFile = NULL;
	FILE *fp;
	if(argc > 2) {
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	if (argc == 2) {
		inputFile = argv[1];
		fp = fopen(inputFile, "r");
		if (fp == NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
	}
	else {
		fp = stdin;
	}
	char *line_buf = NULL;
	paths[0] = "/bin/";
	paths[1] = NULL;
	size_t line_buf_size = 0;
	pid_t  pid;
	int linelen = 0;
	if (fp == stdin) {
		printf("wish> ");
	}
	while((linelen = getline(&line_buf, &line_buf_size, fp)) > 0) {

		if(line_buf[strlen(line_buf) - 1] == '\n') {
			line_buf[strlen(line_buf) - 1] = '\0';
		}

		//printf("Command : %s\n", line_buf);
		while(isspace((unsigned char)*line_buf)) line_buf++;

		char *end = line_buf + strlen(line_buf) - 1;
  		while(end > line_buf && isspace((unsigned char)*end)) end--;

  		// Write new null terminator character
  		end[1] = '\0';
		//printf("Command : %s\n", line_buf);
		
		
		
		char *command = strsep(&line_buf, " ");

		if (strcmp(command, "exit") == 0) {
			if(strsep(&line_buf, " ") != NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				if (fp == stdin) {
					printf("wish> ");
				}
				continue;
			}
			exit(0);
		}
		
		else if (strcmp(command, "path") == 0) {
			addPaths(line_buf);
			checkPaths();
			if (fp == stdin) {
				printf("wish> ");
			}
			continue;
		}
		else if (strcmp(command, "loop") == 0) {
			//printf("Reached inside loop");
			char *number = strsep(&line_buf, " ");
			if(number == NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				if (fp == stdin) {
					printf("wish> ");
				}
				continue;
			}
			char *ptr;
			int iteration = strtol(number, &ptr, 10);
			if (iteration == 0 || iteration < 0) {
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
			else {
				//printf("Iteration %d\n", iteration);
				for (int i = 1; i <= iteration; i++) {
					runLoop(line_buf, i);
				}
			}
			if (fp == stdin) {
				printf("wish> ");
			}
			continue;
		}

		else if (strcmp(command, "cd") == 0) {
			char *path = strsep(&line_buf, " ");
			if(path == NULL || strsep(&line_buf, " ") != NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				if (fp == stdin) {
					printf("wish> ");
				}
				continue;
			}
			//printf("The path parsed is %s\n", path);
			if(chdir(path) != 0){
			       	write(STDERR_FILENO, error_message, strlen(error_message));
			}
			if (fp == stdin) {
				printf("wish> ");
			}
			continue;
		}

		//printf("%s", line_buf);
		pid = fork();
		
		if(pid < 0) {
			printf("Error occured\n");
			exit(1);
		}
		else if(pid == 0) {
			//printf("Command %s\n", command);
			//printf("Path command : %s", paths[0]);
			char *token  = strsep(&line_buf, " ");
			int counter = 0;
			char *args[10];
			//printf("Path command : %s", paths[0]);
			args[counter] = command;
			counter++;
			char *fileName = NULL;
			if(token == NULL) {
				//printf("Token is NULL\n");
			}
			while (token != NULL) {
				//printf("Token %s\n", token);
				char *ptr = NULL;
				if (strcmp(token, ">") == 0) {
					fileName = strsep(&line_buf, " ");
					if((fileName == NULL) || (strsep(&line_buf, " ") != NULL)) {
						write(STDERR_FILENO, error_message, strlen(error_message));
						exit(0);
					}
					break;
				}
				else if(strstr(token, ">") != NULL) {
					ptr = strdup(token);
					token = strsep(&ptr, ">");
					printf("First arg with > %s\n", token);
					fileName = strsep(&ptr, ">");
					printf("File name > %s\n", fileName);
				}
				args[counter] = token;
				token = strsep(&line_buf, " ");
				counter++;
			}
			//printf("Output file name %s\n", fileName);
			if (fileName != NULL) {
				close(STDOUT_FILENO);
				open(fileName, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
			}
			args[counter] = NULL;
			checkPaths();
			char *workingPath = getCommand(command);
			if(workingPath != NULL) {
				//printf("Working Path %s", workingPath);
				execv(workingPath, args);
			}
			else {
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
			exit(0);
		}
		else {
			wait(NULL);
			if (fp == stdin) {
				printf("wish> ");
			}
		}
	}
	return 0;
}
