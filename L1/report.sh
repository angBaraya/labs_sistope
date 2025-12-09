#!/bin/bash

# Procesar opción -o
ARGS=$(getopt -o o: -- "$@") || exit 1
eval set -- "$ARGS"

OUTPUT=""

while [ "$1" != "--" ]; do
  case "$1" in
    -o) OUTPUT="$2"; shift 2 ;;
    *) exit 1 ;;
  esac
done
shift

# Validar que se haya especificado archivo
if [[ -z "$OUTPUT" ]]; then
  echo "Debes especificar un archivo de salida con -o"
  exit 1
fi

# Obtener metadatos
timestamp=$(date --iso-8601=seconds)
user=$(whoami)
host=$(hostname)

# Escribir metadatos y contenido al archivo
{
  echo "# generated_at: $timestamp"
  echo "# user: $user"
  echo "# host: $host"
  echo
  cat  # contenido de aggregate.sh
} > "$OUTPUT"
