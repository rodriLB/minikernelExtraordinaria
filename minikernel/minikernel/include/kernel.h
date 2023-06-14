/*
 *
 * Fichero de cabecera que contiene definiciones usadas por kernel.c
 *
 *      SE DEBE MODIFICAR PARA INCLUIR NUEVA FUNCIONALIDAD
 *
 */

#ifndef _KERNEL_H
#define _KERNEL_H

#include "const.h"
#include "HAL.h"
#include "llamsis.h"

/*
 *
 * Definicion del tipo que corresponde con el BCP.
 * Se va a modificar al incluir la funcionalidad pedida.
 *
 */
typedef struct BCP_t *BCPptr;

typedef struct BCP_t {
	int id;				/* ident. del proceso */
	int estado;			/* TERMINADO|LISTO|EJECUCION|BLOQUEADO*/
	contexto_t contexto_regs;	/* copia de regs. de UCP */
	void * pila;			/* dir. inicial de la pila */
	BCPptr siguiente;		/* puntero a otro BCP */
	void *info_mem;			/* descriptor del mapa de memoria */


	//Dormir
	int segBloqueado; //Tiempo que estará el proceso bloqueado

	//Mutex related
	int descriptoresProcesosUsados[NUM_MUT_PROC]; //Descriptores en uso de cada procesos
	int descriptoresProcesosActivos; //Contador de descriptores que va a ejecutar los procesos

	//Round Robin
	int contadorTicks; //Contador de ticks para round robin
} BCP;

/*
 *
 * Definicion del tipo que corresponde con la cabecera de una lista
 * de BCPs. Este tipo se puede usar para diversas listas (procesos listos,
 * procesos bloqueados en sem�foro, etc.).
 *
 */

typedef struct{
	BCP *primero;
	BCP *ultimo;
} lista_BCPs;


/*
 * Variable global que identifica el proceso actual
 */

BCP * p_proc_actual=NULL;

/*
 * Variable global que representa la tabla de procesos
 */

BCP tabla_procs[MAX_PROC];

/*
 * Variable global que representa la cola de procesos listos
 */
lista_BCPs lista_listos= {NULL, NULL};

/*
* Variable global que representa una lista con los procesos dormidos
*/
lista_BCPs lista_dormidos = { NULL, NULL };

/*
 *
 * Definici�n del tipo que corresponde con una entrada en la tabla de
 * llamadas al sistema.
 *
 */
typedef struct{
	int (*fservicio)();
} servicio;


/*
 * Prototipos de las rutinas que realizan cada llamada al sistema
 */
int sis_crear_proceso();
int sis_terminar_proceso();
int sis_escribir();

/*
 * Variable global que contiene las rutinas que realizan cada llamada
 */
servicio tabla_servicios[NSERVICIOS]={	{sis_crear_proceso},
					{sis_terminar_proceso},
					{sis_escribir}};


int obtener_id_pr();

//Ejercicio Dormir
int dormir(unsigned int segundos);

//Ejercicio Mutex
//DEFINE DE MUTEX
#define OCUPADO 0 	//Mutex ocupado 
#define LIBRE 1 	//Mutex libre 
#define LOCKED 0 	//Mutex bloquedo 
#define UNLOCKED 1 	//Mutex desbloqueado 
//Tipo de Mutex
#define NO_RECURSIVO 0
#define RECURSIVO 1

/*
 * Estructura del mutex
 */

typedef struct Mutex_t*MUTEXptr;
typedef struct Mutex_t {
	char nombre[MAX_NOM_MUT]; //Nombre del Mutex
	int tipo; //Tipo del Mutex: NO_RECURSIVO-0 / RECURSIVO-1 

	
	int estado; //Estado del Mutex
	lista_BCPs procesoEsperando; //Lista de los procesos que estan esperando a tomar el cerrojo
	int numeroProcesosEsperando; //Contador de procesos esperando
	int idProceso; //Id del proceso que posee el cerrojo
	int contBloqueados; //bloqueos que proporciona el mutex
	int mutexLock; //LOCK-1 / UNLOCK-0

} mutex;

mutex listaMutex[NUM_MUT];
int contListaMutexSist; //Contador de la lista de mutex del sistema

lista_BCPs listaProcesosBloqueadosMutex = { NULL, NULL }; //Lista de procesos bloqueados debido a no disponer espacios por el mutex
int contListaMutexBloqueadosSist; //Contador de la lista de mutex del sistema de procesos bloqueados

//Funciones para mutex
int comprobacionesMutex(char *nombre);
int comprobacionMutexNombre(char* nombre);
int comprobacionMutexLonNombre(char* nombre);
int comprobarMutexEspacioLibre();
int comprobacionDescriptorLibre();
void iniciar_lista_mutex();
int* busquedaMutexPorID(int mutexid);

int crear_mutex(char *nombre, int tipo);
int abrir_mutex(char *nombre);
int lock(unsigned int mutexid);
int unlock(unsigned int mutexid);
int cerrar_mutex(unsigned int mutexid);

//Round Robin
void ticksRoundRobin();

#endif /* _KERNEL_H */