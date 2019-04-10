######################################################
CC = gcc
CFLAGS = -Wall
EXE = main A B C
LIB = -lrt
######################################################

all : $(EXE)

$(EXE) : % : %.c
	$(CC) $(CFLAGS) $@.c -o $@ $(LIB)

clean:
	rm -f core $(EXE)

