#include "command.h"

#ifndef EXECUTE_H
#define EXECUTE_H

/* Ejecución de todos los comandos de una pipeline
 * RETURNS:
 *	El entero devuelto indica la correcta/incorrecta ejecución: 
 * si es 0 todos los comandos terminaron bien
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



#endif
