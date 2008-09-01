/* vamos a considerar a parser como una estructura que contiene un Lexer,
 * y un puntero a el ultimo pipeline
 */
#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "bstring/bstrlib.h"
#include "command.h" /*el scommand y el pipeline*/
#include "lexer.h"

/*******	DEFINES		***********/
/*definimos el stdin*/
#define PARSER_IN stdin

/*tenemos que tener en cuenta que el parser va a dejar de verificar cuando 
 *encuentre o un \n o un &, una vez encontrado alguno de esos dos caracteres
 *deja de parsear
*/
/*estos caracteres nos dicen si hay un fin de comando, teniendo en cuenta BLANK
 *de LEXER
*/
#define PARSER_BLANK "\t\r "
#define PARSER_END_CMD "|<>&\n" PARSER_BLANK



typedef struct parser_s parser;


typedef enum {
	PARSER_NO_ERROR, /*no hay error*/
	PARSER_ERROR,	 /*hay error*/
	PARSER_ERROR_NO_DIR_IN,
	PARSER_ERROR_NO_DIR_OUT,
	PARSER_ERROR_NO_CMD,  /*no hay commando*/
	PARSER_ERROR_SINTAXIS /*error desconocido*/
} parser_error;

typedef enum {
	PARSER_STATE_EXIT,   /* salimos */
	PARSER_STATE_CMD,    /* es un comando o un argumento */
	PARSER_STATE_DIR_IN, /* ES UN DIR_IN */
	PARSER_STATE_DIR_OUT,/* ES UN DIR_OUT */
	PARSER_STATE_PIPE,   /* ES UN PIPE */
	PARSER_STATE_NO_WAIT /* termina */
} parser_state;



parser* parser_new (void);
/* genera el parser para leer desde el stdin
	ENSURES:
		"result->lexer != NULL (STARTED)"
		result != NULL
*/

void parser_destroy (parser* self);
/* destruimos el parser, tener en cuenta que solo el lexer es "liberado",
 * lo demas queda para el bash.
 	REQUIRES:
 		self != NULL
*/

pipeline* parse_pipeline (parser* self);
/* convierte lo de PARSER_IN (stdin) en un pipeline
	REQUIRES:
		self != NULL
	ENSURES:
		result->state = PARSER_STATE_CMD // siempre empieza con comando
		result != NULL
*/

/************ misc	******/

parser_error parser_set_state (parser* self);
/* esta funcion setea el "estado proximo" del parser, para saber donde tiene
 * que introducir el siguiente "cmd" obtenido de la cadena de caracteres
 	REQUIRES:
 		self != NULL
 *	
 * en caso de error setea en PARSER_STATE_EXIT
 * returns ERR_NUM 
*/

bstring parser_get_bstrcmd (parser* self);
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
#endif
