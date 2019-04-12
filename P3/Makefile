######################################################
CFLAGS = -Wall
EXE = ejercicio2 ejercicio2_solved ejercicio4 A B C 
EXECOLA = ejercicio3_consumidor ejercicio3_productor ejercicio3_consumidor_fichero ejercicio3_productor_fichero
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

