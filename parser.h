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

/*caracter que determina el fin de la linea, osea la serie de caracteres*/
/*tener en cuenta que debe ser un char: const char PARSER_END_LINE = '&'*/
/*#define PARSER_END_LINE "&" */
/*const char PARSER_END_LINE = '&';*/

/*estos caracteres nos dicen si hay un fin de comando, teniendo en cuenta BLANK
 *de LEXER
*/
#define PARSER_END_CMD "|<>&" BLANK



typedef struct parser_s parser;
typedef enum {
	STATE_EXIT,   /* salimos */
	STATE_CMD,    /* es un comando o un argumento */
	STATE_DIR_IN, /* ES UN DIR_IN */
	STATE_DIR_OUT,/* ES UN DIR_OUT */
	STATE_PIPE,   /* ES UN PIPE */
	STATE_NO_WAIT /* termina */
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
		result->state = STATE_CMD // siempre empieza con comando
		result != NULL
*/

/************ misc	******/

bool continue_reading (bstring endl);
/* funcion que determina si tiene que seguir leyendo o no
 * en caso de endl ser NULL => result = false
*/

parser_state get_state (bstring endl);
/* no nos hace falta ningun "requerimiento" para llamar a esta funcion
 * returns: estado "proximo" que determina que tiene que hacer el parser
*/

#endif
