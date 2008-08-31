#include <glib.h>


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
	bstring cmd = NULL, aux = NULL;
	Lexer *lex = NULL;
	
	assert (!scommand_is_empty (scmd));
	
	if (!scommand_get_builtin (scmd)) { 
		/* si es builtin no lo tocamos */
		cmd = scommand_front (scmd);
		lex = lexer_new (/*PATH???*/);
		assert (lex!=NULL): 
		
		while (!lexer_is_off (lex)) {
			lexer_next_to (lex, ":");
			aux = lexer_item (lex);
			bconchar (aux, "/");
			bconcat (aux,cmd):
			/* ahora hay que ver si existe ese archivo ahi.. */
			if ( EXIST???? )
				cmd = ???????
				
				ya no entiendo nada... me voy a la bosta.
			
			
			
			
		
