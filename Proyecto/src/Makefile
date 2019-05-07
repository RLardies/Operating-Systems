TARGET = ../target

CC = gcc
CFLAGS = -g -Wall -pthread -I.
LDLIBS = -lrt -lncurses

BOLD=\e[1m
NC=\e[0m

all: simulador monitor nave jefe

clean: 
	rm -r -f $(TARGET)

simulador:
	mkdir -p $(TARGET)
	$(CC) $(CFLAGS) mapa.c simulador.c -o $(TARGET)/simulador -lrt -lm

nave:
	mkdir -p $(TARGET)
	$(CC) $(CFLAGS) mapa.c nave.c -o $(TARGET)/nave -lrt -lm

jefe:
	mkdir -p $(TARGET)
	$(CC) $(CFLAGS) mapa.c jefe.c -o $(TARGET)/jefe -lrt -lm
	
monitor:
	mkdir -p $(TARGET)
	$(CC) $(CFLAGS) gamescreen.c mapa.c monitor.c -o $(TARGET)/monitor -lrt -lncurses -lm
		
