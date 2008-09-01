#include <stdio.h>
#include <glib.h>
#include <stdlib.h>


void path_transform (pipeline *pipe) {
	unsigned int i = 0, n = 0;
	scommand *scmd = NULL;
	
	assert (!pipeline_is_empty (pipe));
	
	n = pipeline_length (pipe);
	while (i<n) {
		scmd = pipeline_front (pipe);
		pipeline_pop_front (pipe);
		path_transform_scommand (scmd);
		pipeline_push_back (pipe, scmd);
		i++;
	}
}

void path_transform_scommand (scommand *scmd) {
	char *path = NULL;
	bstring cmd = NULL, aux = NULL;
	Lexer *lex = NULL;
	
	assert (!scommand_is_empty (scmd));
	
	cmd = scommand_front (scmd);
	if (!scommand_get_builtin (scmd) || bstrchr (cmd,'/')==BSTR_ERR) {
		/* buscamos ubicaion del comando si no es un built in o una
		 * ruta absoluta de antemano
		 */
		 
		path = getenv (PATH);
		lex = lexer_new (fmemopen (path, strlen (path) + 1,"r"));
		/* creamos un lexer con los directorios de $PATH */
		
		assert (lex!=NULL): 
		
		while (!lexer_is_off (lex)) {
			lexer_skip (lex, ":");
			lexer_next_to (lex, ":");
			aux = lexer_item (lex);
			bconchar (aux, "/");
			bconcat (aux,cmd):
			
			/* ahora hay que ver si existe ese archivo ahi.. */
			if ( AUX_EXIST?????)
				*cmd = *aux; /* sintaxis ??? */
		}		
	}
}
				
				
				
				
				
				
				
				
				
				
				
			
