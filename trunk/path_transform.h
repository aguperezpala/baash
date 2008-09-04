#ifndef PARSER_H
#define PARSER_H

#define PATH_ERR (-1)
#define PATH_OK  (0)

/* Reemplaza el nombre de los comandos de los scommand de un pipeline por sus 
 * rutas absolutas, asi quedan listos para ser ejecutados.
 */ 


#include "bstring/bstrlib.h"
#include "command.h" /*el scommand y el pipeline*/

int path_transform (pipeline *pipe);
 /* convierte todos los comandos de los scommands de pipe 
  * a sus rutas absolutas correspondiente
  */
 
int path_transform_scommand (scommand *scmd);
 /* convierte el comando de un scmd a una ruta absoluta */ 
