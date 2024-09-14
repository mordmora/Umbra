#!/bin/bash

# Crear la carpeta 'imgs' si no existe
mkdir -p imgs

for file in *.dot; do    
    if [ -e "$file" ]; then        
        base_name=$(basename "$file" .dot)
        dot -Tpng "$file" -o "imgs/${base_name}.png"       
        
        echo "Generado: imgs/${base_name}.png"
    else
        echo "No se encontraron archivos .dot en el directorio."
    fi
done
