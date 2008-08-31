#include <stdio.h>
#include <assert.h>
#include "command.h"


int main (void)
{
	scommand *scmd = NULL;
	bstring str1 = NULL, str2 = NULL, str3 = NULL, str4 = NULL, aux = NULL;
	bool b = false;
	int i=0;

	for (;i<2;i++) {
		scmd = scommand_new ();
		str1 = bfromcstr ("123");
		str2 = bfromcstr ("456");

		scommand_push_back (scmd, str1);
		scommand_push_back (scmd, str2);

		aux = scommand_front (scmd);
		b = biseq (aux, str1);
		aux = NULL;
		scommand_pop_front (scmd);
		aux = scommand_front (scmd);
		b = b && biseq (aux, str2);
		scommand_pop_front (scmd);

		if (b)
			printf ("PASS <-- Test scommand_to_string 'mismo bstring'\n");
		else
			printf ("FAIL <-- Test scommand_to_string 'mismo bstring'\n");

		aux = NULL;
		scommand_set_redir_in (scmd, str1);
		aux = scommand_get_redir_in (scmd);

		assert (biseq (aux, str1));
		assert (scommand_get_redir_in (scmd) == aux);

		printf ("PASS <-- Test scommand_set&get_redir_in\n");

		scommand_push_front (scmd, str2);
		scommand_push_front (scmd, str1);
		str3 = bfromcstr ("archivo_salida");
		scommand_set_redir_out (scmd, str3);
		str4 = bfromcstr ("archivo_entrada");
		scommand_set_redir_in (scmd, str4);
		/* scmd = ( 123 456 < archivo_entrada > archivo_salida) */
	
		aux = scommand_to_string (scmd);
		assert (aux != NULL);
		printf ("scmd = %s\n", aux->data);
		printf ("scmd = %s\n", aux->data);

		scommand_destroy (scmd);
		bdestroy (str1);
		bdestroy (str2);
		bdestroy (str3);
		bdestroy (str4);
		bdestroy (aux);
	}

	return 0;
}


