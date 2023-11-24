#!/bin/bash
echo Using emmake to generate output in ./build/wasm folder
echo Old wasm folder is autonumbered to avoid being rewritten

# Ruta de la carpeta base
base_folder="build/wasm"

# Obtener el último número utilizado en las carpetas
last_number=$(ls -d $base_folder* | grep -oE '[0-9]+' | sort -n | tail -n 1 2>/dev/null)

# Si no hay carpetas existentes, establecer el último número en 0
if [ -z "$last_number" ]; then
  last_number=0
fi

# Calcular el próximo número disponible
next_number=$((last_number + 1))

# Construir el nuevo nombre autonumerado de la carpeta existente
new_folder="${base_folder}_${next_number}"

# Preservar la carpeta existente cambiando su nombre
mv "$base_folder" "$new_folder"

source ../emsdk/emsdk_env.sh
emmake make