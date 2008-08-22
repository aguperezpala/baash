#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>

#include "command.h"
#include "bstring/bstrlib.h"


struct pipeline_s {
	GQueue *scmd; /* scommand */
	bool wait;
};

pipeline *pipeline_new (void) {
	pipeline *p = NULL;
	p = calloc (1,sizeof (pipeline*));
	
	if (p!=NULL) {
		p->scmd = g_queue_new ();
		p->wait = true;
	}
	return p;
}

void pipeline_destroy (pipeline *self) {
	assert (self!=NULL);
	
	g_queue_free (self->scmd);
	free (self);
}

void pipeline_push_back (pipeline *self, scommand *sc) {
	assert (self!=NULL);
	assert (sc!=NULL);
	
	g_queue_push_tail (self->scmd,sc);
}

void pipeline_pop_front (pipeline *self) {
	assert (self!=NULL);
	assert (!pipeline_is_empty(self));
	
	g_queue_pop_head (self->scmd);
}

void pipeline_set_wait (pipeline *self, const bool w) {
	assert (self!=NULL);
	
	self->wait = w;
}

bool pipeline_is_empty (const pipeline *self) {
	assert (self!=NULL);
 
	
	return g_queue_is_empty (self->scmd);
}

unsigned int pipeline_length (const pipeline *self) {
	assert (self!=NULL);
	
	return g_queue_get_length (self->scmd);
}

scommand *pipeline_front (const pipeline *self) {
	assert (self!=NULL);
	assert (!pipeline_is_empty(self));
	
	return g_queue_peek_head (self->scmd);
}

bool pipeline_get_wait (const pipeline *self) {
	assert (self!=NULL);
	
	return self->wait;
}

bstring pipeline_to_string (const pipeline *self) {
	bstring result = NULL;
	scommand *scmd = NULL;
	unsigned int n = pipeline_length(self), i = 0;
	
	assert (self!=NULL);
	
	
	if (!pipeline_is_empty(self)) {
	/* ciclo para concatenar todos los scommand */	
		while (i<n) { 
			scmd = pipeline_front(self);
			pipeline_pop_front(self);
			bconcat (result, scommand_to_string (scmd));
		        /* fijarse el formato de salida (ponemos espacios?) */
			pipeline_push_back(self,scmd);
			i++;
		}
	}
	return result;
}





