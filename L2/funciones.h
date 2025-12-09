/*
 * Laboratorio 2 - Sistemas Operativos
 * Autores: [NOMBRE1] - [RUT1]
 *          [NOMBRE2] - [RUT2]
 * Fecha: Diciembre 2025
 */

#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <sys/types.h>

// ============ ESTRUCTURAS ============

// Entradas: N/A
// Salidas: N/A
// Descripción: Representa un comando individual (un script con sus argumentos)
typedef struct {
    char *script_path;           // Ruta al script (ej: "../L1/generator.sh")
    char **args;                 // Array de argumentos (ej: ["-i", "1", "-t", "10", NULL])
    int argc;                    // Número de argumentos
} Command;

// Entradas: N/A
// Salidas: N/A
// Descripción: Representa el pipeline completo con todos los comandos
typedef struct {
    Command *commands;           // Array de comandos
    int num_commands;            // Cantidad de comandos en el pipeline
} Pipeline;

// Entradas: N/A
// Salidas: N/A
// Descripción: Par de descriptores de archivo para un pipe
typedef struct {
    int read_fd;                 // Extremo de lectura del pipe
    int write_fd;                // Extremo de escritura del pipe
} PipeFD;

// ============ FUNCIONES DE PARSING ============

// Entradas: char *line - Línea de comando completa leída desde stdin
// Salidas: Pipeline* - Estructura con todos los comandos parseados, NULL si error
// Descripción: Parsea la línea de comando completa, divide por '|' y
//              extrae cada comando con sus argumentos
Pipeline* parse_command_line(char *line);

// Entradas: char *cmd_str - String de un comando individual (ej: "./filter.sh -c 10 -m 5")
// Salidas: Command* - Estructura con el script y sus argumentos, NULL si error
// Descripción: Tokeniza un comando individual separando el script y sus flags
Command* tokenize_command(char *cmd_str);

// Entradas: char *str - String a duplicar
// Salidas: char* - Nueva copia del string en heap, NULL si error
// Descripción: Wrapper seguro para strdup con verificación de memoria
char* safe_strdup(const char *str);

// Entradas: char *str - String a limpiar
// Salidas: char* - Puntero al string sin espacios al inicio/final
// Descripción: Elimina whitespace al inicio y final de un string (modifica in-place)
char* trim_whitespace(char *str);

// ============ FUNCIONES DE PIPELINE ============

// Entradas: Pipeline *pipeline - Estructura con comandos a ejecutar
// Salidas: int - 0 en éxito, -1 en error
// Descripción: Crea todos los pipes y procesos hijos, conecta el pipeline
//              y ejecuta cada script. Espera a que terminen todos los hijos.
int execute_pipeline(Pipeline *pipeline);

// Entradas: Command *cmd - Comando a ejecutar
//           PipeFD *pipes - Array de pipes creados
//           int num_pipes - Cantidad de pipes en el array
//           int cmd_index - Índice del comando actual (0 a num_commands-1)
// Salidas: void (no retorna, ejecuta exec o termina con error)
// Descripción: Configura stdin/stdout del hijo usando dup2, cierra pipes
//              no usados y ejecuta el script con execvp
void execute_child_process(Command *cmd, PipeFD *pipes, int num_pipes, int cmd_index);

// ============ FUNCIONES DE LIMPIEZA ============

// Entradas: Pipeline *pipeline - Estructura a liberar
// Salidas: void
// Descripción: Libera toda la memoria asociada a la pipeline (comandos, args, etc.)
void free_pipeline(Pipeline *pipeline);

// Entradas: Command *cmd - Comando a liberar
// Salidas: void
// Descripción: Libera memoria de un comando individual (script_path y args)
void free_command(Command *cmd);

// ============ FUNCIONES UTILITARIAS ============

// Entradas: const char *msg - Mensaje de error a mostrar
// Salidas: void (no retorna, termina el programa)
// Descripción: Imprime error en stderr y termina el programa con EXIT_FAILURE
void error_exit(const char *msg);

#endif // FUNCIONES_H
