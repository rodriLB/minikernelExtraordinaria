/*
 * usuario/lector.c
 *
 *  Minikernel. Versi�n 1.0
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 * Programa de usuario que lee 11 caracteres del teclado.
 */

#include "servicios.h"
#include <stdio.h>

int main(){
	int car, id, i;

	id=obtener_id_pr();
	printf("lector (%d): comienza\n", id);

	printf("lector (%d): pulsa caracteres a partir de ahora\n", id);
	car=getchar();
	printf("lector (%d): has pulsado %c\n", id, car);

	/* ahora duerme 3 segundos.*/
	printf("lector (%d) duerme 3 segundos\n", id);
	dormir(3);

	for (i=1; i<=10; i++) {
		car=getchar();
		printf("lector (%d): has pulsado %c\n", id, car);
	}

	printf("lector (%d): termina\n", id);
	return 0;
}
