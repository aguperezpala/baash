/* $Date: 2008-08-20 16:10:56 -0300 (Wed, 20 Aug 2008) $
 * $Revision: 423 $
 */

/* A partir de man bash, en su seccion de SHELL GRAMMAR,
 * se diseñaron dos TAD scomand (comando simple) y
 * pipeline (secuencia de comandos simples separados por pipe)
 */

#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h> /* para tener bool */
#include "bstring/bstrlib.h" /* los resultados de las funciones *_to_string se dan en bstring */


/* scommand: comando simple. Ejemplo: ls -l ej1.c > out < in
 * Se presenta como una secuencia de cadenas donde la primera se denomina
 * comando y desde la segunda se denominan argumentos.
 * Almacena dos cadenas que representan los redirectores de entrada y salida.
 * Cualquiera de ellos puede estar NULL indicando que no hay redirección.
 * 
 * Queda a cargo del programa que usa este TAD pedir y liberar toda la memoria
 * de las cadenas que maneja, salvo scommand_to_string() que pide memoria
 * internamente y debe ser liberada externamente.
 *
 * Externamente se presenta como una secuencia de strings donde:
 *           _________________________________
 *  front -> | cmd | arg1 | arg2 | ... | argn | <-back
 *           ---------------------------------
 *
*/

struct scommand_s;
typedef struct scommand_s scommand;

scommand *scommand_new (void);
/*
 * Nuevo `scommand', sin comandos o argumentos y los redirectores vacíos
 *   Returns: nuevo comando simple sin ninguna cadena y con ambos redirectores vacíos.
 * Ensures: result != NULL && scommand_is_empty(result) && 
 *	scommand_get_redir_in(result)==NULL &&
 *	scommand_get_redir_out(result)==NULL &&
 *	scommand_get_builtin(result)
 */
 
void scommand_destroy (scommand *self);
/*
 * Destruye `self'.
 *   self: comando simple a destruir
 * Requires: self != NULL
 */

/* Modificadores */

void scommand_push_back (scommand *self, bstring argument);
/*
 * Agrega por detrás una cadena a la secuencia de cadenas.
 *   self: comando simple al cual agregarle la cadena
 *   argument: cadena a agregar
 * Requires: self!=NULL && argument!=NULL
 * Ensures: !scommand_is_empty()
 */

void scommand_push_front (scommand *self, bstring argument);
/*
 * Agrega por adelante una cadena a la secuencia de cadenas.
 *   self: comando simple al cual agregarle la cadena
 *   argument: cadena a agregar
 * Requires: self!=NULL && argument!=NULL
 * Ensures: !scommand_is_empty()
 */

void scommand_pop_front (scommand *self);
/*
 * Quita la cadena de adelante de la secuencia de cadenas.
 *   self: comando simple al cual sacarle la cadena del frente
 * Requires: self!=NULL && !scommand_is_empty(self)
 */

void scommand_set_builtin (scommand *self, bool builtin);
/*
 * Marca el comando simple como interno o no.
 *   self: comando simple al cual establecer si es interno o no.
 * Requires: self!=NULL
 */

void scommand_set_redir_in (scommand *self, bstring filename);
void scommand_set_redir_out (scommand *self, bstring filename);
/*
 * Define la redirección de entrada (salida).
 *   self: comando simple al cual establecer la redirección de entrada (salida)
 *   filename: cadena con el nombre del archivo de la redirección
 *     o NULL si no se quiere redirección.
 * Requires: self!=NULL
 */

/* Proyectores */

bool scommand_is_empty (const scommand *self);
/*
 * Indica si la secuencia de cadenas tiene longitud 0.
 *   self: comando simple a decidir si está vacío
 *   Returns: ¿Está vacío de cadenas el comando simple?
 * Requires: self!=NULL
 */

unsigned int scommand_length (const scommand *self);
/*
 * Da la longitud de la secuencia cadenas que contiene el comando simple
 *   self: comando simple a medir.
 *   Returns: largo del comando simple
 * Requires: self!=NULL
 * Ensures: (scommand_length(self)==0) == scommand_is_empty()
 * 
 * Con esta función podemos hacer un iterador bastante simple:
 *
 * unsigned int n = scommand_length(scmd), i = 0;
 * bstring str = NULL;
 * while (i<n) {
 *	str = scommand_front(scmd);
 *	scommand_pop_front(scmd);
 *	toquetear_mirar(str);
 *	scommand_push_back(str);
 *	i = i+1;
 * }
 */

bstring scommand_front (const scommand *self);
/*
 * Toma la cadena de adelante de la secuencia de cadenas.
 *   self: comando simple al cual tomarle la cadena del frente
 *   Returns: cadena del frente
 * Requires: self!=NULL && !scommand_is_empty(self)
 * Ensures: result!=NULL
 */

bool scommand_get_builtin (const scommand *self);
/*
 * Consulta si el comando simple es interno.
 *   self: comando simple a decidir si es interno.
 *   Returns: ¿Es un comando interno?
 * Requires: self!=NULL
 */

