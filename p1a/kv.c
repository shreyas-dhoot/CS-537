#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

struct Commands {
	char *p;
	int index;
	char *value;
};

struct Node {
	int key;
	char *value;
	struct Node *nextNode;
};

int lastIndex = 0;
struct Node *start = NULL;
struct Node *end = NULL;

void deleteValue(int index, int isPrint) {
	struct Node *temp = start;
	struct Node *prev = NULL;
	if (start == NULL) {
		return;
	}
	if (start -> key == index) {
		start = start -> nextNode;
		return;
	}
	while(temp != NULL) {
		if (temp -> key == index) {
			prev -> nextNode = temp -> nextNode;
			free(temp);
			return;
		}
		prev = temp;
		temp = temp->nextNode;
	}
	if(isPrint == 1) {
		printf("%d not found\n", index);
	}
}

void addNodeToList(int index, char *value) {
	deleteValue(index, 0);
	struct Node *temp = (struct Node *) malloc(sizeof(struct Node));
	temp -> key = index;
	temp -> value = value;
	temp -> nextNode = NULL;
	if (start == NULL) {
		start = temp;
	}
	else {
		temp -> nextNode = start;
		start = temp;
	}
}

void loadFileToDatabaseAndInitialize() {
	FILE *fp;
	fp = fopen("./database.txt", "r");
	if (fp == NULL) {
		return;
	}
	char *line_buf = NULL;
	size_t line_buf_size = 0;
	int linelen = 0;
	while((linelen = getline(&line_buf, &line_buf_size, fp)) > 0) {
		int index = atoi(strsep(&line_buf, ","));
		char *value = strsep(&line_buf, " \t");
		if(value[strlen(value) - 1] == '\n') {
			value[strlen(value) - 1] = '\0';
		}
		addNodeToList(index, value);
		//printf("Content %s Length of string : %d Line : %d", start->value, strlen(start->value), start->key);
	}
}



void findValue(int index) {
	struct Node *temp = start;
	while(temp != NULL) {
		if (temp -> key == index) {
			printf("%d,%s\n", temp -> key, temp -> value);
			return;
		}
		temp = temp->nextNode;
	}
	printf("%d not found\n", index);
}

void printAllValues() {
	struct Node *temp = start;
	while(temp != NULL) {
		printf("%d,%s\n", temp->key, temp->value);
		temp = temp->nextNode;
	}
}

void executeCommand(struct Commands com) {
	if (strcmp(com.p, "p") == 0) {
		if(com.value == NULL) {
			printf("bad command\n");
			return;
		}
		addNodeToList(com.index, com.value);
	}
	else if(strcmp(com.p, "g") == 0) {
		if(com.index == -1 || com.value != NULL) {
			printf("bad command\n");
			return;
		}
		findValue(com.index);
	}
	else if(strcmp(com.p, "d") == 0) {
		if(com.index == -1 || com.value != NULL) {
			printf("bad command\n");
			return;
		}
		deleteValue(com.index, 1);
	}
	else if(strcmp(com.p, "c") == 0) {
		start = NULL;
	}
	else if(strcmp(com.p, "a") == 0) {
		printAllValues();
	}
	else {
		printf("bad command\n");
	}
}

void saveDatabaseToFile() {
	FILE *fp;
	fp = fopen("./database.txt", "w+");
	struct Node *temp = start;
	while(temp != NULL) {
		//struct Node actualNode = *temp;
		fprintf(fp, "%d,%s\n", temp->key, temp->value);
		temp = temp->nextNode;
	}
	fclose(fp);
}

int main(int argc, char *argv[]) {
	/*if (argc <= 1) {
		printf("Bad Command\n");
		exit(1);
	}*/
	assert(argv != NULL);
	loadFileToDatabaseAndInitialize();
	int i = 1;
	while(argv[i] != NULL) {
		char *command;
		//printf("%s", strsep(&argv[i], ","));
		struct Commands com;
		com.p = strsep(&argv[i], ",");
		com.index = -1;
		com.value = NULL;
		if((command = strsep(&argv[i], ",")) != NULL) {
			char *ptr;
			if (strcmp(command, "0") == 0) {
				com.index = strtol(command, &ptr, 10);
			}
			else {
				com.index = strtol(command, &ptr, 10);
				if(com.index == 0) {
					printf("bad command\n");
					i++;
					continue;
				}
			}
			//printf("The changed value is %d", com.index);
			//com.index = atoi(command);
			//printf("Int value %d", com.index);
		}
		if((command = strsep(&argv[i], ",")) != NULL) {
			com.value = command;
			//printf("%s", com.value);
		}
		executeCommand(com);
		i++;
	}
	saveDatabaseToFile();
	return 0;
}
