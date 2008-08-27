#include <check.h>
#include "test_pipeline.h"

#include <signal.h>
#include <assert.h>
#include <string.h> /* para strcmp */
#include <stdlib.h> /* para calloc */
#include <stdio.h> /* para sprintf */
#include "command.h"

#define MAX_LONG 257 /* no hay nada como un primo para molestar */

static pipeline *pipe = NULL; /* para armar pipelines temporales */

/* Testeo precondiciones */
START_TEST (test_destroy_null)
{
	pipeline_destroy (NULL);
}
END_TEST


START_TEST (test_push_back_null)
{
	scommand *scmd = scommand_new ();
	pipeline_push_back (NULL, scmd);
	scommand_destroy (scmd);
}
END_TEST

START_TEST (test_push_back_scmd_null)
{
	pipe = pipeline_new ();
	pipeline_push_back (pipe, NULL);
	pipeline_destroy (pipe); pipe = NULL;
}
END_TEST

START_TEST (test_pop_front_null)
{
	pipeline_pop_front (NULL);
}
END_TEST

START_TEST (test_set_wait_null)
{
	pipeline_set_wait (NULL, false);
}
END_TEST

START_TEST (test_is_empty_null)
{
	pipeline_is_empty (NULL);
}
END_TEST

START_TEST (test_front_null)
{
	pipeline_front (NULL);
}
END_TEST

START_TEST (test_get_wait)
{
	pipeline_get_wait (NULL);
}
END_TEST

START_TEST (test_to_string_null)
{
	pipeline_to_string (NULL);
}
END_TEST


/* Crear y destruir */
START_TEST (test_new_destroy)
{
	pipe = pipeline_new ();
	pipeline_destroy (pipe); pipe = NULL;
}
END_TEST

/* Testeo de funcionalidad */

static void setup (void) {
	pipe = pipeline_new ();
}

static void teardown (void) {
	if (pipe != NULL) {
		pipeline_destroy (pipe);
		pipe = NULL;
	}
}

/* is_empty sea acorde a lo que agregamos y quitamos */
START_TEST (test_adding_emptying)
{
	unsigned int i = 0;
	scommand *scmd = scommand_new ();
	for (i=0; i<MAX_LONG; i++) {
		fail_unless ((i==0) == pipeline_is_empty (pipe));
		pipeline_push_back (pipe, scmd);
	}
	for (i=0; i<MAX_LONG; i++) {
		fail_unless (!pipeline_is_empty (pipe));
		pipeline_pop_front (pipe);
	}
	fail_unless (pipeline_is_empty (pipe));
	scommand_destroy (scmd);
}
END_TEST

/* length sea acorde a lo que agregamos y quitamos */
START_TEST (test_adding_emptying_length)
{
	unsigned int i = 0;
	scommand *scmd = scommand_new ();
	for (i=0; i<MAX_LONG; i++) {
		fail_unless (i == pipeline_length (pipe));
		pipeline_push_back (pipe, scmd);
	}
	for (i=MAX_LONG; i>0; i--) {
		fail_unless (i == pipeline_length (pipe));
		pipeline_pop_front (pipe);
	}
	fail_unless (0 == pipeline_length (pipe));
	scommand_destroy (scmd);
}
END_TEST

/* Meter por atrás y sacar por adelante, da la misma secuencia.
 * Reviso además que sea la misma memoria.
 */
START_TEST (test_fifo)
{
	unsigned int i = 0;
	scommand **scmds = NULL;
	scmds = calloc (MAX_LONG, sizeof(scommand *));
	for (i=0; i<MAX_LONG; i++) {
		scmds[i] = scommand_new ();
	}
	for (i=0; i<MAX_LONG; i++) {
		pipeline_push_back (pipe, scmds[i]);
	}
	for (i=0; i<MAX_LONG; i++) {
		fail_unless (pipeline_front (pipe) == scmds[i]);
		pipeline_pop_front (pipe);
	}
	for (i=0; i<MAX_LONG; i++) {
		free (scmds[i]);
	}
}
END_TEST

