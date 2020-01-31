#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGUMENT_DELIMITERS " \n\t\r"
#define ARGUMENT_BUFFER_SIZE 64;

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

	char **tokens = malloc(buffer_size * sizeof(char*));

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

	if (args[0] == NULL) {
		// No command entered
		return 1;
	}

	// TODO: Execute built-in command if possible

	return execute_program(args);

}

// Configure the environment for the shell
void initialize() {
	// Open .SEESHrc
	// Run commands
}

// Loop: { read and parse user input, execute relevant command }
void interpret(int argc, char **argv) {
	char *line;
	char **args;
	int status;

	do {
		printf("? ");
		line = read_input();
		args = tokenize_input(line);
		status = execute(args);

		free(line);
		free(args);
	} while (status);
}

int main(int argc, char **argv) {
	initialize();

	interpret(argc, argv);

	return EXIT_SUCCESS;
}