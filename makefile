CC=gcc
CFLAGS=-Wall -Werror -g

SEEsh: SEEsh.c
	$(CC) $(CFLAGS) -o SEEsh SEEsh.c
