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
	GQueue * args; /* nombre del comando y sus argumentos, se usa una cola */
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
	bstring aux = NULL;
	/* REQUIRES */
	assert (self != NULL);
	
	bdestroy (self->dirOut);
	bdestroy (self->dirIn);
	
	if (self->args != NULL) {
		/* si hay argumentos hay que liberarlos antes de destruir la cola */
		while(!g_queue_is_empty (self->args)) {
			aux = g_queue_pop_head (self->args);
			bdestroy (aux);
		}
		g_queue_free (self->args);
	}
	
	free (self);
}

/*** Modificadores ***/

void scommand_push_back (scommand *self, bstring argument)
{	/* encolamos por detrás :D */
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
	bdestroy (aux);
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
		bassign (self->dirIn, (const_bstring) filename);
	} else {
		self->dirIn = bfromcstr ("");
	}
}

void scommand_set_redir_out (scommand *self, bstring filename)
{	/* setea con 'filename' la redireccion de salida */
	/* REQUIRES */
	assert (self != NULL);

	if (filename != NULL && blength (filename)>0 ) {
		bassign (self->dirIn, (const_bstring) filename);
	} else {
		self->dirIn = bfromcstr ("");
	}
}

/*** Proyectores ***/

bool scommand_is_empty (const scommand *self)
{	/* chequea si el scommand está vacío (no hay comando ni argumentos) */
	/* REQUIRES */
	assert (self != NULL);
	
	return g_queue_is_empty (self->args);
/* ese casteo da error...ver */
}

unsigned int scommand_length (const scommand *self)
{	/* devuelve la cantidad de comandos + argumentos del scommand */
	/* REQUIRES */
	assert (self != NULL);
	
	if (!scommand_is_empty (self)) {
		return (unsigned int) g_queue_get_length (self->args);
	} else {
		return (unsigned int) 0;
	}
}

bstring scommand_front (const scommand *self)
{	/* devuelve una copia del primer elemento de la cola */
	bstring result = NULL;
	/* REQUIRES */
	assert (self != NULL);
	assert (!scommand_is_empty (self));
	
	result = bstrcpy ((const_bstring) g_queue_peek_head (self->args));
	
	/* ENSURES */
	assert (result != NULL);
	
	return result;
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
	 * si el comando es vacío devuelve una cadena vacía de BASE_ALLOC lugares
	 */
	bstring result = NULL , aux = NULL;
	unsigned int n = scommand_length (self);
	int i = 0;
	/* REQUIRES */
	assert (self != NULL);
	
	result = bfromcstralloc (BASE_ALLOC,"");
	
	/* si hay comandos o argumentos en el scommand los concatenaremos 
	 * de a uno con result, con una separacion de \t entre ellos
	 */
	while ((unsigned int) i < n) {
		/* método forzado por el casteo 'const' del parámetro */
		aux = bstrcpy ((const_bstring) g_queue_peek_nth (self->args, i+1));
		bcatcstr (result, "\t");
		bconcat (result, (const_bstring) aux);
		bdestroy (aux);
		i++;
	}
	/*	aux = scommand_front (self);
		scommand_pop_front (self);
		bcatcstr (result, (const char *) "\t");
		bconcat (result, (const_bstring) aux);
		scommand_push_back (self, aux);
		i++;
		
		Forma sugerida en el .h para tocar cosas del scommand
		El compilador se queja porque tocamos el parámetro de
		 la función, que esta castedo con 'const'
		Si se elimina ese casteo esta versión funciona bien, 
		 y tiene la ventaja de usar funciones ya definidas.
	}*/
	
	/* ENSURES */
	assert (scommand_is_empty (self) || (blength (result)>0));
	
	return result;
}

