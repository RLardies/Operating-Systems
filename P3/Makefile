######################################################
CC = gcc
CFLAGS = -Wall
EXE = ejercicio2 main A B C 
EXECOLA = ejercicio3_consumidor ejercicio3_productor
EXEALL = $(EXE) $(EXECOLA)
LIB = -lrt -pthread
######################################################

all : $(EXEALL)

$(EXECOLA) : % : %.c cola.c
	$(CC) $(CFLAGS) $@.c cola.c -o $@ $(LIB)

$(EXE) : % : %.c
	$(CC) $(CFLAGS) $@.c -o $@ $(LIB)

clean:
	rm -f core $(EXEALL)

