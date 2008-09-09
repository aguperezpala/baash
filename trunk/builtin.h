/* Vamos a usar un define de una cadena de caraceteres, donde cada commando
 * "builtin" va a estar separado de la siguiente forma <cmd1><cmd2>etc..
 * De esta forma vamos a chequear facilmente si existe o no existe un CMD
 * interno.
*/
#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "bstring/bstrlib.h"
#include "command.h"

/*en esta cadena definimos los commandos que son internos y los "encerramos"
 *entre <>*/
#define BUILTIN_COMMANDS "<cd><exit>"
#define EXIT (10)


bool builtin_scommand_is (scommand * self);
/* esta funcion nos determina si algun scommand es interno o no
 * 	REQUIRES:
 		self != NULL
 		
*/


/* Ejecución de un comando interno. Lo debe ejecutar directamente
 * el baash padre, sin forkeo de hijos. 
 * REQUIRES:
 *	scmd != NULL && !scommand_is_empty (scmd)
 * RETURNS:
 *	El entero devuelto es: 0 en caso de correcta ejecución o
 * ausencia de ella; EXIT en caso de haber recibido una señal de exit;
 * cualquier otro  entero en caso de error.
 * ENSURES:
 *	El comando *scmd no es modificado
 */
int exe_cmd_bin (scommand *scmd);




#endif
