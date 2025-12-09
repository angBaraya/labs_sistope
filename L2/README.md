# Laboratorio 2 - Sistemas Operativos
## Pipeline de Procesamiento de Procesos del Sistema

**Autores:**
- [NOMBRE1] - [RUT1]
- [NOMBRE2] - [RUT2]

**Fecha:** Diciembre 2025

---

## Descripción

Este proyecto implementa un orquestador de pipelines en C que ejecuta una cadena de scripts bash para monitorear, filtrar y generar reportes de procesos del sistema operativo. Utiliza llamadas al sistema POSIX (fork, pipe, dup2, exec) para crear procesos hijos conectados mediante pipes, permitiendo el flujo de datos entre los 6 scripts que componen el pipeline.

El programa lee desde `stdin` una línea de comando que especifica los scripts y sus parámetros, separados por el carácter `|`, y crea la infraestructura de comunicación inter-proceso necesaria para ejecutarlos en paralelo.

---

## Arquitectura

```
Proceso Padre (lab2)
    |
    ├─ Hijo 1: generator.sh -i X -t Y
    |     stdout → pipe1 → stdin
    ├─ Hijo 2: preprocess.sh
    |     stdout → pipe2 → stdin
    ├─ Hijo 3: filter.sh -c X -m Y -r "regex"
    |     stdout → pipe3 → stdin
    ├─ Hijo 4: transform.sh --anon-uid
    |     stdout → pipe4 → stdin
    ├─ Hijo 5: aggregate.sh
    |     stdout → pipe5 → stdin
    └─ Hijo 6: report.sh -o output.csv
          (escribe archivo CSV)
```

Cada proceso hijo ejecuta un script bash que:
- Lee datos desde `stdin` (excepto el primero)
- Procesa la información según su función específica
- Escribe resultados a `stdout` (excepto el último que guarda en archivo)

---

## Scripts del Pipeline

### 1. **generator.sh**
Captura información de procesos del sistema usando `ps`.

**Parámetros:**
- `-i INTERVALO`: Intervalo en segundos entre capturas (default: 1)
- `-t TIEMPO`: Duración total en segundos (default: 10)

**Salida:** Líneas con formato `<timestamp> <PID>|<UID>|<COMM>|<PCPU>|<PMEM>`

### 2. **preprocess.sh**
Convierte timestamps a formato ISO-8601.

**Entrada:** Salida de generator.sh
**Salida:** Líneas con timestamp en formato `YYYY-MM-DDTHH:MM:SS-TZ`

### 3. **filter.sh**
Filtra procesos según criterios de uso de recursos y nombre.

**Parámetros:**
- `-c THRESHOLD`: Umbral mínimo de %CPU
- `-m THRESHOLD`: Umbral mínimo de %MEM
- `-r REGEX`: Expresión regular para filtrar por nombre de proceso

**Salida:** Solo líneas que cumplan todos los criterios

### 4. **transform.sh**
Anonimiza información sensible.

**Parámetros:**
- `--anon-uid`: Reemplaza UID reales con valores aleatorios

**Salida:** Datos con UIDs anonimizados

### 5. **aggregate.sh**
Calcula estadísticas agregadas por comando.

**Salida:** TSV con columnas: `command`, `nproc`, `cpu_avg`, `cpu_max`, `mem_avg`, `mem_max`

### 6. **report.sh**
Genera reporte en formato CSV con metadatos.

**Parámetros:**
- `-o ARCHIVO`: Nombre del archivo de salida (default: `reporte.csv`)

**Salida:** Archivo CSV con encabezado de metadatos (fecha, usuario, host) y datos tabulados

---

## Compilación

El proyecto incluye un `Makefile` para facilitar la compilación:

```bash
make
```

Esto genera el ejecutable `lab2` a partir de `lab2.c` y `funciones.c`.

Para limpiar archivos objeto y ejecutable:

```bash
make clean
```

**Requisitos:**
- GCC (compatible con C99)
- Sistema operativo Linux/Unix con soporte POSIX
- Bash (para ejecutar los scripts)

---

## Uso

El programa recibe los argumentos directamente desde la línea de comando:

```bash
./lab2 script1 args \| script2 args \| ... \| scriptN args
```

