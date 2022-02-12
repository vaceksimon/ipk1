CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -g
NAME=hinfosvc

all:
	$(CC) $(CFLAGS) *.c -o $(NAME)
