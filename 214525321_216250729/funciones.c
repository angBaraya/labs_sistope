/*
 * Autores: Angel Benavides - 21.452.532-1
 * Diego Lefiman - 21.625.072-9
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

//FUNCIONES

// Entradas: mensaje de error a mostrar para mostrar fallos
// Salidas: VACIO ya que termina el programa y no retorna nada 
// Descripción: Imprime error en stderr y termina el programa con EXIT_FAILURE, ahorra lineas de codigo
void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Entradas: String a verificar para duplicar
// Salidas: Nueva copia del string en heap, caso contrario retorna NULL
// Descripción: Wrapper seguro para strdup que verifica si falló el malloc
char* safe_strdup(const char *str) {
    if (str == NULL) return NULL;
    
    char *dup = strdup(str);
    if (dup == NULL) {
        error_exit("Error: No se pudo duplicar el string (memoria llena)");
    }
    return dup;
}

// Entradas: String a limpiar
// Salidas: Puntero al string sin espacios al inicio/final
// Descripción: Elimina espacio basura al inicio y final 
char* trim_whitespace(char *str) {
    if (str == NULL) return NULL;

    // Avanzamos el puntero mientras haya espacio al inicio
    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') return str;

    // Vamos al final y retrocedemos borrando espacios
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0'; // Marcamos el nuevo final

    return str;
}


// Entradas: String de un comando individual
// Salidas: Estructura con el script y sus argumentos
// Descripción: Toma un string, lo limpia y separa el nombre del script de sus flags
Command* tokenize_command(char *cmd_str) {
    if (!cmd_str || strlen(cmd_str) == 0) return NULL;

    // Pedimos memoria para la estructura
    Command *cmd = (Command*)malloc(sizeof(Command));
    if (cmd == NULL) error_exit("Malloc falló en Command");

    // Inicializamos por seguridad
    cmd->script_path = NULL;
    cmd->args = NULL;
    cmd->argc = 0;

    // Hacemos una copia limpia para trabajar
    char *buffer_raw = safe_strdup(cmd_str);
    char *buffer = trim_whitespace(buffer_raw);

    // PASO 1: Contar cuántos tokens (palabras) hay para hacer el malloc
    char *copia_contar = safe_strdup(buffer);
    char *ptr_seguro;
    char *token = strtok_r(copia_contar, " \t", &ptr_seguro);
    
    int num_tokens = 0;
    while (token != NULL) {
        num_tokens++;
        token = strtok_r(NULL, " \t", &ptr_seguro);
    }
    free(copia_contar);

    // Si no hay nada, limpiamos y retornamos
    if (num_tokens == 0) {
        free(buffer_raw);
        free(cmd);
        return NULL;
    }

    // PASO 2: Extraer los datos reales
    char *ptr_parseo;
    token = strtok_r(buffer, " \t", &ptr_parseo);
    char *nombre_script = token;

    // Lógica para encontrar dónde está el script (Ruta absoluta, relativa o L1)
    if (nombre_script[0] == '/') {
        cmd->script_path = safe_strdup(nombre_script); // Ruta absoluta
    } 
    else if (strncmp(nombre_script, "./", 2) == 0 || strncmp(nombre_script, "../", 3) == 0) {
        cmd->script_path = safe_strdup(nombre_script); // Ya viene con ruta
    } 
    else {
        // Probamos primero en el directorio actual ./
        char path_test[512];
        snprintf(path_test, sizeof(path_test), "./%s", nombre_script);

        if (access(path_test, F_OK) == 0) {
            cmd->script_path = safe_strdup(path_test);
        } else {
            // Si no está, probamos en la carpeta anterior ../L1/
            char path_l1[512];
            snprintf(path_l1, sizeof(path_l1), "../L1/%s", nombre_script);
            cmd->script_path = safe_strdup(path_l1);
        }
    }

    // PASO 3: Guardar los argumentos
    cmd->args = (char**)malloc(sizeof(char*) * num_tokens);
    if (!cmd->args) error_exit("Malloc falló en args");

    // Reiniciamos el tokenizado (o seguimos con el resto)
    // Ojo: el primer token fue el script, pero strtok guarda estado
    // Necesitamos volver a tokenizar o seguir desde donde quedamos?
    // Como ya sacamos el nombre, seguimos con los argumentos.
    
    // Un detalle: necesitamos guardar los argumentos en el arreglo args.
    // Voy a volver a tokenizar el string original limpio para ser ordenado y guardar todo.
    // Esto es un poco redundante pero seguro.
    
    free(buffer_raw); // libero el anterior, voy a usar uno nuevo fresco
    buffer_raw = safe_strdup(cmd_str);
    buffer = trim_whitespace(buffer_raw);
    
    token = strtok_r(buffer, " \t", &ptr_parseo); // Primer token (script)
    
    // Saltamos el script y vamos por los argumentos reales
    token = strtok_r(NULL, " \t", &ptr_parseo);
    
    int i = 0;
    while (token != NULL) {
        cmd->args[i] = safe_strdup(token);
        i++;
        token = strtok_r(NULL, " \t", &ptr_parseo);
    }
    cmd->argc = i; // Cantidad real de argumentos

    free(buffer_raw);
    return cmd;
}

// Entradas:Línea de comando completa
// Salidas: Estructura con todos los comandos
// Descripción: Corta el string por pipes '|' y procesa cada parte
Pipeline* parse_command_line(char *line) {
    if (!line || strlen(line) == 0) return NULL;

    Pipeline *pipe_struct = (Pipeline*)malloc(sizeof(Pipeline));
    if (!pipe_struct) error_exit("No hay memoria para Pipeline");

    pipe_struct->commands = NULL;
    pipe_struct->num_commands = 0;

    char *linea_trabajo = safe_strdup(line);

    // Contamos pipes para saber tamaño del arreglo
    int num_pipes = 0;
    for (char *p = linea_trabajo; *p; p++) {
        if (*p == '|') num_pipes++;
    }
    int total_cmds = num_pipes + 1;

    pipe_struct->commands = (Command*)malloc(sizeof(Command) * total_cmds);
    if (!pipe_struct->commands) error_exit("No hay memoria para array de comandos");

    // Separamos por '|'
    char *ptr;
    char *segmento = strtok_r(linea_trabajo, "|", &ptr);
    int idx = 0;

    while (segmento != NULL && idx < total_cmds) {
        Command *cmd_temp = tokenize_command(segmento);
        if (cmd_temp != NULL) {
            pipe_struct->commands[idx] = *cmd_temp;
            free(cmd_temp); // Solo liberamos el puntero temporal, el contenido se copió
            idx++;
        }
        segmento = strtok_r(NULL, "|", &ptr);
    }

    pipe_struct->num_commands = idx;
    free(linea_trabajo);
    return pipe_struct;
}

// Entradas: Command *cmd, PipeFD *pipes, indices...
// Salidas: void (ejecuta exec y muere)
// Descripción: Configura la fontanería (dup2) y se transforma en el script (exec)
void execute_child_process(Command *cmd, PipeFD *pipes, int num_pipes, int idx) {
    // 1. Conectar entrada (STDIN)
    // Si no soy el primero, leo del tubo anterior
    if (idx > 0) {
        if (dup2(pipes[idx-1].read_fd, STDIN_FILENO) == -1) 
            error_exit("Fallo dup2 entrada");
    }

    // 2. Conectar salida (STDOUT)
    // Si no soy el último, escribo en mi tubo
    if (idx < num_pipes) {
        if (dup2(pipes[idx].write_fd, STDOUT_FILENO) == -1) 
            error_exit("Fallo dup2 salida");
    }

    // 3. Cerrar TODOS los tubos (Vital para no colgarse)
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i].read_fd);
        close(pipes[i].write_fd);
    }

    // 4. Preparar argumentos para execvp
    // Verificamos si es .sh para invocar bash explícitamente
    int es_sh = (strstr(cmd->script_path, ".sh") != NULL);
    char **argv_final;

    if (es_sh) {
        // Formato: /bin/bash script.sh arg1 arg2 ...
        argv_final = (char**)malloc(sizeof(char*) * (cmd->argc + 3));
        if (!argv_final) error_exit("Malloc argv fallo");

        argv_final[0] = "/bin/bash";
        argv_final[1] = cmd->script_path;
        for (int i = 0; i < cmd->argc; i++) {
            argv_final[i + 2] = cmd->args[i];
        }
        argv_final[cmd->argc + 2] = NULL;
    } else {
        // Formato normal
        argv_final = (char**)malloc(sizeof(char*) * (cmd->argc + 2));
        if (!argv_final) error_exit("Malloc argv fallo");

        argv_final[0] = cmd->script_path;
        for (int i = 0; i < cmd->argc; i++) {
            argv_final[i + 1] = cmd->args[i];
        }
        argv_final[cmd->argc + 1] = NULL;
    }

    // 5. Transformación final
    execvp(argv_final[0], argv_final);
    
    // Si llegamos acá, algo salió muy mal
    error_exit("Error fatal en execvp");
}

// Entradas: Pipeline *pipeline - Estructura completa
// Salidas: int - 0 si todo bien, -1 si falló algo
// Descripción: Orquesta la creación de pipes y forks
int execute_pipeline(Pipeline *pipeline) {
    if (!pipeline || pipeline->num_commands == 0) return -1;

    int n_cmds = pipeline->num_commands;
    int n_pipes = n_cmds - 1;

    // CASO BORDE: Si solo hay 1 comando, no usamos pipes
    if (n_pipes == 0) {
        pid_t pid = fork();
        if (pid == -1) error_exit("Fork fallo");
        
        if (pid == 0) {
            // Soy el hijo único
            Command *cmd = &pipeline->commands[0];
            
            // Misma lógica de bash que en execute_child...
            // Podríamos modularizar esto, pero lo dejo aquí explícito
            int es_sh = (strstr(cmd->script_path, ".sh") != NULL);
            char **argv;

            if (es_sh) {
                argv = malloc(sizeof(char*) * (cmd->argc + 3));
                argv[0] = "/bin/bash";
                argv[1] = cmd->script_path;
                for (int i = 0; i < cmd->argc; i++) argv[i+2] = cmd->args[i];
                argv[cmd->argc + 2] = NULL;
            } else {
                argv = malloc(sizeof(char*) * (cmd->argc + 2));
                argv[0] = cmd->script_path;
                for (int i = 0; i < cmd->argc; i++) argv[i+1] = cmd->args[i];
                argv[cmd->argc + 1] = NULL;
            }
            execvp(argv[0], argv);
            error_exit("Exec fallo en hijo único");
        }
        
        // Padre espera
        int status;
        waitpid(pid, &status, 0);
        return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 0 : -1;
    }

    // CASO NORMAL: Múltiples comandos con pipes
    PipeFD *tubos = (PipeFD*)malloc(sizeof(PipeFD) * n_pipes);
    if (!tubos) error_exit("Malloc tubos fallo");

    // Creamos todos los tubos antes de forkear
    for (int i = 0; i < n_pipes; i++) {
        int fd[2];
        if (pipe(fd) == -1) error_exit("Fallo al crear pipe");
        tubos[i].read_fd = fd[0];
        tubos[i].write_fd = fd[1];
    }

    // Lanzamos los hijos
    for (int i = 0; i < n_cmds; i++) {
        pid_t pid = fork();
        if (pid == -1) error_exit("Fallo fork múltiple");

        if (pid == 0) {
            // El hijo se va a ejecutar y nunca vuelve
            execute_child_process(&pipeline->commands[i], tubos, n_pipes, i);
        }
    }

    // Padre: cerrar pipes y esperar
    for (int i = 0; i < n_pipes; i++) {
        close(tubos[i].read_fd);
        close(tubos[i].write_fd);
    }

    int todo_ok = 1;
    for (int i = 0; i < n_cmds; i++) {
        int status;
        pid_t p = wait(&status);
        
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                fprintf(stderr, "Hijo %d terminó con error %d\n", p, WEXITSTATUS(status));
                todo_ok = 0;
            }
        } else {
            todo_ok = 0; // Terminó por señal
        }
    }

    free(tubos);
    return todo_ok ? 0 : -1;
}


// Entradas: Command *cmd - Comando que se busca liberar
// Salidas: vacia ya que es solo liberación
// Descripción: Libera strings internos
void free_command(Command *cmd) {
    if (!cmd) return;

    if (cmd->script_path) {
        free(cmd->script_path);
        cmd->script_path = NULL;
    }

    if (cmd->args) {
        for (int i = 0; i < cmd->argc; i++) {
            if (cmd->args[i]) free(cmd->args[i]);
        }
        free(cmd->args);
        cmd->args = NULL;
    }
    cmd->argc = 0;
}

// Entradas: Pipeline *pipeline - Estructura a liberar
// Salidas: void
// Descripción: Libera toda la estructura
void free_pipeline(Pipeline *pipeline) {
    if (!pipeline) return;

    if (pipeline->commands) {
        for (int i = 0; i < pipeline->num_commands; i++) {
            free_command(&pipeline->commands[i]);
        }
        free(pipeline->commands);
    }
    pipeline->num_commands = 0;
    free(pipeline);
}