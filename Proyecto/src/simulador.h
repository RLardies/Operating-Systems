#ifndef SRC_SIMULADOR_H_
#define SRC_SIMULADOR_H_

#include <stdbool.h>

#define N_EQUIPOS 3 // Número de equipos
#define N_NAVES 3 // Número de naves por equipo

/*** SCREEN ***/
extern char symbol_equipos[N_EQUIPOS]; // Símbolos de los diferentes equipos en el mapa (mirar mapa.c)
#define MAPA_MAXX 20 // Número de columnas del mapa
#define MAPA_MAXY 20 // Número de
#define SCREEN_REFRESH 10000 // Frequencia de refresco del mapa en el monitor
#define SYMB_VACIO '.' // Símbolo para casilla vacia
#define SYMB_TOCADO '%' // Símbolo para tocado
#define SYMB_DESTRUIDO 'X' // Símbolo para destruido
#define SYMB_AGUA 'w' // Símbolo para agua

/*** SIMULACION ***/
#define VIDA_MAX 50 // Vida inicial de una nave
#define ATAQUE_ALCANCE 20 // Distancia máxima de un ataque
#define ATAQUE_DANO 10 // Daño de un ataque
#define MOVER_ALCANCE 1 // Máximo de casillas a mover
#define TURNO_SECS 3 // Segundos que dura un turno
#define N_ACCIONES 2 // Número de acciones por turno
#define ATACAR "ATACAR" // Mensaje para indicar ataque
#define MOVER_ALEATORIO "MOVER_ALEATORIO" // Mensaje para moverse aleatoriamente
#define DESTRUIR "DESTRUIR" // Mensaje para destruir nave
#define TURNO "TURNO" // Mensaje para ceder turno a un equipo
#define FIN "FIN" // Mensaje para indicar el fin de la partida


/*** MAPA ***/
// Información de nave
typedef struct {
	int vida; // Vida que le queda a la nave
	int posx; // Columna en el mapa
	int posy; // Fila en el mapa
	int equipo; // Equipo de la nave
	int numNave; // Numero de la nave en el equipo
	bool viva; // Si la nave está viva o ha sido destruida
} tipo_nave;

// Información de una casilla en el mapa
typedef struct {
	char simbolo; // Símbolo que se mostrará en la pantalla para esta casilla
	int equipo; // Si está vacia = -1. Si no, número de equipo de la nave que está en la casilla
	int numNave; // Número de nave en el equipo de la nave que está en la casilla
} tipo_casilla;


typedef struct {
	tipo_nave info_naves[N_EQUIPOS][N_NAVES];
	tipo_casilla casillas[MAPA_MAXY][MAPA_MAXX];
	int num_naves[N_EQUIPOS]; // Número de naves vivas en un equipo
	bool finalizado;
} tipo_mapa;

#define SHM_MAP_NAME "/shm_naves"
#define QUEUE_NAME "/cola"
#define SEM_INICIO "/sem_inicio"
#define SEM_MEMORIA "/sem_mem"
#define SEM_PANTALLA "/sem_pantalla"
#define MAXMSGSIZE 100

#endif /* SRC_SIMULADOR_H_ */
