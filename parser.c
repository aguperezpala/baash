#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"


struct parser_s {
	Lexer* lex; /*lexer para el parseo*/
	bstring strtmp; /*vamos a almacenar temporalmente lo obtenido del lexer*/
	parser_state state; /*vamos a determinar 5 estados:
			0->parar de leer = SALIR
			1->comando o argumentos
			2->dirIn
			3->dirOut
			4->pipeline
			5->no_wait
		   */
	parser_error err;
};

parser* parser_new (void){
	parser * result = NULL;
	
	
	result = (parser *) calloc (1, sizeof (struct parser_s));
	
	assert (result != NULL);
	
	result->err = PARSER_NO_ERROR; /*no tenemos ningun error*/
	result->lex = lexer_new (PARSER_IN);/*le pasamos "stdin"*/ 
	/*se asegura lexer de que se haya creado*/
	result->strtmp = NULL; /*no lo inicializamos*/
	result->state = PARSER_STATE_EXIT;
	
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
	

parser_error parser_set_state (parser* self){
	bstring aux = NULL;
	parser_error result = PARSER_NO_ERROR;
	/*en caso de que sea null o sea fin de linea terminamos*/
	
	assert (self != NULL);
	
	/*vamos a setear el state como "normal", osea como cmd siempre, ya que
	 *de caso contrario automaticamente se setea el tipo correcto mas abajo*/
	self->state = PARSER_STATE_CMD;
	/*primero tomamos el caracter de "operador" que determina alguno de los
	 * estados
	*/
	lexer_next_char (self->lex, PARSER_END_CMD);
	
	if (!lexer_is_off (self->lex))
		if ((aux = lexer_item (self->lex)) == NULL){ /*tomamos el caracter*/
			self->state = PARSER_STATE_EXIT;
			return PARSER_ERROR;
		}
	
	if (bstrchrp (aux, '\n', 0) == 0){
		self->state = PARSER_STATE_EXIT;
		result = PARSER_NO_ERROR;
	}else if (bstrchrp (aux, '>', 0) == 0){
		self->state = PARSER_STATE_DIR_OUT;
		result = PARSER_NO_ERROR;
	}else if (bstrchrp (aux, '<', 0) == 0){
		self->state = PARSER_STATE_DIR_IN;
		result = PARSER_NO_ERROR;
	}else if (bstrchrp (aux, '|', 0) == 0){
		self->state = PARSER_STATE_PIPE;
		result = PARSER_NO_ERROR;
	}else if (bstrchrp (aux, '&', 0) == 0){
		self->state = PARSER_STATE_NO_WAIT;
		result = PARSER_NO_ERROR;
	}
	
	
	bdestroy (aux);
	/*si no entro en ninguna de esas es porque hay un espacio en blanco
	 *por lo que tenemos que seguir leyendo la cadena de caracteres */
	return result;
}

bstring parser_get_bstrcmd (parser* self){
	bstring result = NULL;
	
	assert (self != NULL);
	
	/*tenemos que tener en cuenta que cada vez que vamos a usar alguna func.
	 *del lexer tenemos que corroborar que este "prendido"
	*/
	/*primero vamos a limpiar los espacios en blanco*/
	lexer_skip (self->lex, BLANK);
	
	/*ahora leemos el el "dato" si es posible*/
	lexer_next_to (self->lex, PARSER_END_CMD);
	
	/*si es posible lo apuntamos en result*/
	if (!lexer_is_off (self->lex))
		result = lexer_item (self->lex);
	
	lexer_skip (self->lex, BLANK);
	/*en caso de no poder obtener el dato devolvemos NULL*/
	return result;
}
	
	


pipeline* parse_pipeline (parser* self){
	pipeline * result = NULL;
	scommand * scmd = NULL;
	
	assert (self != NULL);
	/*seteamos el estado inicial que es de "command"*/
	self->state = PARSER_STATE_CMD;
	scmd = scommand_new (); /*inicializamos el scmd y el pipe*/
	result = pipeline_new ();
	
	
	
	while (self->state != PARSER_STATE_EXIT && self->err == PARSER_NO_ERROR){
		/*mientras tengamos que leer, y no se haya ningun error*/
		
		if ((self->strtmp = parser_get_bstrcmd (self)) == NULL){
			/*salimos del ciclo, ya no hay mas nada que leer*/
			break;
		}
		
		/*este switch probablemente seria mejor con un arreglo de punteros
		 *a funciones*/
		switch (self->state){
			
			case PARSER_STATE_CMD: /*metemos el cmd o argumento al scmd*/
				scommand_push_back (scmd, self->strtmp);
				break;
			
			case PARSER_STATE_DIR_OUT:
				/*aca podriamos tener en cuenta si ya hay un
				redir_out seteado*/
				if (blength (self->strtmp) == 0){ /*error*/
					printf ("ERRORRRR NO OUT FILE");
					self->err = PARSER_ERROR_DIR_OUT;
				}
				scommand_set_redir_out (scmd, self->strtmp);
				break;
				
			case PARSER_STATE_DIR_IN:
				if (blength (self->strtmp) == 0){ /*error*/
					printf ("ERRORRRR NO IN FILE");
					self->err = PARSER_ERROR_DIR_IN;
				}
				scommand_set_redir_in (scmd, self->strtmp);
				break;
			
			case PARSER_STATE_PIPE:
				/*introducimos el scommand*/
				pipeline_push_back (result, scmd);
				scmd = scommand_new (); /*generamos nuevo*/
				/*y le introducimos la ultima info tomada*/
				scommand_push_back (scmd, self->strtmp);
				
				break;
				
			case PARSER_STATE_EXIT:
				/*no deberia entrar nunca*/
				break;
			
			case PARSER_STATE_NO_WAIT:
				pipeline_set_wait (result, false);
				/*deberiamos hacer un nuevo scommand, pero sin
				*esperar, ta complicado, seria un nuevo pipe*/
				break;
			
		}
		/*seteamos el proximo estado y en caso de error salimos*/
		if (parser_set_state (self) == PARSER_ERROR)
			break;
			
		
		
		
	}
	if (self->state == PARSER_STATE_NO_WAIT)
		pipeline_set_wait (result, false);
	if (scmd != NULL) /*agregamos el ultimo scmd hecho*/
		pipeline_push_back (result, scmd);
	
		
	return result;
}
	
	
