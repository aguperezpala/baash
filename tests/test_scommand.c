#include <check.h>
#include "test_scommand.h"

#include <signal.h>
#include <assert.h>
#include <string.h> /* para strcmp */
#include <stdlib.h> /* para calloc */
#include <stdio.h> /* para sprintf */

#include "bstring/bstrlib.h"
#include "command.h"

#define MAX_LONG 257 /* no hay nada como un primo para molestar */

static scommand *scmd = NULL; /* para armar scommand temporales */

/* Testeo precondiciones */
START_TEST (test_destroy_null)
{
	scommand_destroy (NULL);
}
END_TEST


START_TEST (test_push_back_null)
{
	scommand_push_back (NULL, bfromcstr ("123"));
}
END_TEST

START_TEST (test_push_back_argument_null)
{
	scmd = scommand_new ();
	scommand_push_back (scmd, NULL);
	scommand_destroy (scmd); scmd = NULL;
}
END_TEST

START_TEST (test_push_front_null)
{
	scommand_push_front (NULL, bfromcstr ("123"));
}
END_TEST

START_TEST (test_push_front_argument_null)
{
	scmd = scommand_new ();
	scommand_push_front (scmd, NULL);
	scommand_destroy (scmd); scmd = NULL;
}
END_TEST

START_TEST (test_pop_front_null)
{
	scommand_pop_front (NULL);
}
END_TEST

START_TEST (test_set_builtin_null)
{
	scommand_set_builtin (NULL, false);
}
END_TEST

START_TEST (test_set_redir_in_null)
{
	scommand_set_redir_in (NULL, bfromcstr("123"));
}
END_TEST

START_TEST (test_set_redir_out_null)
{
	scommand_set_redir_out (NULL, bfromcstr("123"));
}
END_TEST

START_TEST (test_is_empty_null)
{
	scommand_is_empty (NULL);
}
END_TEST

START_TEST (test_front_null)
{
	scommand_front (NULL);
}
END_TEST

START_TEST (test_get_builtin_null)
{
	scommand_get_builtin (NULL);
}
END_TEST

START_TEST (test_get_redir_in_null)
{
	scommand_get_redir_in (NULL);
}
END_TEST

START_TEST (test_get_redir_out_null)
{
	scommand_get_redir_out (NULL);
}
END_TEST

START_TEST (test_to_string_null)
{
	scommand_to_string (NULL);
}
END_TEST


/* Crear y destruir */
START_TEST (test_new_destroy)
{
	scmd = scommand_new ();
	scommand_destroy (scmd); scmd = NULL;
}
END_TEST

/* Testeo de funcionalidad */

static void setup (void) {
	scmd = scommand_new ();
}

static void teardown (void) {
	if (scmd != NULL) {
		scommand_destroy (scmd);
		scmd = NULL;
	}
}

/* is_empty sea acorde a lo que agregamos y quitamos */
START_TEST (test_adding_emptying)
{
	unsigned int i = 0;
	bstring s = bfromcstr ("123");
	for (i=0; i<MAX_LONG; i++) {
		fail_unless ((i==0) == scommand_is_empty (scmd));
		scommand_push_front (scmd, s);
	}
	for (i=0; i<MAX_LONG; i++) {
		fail_unless (!scommand_is_empty(scmd));
		scommand_pop_front (scmd);
	}
	fail_unless (scommand_is_empty (scmd));
	bdestroy (s);
}
END_TEST

/* length sea acorde a lo que agregamos y quitamos */
START_TEST (test_adding_emptying_length)
{
	unsigned int i = 0;
	bstring str = bfromcstr ("123");
	for (i=0; i<MAX_LONG; i++) {
		fail_unless (i == scommand_length (scmd));
		scommand_push_back (scmd, str);
	}
	for (i=MAX_LONG; i>0; i--) {
		fail_unless (i == scommand_length (scmd));
		scommand_pop_front (scmd);
	}
	fail_unless (0 == scommand_length (scmd));
}
END_TEST

/* Meter por atrás y sacar por adelante, da la misma secuencia.
 * Reviso además que sea la misma memoria.
 */
