/* Vamos a usar un define de una cadena de caraceteres, donde cada commando
 * "builtin" va a estar separado de la siguiente forma <cmd1><cmd2>etc..
 * De esta forma vamos a chequear facilmente si existe o no existe un CMD
 * interno.
*/
#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdbool.h>
#include "bstring/bstrlib.h"
#include "command.h"

/*en esta cadena definimos los commandos que son internos y los "encerramos"
 *entre <>*/
#define BUILTIN_COMMANDS "<cd><exit>"



bool builtin_scommand_is (scommand * self);
/* esta funcion nos determina si algun scommand es interno o no
 * 	REQUIRES:
 		self != NULL
 		
*/






#endif
