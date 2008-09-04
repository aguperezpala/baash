#include "command.h"

#ifndef EXECUTE_H
#define EXECUTE_H


#define EXIT 10


/* Ejecución de todos los comandos de una pipeline
 * REQUIRES:
 *	pipe != NULL
 * RETURNS:
 *	El entero devuelto indica el resultado de la ejecución: 
 * si es 0 todos los comandos terminaron bien, si es EXIT se proveyó
 * una instrucción "exit" y hay que cerrar el bash. Cualquier otro
 * entero indica un error de ejecución.
 * ENSURES:
 *	El pipeline *spipe no es modificado
 */
int exec_pipe (pipeline *spipe);


/* Ejecución in-situ de un comando no interno
 * cmdN --> posición del comando dentro de su pipeline
 * pipesToll --> total de pipes que serán creadas para el
 *               pipeline donde el comando reside
 * pipe_FD -->  arreglo con los file descriptors de las '|'
 * cmdLen --> largo del comando a ejecutar
 * REQUIRES:
 *	scmd != NULL;
 * ENSURES:
 * 	El comando *scmd no es modificado
 */
void exe_cmd_nbin (scommand *scmd, unsigned int cmdLen, int **pipeFD,
		   unsigned int pipesToll, unsigned int cmdN);


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
