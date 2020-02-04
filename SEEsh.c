#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGUMENT_DELIMITERS " \n\t\r"
#define ARGUMENT_BUFFER_SIZE 64

extern char **environ;

// Linked List for command history
struct command {
	char *cmd;
	struct command* next;
	struct command* prev;
};

struct command* cmd_list;

/*!
 * \brief create Allocate memory and create new single node
 * \param elem The value of node
 * \return Pointer to the new node
 */
struct command* create(char *cmd) {
	int len = strlen(cmd);
	char *cmd_str = (char*)malloc(sizeof(char)*(len+1));
	strcpy(cmd_str, cmd);
	struct command* newCmd = (struct command*)malloc(sizeof(struct command));

	newCmd->cmd = cmd_str;
	newCmd->prev = NULL;
	newCmd->next = NULL;
	return newCmd;
}

/*!
 * \brief erase Remove single given node and free allocated memory
 * \param ref Node to remove
 * \return Next node the newly removed node
 */
struct command* erase(struct command* ref) {
	struct command* nx = ref->next;
	struct command* px = ref->prev;

	free(ref->cmd);
	free(ref);

	if(nx) {
		nx->prev = px;
	}

	if(px) {
		px->next = nx;
	}

	return nx;
}

/*!
 * \brief begin Traverse the linked-list to the head of it
 * \param ref A node from list
 * \return head of list
 */
struct command* begin(struct command* ref) {
	while(ref->prev) {
		ref = ref->prev;
	}
	return ref;
}

/*!
 * \brief end Traverse the linked-list to the tail of it
 * \param ref A node from list
 * \return tail of list
 */
struct command* end(struct command* ref) {
	while(ref->next) {
		ref = ref->next;
	}
	return ref;
}

/*!
 * \brief clear Removes all the nodes of list and free all allocated memory
 * \param ref A node from list
 */
void clear(struct command* ref) {
	ref = begin(ref);
	while((ref = erase(ref)) != NULL);
}

/*!
 * \brief push_back Append new node to the end of list
 * \param ref A node from list
 * \param newElem Value of new element
 * \return The tail of list
 */
struct command* push_back(struct command* ref, char *cmd) {

	struct command* tail = end(ref);

	struct command* newCmd = create(cmd);

	tail->next = newCmd;
	newCmd->prev = tail;

	return newCmd;
}

/*!
 * \brief pop_front Removes a node from head of list
 * \param ref A node from list
 * \return New head of list
 */
struct command* pop_front(struct command* ref) {

	struct command* head = begin(ref);

	return erase(head);
}

/*!
 * \brief unique Eliminates all but the first element from every consecutive group of equivalent elements from the list
 * \param ref A node form list
 * \return The head of the list
 */
struct command* unique(struct command* ref) {
	struct command* cur = begin(ref);
	
	while (cur->next) {
		if (strcmp(cur->cmd, cur->next->cmd) == 0) {
			erase(cur->next);
		} else {
			cur = cur->next;
		}
	}
	
	return begin(cur);
}

void add_cmd(char *cmd) {
	if (cmd[strlen(cmd)-1] == '\n') {
		cmd[strlen(cmd)-1] = '\0';
	}
	if (cmd_list == NULL) {
		cmd_list = create(cmd);
	} else {
		push_back(cmd_list, cmd);
	}
}

/*!
 * \brief print Print all elements of list following a new line
 * \param ref A node from list
 */
void print(struct command* ref) {
	struct command* cmdNode = begin(ref);

	do {
	  printf("%s\n", cmdNode->cmd);
	} while((cmdNode = cmdNode->next) != NULL);
	printf("\n");
}

// Built-in function declarations
int seesh_cd(char **args);
int seesh_pwd(char **args);
int seesh_help(char **args);
int seesh_exit(char **args);
int seesh_set(char **args);
int seesh_unset(char **args);
int seesh_history(char **args);

// Array of built-in function commands
char *builtin_str[] = {
	"cd",
	"pwd",
	"help",
	"exit",
	"set",
	"unset",
	"history"
};

char *builtin_desc[] = {
	"cd [dir]: Changes the current directory to the value of the \"dir\" parameter or HOME if no parameter is provided.",
	"pwd: Prints the current working directory.",
	"help [command]: Prints the usage and description of a given command or if no parameter is given, prints a list of commands with their descriptions and usages.",
	"exit: Exits SEEsh.",
	"set [var] [val]: Sets a given environment variable \"var\" to a given value \"val\". If nothing is given for \"val\", the environment variable called \"var\" will be set to the empty string. If no parameters are given, prints a list of all environment variables.",
	"unset [var]: Destroys a given environment variable \"var\".",
	"history: Prints the last 5 commands written. Also, type \'!\' followed by the prefix of a command and it will autocomplete based on history."
};

// Array of built-in function pointers
int (*builtin_fn[]) (char**) = {
	&seesh_cd,
	&seesh_pwd,
	&seesh_help,
	&seesh_exit,
	&seesh_set,
	&seesh_unset,
	&seesh_history
};

// Returns the number of built-in commands
int seesh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char*);
}

// Change the current directory
int seesh_cd(char **args) {
	char *dir = args[1];
	if (dir == NULL) {
		dir = getenv("HOME");
	}
	if (chdir(dir) != 0) {
		perror("SEEsh");
	}
	char *new_dir = getcwd(NULL, 0);
	if (setenv("PWD", new_dir, 1) != 0) {
		perror("SEEsh");
	}
	free(new_dir);
	return 1;
}

