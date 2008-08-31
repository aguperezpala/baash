#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>


int exec_pipe (pipeline *spipe)
{
	pid_t fst_child = 0 , son_pid = 0;
	unsigned int spipe_len = 0 , i = 0;
	int status = -1 , pipe_fd[2] = {0,0} , p_status = -1;
	scommand *scmd = NULL;
	bool must_wait = false , is_bin = false;
	
	/* REQUIRES */
	if (spipe == NULL)
	{
		perror ("NULL pipe provided\n");
		exit(1);
	}
	
	if (pipeline_is_empty (spipe))
	/* spipe vacío --> no hacemos nada */
		return 0;
	else
	/* spipe_len = total de comandos a ejecutar */
		spipe_len = pipeline_get_length (spipe);
	
	
	if (spipe_len < 2) {
	/* un comando solito, lo ejecutamos sin crear pipes */
		must_wait = pipeline_get_wait (spipe);
		scmd = pipeline_front (spipe)
		is_bin = scommand_get_builtin (scmd);
		fst_child = fork();
		if (fst_child < 0) {
			perror ("Birth complications\n");
			exit(1);
		} else if (fst_child == 0 && is_bin) {
		/* soy el hijo , soy Built In */
			exec_scmd_bin (scmd);
		} else if (fst_child == 0 && !is_bin) {
		/* soy el hijo , no soy Built In */
			exec_scmd_nobin (scmd);
		} else if (fst_child != 0 && must_wait) {
		/* soy el padre , tengo que esperar */
			wait (&status);
			if (s < 0) {
				perror("Esperando el hijo");
				exit(1);
			}
		} else if (fst_child != 0 && !must_wait) {
		/* soy el padre , no espero nada */
		"CORRER EL HIJO EN BACKGROUND"
		"LEER CAPÍTULO 5 <INTERPROCESS COMUNICATION>"


	/* 2 o más comandos, requiere del uso de pipes */
	
	"CREAR PIPE INICIAL"
	
	for (i=1 ; i < spipe_len ; i++)
	{
		/* Creamos la tubería para comunicación entre procesos
		 * hijos. Notar que si forkeamos N hijos, necesitaremos
		 * N-1 tuberías para comunicación 
		 */
		if (i+1 < spipe_len)
			p_status = pipe (pipe_fd);
			if (p_status < 0) {
				perror ("Unable to create pipe\n");
				exit(1);
			}
		}
		
		/* nacimiento de un hijo */
		son_pid = fork();
		scmd = pipeline_front (spipe);
		
		if (son_pid < 0) {
			perror ("Birth complications\n");
			exit(1);
		} else if (son_pid != 0) {  /* proceso padre */
			pipeline_pop_front (spipe);
			pipeline_push_back (spipe, scmd);
			scmd = NULL;
		} else {  /* proceso hijo */
			exec_scommand (scmd);
		"EJECUTAR COMANDOS DEL SPIPE"
	}
	
	
	return EXIT_SUCCESS;
}

int exec_scmd_nobin (scommand *scmd)
{
	bstring 
	"EJECUTAR COMANDO COMUNACHO"
}

int exec_scmd_bin (scommand *scmd)
{
	"EJECUTAR COMANDO INTERNO"
}
