#include "execute.h"
/* Librerías de siempre */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
/* Librerías para syscalls y manejo de archivos */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#ifndef WAIT_ANY
	#define WAIT_ANY -1
#endif


/* Privadas. Para referencia ver las funciones mismas más abajo */
static int exec_long_pipe (pipeline *spipe, unsigned int spipeLen);
static int wait_children (unsigned int childreN);


/* Función para la ejecución de los pipes. Ejecuta pipelines de
 * un solo comando. Si su argumento es más largo llama a la función
 * interna auxiliar "exec_long_pipe" para hacer todo el trabajo.
 */
int exec_pipe (pipeline *spipe)
{
	scommand *scmd = NULL;
	pid_t childPID = 0;
	int exitStatus = 0 , status = 0;
	unsigned int spipeLen = 0 , cmdLen = 0;
	bool mustWait = false;
	
	/* REQUIRES */
	if (spipe == NULL) {
		perror ("NULL pipe provided\n");
		exit(1);
	}
	
	if (pipeline_is_empty (spipe))
	/* no hay comandos, no se ejecuta nada, volvemos */
		return exitStatus;
	
	spipeLen = pipeline_length (spipe);
	if (spipeLen == 1) {
	/* Un comando solito, lo ejecutamos aquí */
		mustWait = pipeline_get_wait (spipe);
		scmd = pipeline_front (spipe);
		cmdLen = scommand_length (scmd);
		
		if (scommand_get_builtin (scmd)) {
		/* Comando interno, debe ejecutarlo el padre */
			exitStatus = exe_cmd_bin (scmd);
		} else {
		/* Comando no-built-in: lo buscamos en el filesystem 
		 * y hacemos que un hijo lo ejecute
		 */
			childPID = fork();
			if (childPID < 0) {
				perror ("Birth complications\n");
				exit(1);
			} else if (childPID != 0 && mustWait) {
			/* Soy padre y espero a mi hijo */
				wait (&status);
				if (!(WIFEXITED (status)))
					fprintf (stderr, "%s ~ While waiting"
					" for child. Info gathered: %d\n",
					 strerror (errno), status);
			} else if (childPID != 0 && !mustWait) {
			/* Soy padre pero no espero nada */;
			} else {
			/* Soy hijo, ejecuto el comando */
				exe_cmd_nbin (scmd, NULL, 0, 1);
			}
		}
	} else {
	/* Comando largo, derivamos a función auxiliar */
		exitStatus = exec_long_pipe (spipe, spipeLen);
	}
	
	return exitStatus;
}

/* Función interna auxiliar para ejecución de pipelines de más de un
 * comando. Es la que hace el trabajo pesado: forkeo de varios hijos, 
 * creación de pipes, e implementación de la lógica de espera.
 * REQUIRES:
 *	spipe != NULL
 * ENSURES:
 *	el pipeline *spipe no es modificado (pero sí toqueteado)
 */
static int exec_long_pipe (pipeline *spipe, unsigned int spipeLen)
{
	/* Variables para forkeo y espera de hijos */
	pid_t childPID = 0;
	scommand *scmd = NULL;
	bool mustWait = false;
	unsigned int scmdLen = 0 , i = 0;
	int childStat = 0;
	/* Variables para creación de pipes */
	int **pipeFD = NULL;
	int aux = 0;
	
	
	/* REQUIRES */
	if (spipe == NULL)
		perror ("NULL pipe provided\n");
	
	/* Creamos las tuberías para comunicación entre procesos hijos
	 * Si forkeamos N hijos, necesitaremos N-1 tuberías
	 */
	pipeFD = (int**) calloc (spipeLen - 1, sizeof (int));
	if (pipeFD == NULL) {
		perror ("While creating pipesFD\n");
		exit(1);
	}
	for (i=0 ; i < spipeLen-1 ; i++) {
		pipeFD[i] = (int*) calloc (2, sizeof (int));
		pipeFD[i][0] = 0;
		pipeFD[i][1] = 0;
	}
		
	for (i=0 ; i < spipeLen-1 ; i++) {
		aux = pipe (pipeFD[i]);
		if (aux < 0)
			perror ("While creating pipes\n");
	}
	
	mustWait = pipeline_get_wait (spipe);
	if (!mustWait){
	/* Ignoramos la muerte de los hijos. Linux evita
	 * por sí solo que se conviertan en zombies :D
	 */}
	
	/* Creación de los hijos */
	for (i=0 ; i < spipeLen ; i++)
	{
		scmd = pipeline_front (spipe);
		
		childPID = fork();
		
		if (childPID < 0) {
			perror ("Birth complications\n");
			exit(1);
		} else if (childPID != 0) {  /* Proceso padre */
			/* Pasamos al siguiente comando */
			pipeline_pop_front (spipe);
			pipeline_push_back (spipe, scmd);
			scmd = NULL;		
		} else {  /* Proceso hijo */
			if (!scommand_get_builtin (scmd)) {
			/* No es interno */
				scmdLen = scommand_length (scmd);
				exe_cmd_nbin (scmd, pipeFD, spipeLen-1, i);
			} else {
			/* Si el comanmdo es interno, es inútil
			 * ejecutarlo en un hijo porque lo que haga
			 * se pierde con la muerte del hijo, así que
			 * no hacemos nada
			 */;
			}
		}
	}/* Sólo el padre pasa este ciclo, todos los hijos desaparecen
	  * durante su llamada a execv()
	  */
	
	
	for (i=0 ; i < spipeLen-1 ; i++) {
	/* Desvinculamos los pipes */
		close (pipeFD[i][0]);
		close (pipeFD[i][1]);
	}
	
	if (mustWait) {
		childStat = wait_children ((unsigned int) spipeLen);
		if (childStat != 0)
			perror ("While waiting for children\n");
	}
	
	for (i=0 ; i < spipeLen-1 ; i++)
		free (pipeFD[i]);
	free (pipeFD);
	pipeFD = NULL;
	
	return EXIT_SUCCESS;
}

