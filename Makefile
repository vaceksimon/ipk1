CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -g
NAME=hinfosvc

all:
	$(CC) $(CFLAGS) $(NAME).c -o $(NAME)

clean:
	rm $(NAME)