// Prints the current working directory
int seesh_pwd(char **args) {
	char *cwd;
	if (args[1] != NULL) {
		fprintf(stderr, "SEEsh: Was not expecting an argument to \"pwd\"\n");
	} else {
		cwd = getcwd(NULL, 0);
		if (cwd == NULL) {
			perror("SEEsh");
		}
	}
	
	puts(cwd);
	free(cwd);
	return 1;
}

// Print the help menu or help for a specific command
int seesh_help(char **args) {
	int i;
	int num_builtins = seesh_num_builtins();


	if (args[1] == NULL) {
		puts("SEEsh: A basic shell implemented in C.");
		puts("--------------------------------------");
		puts("Commands:");
		for (i = 0; i < num_builtins; i++) {
			puts(builtin_desc[i]);
		}
	} else {
		for (i = 0; i < num_builtins; i++) {
			if (strcmp(args[1], builtin_str[i]) == 0) {
				puts(builtin_desc[i]);
				break;
			}
		}
	}
	puts("");
	return 1;
}

// Exit SEEsh
int seesh_exit(char **args) {
	clear(cmd_list);
	return 0;
}

// Set an environment variable
int seesh_set(char **args) {
	char **env_ptr;
	if (args[1] == NULL) {
		for (env_ptr = environ; *env_ptr != 0; env_ptr++) {
			puts(*env_ptr);
		}
	} else if (args[2] == NULL) {
		setenv(args[1], "", 1);
	} else {
		if (setenv(args[1], args[2], 1) != 0) {
			perror("SEEsh");
		}
	}

	return 1;
}

// Unset an environment variable
int seesh_unset(char **args) {
	if (args[1] == NULL || args[2] != NULL) {
		fprintf(stderr, "SEEsh: was expecting one parameter exactly.\n");
	} else {
		if (unsetenv(args[1]) != 0) {
			perror("SEEsh");
		}
	}

	return 1;
}

// Show command history
int seesh_history(char **args) {
	print(cmd_list);
	return 1;
}

// Get input from stdin, return it as a line
char *read_input(void) {
	char *line = NULL;
	size_t buffer_size = 0;
	int status = getline(&line, &buffer_size, stdin);
	if (status == -1) {
		free(line);
		if (cmd_list) clear(cmd_list);
		exit(1);
	}
	return line;
}

// Tokenize a given line, return array of tokens
char **tokenize_input(char *line) {
	int buffer_size = ARGUMENT_BUFFER_SIZE;
	int position = 0;

	char **tokens = calloc(buffer_size, sizeof(char*));

	if (!tokens) {
		fprintf(stderr, "SEEsh: There was a problem allocating memory. Exiting...\n");
		if (cmd_list) clear(cmd_list);
		exit(EXIT_FAILURE);
	}

	char *token = strtok(line, ARGUMENT_DELIMITERS);
	while (token != NULL) {
		tokens[position++] = token;

		if (position >= buffer_size) {
			buffer_size += ARGUMENT_BUFFER_SIZE;
			tokens = realloc(tokens, buffer_size * sizeof(char*));

			if (!tokens) {
				fprintf(stderr, "SEEsh: There was a problem allocating memory. Exiting...\n");
				if (cmd_list) clear(cmd_list);
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, ARGUMENT_DELIMITERS);
	}
	tokens[position] = NULL;
	return tokens;
}

int execute_program(char **args) {
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		// Child process
		if (execvp(args[0], args) == -1) {
			perror("SEEsh");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// Error forking
		perror("SEEsh");
	} else {
		// Parent process
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int execute(char **args) {
	int i;
	int num_builtins = seesh_num_builtins();

	if (args[0] == NULL) {
		// No command entered
		return 1;
	}

	for (i = 0; i < num_builtins; i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			 return (*builtin_fn[i])(args);
		}
	}

	return execute_program(args);
}

// Configure the environment for the shell
void initialize() {
	char *dir = getenv("HOME");
	char filepath[512];
	strcpy(filepath, dir);
	strcat(filepath, "/.SEEshrc");
	puts(filepath);
	FILE *fp = fopen(filepath, "r");

	if (fp == NULL) {
		fprintf(stderr, "SEEsh: Error opening .SEEshrc. Is it in the HOME directory?\n\n");
		return;
	}

	char line[512];
	char **args;

	puts("Reading from .SEEshrc...");
	while (fgets(line, sizeof(line), fp)) {
		printf("\t%s", line);
		args = tokenize_input(line);
		if (execute(args) != 1) {
			fprintf(stderr, "SEEsh: Error running the following command from .SEEshrc:\n%s", line);
		}
		free(args);
	}
	printf("\nDone!\n\n");
	fclose(fp);
}

// Loop: { read and parse user input, execute relevant command }
void interpret(int argc, char **argv) {
	char *line;
	char **args;
	int status;

	do {
		printf("? ");
		line = read_input();
		add_cmd(line);
		args = tokenize_input(line);
		status = execute(args);

		free(line);
		free(args);
	} while (status);
}

int main(int argc, char **argv, char **envp) {
	initialize();

	signal(SIGINT, SIG_IGN);

	interpret(argc, argv);

	return EXIT_SUCCESS;
}