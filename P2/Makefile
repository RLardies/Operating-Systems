###################

CC = gcc
EXE = ejercicio2 ejercicio3d ejercicio4 ejercicio6a ejercicio6b ejercicio8 ejercicio9
LIBFLAGS = -lm -pthread

###################

all: $(EXE)

$(EXE) : % : %.c
	$(CC) $@.c -o $@ $(LIBFLAGS)

clean:
	rm -f core $(EXE)

vej2:
	valgrind --leak-check:full ./ejercicio2

vej3d:
	valgrind --leak-check:full ./ejercicio3d

vej4:
	valgrind --leak-check:full ./ejercicio4

vej6a:
	valgrind --leak-check:full ./ejercicio6a

vej6b:
	valgrind --leak-check:full ./ejercicio6b

vej8:
	valgrind --leak-check:full ./ejercicio8

vej9:
	valgrind --leak-check:full ./ejercicio9