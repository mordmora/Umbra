# Umbra Compiler

Umbra es un compilador experimental para un lenguaje de propósito general, diseñado como laboratorio de compiladores y para aprendizaje de técnicas modernas de frontend y backend.

## Estructura del Proyecto

```
Umbra/
├── include/
│   ├── umbra/
│   │   ├── ast/              # Definiciones de nodos del AST y Visitor
│   │   ├── codegen/          # Contexto y visitantes para generación de código LLVM
│   │   ├── error/            # Manejo de errores y reportes
│   │   ├── io/               # Utilidades de lectura de archivos (UmbraIO)
│   │   ├── lexer/            # Lexer y tokens
│   │   ├── parser/           # Parser y definición de la gramática
│   │   ├── preprocessor/     # Preprocesador para directivas 'use'
│   │   ├── semantic/         # Análisis semántico, tipos y tabla de símbolos
│   │   ├── utils/            # Utilidades generales (formatos, helpers)
│   │   └── ...               # Otros módulos
├── src/
│   ├── ast/
│   ├── codegen/
│   ├── error/
│   ├── io/
│   ├── lexer/
│   ├── parser/
│   ├── preprocessor/
│   ├── semantic/
│   ├── utils/
│   └── main.cpp              # Punto de entrada del compilador
├── examples/                 # Programas de ejemplo en Umbra
├── tests/                    # Pruebas unitarias y de integración
├── CMakeLists.txt            # Build system
└── README.md                 # Este archivo
```

## Principales módulos

- **AST**: Nodos del árbol de sintaxis y patrón Visitor.
- **Lexer**: Tokenización y análisis léxico.
- **Parser**: Construcción del AST desde tokens.
- **Preprocessor**: Resolución de directivas `use` e inclusión de archivos.
- **Semantic**: Recolección de símbolos, validación de tipos y firmas, control de scopes.
- **Codegen**: Generación de LLVM IR y mapeo a código máquina.
- **IO**: Utilidad header-only para lectura eficiente de archivos fuente.
- **Error**: Manejo y reporte de errores léxicos, sintácticos y semánticos.
- **Utils**: Helpers para formatos de impresión, conversiones y otros algoritmos.

## Compilación

Requiere CMake y LLVM (>= 15).

> Guía completa de instalación y configuración: [SETUP.md](SETUP.md)

```sh
mkdir build
cd build
cmake ..
make -j
```

## Uso

Ejemplo de compilación y ejecución de un programa Umbra:

```sh
./bin/umbra ../examples/hola_mundo.umbra
```

## Ejemplos

- `examples/hola_mundo.umbra`: Hola mundo con print.
- `examples/if.umbra`: Condicionales.
- `examples/io.umbra`: Prueba de lectura y print.

## Documentación

- El código está documentado con Doxygen en los headers principales.
- Para generar la documentación:
  ```sh
  doxygen Doxyfile
  ```

## Contribuir

- Forkea el repositorio y crea una rama para tus cambios.
- Haz PRs contra `main` siguiendo la convención de módulos y documentación.
- Usa `clang-format` para mantener el estilo.

> Revisa la guía detallada de contribución: [CONTRIBUTING.md](CONTRIBUTING.md)

## Preguntas Frecuentes

**¿Qué versiones de LLVM son compatibles?**
> LLVM 20.

**¿Cómo agrego un nuevo nodo al AST?**
> Añade la clase en `include/umbra/ast/Nodes.h` y actualiza el archivo de nodos para el Visitor (`VisitorNodes.def`).

**¿Cómo reporto errores léxicos/sintácticos/semánticos?**
> Usa el módulo `ErrorManager` y revisa los ejemplos en `src/error/ErrorManager.cpp`.

**¿Cómo extiendo el preprocesador para nuevas directivas?**
> Modifica `src/preprocessor/Preprocessor.cpp` y añade la lógica en el método `processFile`.

**¿Cómo genero el IR optimizado?**
> Usa `opt` de LLVM sobre el archivo `.ll` generado antes de compilar con `llc`.

## Licencia

MIT
