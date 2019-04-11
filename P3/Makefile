######################################################
CC = gcc
CFLAGS = -Wall -pthread
EXE = main A B C
LIB = -lrt
######################################################
	
CFLAGS = -Wall
EXE = ejercicio2 main A B C 
EXECOLA = ejercicio3_consumidor ejercicio3_productor
EXEALL = $(EXE) $(EXECOLA)
LIB = -lrt -pthread
<<<<<<< HEAD
######################################################
=======
#####################################################
>>>>>>> 08452fcca45a95b69e0057136a03aff8040b5618

all : $(EXEALL)

$(EXECOLA) : % : %.c cola.c
	$(CC) $(CFLAGS) $@.c cola.c -o $@ $(LIB)

$(EXE) : % : %.c
	$(CC) $(CFLAGS) $@.c -o $@ $(LIB)

clean:
	rm -f core $(EXEALL)