/* hacer muchísimas veces front es lo mismo */
START_TEST (test_front_idempotent)
{
	unsigned int i = 0;
	scommand *scmd = scommand_new ();
	pipeline_push_back (pipe, scmd);
	for (i=0; i<MAX_LONG; i++) {
		fail_unless (pipeline_front (pipe) == scmd);
	}
	scommand_destroy (scmd);
}
END_TEST

/* Si hay solo uno, entonces front=back */
START_TEST (test_front_is_back)
{
	scommand *scmd = scommand_new ();
	pipeline_push_back (pipe, scmd);
	fail_unless (pipeline_front (pipe)==scmd);
	scommand_destroy (scmd);
}
END_TEST

/* Si hay dos distintos entonces front!=back */
START_TEST (test_front_is_not_back)
{
	scommand *scmd0 = scommand_new ();
	scommand *scmd1 = scommand_new ();
	pipeline_push_back (pipe, scmd0);
	pipeline_push_back (pipe, scmd1);
	fail_unless (pipeline_front (pipe) != scmd1);
	scommand_destroy (scmd0);
	scommand_destroy (scmd1);
}
END_TEST

START_TEST (test_wait)
{
	pipeline_set_wait (pipe, true);
	fail_unless (pipeline_get_wait (pipe));
	pipeline_set_wait (pipe, false);
	fail_unless (!pipeline_get_wait (pipe));
}
END_TEST

/* Comando nuevo, string vacío */
START_TEST (test_to_string_empty)
{
	bstring str = NULL;
	str = pipeline_to_string (pipe);
	fail_unless (blength (str) == 0);
	bdestroy (str); str = NULL;
}
END_TEST

/* Armamos un pipeline de n y contamos que haya n-1 '|'.
 * Que no espere y buscamos el '&' al final.
 */
START_TEST (test_to_string)
{
	int p = 0, po = 0, n = 0, i = 0;
	bstring s = NULL, str = NULL;
	scommand *scmd = scommand_new ();
	scommand_push_back (scmd, s = bfromcstr ("gtk-fuse"));
	/* MAX_LONG veces el mismo comando simple */
	for (i=0; i<MAX_LONG; i++) {
		pipeline_push_back (pipe, scmd);
	}
	pipeline_set_wait (pipe, false);
	str = pipeline_to_string (pipe);
	/* cuenta cuantos pipes hay en n */
	p = 0; po = 0; n = 0;
	while (0<=p) {
		po = p;
		p = bstrchrp (str, '|', p);
		if (0<=p) {
			n = n+1;
			p = p+1;
		}
	}
	fail_unless (n==MAX_LONG-1);
	p = bstrchrp (str, '&', po);
	fail_unless (0<=p);
	
	bdestroy (s); bdestroy (str);
	scommand_destroy (scmd);
}
END_TEST

/* Armado de la test suite */

Suite *pipeline_suite (void)
{
	Suite *s = suite_create ("pipeline");
	TCase *tc_preconditions = tcase_create ("Precondition");
	TCase *tc_creation = tcase_create ("Creation");
	TCase *tc_functionality = tcase_create ("Functionality");

	/* Precondiciones */
	tcase_add_test_raise_signal (tc_preconditions, test_destroy_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_push_back_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_push_back_scmd_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_pop_front_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_set_wait_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_is_empty_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_front_null, SIGABRT);
	tcase_add_test_raise_signal (tc_preconditions, test_get_wait, SIGABRT);
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
	tcase_add_test (tc_functionality, test_front_idempotent);
	tcase_add_test (tc_functionality, test_front_is_back);
	tcase_add_test (tc_functionality, test_front_is_not_back);
	tcase_add_test (tc_functionality, test_wait);
	tcase_add_test (tc_functionality, test_to_string_empty);
	tcase_add_test (tc_functionality, test_to_string);
	suite_add_tcase (s, tc_functionality);

	return s;
}
