# Umbra Lang Compiler
Este repositorio contiene la implementación del compilador Umbra Lang. El proyecto está estructurado de la siguiente manera:

```plaintext
│ umbra-lang/
│
├── src/                    # Ficheros fuente para el compilador
│   ├── lexer/              # Aplicación del analizador léxico
│   ├── parser/             # Analizador sintáctico 
│   ├── ast/                # Definiciones del AST
│   ├── codegen/            # Generación de código para LLVM IR
│   └── main.cpp            # Punto de entrada principal del compilador
│
├── include/                # Ficheros de cabecera públicos
│
├── lib/                    # Bibliotecas externas
│
├── test/                   # Ficheros de prueba
│   ├── CMakeLists.txt      # Definición de pruebas        
│   ├── lexer_tests/        # Pruebas para el analizador léxico
│   ├── parser_tests/       # Pruebas del analizador sintáctico 
│   └── codegen_tests/      # Pruebas de generación de código
│
├── examples/               # Ejemplos de programas Umbra Lang
│
├── docs/                   # Ficheros de documentación
│
├── build/                  # Construcción de directorio de salida
│
├── .vscode/                # Ajustes específicos de VS Code
│   ├── tasks.json          # Configuración de las tareas de construcción
│   └── launch.json         # Configuración del depurador
│
├── CMakeLists.txt          # Archivo de configuración CMake
├── .gitignore              # Fichero Git Ignore
└── README.md               # Este archivo
```

## Contenido del Directorio

**src/**: Contiene todo el código fuente del compilador Umbra Lang.
- **lexer/:** Implementación del analizador léxico.
- **parser/:** Contendrá el analizador sintáctico (por implementar).
- **ast/:** Definiciones para los nodos del Árbol de Sintaxis Abstracta.
- **codegen/:** Módulo de generación de código para producir LLVM IR.
- **main.cpp: ** El punto de entrada principal del compilador.

**include/:** Ficheros de cabecera públicos, si el compilador expone alguna API pública.

**lib/:** Bibliotecas externas o dependencias, si se utiliza alguna.

**test/:** Contiene todos los ficheros de prueba para asegurar la corrección del compilador.

**examples/:** Programas de ejemplo de Umbra Lang para demostrar las características del lenguaje.

**docs/:** Documentación del proyecto, especificaciones del lenguaje, etc.

**build/:** Directorio para los artefactos de compilación (no rastreados en git).

**.vscode/:** Configuración de Visual Studio Code para facilitar el desarrollo.

**CMakeLists.txt:** Configuración del sistema de compilación CMake.

**.gitignore:** Especifica los archivos no rastreados intencionadamente para ignorarlos.

**README.md:** Proporciona una visión general del proyecto (¡estás aquí!).


## Primeros Pasos

- [Montaje](./SETUP.md)

- [Como contribuir](./HOW_TO_CONTRIBUTE.md)


## Licencia
Apache 2.0 "Umbra" 