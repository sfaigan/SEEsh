#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGUMENT_DELIMITERS " \n\t\r"
#define ARGUMENT_BUFFER_SIZE 64

extern char **environ;

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
	return 1;
}

// Get input from stdin, return it as a line
char *read_input(void) {
	char *line = NULL;
	ssize_t buffer_size = 0;
	getline(&line, &buffer_size, stdin);
	return line;
}

// Tokenize a given line, return array of tokens
char **tokenize_input(char *line) {
	int buffer_size = ARGUMENT_BUFFER_SIZE;
	int position = 0;

	char **tokens = calloc(buffer_size, sizeof(char*));

	if (!tokens) {
		fprintf(stderr, "SEEsh: There was a problem allocating memory. Exiting...\n");
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
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, ARGUMENT_DELIMITERS);
	}
	tokens[position] = NULL;
	return tokens;
}

int execute_program(char **args) {
	pid_t pid, wpid;
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
			wpid = waitpid(pid, &status, WUNTRACED);
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
	FILE *fp = fopen(".SEEshrc", "r");

	if (fp == NULL) {
		fprintf(stderr, "SEEsh: Error opening .SEEshrc. Exiting...");
		exit(1);
	}

	char line[512];
	char **args;
	int status;

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
		if ("line" == NULL) {
			break;
		}
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

/*
TODO:
- implement ctrl+d to exit
- create makefile
- write readme
- test
*/