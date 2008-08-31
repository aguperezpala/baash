#ifndef EXECUTE_H
#define EXECUTE_H

/* Ejecución de todos los comandos de una pipeline
 * RETURNS:
 *	El entero devuelto indica la correcta/incorrecta ejecución: 
 * si es 0 todos los comandos terminaron bien; si es 1 ocurrió algún
 * error que será impreso por stderr
 * REQUIRES:
 *	pipe != NULL
 * ENSURES:
 *	¡¡¡ Lots of happyness !!!
 */
int exec_pipe (pipeline *pipe);
