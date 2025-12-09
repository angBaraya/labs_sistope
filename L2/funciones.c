/*
 * Laboratorio 2 - Sistemas Operativos
 * Autores: [NOMBRE1] - [RUT1]
 *          [NOMBRE2] - [RUT2]
 * Fecha: Diciembre 2025
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>
#include "funciones.h"

// ============ FUNCIONES UTILITARIAS ============

// Entradas: const char *msg - Mensaje de error a mostrar
// Salidas: void (no retorna, termina el programa)
// Descripción: Imprime error en stderr y termina el programa con EXIT_FAILURE
void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Entradas: char *str - String a duplicar
// Salidas: char* - Nueva copia del string en heap, NULL si error
// Descripción: Wrapper seguro para strdup con verificación de memoria
char* safe_strdup(const char *str) {
    if (str == NULL) return NULL;
    char *dup = strdup(str);
    if (dup == NULL) {
        error_exit("Error al duplicar string");
    }
    return dup;
}

// Entradas: char *str - String a limpiar
// Salidas: char* - Puntero al string sin espacios al inicio/final
// Descripción: Elimina whitespace al inicio y final de un string (modifica in-place)
char* trim_whitespace(char *str) {
    if (str == NULL) return NULL;

    // Eliminar espacios al inicio
    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') return str;

    // Eliminar espacios al final
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';

    return str;
}

// ============ FUNCIONES DE PARSING ============

// Entradas: char *cmd_str - String de un comando individual (ej: "./filter.sh -c 10 -m 5")
// Salidas: Command* - Estructura con el script y sus argumentos, NULL si error
// Descripción: Tokeniza un comando individual separando el script y sus flags
Command* tokenize_command(char *cmd_str) {
    if (cmd_str == NULL || strlen(cmd_str) == 0) {
        return NULL;
    }

    // Crear estructura de comando
    Command *cmd = (Command*)malloc(sizeof(Command));
    if (cmd == NULL) {
        error_exit("Error al asignar memoria para Command");
    }

    // Inicializar
    cmd->script_path = NULL;
    cmd->args = NULL;
    cmd->argc = 0;

    // Crear copia del string para tokenizar
    char *cmd_copy = safe_strdup(cmd_str);
    cmd_copy = trim_whitespace(cmd_copy);

    // Contar tokens
    char *temp_copy = safe_strdup(cmd_copy);
    char *token = strtok(temp_copy, " \t");
    int token_count = 0;
    while (token != NULL) {
        token_count++;
        token = strtok(NULL, " \t");
    }
    free(temp_copy);

    if (token_count == 0) {
        free(cmd_copy);
        free(cmd);
        return NULL;
    }

    // Tokenizar para extraer script y argumentos
    token = strtok(cmd_copy, " \t");
    char *script_name = token;

    // Manejar rutas de scripts
    if (script_name[0] == '/') {
        // Ruta absoluta
        cmd->script_path = safe_strdup(script_name);
    } else if (strncmp(script_name, "./", 2) == 0) {
        // Reemplazar ./ con ../L1/
        char path[512];
        snprintf(path, sizeof(path), "../L1/%s", script_name + 2);
        cmd->script_path = safe_strdup(path);
    } else if (strncmp(script_name, "../", 3) == 0) {
        // Ya tiene ../, usar tal cual
        cmd->script_path = safe_strdup(script_name);
    } else {
        // Nombre relativo sin path, agregar ../L1/
        char path[512];
        snprintf(path, sizeof(path), "../L1/%s", script_name);
        cmd->script_path = safe_strdup(path);
    }

    // Extraer argumentos
    cmd->argc = 0;
    cmd->args = (char**)malloc(sizeof(char*) * token_count);
    if (cmd->args == NULL) {
        error_exit("Error al asignar memoria para argumentos");
    }

    token = strtok(NULL, " \t");
    while (token != NULL) {
        cmd->args[cmd->argc] = safe_strdup(token);
        cmd->argc++;
        token = strtok(NULL, " \t");
    }

    free(cmd_copy);
    return cmd;
}

// Entradas: char *line - Línea de comando completa leída desde stdin
// Salidas: Pipeline* - Estructura con todos los comandos parseados, NULL si error
// Descripción: Parsea la línea de comando completa, divide por '|' y
//              extrae cada comando con sus argumentos
Pipeline* parse_command_line(char *line) {
    if (line == NULL || strlen(line) == 0) {
        return NULL;
    }

    // Crear estructura de pipeline
    Pipeline *pipeline = (Pipeline*)malloc(sizeof(Pipeline));
    if (pipeline == NULL) {
        error_exit("Error al asignar memoria para Pipeline");
    }

    pipeline->commands = NULL;
    pipeline->num_commands = 0;

    // Crear copia del string para tokenizar
    char *line_copy = safe_strdup(line);

    // Contar número de comandos (pipes + 1)
    int num_pipes = 0;
    for (char *p = line_copy; *p; p++) {
        if (*p == '|') num_pipes++;
    }
    int num_commands = num_pipes + 1;

    // Asignar memoria para comandos
    pipeline->commands = (Command*)malloc(sizeof(Command) * num_commands);
    if (pipeline->commands == NULL) {
        error_exit("Error al asignar memoria para comandos");
    }

    // Tokenizar por pipes
    char *cmd_str = strtok(line_copy, "|");
    int idx = 0;

    while (cmd_str != NULL && idx < num_commands) {
        Command *cmd = tokenize_command(cmd_str);
        if (cmd != NULL) {
            pipeline->commands[idx] = *cmd;
            free(cmd);  // Solo liberamos la estructura, no el contenido
            idx++;
        }
        cmd_str = strtok(NULL, "|");
    }

    pipeline->num_commands = idx;
    free(line_copy);

    return pipeline;
}

// ============ FUNCIONES DE PIPELINE ============

// Entradas: Command *cmd - Comando a ejecutar
//           PipeFD *pipes - Array de pipes creados
//           int num_pipes - Cantidad de pipes en el array
//           int cmd_index - Índice del comando actual (0 a num_commands-1)
// Salidas: void (no retorna, ejecuta exec o termina con error)
// Descripción: Configura stdin/stdout del hijo usando dup2, cierra pipes
//              no usados y ejecuta el script con execvp
void execute_child_process(Command *cmd, PipeFD *pipes, int num_pipes, int cmd_index) {
    // 1. Redirigir stdin (si no es el primero)
    if (cmd_index > 0) {
        if (dup2(pipes[cmd_index-1].read_fd, STDIN_FILENO) == -1) {
            error_exit("Error en dup2 para stdin");
        }
    }

    // 2. Redirigir stdout (si no es el último)
    if (cmd_index < num_pipes) {
        if (dup2(pipes[cmd_index].write_fd, STDOUT_FILENO) == -1) {
            error_exit("Error en dup2 para stdout");
        }
    }

    // 3. Cerrar TODOS los descriptores de pipes
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i].read_fd);
        close(pipes[i].write_fd);
    }

    // 4. Construir argv para execvp
    // Para scripts .sh, ejecutarlos explícitamente con bash
    int is_script = (strstr(cmd->script_path, ".sh") != NULL);

    char **argv;
    if (is_script) {
        // Ejecutar con bash: /bin/bash script.sh arg1 arg2 ...
        argv = (char**)malloc(sizeof(char*) * (cmd->argc + 3));
        if (argv == NULL) {
            error_exit("Error al asignar memoria para argv");
        }
        argv[0] = "/bin/bash";
        argv[1] = cmd->script_path;
        for (int i = 0; i < cmd->argc; i++) {
            argv[i + 2] = cmd->args[i];
        }
        argv[cmd->argc + 2] = NULL;
    } else {
        // Comando normal
        argv = (char**)malloc(sizeof(char*) * (cmd->argc + 2));
        if (argv == NULL) {
            error_exit("Error al asignar memoria para argv");
        }
        argv[0] = cmd->script_path;
        for (int i = 0; i < cmd->argc; i++) {
            argv[i + 1] = cmd->args[i];
        }
        argv[cmd->argc + 1] = NULL;
    }

    // 5. Ejecutar el comando/script
    execvp(argv[0], argv);

    // Si llegamos aquí, exec falló
    error_exit("Error en execvp");
}

// Entradas: Pipeline *pipeline - Estructura con comandos a ejecutar
// Salidas: int - 0 en éxito, -1 en error
// Descripción: Crea todos los pipes y procesos hijos, conecta el pipeline
//              y ejecuta cada script. Espera a que terminen todos los hijos.
int execute_pipeline(Pipeline *pipeline) {
    if (pipeline == NULL || pipeline->num_commands == 0) {
        return -1;
    }

    int num_commands = pipeline->num_commands;
    int num_pipes = num_commands - 1;

    // Si solo hay un comando, ejecutarlo sin pipes
    if (num_pipes == 0) {
        pid_t pid = fork();
        if (pid == -1) {
            error_exit("Error en fork");
        } else if (pid == 0) {
            // Proceso hijo
            Command *cmd = &pipeline->commands[0];

            // Para scripts .sh, ejecutarlos explícitamente con bash
            int is_script = (strstr(cmd->script_path, ".sh") != NULL);
            char **argv;

            if (is_script) {
                // Ejecutar con bash
                argv = (char**)malloc(sizeof(char*) * (cmd->argc + 3));
                argv[0] = "/bin/bash";
                argv[1] = cmd->script_path;
                for (int i = 0; i < cmd->argc; i++) {
                    argv[i + 2] = cmd->args[i];
                }
                argv[cmd->argc + 2] = NULL;
            } else {
                // Comando normal
                argv = (char**)malloc(sizeof(char*) * (cmd->argc + 2));
                argv[0] = cmd->script_path;
                for (int i = 0; i < cmd->argc; i++) {
                    argv[i + 1] = cmd->args[i];
                }
                argv[cmd->argc + 1] = NULL;
            }

            execvp(argv[0], argv);
            error_exit("Error en execvp");
        }

        // Proceso padre espera
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1;
    }

    // Crear todos los pipes
    PipeFD *pipes = (PipeFD*)malloc(sizeof(PipeFD) * num_pipes);
    if (pipes == NULL) {
        error_exit("Error al asignar memoria para pipes");
    }

    for (int i = 0; i < num_pipes; i++) {
        int fds[2];
        if (pipe(fds) == -1) {
            error_exit("Error al crear pipe");
        }
        pipes[i].read_fd = fds[0];
        pipes[i].write_fd = fds[1];
    }

    // Crear procesos hijos
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            error_exit("Error en fork");
        } else if (pid == 0) {
            // Proceso hijo
            execute_child_process(&pipeline->commands[i], pipes, num_pipes, i);
            // execute_child_process no retorna
        }
        // Proceso padre continúa
    }

    // Padre cierra todos los pipes
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i].read_fd);
        close(pipes[i].write_fd);
    }

    // Padre espera a todos los hijos
    int all_success = 1;
    for (int i = 0; i < num_commands; i++) {
        int status;
        pid_t pid = wait(&status);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                fprintf(stderr, "Hijo %d terminó con código %d\n", pid, exit_code);
                all_success = 0;
            }
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Hijo %d terminado por señal %d\n", pid, WTERMSIG(status));
            all_success = 0;
        }
    }

    free(pipes);
    return all_success ? 0 : -1;
}

// ============ FUNCIONES DE LIMPIEZA ============

// Entradas: Command *cmd - Comando a liberar
// Salidas: void
// Descripción: Libera memoria de un comando individual (script_path y args)
void free_command(Command *cmd) {
    if (cmd == NULL) return;

    if (cmd->script_path != NULL) {
        free(cmd->script_path);
        cmd->script_path = NULL;
    }

    if (cmd->args != NULL) {
        for (int i = 0; i < cmd->argc; i++) {
            if (cmd->args[i] != NULL) {
                free(cmd->args[i]);
            }
        }
        free(cmd->args);
        cmd->args = NULL;
    }

    cmd->argc = 0;
}

// Entradas: Pipeline *pipeline - Estructura a liberar
// Salidas: void
// Descripción: Libera toda la memoria asociada a la pipeline (comandos, args, etc.)
void free_pipeline(Pipeline *pipeline) {
    if (pipeline == NULL) return;

    if (pipeline->commands != NULL) {
        for (int i = 0; i < pipeline->num_commands; i++) {
            free_command(&pipeline->commands[i]);
        }
        free(pipeline->commands);
        pipeline->commands = NULL;
    }

    pipeline->num_commands = 0;
    free(pipeline);
}
