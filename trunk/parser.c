#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"


typedef enum {
	PARSER_STATE_EXIT,   /* salimos */
	PARSER_STATE_CMD,    /* es un comando o un argumento */
	PARSER_STATE_DIR_IN, /* ES UN DIR_IN */
	PARSER_STATE_DIR_OUT,/* ES UN DIR_OUT */
	PARSER_STATE_PIPE,   /* ES UN PIPE */
	PARSER_STATE_NO_WAIT /* termina */
} parser_state;



struct parser_s {
	Lexer* lex; 		/*lexer para el parseo*/
	bstring strtmp; 	/* almacenar lo obtenido del lexer*/
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
	if (!lexer_is_off (self->lex))	
		lexer_next_char (self->lex, PARSER_END_CMD);
	
	if (!lexer_is_off (self->lex)) {
		aux = lexer_item (self->lex);	
	}
	else{
		/*vamos a producir un "abort" forzoso*/
		self->err = PARSER_EOF; 
		self->state = PARSER_STATE_EXIT;
	}
	
	if (bstrchrp (aux, '\n', 0) == 0 || lexer_is_off (self->lex)){
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



/* Esta funcion nos devuelve el nombre del cmd/args/redirout/redirin, que luego
 * segun el "state" del parser va a determinar su accion.
 	REQUIRES:
 		self != NULL
 	returns:
 		NULL => si no hay nada que devolver o hay un "error"
 		bstring => caso contrario
 		
 	se podria decir que se asegura en todo momento que no se haya terminado
 	de leer la cadena de caracteres, en caso de haber terminado returns NULL
 */
static bstring parser_get_bstrcmd (parser* self){
	bstring result = NULL;
	
	assert (self != NULL);

	/*primero vamos a limpiar los espacios en blanco, el PARSER_END_CMD
	 * es para que no tome un enter como un comando */
	lexer_skip (self->lex, PARSER_BLANK);
	
	/*ahora leemos el el "dato" si es posible*/
	if (!lexer_is_off (self->lex))
		lexer_next_to (self->lex, PARSER_END_CMD);
	
	/*si es posible lo apuntamos en result*/
	if (!lexer_is_off (self->lex))
		result = lexer_item (self->lex);
	
	/*limpiamos los espacios en blanco posteriores*/
	if (!lexer_is_off (self->lex))
		lexer_skip (self->lex, PARSER_BLANK);
	/*en caso de no poder obtener el dato devolvemos NULL*/
	return result;
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
	
	assert (self != NULL);
	/*seteamos el estado inicial que es de "command" y no hay error*/
	self->state = PARSER_STATE_CMD;
	self->err = PARSER_NO_ERROR;
	scmd = scommand_new (); /*inicializamos el scmd y el pipe*/
	result = pipeline_new ();
	
	
	
	while (self->state != PARSER_STATE_EXIT && self->err == PARSER_NO_ERROR
		&& !lexer_is_off (self->lex)){
		/*mientras tengamos que leer, y no haya ningun error*/
		
		self->strtmp = parser_get_bstrcmd (self);/*tomamos el cmd*/
		
		/*este switch probablemente seria mejor con un arreglo de punteros
		 *a funciones*/
		switch (self->state){
			
			case PARSER_STATE_CMD: /*metemos el cmd o argumento al scmd*/
				if (self->strtmp != NULL && 
					blength (self->strtmp) > 0)
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
	/*si salimos porque hubo algun error y no llegamos a comer hasta el
	 *final, entonces comemos las "sobras"*/
	if (self->err != PARSER_NO_ERROR){
		bdestroy (self->strtmp); /*destruimos lo ultimo tomado*/
		/*comemos hasta fin de linea si es que no llego*/
		if (self->state != PARSER_STATE_EXIT){
			lexer_skip_to (self->lex, "\n");
			if (!lexer_is_off (self->lex))
				lexer_next_char (self->lex, "\n");
			if (!lexer_is_off (self->lex)) {
				bdestroy (lexer_item (self->lex));
			}
		}
	}
	/*agregamos el ultimo scmd hecho*/
	/*vemos que no sea vacio*/
	if (!scommand_is_empty (scmd)){
		/*seteamos si es builtin*/
		scommand_set_builtin (scmd, builtin_scommand_is (scmd));
		pipeline_push_back (result, scmd);
	} else {
		/*eliminamos el "leak"*/
		bdestroy (self->strtmp);
		scommand_destroy (scmd);
	} 

	/*ultimo chequeo para ver si no se ingreso ningun commando*/
	if (pipeline_is_empty (result) && self->err == PARSER_NO_ERROR){
		self->err = PARSER_ERROR_NO_CMD;/*no hay commandos*/
	}
	/*ahora chequeamos que no se haya terminado de leer el stdin/archivo*/
	if (lexer_is_off (self->lex))
		self->err = PARSER_EOF;
	
	assert (result != NULL); /*nos aseguramos que no devuelva un NULL*/
	
	return result;
}


parser_error parser_get_error (parser * self){
	assert (self != NULL);
	return self->err;
}

