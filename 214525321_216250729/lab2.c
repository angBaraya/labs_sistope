/*
 * Autores: Angel Benavides - 21.452.532-1
 *          Diego Lefiman - 21.625.072-9
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "funciones.h"

#define MAX_LINE_LENGTH 4096 //definicion del largo max  que aceptara el programa

// Entradas: argc -> num de argumentos, argv -> array de argumentos
// Salidas: verifica si se ejecuta correctamente el pipeline, retorna EXIT_SUCCESS o EXIT_FAILURE, dependiendo de que tan correcto se ejecute
// Descripción: Función principal que reconstruye la línea de comando desde argv, la parsea y ejecuta el pipeline de scripts
int main(int argc, char *argv[]) {
    if (argc < 2) { // se verifica que existan los argumentos necesarios
        fprintf(stderr, "No se entregaron los suficientes argumentos para ejecutar el programa.\n");
        fprintf(stderr, "Uso: %s script1 args | script2 args | ... | scriptN args\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s generator.sh -i 1 -t 10 \\| preprocess.sh \\| filter.sh -c 5\n", argv[0]);
        return EXIT_FAILURE;
    }

    
    char line[MAX_LINE_LENGTH];// reconstruye la línea de comando concatenando todos los argumentos, para evitar segmentacion fault
    line[0] = '\0';

    for (int i = 1; i < argc; i++) {
        // se verifica que la linea no exceda el maximo permitido que son lo limitado por MAX_LINE_LENGTH, ya agrega
        if (strlen(line) + strlen(argv[i]) + 2 > MAX_LINE_LENGTH) {
            fprintf(stderr, "Error: Línea de comando demasiado larga\n");
            return EXIT_FAILURE;
        }

        strcat(line, argv[i]); //concatenacion de los argumentos dados por el usuario  junto al line

        //agregar un espacio entre argumentos (excepto el último)
        if (i < argc - 1) {
            strcat(line, " ");
        }
    }

    //verifica que hayan elementos en el line, si no existe, retorna error
    if (strlen(line) == 0) {
        fprintf(stderr, "Error: Línea de comando vacía\n");
        return EXIT_FAILURE;
    }

    //parsear la línea de comando
    Pipeline *pipeline = parse_command_line(line);
    if (pipeline == NULL || pipeline->num_commands == 0) {
        fprintf(stderr, "Error: Línea de comando inválida\n");
        if (pipeline != NULL) {
            free_pipeline(pipeline);
        }
        return EXIT_FAILURE;
    }

    //ejecutar el pipeline
    int result = execute_pipeline(pipeline);

    //libera memoria usada por el pipeline
    free_pipeline(pipeline);

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE; // verifica si el resultado es 0, en caso de serlo, retorna 
}
