#!/bin/bash


while read -r linea; do #lee desde la entrada stdin de forma que se eviten  caracteres especiales
  # Separar la fecha y los datos del proceso
  # Extraer los datos del proceso (últimos campos con formato PID|UID|COMM|PCPU|PMEM)
  info=$(echo "$linea" | grep -oP '\d+\|.+$')
  # Extraer la fecha (todo antes de los datos del proceso)
  fecha=$(echo "$linea" | sed "s/ $info$//")

  # transformación de fecha a ISO 8601
  fecha_iso=$(LC_ALL=C date --date="$fecha" --iso-8601=seconds 2>/dev/null)
  if [[ -z "$fecha_iso" ]]; then # verifica que la fecha ingresada no este vacia, en caso de estar vacia, avisa que la fecha esta invalidada
    echo "Fecha inválida: $fecha"
    continue
  fi

  # Separar campos del proceso con "|" para evitar errores
  IFS='|' read -r pid uid comm pcpu pmem <<< "$info"

  errores=() # crea un arreglo vacio para almacenar "errores" de validacion

  [[ "$pid" =~ ^[0-9]+$ ]] || errores+=("PID inválido: $pid") # verifica que el pid sea un numero entero
  [[ "$uid" =~ ^[0-9]+$ ]] || errores+=("UID inválido: $uid") # verifica que el uid sea un numero entero
  [[ "$comm" =~ ^[[:print:]]+$ ]] || errores+=("COMM inválido: $comm") # verifica que el comm sea un numero entero
  [[ "$pcpu" =~ ^[0-9]+(\.[0-9]+)?$ ]] || errores+=("CPU inválido: $pcpu") # verifica que el cpu sea un decimal
  [[ "$pmem" =~ ^[0-9]+(\.[0-9]+)?$ ]] || errores+=("MEM inválido: $pmem") # verifica que el cpu sea un decimal

  if [ ${#errores[@]} -ne 0 ]; then #ve si errores es distinto de 0, en caso de serlo, significa que ocurrio un error en una linea
    echo "Errores en línea: $fecha_iso $pid $uid $comm $pcpu $pmem"
    for err in "${errores[@]}"; do echo "   → $err"; done #imprime el error
  else
    echo "$fecha_iso $pid|$uid|$comm|$pcpu|$pmem"  #imprime los procesos y fecha en caso de estar bien el procesado
  fi
done