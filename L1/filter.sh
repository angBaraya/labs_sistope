#!/bin/bash

ARGS=$(getopt -o c:m:r: -- "$@") || exit 1
eval set -- "$ARGS"

C=""
M=""
R=""

while [ "$1" != "--" ]; do
  case "$1" in
    -c) C="$2"; shift 2 ;;
    -m) M="$2"; shift 2 ;;
    -r) R="$2"; shift 2 ;;
    *) exit 1 ;;
  esac
done
shift

# Leer líneas desde stdin
while read -r line; do
  # Separar fecha y datos
  fecha="${line%% *}"
  datos="${line#* }"

  # Separar campos del proceso
  IFS='|' read -r pid uid comm pcpu pmem <<< "$datos"

  # Validar numéricamente con bc
  pasa=true

  if [[ -n "$C" ]]; then
    pcpu_check=$(echo "$pcpu >= $C" | bc -l)
    [[ "$pcpu_check" -eq 1 ]] || pasa=false
  fi

  if [[ -n "$M" ]]; then
    pmem_check=$(echo "$pmem >= $M" | bc -l)
    [[ "$pmem_check" -eq 1 ]] || pasa=false
  fi

  if [[ -n "$R" ]]; then
    [[ "$comm" =~ $R ]] || pasa=false
  fi

  # Mostrar si pasa todos los filtros
  if $pasa; then
    echo "$line"
  fi
done