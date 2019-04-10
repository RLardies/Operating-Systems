######################################################
CC = gcc
<<<<<<< HEAD
CFLAGS = -Wall -pthread
EXE = main A B C
LIB = -lrt
######################################################
=======
CFLAGS = -Wall
EXE = ejercicio2 main A B C ejercicio3_consumidor ejercicio3_productor
LIB = -lrt -pthread
#####################################################j
>>>>>>> c20d78e7d07199f2c6f9ff897ae812e86ee53f1a

all : $(EXE)

$(EXE) : % : %.c cola.c
	$(CC) $(CFLAGS) $@.c cola.c -o $@ $(LIB)

clean:
	rm -f core $(EXE)

