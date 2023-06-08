/*
 *  usuario/lib/serv.c
 *
 *  Minikernel. Versi�n 1.0
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 *
 * Fichero que contiene las definiciones de las funciones de interfaz
 * a las llamadas al sistema. Usa la funcion de apoyo llamsis
 *
 *      SE DEBE MODIFICAR AL INCLUIR NUEVAS LLAMADAS
 *
 */

#include "llamsis.h"
#include "servicios.h"

/* Funci�n del m�dulo "misc" que prepara el c�digo de la llamada
   (en el registro 0), los par�metros (en registros 1, 2, ...), realiza la
   instruccion de llamada al sistema  y devuelve el resultado 
   (que obtiene del registro 0) */

int llamsis(int llamada, int nargs, ... /* args */);


/*
 *
 * Funciones interfaz a las llamadas al sistema
 *
 */


int crear_proceso(char *prog){
	return llamsis(CREAR_PROCESO, 1, (long)prog);
}
int terminar_proceso(){
	return llamsis(TERMINAR_PROCESO, 0);
}
int escribir(char *texto, unsigned int longi){
	return llamsis(ESCRIBIR, 2, (long)texto, (long)longi);
}

//Obtener ID
int obtener_id_pr() {
	return llamsis(OBTENERID, 0);
}

//Ejericio Dormir
int dormir(unsigned int segundos) {
	return llamsis(DORMIR, 1, (long)segundos);
}

//Ejercicio Mutex
int crear_mutex(char* nombre, unsigned int tipo) {
	return llamsis(CREARMUTEX, 2, (long)nombre, (long)tipo);
}
int abrir_mutex(char* nombre) {
	return llamsis(ABRIRMUTEX, 1, (long)nombre);
}
int lock(unsigned int mutexid) {
	return llamsis(LOCK, 1, (long)mutexid);
}
int unlock(unsigned int mutexid) {
	return llamsis(UNLOCK, 1, (long)mutexid);
}
int cerrar_mutex(unsigned int mutexid) {
	return llamsis(CERRARMUTEX, 1, (long)mutexid);
}