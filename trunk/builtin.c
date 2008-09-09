#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

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
	bconcat (cmd, (const_bstring) scommand_front (self));
	bconchar (cmd, '>');
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


/* Ejecución de comandos internos. Preparado sólo para "cd" y "exit" */
int exe_cmd_bin (scommand *scmd)
{
	bstring command = NULL , arg = NULL, home = NULL;
	int exitStatus = 0; char * aux = NULL;
	
	/* REQUIRES */
	assert (scmd != NULL);
	assert (!scommand_is_empty (scmd));
	
	command = scommand_front (scmd);
	scommand_pop_front (scmd);
	if (!scommand_is_empty (scmd))
		arg = scommand_front (scmd);
	scommand_push_front (scmd, command);
	
	if (0 == strncmp ((char*) command->data, "cd", 2)) {
	/* Nos pasaron un cd, vamos al directorio proveído
	 * chdir() se encarga solito de reportar errores
	 */
	 	if (arg != NULL && blength (arg) > 0) {
	 		if (bisstemeqblk (arg, "~", 1) == 1) {
	 		 	home = bfromcstr (getenv ("HOME"));
	 		 	assert (home != NULL);
	 		 	breplace (arg, 0, 1, home, ' ');
	 		 	bdestroy (home);
	 		}
			exitStatus = chdir ((char*) arg->data);
		} else {
			/* si llamamos a cd sin argumentos, vamos a $HOME */
			aux = getenv ("HOME");
			exitStatus = chdir (aux);
		}
	} else if (0 == strncmp ((char*) command->data, "exit", 4)) {
	/* Nos pasaron un exit, salimoooo */
		exitStatus = EXIT;
	}
	
	command = NULL;
	arg = NULL;
	
	return exitStatus;
}


