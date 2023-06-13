/*
 *
 * Fichero que contiene la funcionalidad del sistema operativo
 *
 */

#include "include/kernel.h"	/* Contiene defs. usadas por este modulo */

/*
 *
 * Funciones relacionadas con la tabla de procesos:
 *	iniciar_tabla_proc buscar_BCP_libre
 *
 */

/*
 * Funci�n que inicia la tabla de procesos
 */
static void iniciar_tabla_proc(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		tabla_procs[i].estado=NO_USADA;
}

/*
 * Funci�n que busca una entrada libre en la tabla de procesos
 */
static int buscar_BCP_libre(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		if (tabla_procs[i].estado==NO_USADA)
			return i;
	return -1;
}

/*
 *
 * Funciones que facilitan el manejo de las listas de BCPs
 *	insertar_ultimo eliminar_primero eliminar_elem
 *
 * NOTA: PRIMERO SE DEBE LLAMAR A eliminar Y LUEGO A insertar
 */

/*
 * Inserta un BCP al final de la lista.
 */
static void insertar_ultimo(lista_BCPs *lista, BCP * proc){
	if (lista->primero==NULL)
		lista->primero= proc;
	else
		lista->ultimo->siguiente=proc;
	lista->ultimo= proc;
	proc->siguiente=NULL;
}

/*
 * Elimina el primer BCP de la lista.
 */
static void eliminar_primero(lista_BCPs *lista){

	if (lista->ultimo==lista->primero)
		lista->ultimo=NULL;
	lista->primero=lista->primero->siguiente;
}

/*
 * Elimina un determinado BCP de la lista.
 */
static void eliminar_elem(lista_BCPs *lista, BCP * proc){
	BCP *paux=lista->primero;

	if (paux==proc)
		eliminar_primero(lista);
	else {
		for ( ; ((paux) && (paux->siguiente!=proc));
			paux=paux->siguiente);
		if (paux) {
			if (lista->ultimo==paux->siguiente)
				lista->ultimo=paux;
			paux->siguiente=paux->siguiente->siguiente;
		}
	}
}

/*
 *
 * Funciones relacionadas con la planificacion
 *	espera_int planificador
 */

/*
 * Espera a que se produzca una interrupcion
 */
static void espera_int(){
	int nivel;

	printk("-> NO HAY LISTOS. ESPERA INT\n");

	/* Baja al m�nimo el nivel de interrupci�n mientras espera */
	nivel=fijar_nivel_int(NIVEL_1);
	halt();
	fijar_nivel_int(nivel);
}

/*
 * Funci�n de planificacion que implementa un algoritmo FIFO.
 */
static BCP * planificador(){
	while (lista_listos.primero==NULL)
		espera_int();		/* No hay nada que hacer */
	return lista_listos.primero;
}

/*
 *
 * Funcion auxiliar que termina proceso actual liberando sus recursos.
 * Usada por llamada terminar_proceso y por rutinas que tratan excepciones
 *
 */
static void liberar_proceso(){
	BCP * p_proc_anterior;

	liberar_imagen(p_proc_actual->info_mem); /* liberar mapa */

	p_proc_actual->estado=TERMINADO;
	eliminar_primero(&lista_listos); /* proc. fuera de listos */

	/* Realizar cambio de contexto */
	p_proc_anterior=p_proc_actual;
	p_proc_actual=planificador();

	printk("-> C.CONTEXTO POR FIN: de %d a %d\n",
			p_proc_anterior->id, p_proc_actual->id);

	liberar_pila(p_proc_anterior->pila);
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
        return; /* no deber�a llegar aqui */
}

/*
 *
 * Funciones relacionadas con el tratamiento de interrupciones
 *	excepciones: exc_arit exc_mem
 *	interrupciones de reloj: int_reloj
 *	interrupciones del terminal: int_terminal
 *	llamadas al sistemas: llam_sis
 *	interrupciones SW: int_sw
 *
 */

/*
 * Tratamiento de excepciones aritmeticas
 */
