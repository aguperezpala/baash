#ifndef PATH_TRANSFORM_H
#define PATH_TRANSFORM_H

#define PATH_OK  (0)
#define PATH_ERR (-1)

/* Reemplaza el nombre de los comandos de los scommand de un pipeline por sus 
 * rutas absolutas, asi quedan listos para ser ejecutados.
 */ 

#include "command.h" /*el scommand y el pipeline*/

int path_transform (pipeline *pipe);
 /* convierte todos los comandos de los scommands de pipe 
  * a sus rutas absolutas correspondientes.
  * REQUIRES:
  *	pipe != NULL
  *	!pipeline_is_empty (pipe)
  * RETURNS:
  *	PATH_OK		pipeline succesfully transformed.
  *	PATH_ERR	some command was no found, its left in
  *			pipeline_front (pipe) for easy error handling. 
  */ 
 
int path_transform_scommand (scommand *scmd);
 /* convierte el comando de un scmd a una ruta absoluta, 
  * es llamada por path_transform, pensada para uso interno.
  * REQUIRES:
  *	scmd != NULL;
  *	!scommand_is_empty (scmd)
  *
  * RETURNS:
  *	PATH_OK		scommand succesfully transformed.
  *	PATH_ERR	command not found.
  */
 
#endif
