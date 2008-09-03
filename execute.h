#include "command.h"

#ifndef EXECUTE_H
#define EXECUTE_H


#define EXIT 10


/* Ejecución de todos los comandos de una pipeline
 * RETURNS:
 *	El entero devuelto indica el resultado de la ejecución: 
 * si es 0 todos los comandos terminaron bien, si es 10 se proveyó
 * una instrucción "exit" y hay que cerrar el bash. Cualquier otro
 * entero indica un error de ejecución.
 * REQUIRES:
 *	pipe != NULL
 */
int exec_pipe (pipeline *spipe);


/* Ejecución in-situ de un comando no interno
 * cmd_no --> posición del comando dentro de su pipeline
 * pipes_no --> total de pipes que serán creadas para el
 *              pipeline donde el comando reside
 * pipe_fd --> todas las pipes de dicho pipeline
 * scmd_len --> largo del comando a ejecutar
 * REQUIRES:
 *	scmd != NULL;
 */
void exe_cmd_nbin (scommand *scmd, unsigned int scmd_len, int **pipe_fd,
		   unsigned int pipes_no, unsigned int cmd_no);


/* Ejecución de un comando interno. Lo debe ejecutar directamente
 * el baash padre, sin forkeo de hijos. 
 * REQUIRES:
 *	scmd != NULL && !scommand_is_empty (scmd)
 * RETURNS:
 *	El entero devuelto es: 0 en caso de correcta ejecución o
 * ausencia de ella; EXIT en caso de haber recibido una señal de exit;
 * cualquier otro  entero en caso de error.
 */
int exe_cmd_bin (scommand *scmd);


#endif