bstring scommand_get_redir_in (const scommand *self);
bstring scommand_get_redir_out (const scommand *self);
/*
 * Obtiene los nombres de archivos a donde redirigir la entrada (salida).
 *   self: comando simple a decidir si está vacío
 *   Returns: nombre del archivo a donde redirigir la entrada (salida)
 *	o NULL si no está redirigida.
 * Requires: self!=NULL
 */

bstring scommand_to_string (const scommand *self);
/* Preety printer para hacer debugging/logging.
 * Genera una representación del comando simple en un string (aka "serializar")
 *   self: comando simple a convertir
 *   Returns: un string con la representación del comando simple similar
 *     a lo que se escribe en un shell
 * Requires: self!=NULL
 * Ensures: scommand_is_empty(self) ||
 *   scommand_get_redir_in(self)==NULL || scommand_get_redir_in(self)==NULL ||
 *   blength(result)>0
 */


/*
 * pipeline: tubería de comandos. Ejemplo: ls -l *.c > out < in  |  wc  |  grep -i glibc  &
 * Secuencia de comandos simples que se ejecutarán en un pipeline,
 *	más un booleano que indica si hay que esperar o continuar.
 *
 * Queda a cargo del programa que usa este TAD pedir y liberar toda la memoria
 * de los comandos simples que maneja, salvo pipeline_to_string() que pide memoria
 * internamente y debe ser liberada externamente.
 *
 * Externamente se presencta como una secuencia de comandos simples donde:
 *           ______________________________
 *  front -> | scmd1 | scmd2 | ... | scmdn | <-back
 *           ------------------------------
 */ 

struct pipeline_s;
typedef struct pipeline_s pipeline;

pipeline *pipeline_new (void);
/*
 * Nuevo `pipeline', sin comandos simples y establecido para que espere.
 *   Returns: nuevo pipeline sin comandos simples y la espera puesta en verdadero
 * Ensures: result != NULL
 *	&& pipeline_is_empty(result) 
 *	&& pipeline_get_wait(result)
 */

void pipeline_destroy (pipeline *self);
/*
 * Destruye `self'.
 *   self: tubería a a destruir
 * Requires: self != NULL
 */

/* Modificadores */

void pipeline_push_back (pipeline *self, scommand *sc);
/*
 * Agrega por detrás un comando simple a la secuencia.
 *   self: pipeline al cual agregarle el comando simple
 *   sc: comando simple a agregar
 * Requires: self!=NULL && sc!=NULL
 * Ensures: !pipeline_is_empty()
 */

void pipeline_pop_front (pipeline *self);
/*
 * Quita el comando simple de adelante de la secuencia.
 *   self: pipeline al cual sacarle el comando simple del frente
 * Requires: self!=NULL && !pipeline_is_empty(self)
 */

void pipeline_set_wait (pipeline *self, const bool w);
/*
 * Define si el pipeline tiene que esperar o no.
 *   self: pipeline que quiere ser establecido en su atributo de espera.
 * Requires: self!=NULL
 */

/* Proyectores */

bool pipeline_is_empty (const pipeline *self);
/*
 * Indica si la secuencia de comandos simples tiene longitud 0.
 *   self: pipeline a decidir si está vacío
 *   Returns: ¿Está vacío de comandos simples el pipeline?
 * Requires: self!=NULL
 */

unsigned int pipeline_length (const pipeline *self);
/*
 * Da la longitud de la secuencia de comandos simples
 *   self: pipeline a medir.
 *   Returns: largo del pipeline
 * Requires: self!=NULL
 * Ensures: (pipeline_length(self)==0) == pipeline_is_empty()
 * 
 * Con esta función podemos hacer un iterador bastante simple:
 *
 * unsigned int n = pipeline_length(pipe), i = 0;
 * scommand *scmd = NULL;
 * while (i<n) {
 *	scmd = pipeline_front(pipe);
 *	pipeline_pop_front(pipe);
 *	toquetear(scmd);
 *	pipeline_push_back(scmd);
 *	i = i+1;
 * }
 */

scommand *pipeline_front (const pipeline *self);
/*
 * Devuelve el comando simple de adelante de la secuencia.
 *   self: pipeline al cual consultar cual es el comando simple del frente
 *   Returns: comando simple del frente
 * Requires: self!=NULL && !pipeline_is_empty(self)
 * Ensures: result!=NULL
 */

bool pipeline_get_wait (const pipeline *self);
/*
 * Consulta si el pipeline tiene que esperar o no.
 *   self: pipeline a decidir si hay que esperar.
 *   Returns: ¿Hay que esperar en el pipeline self?
 * Requires: self!=NULL
 */

bstring pipeline_to_string (const pipeline *self);
/* Preety printer para hacer debugging/logging.
 * Genera una representación del pipeline en una cadena (aka "serializar").
 *   self: pipeline a convertir
 *   Returns: una cadena con la representación del pipeline similar
 *     a lo que se escribe en un shell
 * Requires: self!=NULL
 * Ensures: pipeline_is_empty(self) || pipeline_get_wait(self) || blength(result)>0
 */

#endif /* COMMAND_H */
