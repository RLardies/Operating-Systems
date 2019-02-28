##
CC = gcc
EXE = ejercicio3 ejercicio4 ejercicio5 ejercicio7 ejercicio9 ejercicio11 ejercicio12
##

all: $(EXE)

$(EXE) : % : %.o
	$(CC) -o $@ $@.o -lm -pthread

clean:
	rm -f *.o core $(EXE)

vej3:
	valgrind --leak-check:full ./ej3

vej4:
	valgrind --leak-check:full ./ej4

vej5:
	valgrind --leak-check:full ./ej5

vej7:
	valgrind --leak-check:full ./ej7

vej9:
	valgrind --leak-check:full ./ej9

vej11:
	valgrind --leak-check:full ./ej11

vej12:
	valgrind --leak-check:full ./ej12