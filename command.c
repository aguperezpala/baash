/*	Implementacion del command.c con el GQueue
 *	Vamos a guardar el nombre del comando en el args[0] de la estructura
*/
#include "command.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


struct scommand_s
{
	bool builtin; 
	GQueue * args; /*argumentos y nombre del representado por una cola*/
	bstring dirOut;
	bstring dirIn;
};


scommand *scommand_new (void)
{
	scommand * result = NULL;
	
	result = (scommand *) calloc (1,sizeof (struct scommand_s) );
	assert (result != NULL);
	
	result->builtin=false;
	result->args=g_queue_new (); /*generamos la cola vacia*/
	
	assert (result->args != NULL); /*nos aseguramos que se haya creado*/
	
	result->dirOut = NULL;
	result->dirIn = NULL;
	
	
	
	/*assert( COMPLETAR********************************************/
	
	return result;
}

void scommand_destroy (scommand *self)
{
	bstring aux = NULL;
	
	assert (self != NULL);
	
	bdestroy (self->dirOut);
	bdestroy (self->dirIn);
	
	assert (self->args!=NULL); /*evitamos que nunca haya sido creado*/
	
	while(!g_queue_is_empty (self->args))
	{
		aux=(bstring)g_queue_pop_head (self->args);
		bdestroy (aux);
	}
	
	g_queue_free (self->args);
	
	free(self);
}


void scommand_push_back (scommand *self, bstring argument)
{
	assert (self != NULL);
	assert (argument != NULL);
	
	g_queue_push_tail (self->args, argument); /*encolamos por detras*/
	
	assert (!g_queue_is_empty (self->args));
}


void scommand_push_front (scommand *self, bstring argument)
{	
	assert (self != NULL);
	assert (argument != NULL);
	
	g_queue_push_head (self->args, argument); /*encolamos por delante*/
	
	assert (!g_queue_is_empty (self->args));
}

void scommand_pop_front (scommand *self)
{	
	bstring aux=NULL;
	
	assert (self != NULL);
	
	aux = g_queue_pop_tail (self->args);
	bdestroy(aux);
	
	assert (!g_queue_is_empty (self->args));
}


void scommand_set_builtin (scommand *self, bool builtin)
{
	assert (self != NULL);
	
	self->builtin = builtin;
}

void scommand_set_redir_in (scommand *self, bstring filename)
{
	assert (self != NULL);
	
	bassign (self->dirIn, (const_bstring) filename);
}
void scommand_set_redir_out (scommand *self, bstring filename)
{
	assert (self != NULL);
	
	bassign (self->dirOut, (const_bstring) filename);
}

/* Proyectores */

bool scommand_is_empty (const scommand *self)
{
	assert (self != NULL);
	
	return (bool) g_queue_is_empty (self->args);
}

unsigned int scommand_length (const scommand *self)
{
	assert (self != NULL);
	
	return (unsigned int) g_queue_get_length (self->args);
}

bstring scommand_front (const scommand *self)
{
	bstring result = NULL;
	
	assert (self != NULL);
	assert (!scommand_is_empty (self));
	
	result = (bstring) g_queue_peek_head (self->args);
	/* esta funcion nos devuelve el puntero al primer elemento de la cola */
	
	assert (result != NULL);
	
	return result;
}

bool scommand_get_builtin (const scommand *self)
{
	assert (self != NULL);
	
	return self->builtin;
}

bstring scommand_get_redir_in (const scommand *self)
{
	assert (self != NULL);
	
	return self->dirIn;
	/* como se ve en la implementacion si o si dirIn se guarda un dato o
	   NULL en caso contrario, por lo que no hace falta comprobarlo     */
}
	
	
bstring scommand_get_redir_out (const scommand *self)
{
	assert (self != NULL);
	
	return self->dirOut;
}

bstring scommand_to_string (const scommand *self)
{
	bstring result = NULL;
	unsigned int i = 0; 
	
	assert (self != NULL);
	
	if (!balloc (result,1)) /*se produjo un error*/
		return NULL;
	
	i = scommand_length (self);
	while (i > 0)
	{
		/*COMPLETAR*/
	}
	
	return result;
}
	
	
/* Preety printer para hacer debugging/logging.
 * Genera una representación del comando simple en un string (aka "serializar")
 *   self: comando simple a convertir
 *   Returns: un string con la representación del comando simple similar
 *     a lo que se escribe en un shell
 * Requires: self!=NULL
 * Ensures: scommand_is_empty(self) ||
 *   scommand_get_redir_in(self)==NULL || scommand_get_redir_in(self)==NULL ||
 *   blength(result)>0
 */
	