#include <stdio.h>
#include <assert.h>
#include "command.h"


int main (void)
{
	pipeline *pipe = NULL;
	scommand *scmd1 = NULL, *scmd2 = NULL;
	bstring str1 = NULL, str2 = NULL, str3 = NULL, str4 = NULL;
	bstring str5 = NULL, str6 = NULL, aux = NULL;
	bool b = false;
	
	pipe = pipeline_new ();
	aux = pipeline_to_string (pipe);
	assert (aux != NULL);
	if (aux->data != NULL)
		printf ("pipe = %s\n", aux->data);
	
	scmd1 = scommand_new ();
	scmd2 = scommand_new ();
	
	str1 = bfromcstr ("com1");
	str2 = bfromcstr ("com2");
	str3 = bfromcstr ("archivo_entrada");
	str4 = bfromcstr ("archivo_salida");
	str5 = bfromcstr ("arg1");
	str6 = bfromcstr ("arg2");
	scommand_push_back (scmd1, str1);
	scommand_push_back (scmd1, str5);
	scommand_set_redir_in (scmd1, str3);
	/* scmd1 = (false,["com1","arg1"],"","archivo_entrada") */
	scommand_push_front (scmd2, str6);
	scommand_push_front (scmd2, str2);
	scommand_set_redir_out (scmd2, str4);
	/* scmd2 = (false,["com2","arg2"],"archivo_salida","") */
	
	bdestroy (aux);
	aux = scommand_to_string (scmd1);
	assert (aux != NULL);
	if (aux->data != NULL)
		printf ("scmd1 = %s\n", aux->data);
	
	bdestroy (aux);
	aux = scommand_to_string (scmd2);
	assert (aux != NULL);
	if (aux->data != NULL)
		printf ("scmd2 = %s\n", aux->data);
	
	pipeline_push_back (pipe, scmd1);
	pipeline_push_back (pipe, scmd2);
	pipeline_set_wait (pipe, b);
	/* pipe = { [ (false,["com1","arg1"],"","archivo_entrada") , 
		    (false,["com2","arg2"],"archivo_salida","") ] , false}  */
	bdestroy (aux);
/*	aux = pipeline_to_string (pipe);*/
	assert (aux != NULL);
	if (aux->data != NULL)
		printf ("pipe = %s\n",aux->data);
	
	pipeline_destroy (pipe);
	scommand_destroy (scmd1);
	scommand_destroy (scmd2);
	bdestroy (aux);
	bdestroy (str1);
	bdestroy (str2);
	bdestroy (str3);
	bdestroy (str4);
	bdestroy (str5);
	bdestroy (str6);
	
	return 0;
}

