

## Como lo hace

El programa lee desde `stdin` una línea de comando que especifica los scripts y sus parámetros, separados por el carácter `|`, y crea la infraestructura de comunicación inter-proceso necesaria para ejecutarlos en paralelo. También se debe ejecutar la en terminal: 'chmod +x *' para dar permisos de ejecutar los .sh 



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
## EJEMPLOS DE LA ENTREGA (LAB2_SO.PDF):
"" 
./lab2 generator.sh -i 1 -t 10 \
| ./preprocess.sh \
| ./filter.sh -c 10 -m 5 -r "^(python|chrome)$" \
| ./transform.sh --anon-uid \
| ./aggregate.sh \
| ./report.sh -o reporte.tsv
""


EJEMPLO 1:

./lab2 generator.sh -i 1 -t 5 | ./preprocess.sh | ./transform.sh --anon-uid | ./aggregate.sh | ./report.sh -o report2.csv 

MUESTRA TODOS LOS PROCESOS SIN FILTRAR

EJEMPLO 2:






### Problema en preprocess.sh

El parsing de fecha fallaba para días con un solo dígito (1-9) debido a espaciado variable en la salida de `date`.

