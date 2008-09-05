#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "path_transform.h"

#define PATH PATH

int path_transform (pipeline *pipe) {
	unsigned int i = 0, n = 0; int err = 0;
	scommand *scmd = NULL;

	assert (!pipeline_is_empty (pipe));
	
	n = pipeline_length (pipe);
	while (i<n) {
		scmd = pipeline_front (pipe);
		pipeline_pop_front (pipe);
		err = path_transform_scommand (scmd);
		pipeline_push_back (pipe, scmd);
		i++;
		if (err == PATH_ERR)
			return err;
	}
	return PATH_OK;
}

int path_transform_scommand (scommand *scmd) {
	char *path = NULL; bool fileExist = false;
	bstring cmd = NULL, aux = NULL;
	Lexer *lex = NULL; FILE * fpath = NULL;
	
	assert (!scommand_is_empty (scmd));
	
	cmd = scommand_front (scmd);
	if (scommand_get_builtin (scmd) || bstrchr (cmd,'/')!=BSTR_ERR)
	/* el comando es built-in o ya es ruta absoluta: nada para hacer */
		return PATH_OK;

	path = getenv ("PATH");
	assert (path!=NULL);
	
	fpath = (FILE *)fmemopen (path, strlen (path) + 1,"r");
	if (fpath != NULL)
		lex = lexer_new (fpath);
		/* creamos un lexer con el contenido de $PATH */
	assert (lex!=NULL);
	
	lexer_skip (lex, ":");
	lexer_next_to (lex, ":");
	while (!lexer_is_off (lex) && !fileExist) {
		aux = lexer_item (lex);
		assert (aux != NULL);
		
		bconchar (aux, '/');
		bconcat (aux,cmd);
		
		if (access (bdata (aux),X_OK) == 0) {
		/* existe ese archivo con permisos de ejecucion? */
			scommand_pop_front (scmd);
			scommand_push_front (scmd, aux);
			/* reemplazamos el comando por su ruta absoluta  */
			fileExist = true;
		} else {
			bdestroy (aux);
			aux = NULL;
		}
	
		lexer_skip (lex, ":");
		lexer_next_to (lex, ":");	
	}
	lexer_destroy (lex);
	free (path);
	fclose (fpath);
	
	if (fileExist)
		return PATH_OK;
	else
		return PATH_ERR;
}
				
				
				
				
				
				
				
				
				
				
				
			
