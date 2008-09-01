#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"


struct parser_s {
	Lexer* lex; 		/*lexer para el parseo*/
	bstring strtmp; 	/*vamos a almacenar temporalmente lo obtenido del lexer*/
	parser_state state; 	/*vamos a determinar 5 estados:
				0->parar de leer = SALIR
				1->comando o argumentos
				2->dirIn
				3->dirOut
				4->pipeline
				5->no_wait
			   */
	parser_error err;
};

/****************	FUNCIONES INTERNAS	***********************/
/* esta funcion setea el "estado proximo" del parser, para saber donde tiene
 * que introducir el siguiente "cmd" obtenido de la cadena de caracteres
 	REQUIRES:
 		self != NULL
 *	
 * en caso de error setea en PARSER_STATE_EXIT
 */
static void parser_set_state (parser* self){
	bstring aux = NULL;
		
	assert (self != NULL);
	
	/*vamos a setear el state como "normal", osea como cmd siempre, ya que
	*de caso contrario automaticamente se setea el tipo correcto mas abajo*/
	self->state = PARSER_STATE_CMD;
	/*primero tomamos el caracter de "operador" que determina alguno de los
	*estados*/
	lexer_next_char (self->lex, PARSER_END_CMD);
	
	if (!lexer_is_off (self->lex)) {
		aux = lexer_item (self->lex);	
	}
	else{
		/*vamos a producir un "abort" forzoso*/
		self->err = PARSER_ERROR; 
		self->state = PARSER_STATE_EXIT;
	}
	
	if (bstrchrp (aux, '\n', 0) == 0){
		self->state = PARSER_STATE_EXIT;
	}else if (bstrchrp (aux, '>', 0) == 0){
		self->state = PARSER_STATE_DIR_OUT;
	}else if (bstrchrp (aux, '<', 0) == 0){
		self->state = PARSER_STATE_DIR_IN;
	}else if (bstrchrp (aux, '|', 0) == 0){
		self->state = PARSER_STATE_PIPE;
	}else if (bstrchrp (aux, '&', 0) == 0){
		self->state = PARSER_STATE_NO_WAIT;
	}
	
	bdestroy (aux);
	/*si no entro en ninguna de esas es porque hay un espacio en blanco
	*por lo que tenemos que seguir leyendo la cadena de caracteres */
}



/* Esta funcion nos devuelve el nombre del cmd/args/dirout/dirin, que luego
 * segun el "state" del parser va a determinar su accion.
 	REQUIRES:
 		self != NULL
 	returns:
 		NULL => si no hay nada que devolver o hay un "error"
 		bstring => caso contrario
 		
 	se podria decir que se asegura en todo momento que no se haya terminado
 	de leer la cadena de caracteres, en caso de haberse terminado returns NULL
 */
static bstring parser_get_bstrcmd (parser* self){
	bstring result = NULL;
	
	assert (self != NULL);

	/*primero vamos a limpiar los espacios en blanco*/
	lexer_skip (self->lex, PARSER_BLANK);
	
	/*ahora leemos el el "dato" si es posible*/
	lexer_next_to (self->lex, PARSER_END_CMD);
	
	/*si es posible lo apuntamos en result*/
	if (!lexer_is_off (self->lex))
		result = lexer_item (self->lex);
	
	/*limpiamos los espacios en blanco posteriores*/
	lexer_skip (self->lex, PARSER_BLANK);
	/*en caso de no poder obtener el dato devolvemos NULL*/
	return result;
}


/*esta funcion nos borra todo un scommand, libera toda la memoria que puede
 *estar apuntando el scommand.
 *	REQUIRES:
 		self != NULL
*/
static void parser_free_scommand (scommand* self){
	unsigned int i = 0;
	bstring aux = NULL;
	
	assert (self != NULL);
	
	for (i = scommand_length (self); i > 0; i--){
		aux = scommand_front (self);
		bdestroy (aux);
		aux = NULL;
		scommand_pop_front (self);
	}
	
	aux = scommand_get_redir_in (self);
	bdestroy (aux);
	
	aux = scommand_get_redir_out (self);
	bdestroy (aux);
	
	scommand_destroy (self);
	
}
	
	
	

/***********************************************************************/
/****************	FUNCIONES EXTERNAS	************************/

