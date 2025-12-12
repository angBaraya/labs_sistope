#!/bin/bash

# Procesar opción --anon-uid con getopt
ARGS=$(getopt -o "" --long anon-uid -- "$@") || exit 1
eval set -- "$ARGS"

ANON_UID=false

while [ "$1" != "--" ]; do
  case "$1" in
    --anon-uid) ANON_UID=true; shift ;;
    *) exit 1 ;;
  esac
done
shift

# leer líneas desde stdin
while read -r line; do
  # separar fecha y datos
  fecha=$(echo "$line" | awk '{print $1}')
  datos=$(echo "$line" | cut -d' ' -f2-)

  # separamos los campos del proceso
  IFS='|' read -r pid uid comm pcpu pmem <<< "$datos"

  # anonimizar UID si se pidió
  if $ANON_UID; then
    uid=$(echo -n "$uid" | sha256sum | awk '{print substr($1,1,12)}')
  fi

  # mostramos la linea hasheada
  echo "$fecha $pid|$uid|$comm|$pcpu|$pmem"
done