/* Ejecución de un comando no interno */
void exe_cmd_nbin (scommand *scmd, int **pipeFD,
		    unsigned int pipesToll, unsigned int cmdN)
{
	/* Variables para la ejecución */
	bstring arg = NULL;
	char **argv = NULL;
	unsigned int i = 0 , cmdLen = 0;
	/* Variables para la redirección I/O */
	bstring redirIn = NULL , redirOut = NULL;
	int fdIn = 0 , fdOut = 0 , aux = 0;
	int rdwrPerm = 0 , rdFlags = 0 , wrFlags = 0;
	
	/* REQUIRES */
	assert (scmd != NULL);
	
	/* Flags y permisos para lectura/escritura de archivos */
	rdFlags = O_RDONLY;
	wrFlags = O_WRONLY | O_CREAT | O_TRUNC;
	rdwrPerm = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	
	cmdLen = scommand_length (scmd);
	argv = (char **) calloc (cmdLen+1, sizeof (char*));
	
	/* Guardamos comando y argumentos en argv[] */
	for (i=0 ; i < cmdLen ; i++) {
		arg = scommand_front (scmd);
		argv[i] = (char*) arg->data;
		scommand_pop_front (scmd);
		scommand_push_back (scmd, arg);
		arg = NULL;
	}
	
	
	/* Manejo de pipes */
	for (i=0 ; i < pipesToll ; i++) {
				
		if (i == cmdN-1) {
		/* Pipe anterior, de aquí leeremos */
			close (pipeFD[i][1]);
			aux = dup2 (pipeFD[i][0] , STDIN_FILENO);
			if (aux < 0)
				perror ("While redirecting pipes\n");
		} else if (i == cmdN) {
		/* Aquí escribiremos */
			close (pipeFD[i][0]);
			aux = dup2 (pipeFD[i][1] , STDOUT_FILENO);
			if (aux < 0)
				perror ("While redirecting pipes\n");
		} else {
		/* Estos pipes no nos conciernen */
			close (pipeFD[i][0]);
			close (pipeFD[i][1]);
		}
	}
	
	/* Manejo de redirección de entrada/salida */
	redirIn = scommand_get_redir_in (scmd);
	redirOut = scommand_get_redir_out (scmd);
	if (redirIn != NULL) {
		fdIn = open ((char*) redirIn->data, rdFlags, rdwrPerm);
		if (fdIn < 0)
			perror ("While opening redir_in file\n");
		aux = dup2 (fdIn , STDIN_FILENO);
		if (aux < 0)
			perror ("While duplicating fd_in\n");
	}
	aux = 0;
	if (redirOut != NULL) {
		fdOut = open ((char*) redirOut->data, wrFlags, rdwrPerm);
		if (fdOut < 0)
			perror ("While opening redir_out file\n");
		aux = dup2 (fdOut , STDOUT_FILENO);
		if (aux < 0)
			perror ("While duplicating fd_out\n");
	}
	
	/* Al fin, ejecutamos el comando */
	execv (argv[0], argv);
	/* Si llega hasta acá algo anduvo mal */
	perror ("Problems with given command\n");
}




/* Espera por la muerte de los (childreN) procesos hijos. En la variable
 * status se guarda info sobre la salida de cada wait.
 * ENSURES:
 *	exitStatus==0 && EXIT_SUCCESS || exitStatus!=0 && EXIT_FAILURE
 * RETURNS:
 *	El entero indica el estado de terminación de todo el proceso
 *	de espera. Una sola espera fallida causa un EXIT_FAILURE
 */
static int wait_children (unsigned int childreN)
{
	int i = 0 , status = 0 , exitStatus = 0;
	
	for (i=0 ; i < (int) childreN ; i++) {
		wait(&status);
		if (!(WIFEXITED (status))) {
			fprintf (stderr,"%s ~ While waiting for child term.\n"
				" Line: %d  Info gathered: %d\n",
				strerror (errno),(int) 290,status);
			exitStatus = -1;
		}
	}
	
	return exitStatus;
}