parser* parser_new (void){
	parser * result = NULL;
	
	
	result = (parser *) calloc (1, sizeof (struct parser_s));
	
	assert (result != NULL);
	
	result->err = PARSER_NO_ERROR; 		/*no tenemos ningun error*/
	result->lex = lexer_new (PARSER_IN);	/*le pasamos "stdin"*/ 
	/*se asegura lexer de que se haya creado*/
	result->strtmp = NULL; /*no lo inicializamos*/
 	result->state = PARSER_STATE_EXIT;
	
	return result;
}

void parser_destroy (parser* self){
	assert (self != NULL);
	
	if (self->lex != NULL) /*por las dudas*/
		lexer_destroy (self->lex);
		
	free (self);
}
	





pipeline* parse_pipeline (parser* self){
	pipeline * result = NULL;
	scommand * scmd = NULL;
	unsigned int i = 0;
	
	assert (self != NULL);
	/*seteamos el estado inicial que es de "command"*/
	self->state = PARSER_STATE_CMD;
	scmd = scommand_new (); /*inicializamos el scmd y el pipe*/
	result = pipeline_new ();
	
	
	
	while (self->state != PARSER_STATE_EXIT && self->err == PARSER_NO_ERROR){
		/*mientras tengamos que leer, y no haya ningun error*/
		
		self->strtmp = parser_get_bstrcmd (self);/*tomamos el cmd*/
		
		/*este switch probablemente seria mejor con un arreglo de punteros
		 *a funciones*/
		switch (self->state){
			
			case PARSER_STATE_CMD: /*metemos el cmd o argumento al scmd*/
				if (self->strtmp != NULL)
					scommand_push_back (scmd, self->strtmp);
				break;
			
			case PARSER_STATE_DIR_OUT:
				/*aca podriamos tener en cuenta si ya hay un
				redir_out seteado*/
				if (blength (self->strtmp) == 0){ /*error*/
					self->err = PARSER_ERROR_NO_DIR_OUT;
				}
				else
					scommand_set_redir_out (scmd, self->strtmp);
				break;
				
			case PARSER_STATE_DIR_IN:
				if (blength (self->strtmp) == 0){ /*error*/
					self->err = PARSER_ERROR_NO_DIR_IN;
				}
				else
					scommand_set_redir_in (scmd, self->strtmp);
				break;
			
			case PARSER_STATE_PIPE:
				/*verificamos errores*/
				if (blength (self->strtmp) == 0){
					self->err = PARSER_ERROR_SINTAXIS;
				}
				else{
					/*ahora tenemos que chequear si es builtin
					  o no*/
					scommand_set_builtin (scmd,
							builtin_scommand_is (scmd));
					pipeline_push_back (result, scmd);
					scmd = scommand_new (); /*generamos nuevo*/
					/*y le introducimos la ultima info tomada*/
					if (self->strtmp != NULL)
						scommand_push_back (scmd,
							self->strtmp);
				}
					
				break;
				
			case PARSER_STATE_EXIT:
				/*no deberia entrar nunca*/
				break;
			
			case PARSER_STATE_NO_WAIT:
				if (blength (self->strtmp) != 0){
					self->err = PARSER_ERROR_SINTAXIS;
				}
				else
					pipeline_set_wait (result, false);
				break;
			
		}
		/*seteamos el proximo estado y en caso de error salimos*/
		parser_set_state (self);	
	}
	/*si salimos porque hubo algun error entonces tenemos que "comer" hasta
	 *el \n, y ademas vamos a liberar toda la memoria usada*/
	if (self->err != PARSER_NO_ERROR){
		lexer_skip_to (self->lex, "\n"); /*comemos hasta fin de linea*/
		bdestroy (self->strtmp); /*destruimos lo ultimo tomado*/
		parser_free_scommand (scmd); /*borramos el ultimo scommand*/
		scmd = NULL;
		for (i = pipeline_length (result); i > (unsigned int ) 0; i--){
			scmd = pipeline_front (result);
			parser_free_scommand (scmd);
			pipeline_pop_front (result);
			scmd = NULL;
		}
		pipeline_destroy (result);
		return NULL; /*devolvemos null*/
	}
		
	/*agregamos el ultimo scmd hecho*/
	if (scmd != NULL){ 
		/*seteamos si es builtin*/
		scommand_set_builtin (scmd, builtin_scommand_is (scmd));
		pipeline_push_back (result, scmd);
	}	
	
	return result;
}


parser_error parser_get_error (parser * self){
	assert (self != NULL);
	return self->err;
}

