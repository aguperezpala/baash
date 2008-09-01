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

/* Para referencia ver la función en sí */
int exec_long_pipe (pipeline *spipe, unsigned int spipe_len);

/* Función para la ejecución de los pipes. Ejecuta pipelines de
 * un solo comando. Si su argumento es más largo llama a la función
 * interna auxiliar "exec_long_pipe" para hacer todo el trabajo.
 */
int exec_pipe (pipeline *pipe)
{
	scommand *scmd = NULL;
	pid_t child_pid = 0;
	int exit_status = 0 , i = 0 , status = 0;
	unsigned int pipe_len = 0 , scmd_len = 0;
	bstring command = NULL; arg = NULL
	bool must_wait = false;
	
	/* REQUIRES */
	if (spipe == NULL)
	{
		perror ("NULL pipe provided\n");
		exit(1);
	}
	
	if (pipeline_is_empty (pipe))
	/* no hay comandos, no se ejecuta nada, volvemos */
		return exit_status;
	
	pipe_len = pipeline_length (pipe);
	if (pipe_len == 1) {
	/* Un comando solito, lo ejecutamos aquí */
		must_wait = pipeline_get_wait (spipe);
		scmd = pipeline_front (spipe)
		cmd_len = scommand_length (scmd);
		
		if (scommand_get_builtin (scmd)) {
		/* Comando interno, debe ejecutarlo el padre */
		### TODO ESTO PUEDE REEMPLAZARLO EXE_CMD_BIN" ###
			char *argv[cmd_len-1] = NULL;
			/* Extraemos el comando */
			command = scommand_front (scmd);
			scommand_pop_front (scmd);
			scommand_push_back (scmd, command);
			/* Guardamos los (scmd_len-1) argumentos en argv */
			for (i=0 ; i < scmd_len-1 ; i++) {
				arg = scommand_front (scmd);
				argv[i] = arg->data;
				scommand_pop_front (scmd);
				scommand_push_back (scmd, arg);
				arg = NULL;
			}
			execv (command->data, argv);
			/* Si llegamos acá algo anduvo mal */
			perror ("Problems with given command\n");
			exit(1);
		} else {
		/* Comando no-built-in: lo buscamos en el filesystem 
		 * y hacemos que un hijo lo ejecute
		 */
			child_pid = fork();
			if (child_pid < 0) {
				perror ("Birth complications\n");
				exit(1);
			} else if (child_pid != 0 && must_wait) {
			/* Soy padre y espero a mi hijo */
				wait (&status)
				if (status < 0) {
					perror("Problems while waiting"
						"for my child\n");
					exit(1);
				}
			} else if (child_pid != 0 && !must_wait) {
			/* Soy padre pero no espero nada */
			### COMPLETAR MODO DE NO ESPERA ###
			} else {
			/* Soy hijo, ejecuto el comando */
				exe_cmd_nbin (scmd, cmd_len, NULL, 0, 1);
			}
		}
	} else {
	/* Comando largo, derivamos a función auxiliar */
		exit_status = exec_long_pipe (pipe, pipe_len);
	}
	
	return exit_status;
}

/* Función interna auxiliar para ejecución de pipelines de más de un
 * comando. Es la que hace el trabajo pesado, con creación de pipes y todo.
 */
int exec_long_pipe (pipeline *spipe, unsigned int spipe_len)
{
	/* Variables para forkeo de hijos */
	pid_t child_pid = 0;
	scommand *scmd = NULL;
	bool must_wait = false , is_bin = false;
	unsigned int scmd_len = 0 , i = 0;
	/* Variables para creación de pipes */
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
	
	/* Creación de los hijos */
	for (i=0 ; i < spipe_len ; i++)
	{
		scmd = pipeline_front (spipe);
		
		child_pid = fork();
		
		if (child_pid < 0) {
			perror ("Birth complications\n");
			exit(1);
		} else if (child_pid != 0) {  /* Proceso padre */
			/* Pasamos al siguiente comando */
			pipeline_pop_front (spipe);
			pipeline_push_back (spipe, scmd);
			scmd = NULL;
			/* Desvinculamos los pipes */
			close (pipe_fd[i][0]);
			close (pipe_fd[i][1]);
		} else {  /* Proceso hijo */
			if (scommand_get_builtin (scmd)) {
				exe_cmd_bin (scmd);
		### COMPLETAR CON MODO DE ESPERA ###
			} else if (must_wait) {
				scmd_len = scommand_length (scmd);
				exe_cmd_nbin (scmd, scmd_len, pipe_fd,
						spipe_len-1, i);
			} else {
				scmd_len = scommand_length (scmd);
				exe_cmd_nbin (scmd, scmd_len, pipe_fd,
						spipe_len-1, i);
			}
		}
	}
	
	return EXIT_SUCCESS;
}

/* Ejecución de un comando no interno */
void exe_cmd_nbin (scommand *scmd, unsigned int scmd_len, int **pipe_fd,
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
	
	/* REQUIRES */
	assert (scmd != NULL);
	
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
			/* Pipe anterior, de aquí leeremos */
				close (pipe_fd[i][1]);
				if (redir_in != NULL) {
					close (pipe_fd[i][0]);
			break;
				
			case cmd_no:
			/* Aquí escribiremos */
				close (pipe_fd[i][0]);
				if (redir_out != NULL) {
					close (pipe_fd[i][1]);
			break;
				
			default:
			/* Estos pipes no nos conciernen */
				close (pipe_fd[i][0]);
				close (pipe_fd[i][1]);
			break;
		}
	}
	
	/* Al fin, ejecutamos el comando */
	execv (command->data, argv);
	/* Si llega hasta acá algo anduvo mal */
	perror ("Problems with given command\n");
	close
	exit(1);
}
