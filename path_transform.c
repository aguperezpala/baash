#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "path_transform.h"

#include "lexer.h"


int path_transform (pipeline *pipe) {
	unsigned int i = 0, n = 0;
	scommand *scmd = NULL;
	int pathResult = PATH_OK;

	assert (!pipeline_is_empty (pipe));
	
	n = pipeline_length (pipe);
	for (i = 0; i < n; i++) {
		scmd = pipeline_front (pipe);
		if (path_transform_scommand (scmd) == PATH_ERR) {
			pathResult = PATH_ERR;
			i = n;
		} else {
			pipeline_pop_front (pipe);
			pipeline_push_back (pipe, scmd);
		}
	}
	return pathResult;
}	

int path_transform_scommand (scommand *scmd) {
	char *path = NULL; bool fileFound = false;
	bstring cmd = NULL, aux = NULL;
	Lexer *lex = NULL; FILE * fpath = NULL;
	
	assert (!scommand_is_empty (scmd));
	
	cmd = scommand_front (scmd);
	
	assert (cmd != NULL); 
	/* por las dudas */
	
	if (scommand_get_builtin (scmd))
	/* el comando es builtin: nada para hacer */
		return PATH_OK;

	if (bstrchr (cmd,'/')!=BSTR_ERR){
	/* si ya es una ruta, igual verificamos que se pueda ejecutar */	
		if (access ((char *)cmd->data,X_OK) == 0)
			return PATH_OK;
		 else 
			return PATH_ERR; 
	}
	
	/* a buscar.. */
	
	path = getenv ("PATH");
	assert (path!=NULL);
	
	fpath = (FILE *)fmemopen (path, strlen (path),"r");
	if (fpath != NULL)
		lex = lexer_new (fpath);
		/* creamos un lexer con el contenido de $PATH */
	assert (lex!=NULL);
	
	lexer_skip (lex, ":");
	lexer_next_to (lex, ":");
	while (!lexer_is_off (lex) && !fileFound) {
		aux = lexer_item (lex);
		assert (aux != NULL);
		
		bconchar (aux,'/');
		bconcat (aux,cmd);
		if (access ((char *)aux->data,X_OK) == 0) {
		/* existe ese archivo con permisos de ejecucion? */
			breplace (cmd, 0, aux->slen, aux,' ');
			/* reemplazamos el comando por su ruta absoluta */
			fileFound = true;
		}
		bdestroy (aux);
		aux = NULL;
		if (!lexer_is_off (lex))
			lexer_skip (lex, ":");
		if (!lexer_is_off (lex))
			lexer_next_to (lex, ":");	
	}
	lexer_destroy (lex);
	fclose (fpath);
	
	if (fileFound)
		return PATH_OK;
	else
		return PATH_ERR;
}
