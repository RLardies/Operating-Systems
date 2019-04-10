######################################################
CC = gcc
CFLAGS = -Wall
EXE = ejercicio2 main A B C ejercicio3_consumidor ejercicio3_productor
LIB = -lrt -pthread
#####################################################j

all : $(EXE)

$(EXE) : % : %.c cola.c
	$(CC) $(CFLAGS) $@.c cola.c -o $@ $(LIB)

clean:
	rm -f core $(EXE)

