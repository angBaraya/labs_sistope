/*
 * Laboratorio 2 - Sistemas Operativos
 * Autores: [NOMBRE1] - [RUT1]
 *          [NOMBRE2] - [RUT2]
 * Fecha: Diciembre 2025
 *
 * Descripción: Programa principal que recibe argumentos de línea de comando,
 *              reconstruye el pipeline, parsea los scripts y sus argumentos,
 *              y ejecuta el pipeline usando fork(), pipe(), dup2() y exec().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "funciones.h"

#define MAX_LINE_LENGTH 4096

// Entradas: int argc - número de argumentos, char *argv[] - array de argumentos
// Salidas: int - EXIT_SUCCESS si éxito, EXIT_FAILURE si error
// Descripción: Función principal que reconstruye la línea de comando desde argv,
//              la parsea y ejecuta el pipeline de scripts
int main(int argc, char *argv[]) {
    // Verificar que haya argumentos
    if (argc < 2) {
        fprintf(stderr, "Uso: %s script1 args | script2 args | ... | scriptN args\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s generator.sh -i 1 -t 10 \\| preprocess.sh \\| filter.sh -c 5\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Reconstruir la línea de comando concatenando todos los argumentos
    char line[MAX_LINE_LENGTH];
    line[0] = '\0';

    for (int i = 1; i < argc; i++) {
        // Verificar que no excedamos el buffer
        if (strlen(line) + strlen(argv[i]) + 2 > MAX_LINE_LENGTH) {
            fprintf(stderr, "Error: Línea de comando demasiado larga\n");
            return EXIT_FAILURE;
        }

        strcat(line, argv[i]);

        // Agregar espacio entre argumentos (excepto el último)
        if (i < argc - 1) {
            strcat(line, " ");
        }
    }

    // Verificar que la línea no esté vacía
    if (strlen(line) == 0) {
        fprintf(stderr, "Error: Línea de comando vacía\n");
        return EXIT_FAILURE;
    }

    // Parsear la línea de comando
    Pipeline *pipeline = parse_command_line(line);
    if (pipeline == NULL || pipeline->num_commands == 0) {
        fprintf(stderr, "Error: Línea de comando inválida\n");
        if (pipeline != NULL) {
            free_pipeline(pipeline);
        }
        return EXIT_FAILURE;
    }

    // Ejecutar el pipeline
    int result = execute_pipeline(pipeline);

    // Liberar memoria
    free_pipeline(pipeline);

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