**Nota importante:** Los pipes `|` deben estar escapados con `\|` para que el shell no los interprete como pipes del sistema operativo.

### Formato de Entrada

Cada script puede tener sus propios parámetros:
- Scripts con rutas relativas (`./script.sh`) se buscan primero en el directorio actual
- Scripts sin path (`script.sh`) se buscan en `./` y luego en `../L1/`
- Se pueden usar rutas absolutas (`/path/to/script.sh`)
- Los argumentos se separan con espacios
- Los scripts del pipeline se separan con `|` (escapado como `\|`)

---

## Ejemplos de Uso

### Ejemplo 1: Pipeline Básico (2 scripts)

Capturar procesos durante 3 segundos y convertir timestamps:

```bash
./lab2 ./generator.sh -i 1 -t 3 \| ./preprocess.sh
```

**Salida esperada:**
```
2025-12-09T10:30:15-03:00 1234|1000|bash|2.5|1.2
2025-12-09T10:30:16-03:00 1235|1000|firefox|15.3|8.7
...
```

---

### Ejemplo 2: Filtrado por CPU

Capturar procesos con más de 5% de CPU y generar reporte:

```bash
./lab2 ./generator.sh -i 1 -t 5 \| ./preprocess.sh \| ./filter.sh -c 5 \| ./aggregate.sh \| ./report.sh -o cpu_alto.csv
```

**Archivo generado (`cpu_alto.csv`):**
```csv
# generated_at: 2025-12-09T10:35:22-03:00
# user: angel
# host: ullr

command nproc   cpu_avg cpu_max mem_avg mem_max
firefox 3       12.5    18.2    7.3     9.1
chrome  2       8.7     10.5    5.2     6.8
```

---

### Ejemplo 3: Pipeline Completo con Anonimización

Capturar procesos, filtrar por CPU>5% y MEM>1%, anonimizar UIDs y generar reporte:

```bash
./lab2 ./generator.sh -i 1 -t 5 \| ./preprocess.sh \| ./filter.sh -c 5 -m 1 \| ./transform.sh --anon-uid \| ./aggregate.sh \| ./report.sh -o reporte_completo.csv
```

**Resultado:** Archivo `reporte_completo.csv` con estadísticas agregadas de procesos que consumen más de 5% CPU y 1% memoria, con UIDs anonimizados.

---

### Ejemplo 4: Filtrado por Regex

Capturar solo procesos cuyo nombre coincida con una expresión regular:

```bash
./lab2 ./generator.sh -i 1 -t 5 \| ./preprocess.sh \| ./filter.sh -r '^bash$' \| ./aggregate.sh \| ./report.sh -o bash_only.csv
```

**Resultado:** Reporte solo con procesos llamados exactamente "bash".

---

### Ejemplo 5: Filtros Combinados

Capturar procesos bash con CPU>1%:

```bash
./lab2 ./generator.sh -i 1 -t 10 \| ./preprocess.sh \| ./filter.sh -c 1 -r '^bash' \| ./aggregate.sh \| ./report.sh -o bash_cpu.csv
```

---

## Formato de Salida

### Salida Intermedia (stdout del pipeline)

Durante la ejecución, los datos fluyen entre scripts en formato TSV (Tab-Separated Values):

```
2025-12-09T10:40:15-03:00    1234|1000|bash|2.5|1.2
2025-12-09T10:40:16-03:00    5678|2341|firefox|18.3|9.7
```

### Archivo CSV Final

El script `report.sh` genera un archivo CSV con:

**Encabezado de metadatos:**
```csv
# generated_at: 2025-12-09T10:55:18-03:00
# user: angel
# host: ullr
```

**Datos tabulados:**
```csv
command nproc   cpu_avg cpu_max mem_avg mem_max
firefox 5       15.2    22.3    8.1     10.5
bash    3       2.1     3.5     1.2     1.8
chrome  2       10.5    12.8    6.3     7.9
```

**Columnas:**
- `command`: Nombre del proceso
- `nproc`: Número de instancias observadas
- `cpu_avg`: Promedio de %CPU
- `cpu_max`: Máximo %CPU observado
- `mem_avg`: Promedio de %MEM
- `mem_max`: Máximo %MEM observado

