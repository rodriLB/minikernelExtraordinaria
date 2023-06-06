/*
 *  minikernel/kernel/include/llamsis.h
 *
 *  Minikernel. Versi�n 1.0
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 *
 * Fichero de cabecera que contiene el numero asociado a cada llamada
 *
 * 	SE DEBE MODIFICAR PARA INCLUIR NUEVAS LLAMADAS
 *
 */

#ifndef _LLAMSIS_H
#define _LLAMSIS_H

/* Numero de llamadas disponibles */
#define NSERVICIOS 3

#define CREAR_PROCESO 0
#define TERMINAR_PROCESO 1
#define ESCRIBIR 2

//Ejercicio Dormir
#define DORMIR 3

//Ejercicio Mutex
#define CREARMUTEX 4
#define ABRIRMUTEX 5
#define LOCK 6
#define UNLOCK 7
#define CERRARMUTEX 8


#endif /* _LLAMSIS_H */

