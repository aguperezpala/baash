/* El parser va tener una estructura que va a almacenar el ultimo error, si es 
 * que hubo alguno (el cual puede ser obtenido desde parser_get_error()).
 * Ademas en caso de que se produzca un error vamos a liberar toda la memoria
 * y devolver NULL. Esto sucede en alguno de los casos PARSE_ERROR_*.
 *
 */
#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "bstring/bstrlib.h"
#include "command.h" /*el scommand y el pipeline*/
#include "lexer.h"
#include "builtin.h"

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
	PARSER_NO_ERROR, 	/*no hay error*/
	PARSER_EOF,	 	/*hay error alguno*/
	PARSER_ERROR_NO_DIR_IN, /*no redirin*/
	PARSER_ERROR_NO_DIR_OUT,/*no redirout*/
	PARSER_ERROR_NO_CMD,    /*no hay commando, no hacemos nada*/
	PARSER_ERROR_SINTAXIS   /*error desconocido*/
} parser_error;



parser* parser_new (void);
/* genera el parser para leer desde el stdin
	ENSURES:
		result != NULL
*/

void parser_destroy (parser* self);
/* destruimos el parser, tener en cuenta que solo el lexer es "liberado",
 * lo demas queda para el bash.
 	REQUIRES:
 		self != NULL
*/


GQueue* parse_pipeline (parser* self);
/* convierte lo de PARSER_IN (stdin en este caso) en un pipeline
	REQUIRES:
		self != NULL
	ENSURES:
		result != NULL
*/

parser_error parser_get_error (parser * self);
/*esta funcion devuelve el estado de error del self (del enum parser_error).
 *Tener en cuenta que esto sirve para poder leer el error externamente.
 *	REQUIRES:
 		self != NULL
*/

#endif
