/*
 * Laboratorio 2 - Sistemas Operativos
 * Autores: [NOMBRE1] - [RUT1]
 *          [NOMBRE2] - [RUT2]
 * Fecha: Diciembre 2025
 *
 * Descripción: Programa principal que lee una línea de comando desde stdin,
 *              parsea los scripts y sus argumentos, y ejecuta un pipeline
 *              usando fork(), pipe(), dup2() y exec().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "funciones.h"

#define MAX_LINE_LENGTH 4096

// Entradas: int argc - número de argumentos, char *argv[] - array de argumentos
// Salidas: int - EXIT_SUCCESS si éxito, EXIT_FAILURE si error
// Descripción: Función principal que lee la línea de comando desde stdin,
//              la parsea y ejecuta el pipeline de scripts
int main(int argc, char *argv[]) {
    char line[MAX_LINE_LENGTH];

    // Leer línea de comando desde stdin
    if (fgets(line, sizeof(line), stdin) == NULL) {
        fprintf(stderr, "Error: No se pudo leer la línea de comando\n");
        return EXIT_FAILURE;
    }

    // Remover newline final si existe
    size_t len = strlen(line);
    if (len > 0 && line[len-1] == '\n') {
        line[len-1] = '\0';
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