static void exc_arit(){

	if (!viene_de_modo_usuario())
		panico("excepcion aritmetica cuando estaba dentro del kernel");


	printk("-> EXCEPCION ARITMETICA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no deber�a llegar aqui */
}

/*
 * Tratamiento de excepciones en el acceso a memoria
 */
static void exc_mem(){

	if (!viene_de_modo_usuario())
		panico("excepcion de memoria cuando estaba dentro del kernel");


	printk("-> EXCEPCION DE MEMORIA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no deber�a llegar aqui */
}

/*
 * Tratamiento de interrupciones de terminal
 */
static void int_terminal(){
	char car;

	car = leer_puerto(DIR_TERMINAL);
	printk("-> TRATANDO INT. DE TERMINAL %c\n", car);

        return;
}

/*
 * Tratamiento de interrupciones de reloj
 */
static void int_reloj(){

	ticksRound_robin();
	
	
	printk("-> TRATANDO INT. DE RELOJ\n");
	

	//Llamamos a los procesos dormidos
	BCPptr procesoActualmente = lista_dormidos.primero;
	//Recorrer por medio de un bucle
	while (procesoActualmente != NULL) {
		BCPptr procesoSiguiente = procesoActualmente->siguiente;
		procesoActualmente->seg_bloqueado--;
		if (procesoActualmente->seg_bloqueado <= 0) { //Tiempo agotado
			int nivelAnterior = fijar_nivel_int(NIVEL_3);
			procesoActualmente->estado = LISTO;
			eliminar_elem(&lista_dormidos, procesoActualmente);
			insertar_ultimo(&lista_listos, procesoActualmente);
			fijar_nivel_int(nivelAnterior);
		}
		procesoActualmente = procesoSiguiente;
	}
	
	return;
}

/*
 * Tratamiento de llamadas al sistema
 */
static void tratar_llamsis(){
	int nserv, res;

	nserv=leer_registro(0);
	if (nserv<NSERVICIOS)
		res=(tabla_servicios[nserv].fservicio)();
	else
		res=-1;		/* servicio no existente */
	escribir_registro(0,res);
	return;
}

/*
 * Tratamiento de interrupciuones software
 */
static void int_sw(){

	printk("-> TRATANDO INT. SW\n");

	return;
}

/*
 *
 * Funcion auxiliar que crea un proceso reservando sus recursos.
 * Usada por llamada crear_proceso.
 *
 */
static int crear_tarea(char *prog){
	void * imagen, *pc_inicial;
	int error=0;
	int proc;
	BCP *p_proc;

	proc=buscar_BCP_libre();
	if (proc==-1)
		return -1;	/* no hay entrada libre */

	/* A rellenar el BCP ... */
	p_proc=&(tabla_procs[proc]);

	/* crea la imagen de memoria leyendo ejecutable */
	imagen=crear_imagen(prog, &pc_inicial);
	if (imagen)
	{
		p_proc->info_mem=imagen;
		p_proc->pila=crear_pila(TAM_PILA);
		fijar_contexto_ini(p_proc->info_mem, p_proc->pila, TAM_PILA,
			pc_inicial,
			&(p_proc->contexto_regs));
		p_proc->id=proc;
		p_proc->estado=LISTO;

		/* lo inserta al final de cola de listos */
		insertar_ultimo(&lista_listos, p_proc);
		error= 0;
	}
	else
		error= -1; /* fallo al crear imagen */

	return error;
}

/*
 *
 * Rutinas que llevan a cabo las llamadas al sistema
 *	sis_crear_proceso sis_escribir
 *
 */

/*
 * Tratamiento de llamada al sistema crear_proceso. Llama a la
 * funcion auxiliar crear_tarea sis_terminar_proceso
 */
int sis_crear_proceso(){
	char *prog;
	int res;

	printk("-> PROC %d: CREAR PROCESO\n", p_proc_actual->id);
	prog=(char *)leer_registro(1);
	res=crear_tarea(prog);
	return res;
}

/*
 * Tratamiento de llamada al sistema escribir. Llama simplemente a la
 * funcion de apoyo escribir_ker
 */
int sis_escribir()
{
	char *texto;
	unsigned int longi;

	texto=(char *)leer_registro(1);
	longi=(unsigned int)leer_registro(2);

	escribir_ker(texto, longi);
	return 0;
}

/*
 * Tratamiento de llamada al sistema terminar_proceso. Llama a la
 * funcion auxiliar liberar_proceso
 */
int sis_terminar_proceso(){

	printk("-> FIN PROCESO %d\n", p_proc_actual->id);

	liberar_proceso();

        return 0; /* no deber�a llegar aqui */
}


//Obtener ID
int obtener_id_pr(){
	return p_proc_actual->id;
}

//Ejercio Dormir
int dormir(unsigned int segundos){

	int nivelAnterior = fijar_nivel_int(NIVEL_3);
	BCPptr proceso_dormir = p_proc_actual;

	proceso_dormir->estado = BLOQUEADO;
	proceso_dormir->seg_bloqueado = segundos * TICK;
	eliminar_primero(&lista_listos);
	insertar_ultimo(&lista_dormidos, proceso_dormir);

	p_proc_actual = planificador();

	cambio_contexto(&(proceso_dormir->contexto_regs), &(p_proc_actual->contexto_regs));
	fijar_nivel_int(nivelAnterior);
	return 0;
}


//Ejercicio Mutex
int comprobacionesMutex(char *nombre){
	//COMPROBACCIONES
	//Longitud del nombre
	if (comprobacionMutexLonNombre(nombre) != 0)
		return -1;

	//Nombres Duplicado
	if (comprobacionMutexNombre(nombre)!= -1){
		printk("Error, mutex %s ya existe en el sistema\n", nombre);
		return -1;
	}

	//Descriptores Libres
	int decrementarProceso = comprobacionDescriptorLibre();
	if (decrementarProceso == -1) {
		printk("Error, el proceso id: %d no tiene descriptores libres\n", p_proc_actual->id);
		return -1;
	}

	return decrementarProceso;
}

int comprobacionMutexNombre(char *nombre) {
	for (int i = 0; i < NUM_MUT; i++){
		if (strcmp(listaMutex[i].nombre, nombre) == 0)
			return i;	//Devuelve la posicion del mutex encontrado
	}
	return -1;
}

int comprobacionMutexLonNombre(char *nombre){
	if (strlen(nombre) > MAX_NOM_MUT){
		printk("Error, nombre demasiado largo\n");
		return -1;
	}
	return 0;
}

 int comprobarMutexEspacioLibre() {
	for (int i = 0; i < NUM_MUT; i++){
		if (listaMutex[i].estado == LIBRE)
			return i;
	}
	return -1;
}

int comprobacionDescriptorLibre() {
	for (int i = 0; i < NUM_MUT_PROC; i++){
		if (p_proc_actual->descriptoresProcesosUsados[i] == -1)
			return i;
	}
	return -1;
}

void iniciar_lista_mutex() {
	for (int i = 0; i < NUM_MUT; i++) {
		listaMutex[i].estado = LIBRE;
		listaMutex[i].contBloqueados = 0;
		listaMutex[i].idProceso = -1;

		listaMutex[i].procesoEsperando.primero = NULL;

		listaMutex[i].numeroProcesosEsperando = 0;
		listaMutex[i].mutexLock = UNLOCKED;
	}
}

int* busquedaMutexPorID(int mutexid) {
	int i;
	static int retorno[2] = { -1,-1 };
	for (i = 0; i < NUM_MUT_PROC; i++) {
		if (p_proc_actual->descriptoresProcesosUsados[i] == mutexid) {
			retorno[0] = i;
			retorno[1] = p_proc_actual->descriptoresProcesosUsados[i];
			return retorno;
		}
	}
	return retorno;
}


int crear_mutex(char *nombre, int tipo){
	nombre = (char*)leer_registro(1);
	tipo = (unsigned int)leer_registro(2);

	int nivelAnterior, decrementarMutex, decrementarProceso, mutexCreado;
	
	//Comprobaciones
	decrementarProceso = comprobacionesMutex(nombre);

	//Espacio libre mutex
	mutexCreado = 0;
	while (mutexCreado == 0) {
		decrementarMutex = comprobarMutexEspacioLibre();

		if (decrementarMutex == -1) {
			nivelAnterior = fijar_nivel_int(NIVEL_3);
			mutexCreado = 0;
			printk("Error, numero de mutex maximo alcanzado\n");
			printk("Bloqueando proceso con id: %d\n", p_proc_actual->id);
			contListaMutexBloqueadosSist++;
			BCPptr p_proc_bloqueado = p_proc_actual;
			p_proc_bloqueado->estado = BLOQUEADO;
			eliminar_primero(&lista_listos);
			insertar_ultimo(&listaProcesosBloqueadosMutex, p_proc_bloqueado);
			p_proc_actual = planificador();
			printk("C.CONTEXTO POR BLOQUEO de %d a %d\n", p_proc_bloqueado->id, p_proc_actual->id);
			cambio_contexto(&(p_proc_bloqueado->contexto_regs), &(p_proc_actual->contexto_regs));
			fijar_nivel_int(nivelAnterior);
		}
		
		MUTEXptr mut = &listaMutex[decrementarMutex];
		strcpy(mut->nombre, nombre);
		mut->tipo = tipo;
		mut->estado = OCUPADO;
		contListaMutexSist++;
		
		p_proc_actual->descriptoresProcesosUsados[decrementarProceso] = decrementarMutex;
		p_proc_actual->descriptoresProcesosActivos++;
		printk("Mutex %s CREADO y ABIERTO\n", mut->nombre);
		mutexCreado = 1;
	}
	
	return decrementarProceso;
}

int abrir_mutex(char *nombre){

	nombre = (char*)leer_registro(1);
	int posicionListaMutex, decrementarProceso;
	

	//Buscamos por nombre
	posicionListaMutex = comprobacionMutexNombre(nombre);
	if (posicionListaMutex != -1) {
		printk("Error, mutex %s ya existe en el sistema\n", nombre);
		return -1;
	}

	//Mutex libre
	decrementarProceso = comprobarMutexEspacioLibre();
	if (decrementarProceso == -1) {
		printk("Error, el proceso id: %d no tiene descriptores libres\n", p_proc_actual->id);
		return -1;
	}
	p_proc_actual->descriptoresProcesosUsados[decrementarProceso] = posicionListaMutex;
	p_proc_actual->descriptoresProcesosActivos++;
	printk("Mutex %s ABIERTO\n", nombre);

	return decrementarProceso;
}

int lock(unsigned int mutexid){
	unsigned int mutexId = (unsigned int)leer_registro(1);
	int nivelAnterior, decrementarProceso, posicionListaMutex, lock;
	int *ret;
	if (mutexId < NUM_MUT) 
		mutexid = mutexId;

	//Buscar Mutex por ID
	ret = busquedaMutexPorID((int)mutexid);
	decrementarProceso = *ret;
	posicionListaMutex = *(ret+1);
	if (decrementarProceso == -1) {
		printk("Error, mutex con mutexid: %d no encontrado\n", mutexid);
		return -1;
	}

	MUTEXptr mut = &listaMutex[posicionListaMutex];
	lock = 0;
	while (lock == 0){
		if (mut->idProceso == -1 && mut->contBloqueados == 0){
			mut->contBloqueados++;
			mut->idProceso = p_proc_actual->id;
			mut->mutexLock = LOCKED;

			printk("Mutex %s BLOQUEADO\n", mut->nombre);

			return decrementarProceso;
		}
		if (mut->idProceso == p_proc_actual->id){
			if (mut->mutexLock == LOCKED && mut->tipo == RECURSIVO)
			{
				lock = 1;
				mut->contBloqueados++;
				printk("Mutex RECURSIVO %s BLOQUEADO\n", mut->nombre);
				return decrementarProceso;
			}
			else if (mut->mutexLock == LOCKED)
			{
				printk("Error, intento de bloquear mutex %s ya bloqueado y de tipo NO RECURSIVO\n", mut->nombre);
				return -1;
			}
		}

		//Si llega hasta aqui es que proceso actual no es propietario del mutex y sera bloqueado
		nivelAnterior = fijar_nivel_int(NIVEL_3);
		lock = 0;
		mut->numeroProcesosEsperando++;
		BCPptr p_proc_bloqueado = p_proc_actual;
		p_proc_bloqueado->estado = BLOQUEADO;
		eliminar_primero(&lista_listos);
		insertar_ultimo(&(mut->procesoEsperando), p_proc_bloqueado);
		p_proc_actual = planificador();
		printk("C.CONTEXTO POR BLOQUEO de %d a %d\n", p_proc_bloqueado->id, p_proc_actual->id);
		cambio_contexto(&(p_proc_bloqueado->contexto_regs), &(p_proc_actual->contexto_regs));
		fijar_nivel_int(nivelAnterior);
	}

	
	return 0;
}

int unlock(unsigned int mutexid){
	int nivelAnterior, decrementarProceso, posicionListaMutex;
	int* ret;
	unsigned int mutexId = (unsigned int)leer_registro(1);
	if (mutexId < NUM_MUT) mutexid = mutexId;

	//Buscar Mutex por id
	ret = busquedaMutexPorID((int)mutexid);
	decrementarProceso = *ret;
	posicionListaMutex = *(ret + 1);
	if (decrementarProceso == -1) {
		printk("Error, mutex con ID: %d no encontrado\n", mutexid);
		return -1;
	}

	MUTEXptr mut = &listaMutex[posicionListaMutex];
	if (mut->mutexLock == LOCKED)
	{		
		mut->contBloqueados--;
		if (mut->contBloqueados == 0)
		{
			mut->mutexLock = UNLOCKED;
			mut->idProceso = -1;
			printk("Mutex %s DESBLOQUEADO\n", mut->nombre);

			if (mut->numeroProcesosEsperando > 0)
			{
				nivelAnterior = fijar_nivel_int(NIVEL_3);
				mut->numeroProcesosEsperando--;
				BCPptr p_proc_bloqueado = mut->procesoEsperando.primero;
				p_proc_bloqueado->estado = LISTO;
				eliminar_primero(&(mut->procesoEsperando));
				insertar_ultimo(&lista_listos, p_proc_bloqueado);
				printk("Proceso id: %d DESBLOQUEADO\n", p_proc_bloqueado->id);
				fijar_nivel_int(nivelAnterior);
			}

			return decrementarProceso;
		}

	}
	else { 	
		// si el mutex no esta bloqueado el intento de desbloquearlo producira un error 
		printk("Error, mutex %s no bloqueado\n", mut->nombre);
		return -1;
	}
	return 0;
}

int cerrar_mutex(unsigned int mutexid){
	int nivelAnterior, decrementarProceso, posicionListaMutex;
	int* ret;
	unsigned int mutex_id = (unsigned int)leer_registro(1);
	if (mutex_id < NUM_MUT) mutexid = mutex_id;

	//Buscar Mutex por id
	ret = busquedaMutexPorID((int)mutexid);
	decrementarProceso = *ret;
	posicionListaMutex = *(ret + 1);
	if (decrementarProceso == -1) {
		printk("Error, mutex con ID: %d no encontrado\n", mutexid);
		return -1;
	}

	MUTEXptr mut = &listaMutex[posicionListaMutex];
	while (mut->mutexLock == LOCKED) {
		unlock(mutexid);
	}
	mut->estado = LIBRE;
	mut->contBloqueados = 0;
	mut->numeroProcesosEsperando = 0;
	p_proc_actual->descriptoresProcesosUsados[decrementarProceso] = -1;
	p_proc_actual->descriptoresProcesosActivos--;
	contListaMutexSist--;
	printk("Mutex %s CERRADO\n", mut->nombre);
	//Revisar lista de bloqueados
	if (contListaMutexBloqueadosSist > 0){
		nivelAnterior = fijar_nivel_int(NIVEL_3);
		contListaMutexBloqueadosSist--;
		BCPptr p_proc_bloqueado = listaProcesosBloqueadosMutex.primero;
		p_proc_bloqueado->estado = LISTO;
		eliminar_primero(&listaProcesosBloqueadosMutex);
		insertar_ultimo(&lista_listos, p_proc_bloqueado);
		printk("Proceso id %d DESBLOQUEADO\n", p_proc_bloqueado->id);
		fijar_nivel_int(nivelAnterior);
	}
	return 0;
}

//Fin de Mutex

//Round Robin
void ticksRound_robin() {
	if (p_proc_actual->estado == LISTO) { //Caso en el que hay proceso
		//Si el contador de ticks es terminado que mande una interrupcion
		if (p_proc_actual->contadorTicks > 0) {
			p_proc_actual->contadorTicks--;
			printk("Proceso id: %d, contador de tiks que faltan: %d\n", p_proc_actual->id, p_proc_actual->contadorTicks);
		}
		if (p_proc_actual->contadorTicks == 0) {
			printk("Proceso id: %d, lanzando interrupcion\n", p_proc_actual->id);
			activar_int_SW(); //Rutina de tratamiento proporcionada en HAL.h --> se encargará de lanzar una interrupcion de software
		}
	}
}


/*
 *
 * Rutina de inicializaci�n invocada en arranque
 *
 */
int main(){
	/* se llega con las interrupciones prohibidas */

	instal_man_int(EXC_ARITM, exc_arit); 
	instal_man_int(EXC_MEM, exc_mem); 
	instal_man_int(INT_RELOJ, int_reloj); 
	instal_man_int(INT_TERMINAL, int_terminal); 
	instal_man_int(LLAM_SIS, tratar_llamsis); 
	instal_man_int(INT_SW, int_sw); 

	iniciar_cont_int();		/* inicia cont. interr. */
	iniciar_cont_reloj(TICK);	/* fija frecuencia del reloj */
	iniciar_cont_teclado();		/* inici cont. teclado */

	iniciar_tabla_proc();		/* inicia BCPs de tabla de procesos */

	/* crea proceso inicial */
	if (crear_tarea((void *)"init")<0)
		panico("no encontrado el proceso inicial");
	
	/* activa proceso inicial */
	p_proc_actual=planificador();
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	panico("S.O. reactivado inesperadamente");
	return 0;
}
