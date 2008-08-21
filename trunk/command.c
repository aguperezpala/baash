/*	Implementacion del command.c con el GQueue (cola)
 *	Vamos a guardar el nombre del comando en el args[0] de la estructura
*/
#include "command.h"
#include <glib.h>

struct scommand_s
{
	bool builtin; 
	GQueue * args; /*argumentos y nombre del representado por una cola*/
	bstring * dout;
	bstring * din;
};

scommand *scommand_new (void)
{
	scommand * result=NULL;
	
	result=(scommand *)calloc(1,sizeof(struct scommand_s));
	if(result==NULL)
		
	
	assert(result != NULL);
	assert(
	
	return result;
	
