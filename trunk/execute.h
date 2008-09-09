#ifndef EXECUTE_H
#define EXECUTE_H

#include "command.h"
#include "builtin.h"


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
 * REQUIRES:
 *	scmd != NULL;
 * ENSURES:
 * 	El comando *scmd no es modificado
 */
void exe_cmd_nbin (scommand *scmd, int **pipeFD,
		    unsigned int pipesToll, unsigned int cmdN);




#endif
