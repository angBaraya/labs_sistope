#!/usr/bin/env bash
#se fuerza la configuracion regional para tener un estandar 
LC_ALL=C awk '
{
  # Separar fecha y bloque de datos
  fecha = $1
  datos = substr($0, index($0, $2))
 
  # Separar datos por pipe, guardandolos en un arreglo f
  split(datos, f, "|")

  # asigna a cada posicion del arreglo una variable correspondiente
  pid  = f[1]
  uid  = f[2]
  comm = f[3]
  pcpu = f[4] + 0
  pmem = f[5] + 0
  
  # incrementa el contador de procesos para el commando con mismo nombre
  n[comm]++
  #suma el uso de cpu, para sacar el promedio
  sum_cpu[comm] += pcpu
  #suma el uso de memoria para sacar un promedio de estos
  sum_mem[comm] += pmem
  # compara entre el valor maximo registrado y lo compara con el actual, actualizando en caso de ser mayor el actual
  if (pcpu > max_cpu[comm]) max_cpu[comm] = pcpu
  if (pmem > max_mem[comm]) max_mem[comm] = pmem
}
END {
  #imprime el encabezado de los nombres dados
  print "command\tnproc\tcpu_avg\tcpu_max\tmem_avg\tmem_max"
  for (comm in n) {
    printf "%s\t%d\t%.1f\t%.1f\t%.1f\t%.1f\n",
    #recorre cada comm, sacando el promedio y el maximo de cada uno
      comm, n[comm],
      sum_cpu[comm]/n[comm], max_cpu[comm],
      sum_mem[comm]/n[comm], max_mem[comm]
  }
}
' "${1:-/dev/stdin}"