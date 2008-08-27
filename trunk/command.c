/*	Implementacion del command.c con el GQueue
 *	Vamos a guardar el nombre del comando en el args[0] de la estructura
 */

#include "command.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>



/***     SCOMMAND       ***/
/***  Comando simple    ***/

struct scommand_s {
	bool builtin; 
	GQueue * args; /* nombre del comando y sus argumentos */
	bstring dirOut;
	bstring dirIn;
};

#define BASE_ALLOC 1

/*** Inicialización y destrucción ***/

scommand *scommand_new (void)
{
	scommand * result = NULL;
	
	result = (scommand *) calloc (1,sizeof (struct scommand_s) );
	assert (result != NULL);
	
	/* por defecto se inicializa todo comando como "no interno" */
	result->builtin = false;
	/*generamos la cola vacia*/
	result->args = g_queue_new ();
	
	assert (result->args != NULL); /*nos aseguramos que se haya creado*/
	
	result->dirOut = NULL;
	result->dirIn = NULL;
	
	/* ENSURES */
	assert (result != NULL);
	assert (g_queue_is_empty (result->args));
	
	return result;
}

void scommand_destroy (scommand *self)
{
/*	bstring aux = NULL;*/
	unsigned int n = 0, i = 0;
	bstring aux = NULL;
	/* REQUIRES */
	assert (self != NULL);
	
	bdestroy (self->dirOut);
	bdestroy (self->dirIn);
	
	if (self->args != NULL) {
		n = g_queue_get_length (self->args);
		/* si hay argumentos se liberan antes de destruir la cola*/
		while (i < n){
			if ((aux = g_queue_peek_nth (self->args,i)) != NULL) {
				bdestroy (aux);
				aux = NULL;
			}
			i++;
		}
		g_queue_free (self->args);
	}
	
	free (self);
}

/*** Modificadores ***/

void scommand_push_back (scommand *self, bstring argument)
{	/* encolamos por detras :D */
	/* REQUIRES */
	assert (self != NULL);
	assert (argument != NULL);
	
	g_queue_push_tail (self->args, argument);
	
	/* ENSURES */
	assert (!g_queue_is_empty (self->args));
}


void scommand_push_front (scommand *self, bstring argument)
{	/* encolamos por adelante */
	/* REQUIRES */
	assert (self != NULL);
	assert (argument != NULL);
	
	g_queue_push_head (self->args, argument);
	
	/* ENSURES */
	assert (!g_queue_is_empty (self->args));
}

void scommand_pop_front (scommand *self)
{	/* removemos el primer elemento */
	bstring aux = NULL;
	/* REQUIRES */
	assert (self != NULL);
	assert (!g_queue_is_empty (self->args));
	
	aux = g_queue_pop_head (self->args);
	aux = NULL;
}


void scommand_set_builtin (scommand *self, bool builtin)
{	/* marca al comando simple como interno o no interno */
	/* REQUIRES */
	assert (self != NULL);
	
	self->builtin = builtin;
}

void scommand_set_redir_in (scommand *self, bstring filename)
{	/* setea con 'filename' la redireccion de entrada */
	/* REQUIRES */
	assert (self != NULL);
	
	if (filename != NULL && blength (filename)>0 ) {
		self->dirIn = filename;
	} else {
		self->dirIn = NULL;
	}
}

void scommand_set_redir_out (scommand *self, bstring filename)
{	/* setea con 'filename' la redireccion de salida */
	/* REQUIRES */
	assert (self != NULL);

	if (filename != NULL && blength (filename)>0 ) {
		self->dirOut = filename;
	} else {
		self->dirOut = NULL;
	}
}

/*** Proyectores ***/

bool scommand_is_empty (const scommand *self)
{	/* chequea si el scommand está vacío (no hay comando ni argumentos) */
	/* REQUIRES */
	assert (self != NULL);
	
	return g_queue_is_empty (self->args);
}

unsigned int scommand_length (const scommand *self)
{	/* devuelve la cantidad de comandos + argumentos del scommand */
	/* REQUIRES */
	assert (self != NULL);
	
	if (!scommand_is_empty (self)) {
		return g_queue_get_length (self->args);
	} else {
		return (unsigned int) 0;
	}
}

bstring scommand_front (const scommand *self)
{	/* devuelve el primer elemento de la cola */
	bstring aux = NULL;
	/* REQUIRES */
	assert (self != NULL);
	assert (!scommand_is_empty (self));
	
	aux = g_queue_peek_head (self->args);
	
	/* ENSURES */
	assert (aux != NULL);
	
	return aux;
}

bool scommand_get_builtin (const scommand *self)
{	/* ¿self es un comando interno? */
	/* REQUIRES */
	assert (self != NULL);
	
	return self->builtin;
}

bstring scommand_get_redir_in (const scommand *self)
{	/* obtiene el nombre de la redireccion de entrada */
	/* REQUIRES */
	assert (self != NULL);
	
	return self->dirIn;
	/* como se ve en la implementacion, si o si dirIn se guarda un dato o
	 * NULL en caso contrario, por lo que no hace falta comprobarlo 
	 */
}

bstring scommand_get_redir_out (const scommand *self)
{	/* obtiene el nombre de la redireccion de salida */
	/* REQUIRES */
	assert (self != NULL);
	
	return self->dirOut;
}