START_TEST (test_fifo)
{
	unsigned int i = 0;
	bstring *strings = NULL;
	strings = calloc (MAX_LONG, sizeof(bstring));
	for (i=0; i<MAX_LONG; i++) {
		strings[i] = bformat ("%d", i);
	}
	/* strings = ["0","1", ..., "127"] */
	for (i=0; i<MAX_LONG; i++) {
		scommand_push_back (scmd, strings[i]);
	}
	for (i=0; i<MAX_LONG; i++) {
		/* misma memoria */
		fail_unless (scommand_front (scmd) == strings[i]);
		/* mismo string */
		fail_unless (biseq (scommand_front (scmd),strings[i]) == 1);
		scommand_pop_front (scmd);
	}
	for (i=0; i<MAX_LONG; i++) {
		bdestroy (strings[i]);
	}
	free (strings);
}
END_TEST

/* Meter por adelante y sacar por adelante, la secuencia al vésre.
 * Idem con que sea la misma memoria.
 */
START_TEST (test_lifo)
{
	unsigned int i = 0;
	bstring *strings = NULL;
	strings = calloc (MAX_LONG, sizeof (bstring));
	for (i=0; i<MAX_LONG; i++) {
		strings[i] = bformat ("%d", i);
	}
	/* strings = ["0","1", ..., "127"] */
	for (i=0; i<MAX_LONG; i++) {
		scommand_push_front (scmd, strings[i]);
	}
	for (i=0; i<MAX_LONG; i++) {
		/* misma memoria */
		fail_unless (scommand_front (scmd) == strings[MAX_LONG-i-1]);
		/* mismo string */
		fail_unless (biseq (scommand_front (scmd), strings[MAX_LONG-i-1]) == 1);
		scommand_pop_front (scmd);
	}
	for (i=0; i<MAX_LONG; i++) {
		bdestroy (strings[i]);
	}
	free (strings);
}
END_TEST

/* hacer muchísimas veces front es lo mismo */
START_TEST (test_front_idempotent)
{
	unsigned int i = 0;
	bstring s = NULL;
	scommand_push_front (scmd, s=bfromcstr ("123"));
	for (i=0; i<MAX_LONG; i++) {
		fail_unless (biseq (scommand_front (scmd), s) == 1);
	}
	bdestroy (s);
}
END_TEST

/* Si hay solo uno, entonces front=back */
START_TEST (test_front_is_back)
{
	bstring s = NULL;
	scommand_push_back (scmd, s=bfromcstr ("123"));
	fail_unless (biseq (scommand_front (scmd), s) == 1);
	bdestroy (s);
}
END_TEST

/* Si hay dos distintos entonces front!=back */
START_TEST (test_front_is_not_back)
{
	bstring s1 = NULL, s2 = NULL;
	scommand_push_back(scmd, s1=bfromcstr ("123"));
	scommand_push_back(scmd, s2=bfromcstr ("456"));
	fail_unless (biseq (scommand_front (scmd), s2) != 1);
	bdestroy (s1); bdestroy (s2);
}
END_TEST

/* Que almacene el booleanito */
START_TEST (test_builtin)
{
	scommand_set_builtin (scmd, true);
	fail_unless (scommand_get_builtin (scmd));
	scommand_set_builtin (scmd, !true);
	fail_unless (!scommand_get_builtin (scmd));
}
END_TEST


/* Que la tupla de redirectores sea un par independiente */
START_TEST (test_redir)
{
	bstring s1 = NULL, s2 = NULL;
	scommand_set_redir_in (scmd, s1=bfromcstr ("123"));
	scommand_set_redir_out (scmd, s2=bfromcstr ("456"));
	/* Los redirectores tienen que ser distintos */
	fail_unless (biseq (scommand_get_redir_in (scmd),
			scommand_get_redir_out (scmd)) != 1);
	/* En realidad es la misma memoria */
	fail_unless (scommand_get_redir_in (scmd) == s1 &&
		scommand_get_redir_out (scmd) == s2);
	/* ahora si ambos idem */
	scommand_set_redir_out (scmd, s1);
	fail_unless (biseq (scommand_get_redir_in (scmd), scommand_get_redir_out (scmd)) == 1);
	bdestroy (s1); bdestroy (s2);
}
END_TEST


