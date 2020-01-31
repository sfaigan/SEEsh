#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

// Configure the environment for the shell
void initialize() {
	// Open .SEESHrc
	// Run commands
}

// Loop: { read and parse user input, execute relevant command }
void interpret(int argc, char **argv) {
	char *line;
	char **args;
	int status = 1;

	while (status) {
		printf("\n? ");
		line = read_input();
		args = tokenize_input(line);

		for (int i = 0; args[i] != NULL; i++) {
			printf("args[%d] = %s, ", i, args[i]);
		}

		free(line);
		free(args);
	}
}

int main(int argc, char **argv) {
	initialize();

	interpret(argc, argv);

	return EXIT_SUCCESS;
}