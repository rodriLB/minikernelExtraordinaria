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

//Obtener ID
#define OBTENERID 3

//Dormir
#define DORMIR 4

//Mutex
#define CREARMUTEX 5
#define ABRIRMUTEX 6
#define LOCK 7
#define UNLOCK 8
#define CERRARMUTEX 9


#endif /* _LLAMSIS_H */

