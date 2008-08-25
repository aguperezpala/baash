#include <stdio.h>
#include <assert.h>
#include "command.h"


int main (void)
{
	scommand *scmd = NULL;
	bstring str1 = NULL , str2 = NULL, aux = NULL;
	bool b = false;

	scmd = scommand_new ();
	str1 = bfromcstr ("123");
	str2 = bfromcstr ("456");

	scommand_push_back (scmd, str1);
	scommand_push_back (scmd, str2);

	aux = scommand_front (scmd);
	b = biseq (aux, str1);
	scommand_pop_front (scmd);
	aux = scommand_front (scmd);
	b = b && biseq (aux, str2);
	scommand_pop_front (scmd);

	if (b)
		printf ("PASS <-- Test scommand_to_string 'mismo bstring'\n");
	else
		printf ("FAIL <-- Test scommand_to_string 'mismo bstring'\n");

	bdestroy (str1);
	bdestroy (str2);
	bdestroy (aux);
	assert (scommand_is_empty (scmd));
	scommand_destroy (scmd);

	return 0;
}


