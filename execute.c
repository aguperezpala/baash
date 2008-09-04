#include "execute.h"
/* Librerías de siempre */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* Librerías para syscalls y manejo de archivos */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>



/* Privadas. Para referencia ver las funciones mismas más abajo */
int exec_long_pipe (pipeline *spipe, unsigned int spipeLen);
int wait_children (unsigned int childreN);


/* Función para la ejecución de los pipes. Ejecuta pipelines de
 * un solo comando. Si su argumento es más largo llama a la función
 * interna auxiliar "exec_long_pipe" para hacer todo el trabajo.
 */
int exec_pipe (pipeline *spipe)
{
	scommand *scmd = NULL;
	pid_t childPID = 0 , waitRes = 0;
	int exitStatus = 0 , i = 0 , status = 0;
	unsigned int spipeLen = 0 , cmdLen = 0;
	bstring command = NULL; arg = NULL
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
		scmd = pipeline_front (spipe)
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
				waitRes = wait (&status)
				if (waitRes < 0)
					perror("While waiting for child.\n"
					"Info gathered: %d\n",status);
			} else if (childPID != 0 && !mustWait) {
			/* Soy padre pero no espero nada */	
				signal(SIGCHLD , SIG_IGN);
			} else {
			/* Soy hijo, ejecuto el comando */
				exe_cmd_nbin (scmd, cmdLen, NULL, 0, 1);
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
int exec_long_pipe (pipeline *spipe, unsigned int spipeLen)
{
	/* Variables para forkeo y espera de hijos */
	pid_t childPID = 0 , killToll = 0;
	scommand *scmd = NULL;
	bool mustWait = false , isBin = false;
	unsigned int scmdLen = 0 , i = 0;
	int childStat = 0 , waitStat = 0;
	/* Variables para creación de pipes */
	int pipeFD[spipeLen-1][2] = NULL;
	int aux = 0;
	
	/* REQUIRES */
	if (spipe == NULL)
		perror ("NULL pipe provided\n");
	
	/* Creamos las tuberías para comunicación entre procesos hijos
	 * Si forkeamos N hijos, necesitaremos N-1 tuberías
	 */
	for (i=0 ; i < spipeLen-1 ; i++) {
		pipeFD[i] = {0,0};
		aux = pipe (pipeFD[i]);
		if (aux < 0)
			perror ("While creating pipes\n");
		
	}
	
	mustWait = pipeline_get_wait (spipe);
	if (!mustWait)
	/* Ignoramos la muerte de los hijos. Linux evita
	 * por sí solo que se conviertan en zombies :D
	 */
		signal (SIGCHLD, SIG_IGN);
	
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
			
			if (mustWait) {
			/* Sin detenernos, vamos limpiando todos los
			 * hijos posibles dentro del mismo ciclo de
			 * creación, para ganar tiempo
			 */
				waitStat = waitpid (WAIT_ANY,
						&childStatus, WNOHANG);
				if (waitStat != 0) {
					killToll++;
					waitStat = 0;
				}
			}
			
		} else {  /* Proceso hijo */
			if (!scommand_get_builtin (scmd)) {
			/* No es interno */
				scmdLen = scommand_length (scmd);
				exe_cmd_nbin (scmd, scmdLen, pipeFD,
						spipeLen-1, i);
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
		childStat = wait_children ((unsigned int) spipeLen-killToll);
		if (childStat != 0)
			perror ("While waiting for children\n");
	} else { /* Volvemos al comportamiento de espera por defecto */
		signal (SIGCHLD, SIG_DFL);
	
	return EXIT_SUCCESS;
}

/* Ejecución de un comando no interno */
void exe_cmd_nbin (scommand *scmd, unsigned int cmdLen, int **pipeFD,
		   unsigned int pipesToll, unsigned int cmdN)
{
	/* Variables para la ejecución */
	bstring command = NULL , arg = NULL;
	char *argv[cmdLen-1] = NULL;
	unsigned int i = 0;
	/* Variables para la redirección I/O */
	bstring redirIn = NULL , redirOut = NULL;
	int fdIn = 0 , fdOut = 0 , aux = 0;
	int wrFlags = 0 , wrPerm = 0 ;
	mode_t rdFlags = 0 , rdPerm = 0 ;
	
	/* REQUIRES */
	assert (scmd != NULL);
	
	/* Flags y permisos para lectura/escritura de archivos */
	rdFlags = O_RDONLY;
	rdPerm = S_IRUSR;
	wrFlags = O_WRONLY | O_CREAT | O_TRUNC;
	wrPerm = S_IWUSR;
	
	command = scommand_front (scmd);
	scommand_pop_front (scmd);
	scommand_push_back (scmd, command);
	
	/* Guardamos los (cmdLen - 1) argumentos en args[] */
	for (i=0 ; i < cmdLen-1 ; i++) {
		arg = scommand_front (scmd);
		argv[i] = arg->data;
		scommand_pop_front (scmd);
		scommand_push_back (scmd, arg);
		arg = NULL;
	}	
	
	/* Manejo de redirección de entrada/salida */
	redirIn = scommand_get_redir_in (scmd);
	redirOut = scommand_get_redir_out (scmd);
	if (redirIn != NULL) {
		fdIn = open (redirIn->data, rdFlags, rdPerm);
		if (fdIn < 0) {
			perror ("While opening redir_in file\n");
		aux = dup2 (fdIn , STDIN_FILENO);
		if (aux < 0)
			perror ("While duplicating fd_in\n");
	}
	aux = 0;
	if (redirOut != NULL) {
		fdOut = open (redirOut->data, wrFlags, wrPerm);
		if (fdOut < 0)
			perror ("While opening redir_out file\n");
		aux = dup2 (fdOut , STDOUT_FILENO);
		if (aux < 0)
			perror ("While duplicating fd_out\n");
	}
	
	/* Manejo de pipes */
	for (i=0 ; i < pipesToll ; i++) {
		switch (i) {
			
			case cmdN-1:
			/* Pipe anterior, de aquí leeremos */
				close (pipeFD[i][1]);
				aux = dup2 (pipeFD[i][0] , STDIN_FILENO);
				if (aux < 0)
					perror ("While redirecting pipes\n");
			break;
				
			case cmdN:
			/* Aquí escribiremos */
				close (pipeFD[i][0]);
				aux = dup2 (pipeFD[i][1] , STDOUT_FILENO);
				if (aux < 0)
					perror ("While redirecting pipes\n");
			break;
				
			default:
			/* Estos pipes no nos conciernen */
				close (pipeFD[i][0]);
				close (pipeFD[i][1]);
			break;
		}
	}
	
	/* Al fin, ejecutamos el comando */
	execv (command->data, argv);
	/* Si llega hasta acá algo anduvo mal */
	perror ("Problems with given command\n");
}


/* Ejecución de comandos internos. Preparado sólo para "cd" y "exit" */
int exe_cmd_bin (scommand *scmd)
{
	bstring command = NULL , arg = NULL;
	int exitStatus = 0;
	
	/* REQUIRES */
	assert (scmd != NULL);
	assert (!scommand_is_empty (scmd));
	
	command = scommand_front (scmd);
	scommand_pop_front (scmd);
	if (!scommand_is_empty (scmd))
		arg = scommand_front (scmd);
	scommand_push_front (scmd, command);
	
	if (0 == strncmp (command->data, "cd", 2)) {
	/* Nos pasaron un cd, vamos al directorio proveído */
		exit_status = chdir (arg->data);
	} else if (0 == strncmp (command->data, "exit", 4)) {
	/* Nos pasaron un exit, salimoooo */
		exitStatus = EXIT;
	}
	
	command = NULL;
	arg = NULL;
	
	return exitStatus;
}

/* Espera por la muerte de los (childreN) procesos hijos. En la variable
 * status se guarda info sobre la salida de cada wait.
 * ENSURES:
 *	exitStatus==0 && EXIT_SUCCESS || exitStatus!=0 && EXIT_FAILURE
 * RETURNS:
 *	El entero indica el estado de terminación de todo el proceso
 *	de espera. Una sola espera fallida causa un EXIT_FAILURE
 */
int wait_children (unsigned int childreN)
{
	pid_t waitRes = 0;
	int i = 0 , status = 0 , exitStatus = 0;
	
	for (i=0 ; i < childreN ; i++) {
		waitRes = wait(&status);
		if (waitRes < 0) {
			perror ("While waiting for child termination.\n"
				"Info gathered: %d\n", status);
			exitStatus = waitRes;
		}
	
	return exitStatus
}
