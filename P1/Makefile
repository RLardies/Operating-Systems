###################

CC = gcc
EXE = ejercicio3 ejercicio4 ejercicio5 ejercicio7 ejercicio9 ejercicio12
LIBFLAGS = -lm -pthread

###################

all: $(EXE)

$(EXE) : % : %.c
	$(CC) $@.c -o $@ $(LIBFLAGS)

clean:
	rm -f *.o core $(EXE)

vej3:
	valgrind --leak-check:full ./ejercicio3

vej4:
	valgrind --leak-check:full ./ejercicio4

vej5:
	valgrind --leak-check:full ./ejercicio5

vej7:
	valgrind --leak-check:full ./ejercicio7

vej9:
	valgrind --leak-check:full ./ejercicio9

vej12:
	valgrind --leak-check:full ./ejercicio12