bstring scommand_to_string (const scommand *self)
{	/* serializador del scommand, para debuggeo solamente
	 * imprime el comando de forma legible para el ojo humano
	 * si el comando es vacío devuelve una cadena vacía
	 */
	bstring result = NULL , aux = NULL;
	unsigned int n = scommand_length (self);
	int i = 0;
	/* REQUIRES */
	assert (self != NULL);
	
	result = bfromcstr ("");
	
	/* impresión del comando y sus argumentos */
	while ((unsigned int) i < n) {
	/* si hay cadenas en el scommand las concatenaremos de a una
	 * en result, con una separacion de un espacio entre ellas
	 */
		aux = g_queue_peek_nth (self->args, i);
		bcatcstr (result, " ");
		bconcat (result, aux);
		aux = NULL;
		i++;
	}
	
	/* impresión de redirectores */
	if ((aux = scommand_get_redir_in (self)) != NULL) {
		bcatcstr (result, " < ");
		bconcat (result, aux);
		aux = NULL;
	}
	if ((aux = scommand_get_redir_out (self)) != NULL) {
		bcatcstr (result, " > ");
		bconcat (result, aux);
		aux = NULL;
	}
	/* caso "no hay comando" */
	if (blength (result) <= 0)
		bdestroy (result);
	
	/* ENSURES */
	assert (scommand_is_empty (self) || (blength (result)>0));
	
	return result;
}




/***             PIPELINE              ***/
/***   Tubería: cadena de scommands    ***/


struct pipeline_s {
	GQueue *scmd;	/* cola de scommands */
	bool wait;	/* correr en segundo plano? */
};

/*** Inicialización y destrucción ***/

pipeline *pipeline_new (void)
{
	pipeline *result = NULL;
	
	result = (pipeline *) calloc (1,sizeof (struct pipeline_s));
	assert (result != NULL);
	
	result->scmd = g_queue_new ();
	result->wait = true;
	
	/* ENSURES */
	assert (result != NULL);
	
	return result;
}

void pipeline_destroy (pipeline *self)
{
	
	/* REQUIRES */
	assert (self!=NULL);
	
	
	
	if (self->scmd != NULL) {
		/*luego liberamos la misma cola*/
		g_queue_free (self->scmd);
	}
	
	free (self);
}

/*** Modificadores ***/

void pipeline_push_back (pipeline *self, scommand *sc)
{	/* agrega por detrás un scommand a la cola */
	/* REQUIRES */
	assert (self!=NULL);
	assert (sc!=NULL);
	
	g_queue_push_tail (self->scmd, sc);
	
	/* ENSURES */
	assert (!pipeline_is_empty (self));
}

void pipeline_pop_front (pipeline *self)
{	/* quita el primer scommand de la cola */
	bstring aux = NULL;
	/* REQUIRES */
	assert (self!=NULL);
	assert (!pipeline_is_empty(self));
	
	aux = g_queue_pop_head (self->scmd);
	aux = NULL;
}

void pipeline_set_wait (pipeline *self, const bool w)
{	/* define si el pipe corre en segundo plano o no */
	/* REQUIRES */
	assert (self!=NULL);
	
	self->wait = w;
}

/*** Proyectores ***/

bool pipeline_is_empty (const pipeline *self)
{	/* el pipe está vacío? */
	/* REQUIRES */
	assert (self!=NULL);
	
	return g_queue_is_empty (self->scmd);
}

unsigned int pipeline_length (const pipeline *self)
{	/* cantidad de scommands que contiene el pipe */
	/* REQUIRES */
	assert (self!=NULL);

	if (!pipeline_is_empty (self)) {
		return g_queue_get_length (self->scmd);
	} else {
		return (unsigned int) 0;
	}
}

scommand *pipeline_front (const pipeline *self)
{	/* devuelve el primer scommand del pipe */
	scommand* result = NULL;
	/* REQUIRES */
	assert (self!=NULL);
	assert (!pipeline_is_empty(self));
	
	result = g_queue_peek_head (self->scmd);
	
	/* ENSURES */
	assert (result != NULL);
	
	return result;
}

bool pipeline_get_wait (const pipeline *self)
{
	/* REQUIRES */
	assert (self!=NULL);
	
	return self->wait;
}

bstring pipeline_to_string (const pipeline *self)
{	/* serializador del pipe, para debuggeo solamente
	 * imprime el pipe de forma legible para el ojo humano
	 * si el pipe es vacío devuelve una cadena vacía
	 */
	bstring result = NULL, aux = NULL;
	scommand* scaux = NULL;
	unsigned int n = pipeline_length (self);
	int i = 0;
	
	/* REQUIRES */
	assert (self!=NULL);
	
	result = bfromcstr ("");
	
	if (!pipeline_is_empty (self)) {
	/* concatenamos todos los scommand, con pipes '|' para separarlos */
		while ((unsigned int) i < n) { 
			scaux = g_queue_peek_nth (self->scmd, i);
			if (i != 0)
				bcatcstr (result, " |");
			/*almacenamos temporalmente*/
			aux = scommand_to_string (scaux);
			bconcat (result, aux);
			bdestroy (aux); /*liberamos*/
			scaux = NULL;
			i++;
		}
	}
	
	if (blength (result) > 0 && !pipeline_get_wait (self)) {
	/* impresión de modo espera/no_espera */
		bcatcstr (result, " &");
	} else if (blength (result) <= 0) {
	/* caso "no hay nada en el pipeline" */
		bdestroy (result);
	}
	
	/* ENSURES */
	assert (pipeline_is_empty (self) || pipeline_get_wait (self) ||
		blength (result)>0 );
	
	return result;
}

