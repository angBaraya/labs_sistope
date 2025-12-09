#!/bin/bash
# ejecucion con interprete de bash


ARGS=$(getopt -o i:t: -- "$@") || exit 1 #Uso de getopt para interpretar los valor de i (intervalos de muestreo en segundos) y t (duración total de la captura en segundos)
eval set -- "$ARGS" 
#Variables que almacenaran los valores de i y  t 
I=""
T=""


while [ "$1" != "--" ]; do
  case "$1" in
    -i) I="$2"; shift 2 ;; #asigna i a la variable I
    -t) T="$2"; shift 2 ;; #asigna t a la variable T
    *) exit 1 ;; #si la opcion ingresada es distinta a las anteriores mencionadas, termina la ejecuciṕn
  esac
done
shift

SECONDS=0 #varible que sera un contador de segundos para controlar el muestreo de procesos

while [ "$SECONDS" -lt "$T" ]; do #ejecuta el bloque dentro del while mientras segundos sea menor a T 
  timestamp=$(LC_ALL=C date) #se obtiene fecha y hora actual en formato estandar
  ps -eo pid=,uid=,comm=,pcpu=,pmem= --sort=-%cpu --no-headers | #ejecuta ps para listar los procesos
  awk '
    {
  3    # reconstruye comm si tiene espacios
      comm = $3 
      #accede a la primera palabra del coom
      for (i = 4; i <= NF - 2; i++) comm = comm " " $i #llega hasta el antepenultima opcion, la cual sera la ultima palabra del comm
      print $1 "|" $2 "|" comm "|" $(NF-1) "|" $NF # imprime los procesos con "|"
    }' | while read -r line; do
    
    echo "$timestamp $line" # añade marca de tiempo a cada procesado
  done
  sleep "$I" # pausa el script por i segundos dados anteriormente, para luego repetir el ciclo
done