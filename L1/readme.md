# Instrucciones de ejecución

Para realizar la ejecución del pipeline completo, se debe ejecutar de la siguiente manera:
-Primero debemos usar el comando `chmod +x` para cada archivo bash. Este comando sirve para darle permisos de ejecución a un archivo en sistemas tipo Unix (como Linux). Es decir, convierte un archivo —por ejemplo, un script .sh— en algo que puedes ejecutar directamente como un programa.

Ejecutando el siguiente comando en el directorio con los archivos se da permisos a todos los archivos .sh de la carpeta:

```bash
chmod +x *.sh
```


Para avanzar con la ejecucion del pipeline completo donde tendremos que colocar las siguientes lineas de comando en la terminal ubicada en el directorio, para generar distintos csv's con distintos parametros.

**Ejemplo de reporte generado usado para la entrega:**
```console
./generator.sh -i 2 -t 30 | ./preprocess.sh | ./filter.sh -c 0.2 -m 0.1 -r "firefox|d|w|a" | ./transform.sh --anon-uid | ./aggregate.sh | ./report.sh -o reporte.csv

```

Ejemplo 1:

```console
./generator.sh -i 2 -t 4 | ./preprocess.sh | ./filter.sh -c 0.1 -m 0.1 | ./transform.sh --anon-uid | ./aggregate.sh | ./report.sh -o report1.csv
```

Ejemplo 2:

```console
./generator.sh -i 1 -t 5 | ./preprocess.sh | ./filter.sh -c 0.2 -m 0.1 -r "d" | ./transform.sh --anon-uid | ./aggregate.sh | ./report.sh -o report2.csv 
```

Ejemplo 3:

```console
./generator.sh -i 2 -t 8 | ./preprocess.sh | ./filter.sh -c 0.1 -m 0.5  | ./transform.sh --anon-uid | ./aggregate.sh | ./report.sh -o report3.csv 
```
Ejemplo 4:

```console
./generator.sh -i 3 -t 12 | ./preprocess.sh | ./filter.sh -c 0.2 -m 0.1 -r "^code|firefox" | ./transform.sh --anon-uid | ./aggregate.sh | ./report.sh -o report4.csv
```


# Detalles

- Se usa getopt para la captura de parametros mediante flags (-i, -t, -c, -m, -r) en los scripts correspondientes.

- Se usa stdin/stdout para la comunicación entre scripts

- Para la funcionalidad y lograr responder correctamente cada uno de los codigo implementados, se tuvo que recurrir al uso de "|" entre cada proceso(comm, pcpu, pmem, etc), esto debido a que existian nombres que contenian espacios, pero no interfiere en el proyecto.

- Se cumple parcialmente la validación de parámetros, las flags de generator.sh aceptan números negativos (aunque se detiene la ejecución casi al instante) pero no acepta letras ni otras flags. Una situación similar ocurre con las flags de transform.sh

- para hashear el UID se utilizó SHA-256, que genera una cadena hexaedecimal de 64 caracteres, pero se corto en el código.

La linea clave en el código es:
```bash
uid=$(echo -n "$uid" | sha256sum | awk '{print substr($1,1,12)}')
```
**echo -n "$uid"**, muestra el uid sin el salto de linea \n

**wk '{print substr($1,1,12)}**, extrae los pirmeros 12 caracteres del hash, siguiendo las tablas de ejemplo enviadas.