---

## Estructura del Código

### Archivos Principales

- **`lab2.c`**: Programa principal, lee stdin y ejecuta pipeline
- **`funciones.c`**: Implementación de parsing y ejecución del pipeline
- **`funciones.h`**: Definiciones de estructuras y firmas de funciones
- **`Makefile`**: Reglas de compilación
- **Scripts bash**: `generator.sh`, `preprocess.sh`, `filter.sh`, `transform.sh`, `aggregate.sh`, `report.sh`

### Estructuras de Datos

```c
// Representa un comando individual con sus argumentos
typedef struct {
    char *script_path;    // Ruta al script
    char **args;          // Array de argumentos
    int argc;             // Cantidad de argumentos
} Command;

// Representa el pipeline completo
typedef struct {
    Command *commands;    // Array de comandos
    int num_commands;     // Cantidad de comandos
} Pipeline;

// Descriptores de un pipe
typedef struct {
    int read_fd;          // Extremo de lectura
    int write_fd;         // Extremo de escritura
} PipeFD;
```

### Funciones Principales

- `parse_command_line()`: Parsea la línea de entrada y crea estructura Pipeline
- `tokenize_command()`: Extrae script y argumentos de un comando individual
- `execute_pipeline()`: Crea pipes y procesos, ejecuta el pipeline completo
- `execute_child_process()`: Configura redirección stdin/stdout y ejecuta script
- `free_pipeline()`: Libera toda la memoria asignada

---

## Notas Técnicas

### Manejo de Pipes

El programa crea `N-1` pipes para `N` comandos. Cada proceso hijo:
1. Redirige `stdin` desde el pipe anterior (si no es el primero)
2. Redirige `stdout` hacia el siguiente pipe (si no es el último)
3. Cierra **todos** los descriptores de pipes (crítico para evitar deadlocks)
4. Ejecuta el script usando `execvp()`

### Ejecución de Scripts

Los scripts `.sh` se ejecutan mediante `/bin/bash`:
```c
execvp("/bin/bash", {"/bin/bash", "script.sh", "arg1", "arg2", NULL})
```

### Manejo de Errores

- El proceso padre espera a todos los hijos con `wait()`
- Se reportan códigos de salida no-cero y señales de terminación
- Se valida la existencia de scripts antes de ejecutar

---

## Solución de Problemas

### Error: "Error en execvp"
- **Causa:** Script no encontrado o sin permisos de ejecución
- **Solución:** Verificar que los scripts estén en el directorio correcto y tengan permisos `chmod +x *.sh`

### Error: Pipeline no produce salida
- **Causa:** Filtros demasiado restrictivos
- **Solución:** Relajar umbrales de CPU/MEM o probar sin filtros primero

### Error: "Hijo terminó con código X"
- **Causa:** Script bash falló (sintaxis, argumentos inválidos)
- **Solución:** Ejecutar el script manualmente para ver el error: `./script.sh args`

---

## Correcciones Realizadas

### Bug en preprocess.sh (Líneas 6-7)

**Problema:** El parsing de fecha fallaba para días con un solo dígito (1-9) debido a espaciado variable en la salida de `date`.

**Solución implementada:**
```bash
# Extraer datos del proceso (último campo con formato PID|UID|COMM|PCPU|PMEM)
info=$(echo "$linea" | grep -oP '\d+\|.+$')
# Extraer fecha (todo antes de los datos del proceso)
fecha=$(echo "$linea" | sed "s/ $info$//")
```

Esto permite parsear correctamente fechas independiente del espaciado:
- `Mon Dec  9 10:30:15 2025` (día con 1 dígito)
- `Mon Dec 12 10:30:15 2025` (día con 2 dígitos)

---

## Referencias

- Manual de pipes: `man 2 pipe`
- Manual de fork: `man 2 fork`
- Manual de dup2: `man 2 dup2`
- Manual de execvp: `man 3 execvp`
- Guía de procesos POSIX: [https://pubs.opengroup.org/onlinepubs/9699919799/](https://pubs.opengroup.org/onlinepubs/9699919799/)

---

## Licencia

Este código es parte del Laboratorio 2 del curso de Sistemas Operativos - DIINF USACH.
