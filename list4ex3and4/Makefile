# lsh makefile

CC = gcc
CFLAGS  = -Wall -g
OBJ = lsh.o

all: lsh

simple-c-shell: $(OBJ)
	$(CC) $(CFLAGS) -o lsh $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