/* Comando nuevo, string vacío */
START_TEST (test_to_string_empty)
{
	bstring str = NULL;
	str = scommand_to_string (scmd);
	fail_unless (blength (str) == 0);
	bdestroy (str);
}
END_TEST

/* Algo más fuerte. Poner muchos argumentos, mirar el orden 
 * Poner redirectores, mirar el orden
 */
START_TEST (test_to_string)
{
	bstring str = NULL;
	bstring *strings = NULL;
	int i = 0;
	int last_pos = 0;
	strings = calloc (MAX_LONG, sizeof(bstring));
	for (i=0; i<MAX_LONG; i++) {
		strings[i] = bformat ("%d", i);
	}
	/* strings = ["0","1", ..., "127"] */

	assert (MAX_LONG>2);
	/* comando "0 1 2 .... N-3 < N-2 > N-1" tiene que tener todos los números y los dos piquitos */
	for (i=0; i<MAX_LONG; i++) {
		if (i<MAX_LONG-2) {
			scommand_push_back (scmd, strings[i]);
		} else if (i==MAX_LONG-2) {
			scommand_set_redir_in (scmd, strings[i]);
		} else {
			assert(i==MAX_LONG-1);
			scommand_set_redir_out (scmd, strings[i]);			
		}
	}
	str = scommand_to_string (scmd);
	last_pos = 0;
	for (i=0; i<MAX_LONG; i++) {
		if (i<MAX_LONG-2) {
			fail_unless (binstr(str,0,strings[i])>=last_pos);
			last_pos = binstr (str,0,strings[i]);
		} else if (i==MAX_LONG-2) {
			fail_unless (binstr(str,0,strings[i])>=0);
			fail_unless (bstrchr(str, '<'));
			fail_unless (binstr(str,0,strings[i])>bstrchr(str, '<'));
		} else {
			assert(i==MAX_LONG-1);
			fail_unless (binstr(str,0,strings[i])>=0);
			fail_unless (bstrchr(str, '>'));
			fail_unless (binstr(str,0,strings[i])>bstrchr(str, '>'));
		}
	}
	bdestroy (str);
		
	for (i=0; i<MAX_LONG; i++) {
		bdestroy (strings[i]);
	}
	free (strings);
}
END_TEST

/* Armado de la test suite */

Suite *scommand_suite (void)
{
	Suite *s = suite_create ("scommand");
	TCase *tc_preconditions = tcase_create ("Precondition");
	TCase *tc_creation = tcase_create ("Creation");
	TCase *tc_functionality = tcase_create ("Functionality");

	/* Precondiciones */
	tcase_add_test_raise_signal (tc_preconditions, test_destroy_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_push_back_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_push_back_argument_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_push_front_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_push_front_argument_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_pop_front_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_set_builtin_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_set_redir_in_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_set_redir_out_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_is_empty_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_front_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_get_builtin_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_get_redir_in_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_get_redir_out_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_to_string_null, SIGABRT);
	suite_add_tcase (s, tc_preconditions);

	/* Creation */
	tcase_add_test (tc_creation, test_new_destroy);
	suite_add_tcase (s, tc_creation);

	/* Funcionalidad */
	tcase_add_checked_fixture (tc_functionality, setup, teardown);
	tcase_add_test (tc_functionality, test_adding_emptying);
	tcase_add_test (tc_functionality, test_adding_emptying_length);
	tcase_add_test (tc_functionality, test_fifo);
	tcase_add_test (tc_functionality, test_lifo);
	tcase_add_test (tc_functionality, test_front_idempotent);
	tcase_add_test (tc_functionality, test_front_is_back);
	tcase_add_test (tc_functionality, test_front_is_not_back);
	tcase_add_test (tc_functionality, test_builtin);
	tcase_add_test (tc_functionality, test_redir);
	tcase_add_test (tc_functionality, test_to_string_empty);
	tcase_add_test (tc_functionality, test_to_string);
	suite_add_tcase (s, tc_functionality);

	return s;
}
