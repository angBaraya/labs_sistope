/*
 * Autores: Angel Benavides - 21.452.532-1
 *          Diego Lefiman - 21.625.072-9
 */

#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <sys/types.h>
//ESTRUCTURAS

//Se crea una estructura comando para almacenar la informacion de cada comando
typedef struct {
    char *script_path;           //ruta del script a ver (ej: "../L1/generator.sh")
    char **args;                 //array de argumentos (ej: ["-i", "1", "-t", "10", NULL])
    int argc;                    //numero de argumentos
} Command; //nombre de la estructura Command

//creacion de la estructura pipeline para almacenar los comandos
typedef struct {
    Command *commands;           //array de comandos
    int num_commands;            //cantidad de comandos en el pipeline
} Pipeline;//nombre de la estructura Pipeline

//estructura para manejar los file descriptors de los pipes
typedef struct {
    int read_fd;                 // Extremo de lectura del pipe
    int write_fd;                // Extremo de escritura del pipe
} PipeFD;//nombre de la estructura PipeFD


Pipeline* parse_command_line(char *line);


Command* tokenize_command(char *cmd_str);


char* safe_strdup(const char *str);


char* trim_whitespace(char *str);


int execute_pipeline(Pipeline *pipeline);


void execute_child_process(Command *cmd, PipeFD *pipes, int num_pipes, int cmd_index);


void free_pipeline(Pipeline *pipeline);


void free_command(Command *cmd);


void error_exit(const char *msg);

#endif 
