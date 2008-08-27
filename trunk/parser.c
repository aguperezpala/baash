#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"


struct parser_s {
	Lexer* lex; /*lexer para el parseo*/
	pipeline* pipe; /*vamos a apuntar al ultimo pipeline =*/
	scommand* scmd; /*scmd temporal*/
	bstring strtmp; /*vamos a almacenar temporalmente lo obtenido del lexer*/
	parser_state state; /*vamos a determinar 5 estados:
			0->parar de leer = SALIR
			1->comando o argumentos
			2->dirIn
			3->dirOut
			4->pipeline
			5->no_wait
		   */
};

parser* parser_new (void){
	parser * result = NULL;
	
	
	result = (parser *) calloc (1, sizeof (struct parser_s));
	
	assert (result != NULL);
		
	result->lex = lexer_new (PARSER_IN);/*le pasamos "stdin"*/ 
	/*se asegura lexer de que se haya creado*/
	result->strtmp = NULL; /*no lo inicializamos*/
	result->pipe = NULL;
	result->state = STATE_EXIT;
	
	return result;
}

void parser_destroy (parser* self){
	assert (self != NULL);
	
	if (self->lex != NULL) /*por las dudas*/
		lexer_destroy (self->lex);
	
	if (self->strtmp != NULL)
		bdestroy (self->strtmp);
	
	free (self); /*tener en cuenta que el pipeline no quede inalcanzable*/
}


bool continue_reading (bstring endl){
	bool result = false;
	
	if (endl != NULL)
		result = (bstrchrp (endl, '\n', 0) == 0);
	
	return result;
}
	
/* no nos hace falta ningun "requerimiento" para llamar a esta funcion*/
parser_state get_state (bstring endl){
	
	if (bstrchrp (endl, '\n', 0) == 0)
		return STATE_EXIT;
		
	if (bstrchrp (endl, '>', 0) == 0)
		return STATE_DIR_OUT;
	
	if (bstrchrp (endl, '<', 0) == 0)
		return STATE_DIR_IN;
	
	if (bstrchrp (endl, '|', 0) == 0)
		return STATE_PIPE;
	
	if (bstrchrp (endl, '&', 0) == 0)
		return STATE_NO_WAIT;
		
	/*si no se da ninguno de los casos anteriores entonces estamos en cmd*/
	return STATE_CMD;
}
	
	
/*ls -a -l -j > gran_mono | grep < fede*/

pipeline* parse_pipeline (parser* self){
	bstring aux = NULL;
	
	assert (self != NULL);
	/*seteamos el estado inicial que es de "command"*/
	self->state = STATE_CMD;
	self->scmd = scommand_new (); /*inicializamos el scmd y el pipe*/
	self->pipe = pipeline_new ();
	
	
	
	while (self->state != STATE_EXIT && self->state != STATE_NO_WAIT){
		/*mientras tengamos que leer*/
		
		lexer_next_to (self->lex, PARSER_END_CMD);/*leemos*/
		self->strtmp = lexer_item (self->lex); /*almacenamos temp*/
	
		/*ahora obtenemos el ultimo caracter, y queda almacenado en
		*lexer_item ();*/
		lexer_next_char (self->lex, PARSER_END_CMD);
		aux = lexer_item (self->lex);
			
		/*ACA TENEMOS QUE CHEQUEAR QUE self->strtmp != NULL, creo quede
		 * nunca sucede pero revisar mejor */
		
		/*este switch probablemente seria mejor con un arreglo de punteros
		 *a funciones*/
		switch (self->state){
			
			case STATE_CMD: /*metemos el cmd o argumento al scmd*/
				scommand_push_back (self->scmd, self->strtmp);
				break;
			
			case STATE_DIR_OUT:
				scommand_set_redir_out (self->scmd, self->strtmp);
				break;
				
			case STATE_DIR_IN:
				scommand_set_redir_in (self->scmd, self->strtmp);
				break;
			
			case STATE_PIPE:
				/*introducimos el scommand*/
				pipeline_push_back (self->pipe, self->scmd);
				self->scmd = scommand_new (); /*generamos nuevo*/
				break;
				
			case STATE_EXIT:
				/*no deberia entrar nunca*/
				break;
			
			case STATE_NO_WAIT:
				/*no deberia entrar nunca*/
				break;
			
		}
		/*salteamos los espacios en blanco*/
		lexer_skip (self->lex, BLANK);
		
		self->state = get_state (aux); /*obtenemos el estado "proximo"*/
				
		bdestroy (aux); /*eliminamos aux*/
	}
	
	if (self->scmd != NULL) /*agregamos el ultimo scmd hecho*/
		pipeline_push_back (self->pipe, self->scmd);
	
		
	return self->pipe;
}
	
	
