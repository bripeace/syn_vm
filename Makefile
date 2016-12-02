CC = gcc
CFLAGS = -g -Wall -pedantic

all: vm.c
	$(CC) $(CFLAGS) -o vm vm.c 
