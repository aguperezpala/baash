#include <assert.h>
#include <stdlib.h>
#include "builtin.h"

bool builtin_scommand_is (scommand * self){
	bool result = false;
	bstring constCmds = NULL; /*vamos a transformar los "cmds" en un bstring*/
	bstring cmd = NULL; /*cmd transformado para compararlo con la cadena de 
			     *commandos internos */
	
	assert (self != NULL);
	
	constCmds = bfromcstr (BUILTIN_COMMANDS); /*guardamos temporalmente*/
	
	/*ahora vamos hacer una copia directa del commando para poder modificarlo
	 *sin afectar el scommand verdadero, agregandole <>*/
	cmd = bfromcstr ("<");
	bconcat (constCmds, (const_bstring) scommand_front (self));
	bconchar (constCmds, '>');
	/*aca ya tendriamos constCmds = <command>, donde command es el nombre
	 *del commando en si*/
	
	if ((binstr (constCmds, 0, cmd)) < 0) /*no se encontro cmd*/
		result = false;
	else
		result = true;
	
	/*libreamos la memoria*/
	bdestroy (constCmds);
	bdestroy (cmd);
	
	return result;
}
