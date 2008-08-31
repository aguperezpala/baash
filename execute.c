#include "execute.h"
/* Librerías de siempre */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* Librerías para syscalls y manejo de archivos */
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int exec_pipe (pipeline *spipe, unsigned int spipe_len)
{
	/* variables para forkeo de hijos */
	pid_t child_pid = 0;
	scommand *scmd = NULL;
	bool must_wait = false , is_bin = false;
	unsigned int scmd_len = 0 , i = 0;
	/* variables para creación de pipes */
	int **pipe_prev = NULL , **pipe_next = NULL;
	int pipe_fd[spipe_len-2][2] = NULL;
	int status = -1 , p_status = -1;
	
	/* REQUIRES */
	if (spipe == NULL)
	{
		perror ("NULL pipe provided\n");
		exit(1);
	}
	
	/* Creamos las tuberías para comunicación entre procesos hijos
	 * Si forkeamos N hijos, necesitaremos N-1 tuberías
	 */
	for (i=0 ; i < spipe_len-1 ; i++) {
		pipe_fd[i] = {0,0};
		aux = pipe (pipe_fd[i]);
		if (aux < 0) {
			perror ("While creating pipes\n");
			exit(1);
		}
		
	}
	
	must_wait = pipeline_get_wait (spipe);
	
	/* creacióm de los hijos */
	for (i=0 ; i < spipe_len ; i++)
	{
		scmd = pipeline_front (spipe);
		
		child_pid = fork();
		
		if (child_pid < 0) {
			perror ("Birth complications\n");
			exit(1);
		} else if (child_pid != 0) {  /* proceso padre */
			/* pasamos al siguiente comando */
			pipeline_pop_front (spipe);
			pipeline_push_back (spipe, scmd);
			scmd = NULL;
			/* desvinculamos los pipes */
			close (pipe_fd[i][0]);
			close (pipe_fd[i][1]);
		} else {  /* proceso hijo */
			scmd_len = scommand_length (scmd);
			exec_scmd (scmd, scmd_len, pipe_fd, spipe_len-1, i);
		"EJECUTAR COMANDOS DEL SPIPE"
		}
	}
	
	
	return EXIT_SUCCESS;
}

void exec_scmd (scommand *scmd, unsigned int scmd_len, int **pipe_fd,
		unsigned int pipes_no, unsigned int cmd_no)
{
	/* Variables para la ejecución */
	bstring command = NULL , arg = NULL;
	char *argv[scmd_len-1] = NULL;
	unsigned int i = 0;
	/* Variables para la redirección I/O */
	bstring redir_in = NULL , redir_out = NULL;
	int fd_in = 0 , fd_out = 0 , aux = 0;
	int rd_flags = 0 , rd_perm = 0 , wr_flags = 0 , wr_perm = 0 ;
	
	/* Flags y permisos para lectura/escritura de archivos */
	rd_flags = O_RDONLY;
	rd_perm = S_IRUSR;
	wr_flags = O_WRONLY | O_TRUNC;
	wr_perm = S_IWUSR;
	
	command = scommand_front (scmd);
	scommand_pop_front (scmd);
	scommand_push_back (scmd, command);
	
	/* Guardamos los (scmd_len - 1) argumentos en args[] */
	for (i=0 ; i < scmd_len-1 ; i++) {
		arg = scommand_front (scmd);
		argv[i] = arg->data;
		scommand_pop_front (scmd);
		scommand_push_back (scmd, arg);
		arg = NULL;
	}	
	
	/* Manejo de redirección de entrada/salida */
	redir_in = scommand_get_redir_in (scmd);
	redir_out = scommand_get_redir_out (scmd);
	if (redir_in != NULL) {
		fd_in = open (redir_in->data, rd_flags, rd_perm);
		if (fd_in < 0) {
			perror ("While opening redir_in file\n");
			exit(1);
		}
		aux = dup2 (fd_in , STDIN_FILENO);
		if (aux < 0) {
			perror ("While duplicating fd_in\n");
			exit (1);
		}
	}
	aux = 0;
	if (redir_out != NULL) {
		fd_out = open (redir_out->data, wr_flags, wr_perm);
		if (fd_out < 0) {
			perror ("While opening redir_out file\n");
			exit(1);
		}
		aux = dup2 (fd_out , STDOUT_FILENO);
		if (aux < 0) {
			perror ("While duplicating fd_out\n");
			exit (1);
		}
	}
	
	/* Manejo de pipes */
	for (i=0 ; i < pipes_no ; i++) {
		switch (i) {
			
			case cmd_no-1:
			/* pipe anterior, de aquí leeremos */
				close (pipe_fd[i][1]);
				if (redir_in != NULL) {
					close (pipe_fd[i][0]);
			break;
				
			case cmd_no:
			/* aquí escribiremos */
				close (pipe_fd[i][0]);
				if (redir_out != NULL) {
					close (pipe_fd[i][1]);
			break;
				
			default:
			/* estos pipes no nos conciernen */
				close (pipe_fd[i][0]);
				close (pipe_fd[i][1]);
			break;
		}
	}
	
	/* al fin, ejecutamos el comando */
	execv (command->data, argv);
	/* si llega hasta acá algo anduvo mal */
	perror ("Problems with given command\n");
	close
	exit(1);
}


	/* ¿BASURA?  REVISAR - REVISAR - REVISAR  ¿BASURA?*/
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
				perror("Problems while waiting for son\n");
				exit(1);
			}
		} else if (fst_child != 0 && !must_wait) {
		/* soy el padre , no espero nada */
		"CORRER EL HIJO EN BACKGROUND"
		"LEER CAPÍTULO 5 <INTERPROCESS COMUNICATION>"


	/* 2 o más comandos, requiere del uso de pipes */
	
	"CREAR PIPE INICIAL"
	
	for (i=0 ; i < spipe_len ; i++)
	{
		/* Creamos las tuberías para comunicación entre procesos
		 * hijos. Notar que si forkeamos N hijos, necesitaremos
		 * N-1 tuberías para comunicación 
		 */
		if (i>0)
		/* recordamos la tubería anterior */
			pipe_prev = pipe_fd;
		if (i+1 < spipe_len)
		/* construimos la nueva tubería */
			p_status = pipe (pipe_fd);
			if (p_status < 0) {
				perror ("Unable to create pipe\n");
				exit(1);
			}
			pipe_next = pipe_fd;
	}