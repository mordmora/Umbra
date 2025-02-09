# Configuración y Ejecución de Pruebas en UmbraLang
Este documento describe cómo configurar y ejecutar pruebas en el proyecto **UmbraLang** utilizando **CTest** y **GoogleTest**.

## Configuración de las Pruebas

### 1. Modificar el archivo `CMakeLists.txt` en la carpeta `tests`

Utiliza el comando `add_subdirectory()` para incluir los directorios con las pruebas. Un ejemplo completo del archivo `tests/CMakeLists.txt` sería:

```cmake
# Incluir GoogleTest usando FetchContent
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

# Pruebas unitarias para lógica del Lexer
add_subdirectory(lexer)

# Añade tu nuevo subdirectorio de prueba - Reemplaza este comentario siguiendo la convención
add_subdirectory(otro) # Cambia "otro" por el nombre de tu carpeta de pruebas
```

### 2. Configurar cada subdirectorio de pruebas
En cada subdirectorio de pruebas, crea un archivo `CMakeLists.txt`. Repite este proceso en caso que crees nuevos subdirectorios, ajustando los nombres y rutas según sea necesario. Por ejemplo, en el subdirectorio `lexer`:

```cmake
# Incluir todos los archivos de prueba en el directorio lexer/
file(GLOB LEXER_TEST_SOURCES "*.cpp")

# Crear un ejecutable para las pruebas del lexer
add_executable(lexer_tests ${LEXER_TEST_SOURCES})

# Enlazar GoogleTest y la biblioteca del proyecto
target_link_libraries(lexer_tests umbra_lexer gtest gtest_main)

# Agregar las pruebas del lexer a CTest
add_test(
    NAME lexer_tests
    COMMAND lexer_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Establecer el directorio de salida para el ejecutable
set_target_properties(lexer_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
```

### 2.1 Desglose de la configuración
Explicación de las variables, parametros y funciones del archivo de configuración para cada subdirectorio.

```cmake
# Incluir todos los archivos de prueba en el directorio lexer/
file(GLOB LEXER_TEST_SOURCES "*.cpp")
```

- `file(GLOB ...)`: Este comando busca y almacena en una variable (`LEXER_TEST_SOURCES`) todos los archivos que coincidan con el patrón especificado (`*.cpp`) en el directorio actual.
- En este caso, la variable `LEXER_TEST_SOURCES` contendrá una lista de archivos `.cpp` que representan las pruebas del módulo lexer.

```cmake
# Crear un ejecutable para las pruebas del lexer
add_executable(lexer_tests ${LEXER_TEST_SOURCES})
```

- `add_executable(...)`: Este comando crea un ejecutable con el nombre lexer_tests.
    - `lexer_tests`: Nombre del ejecutable que se generará.
    - `${LEXER_TEST_SOURCES}`: Lista de archivos fuente que se usarán para compilar el ejecutable. Aquí, la variable `${...}` se usa para referirse a valores almacenados en otras partes del archivo.

**Nota:** Cambia el nombre `lexer_tests` para reflejar el propósito del conjunto de pruebas si estás trabajando con otros módulos.

```cmake
# Enlazar GoogleTest y la biblioteca del proyecto
target_link_libraries(lexer_tests umbra_lexer gtest gtest_main)
```

- `target_link_libraries(...)`: Este comando enlaza bibliotecas necesarias al ejecutable.
    - `lexer_tests`: El nombre del ejecutable que estamos configurando.
    - `umbra_lexer`: Biblioteca interna de tu proyecto que se probará.
    - `gtest` y `gtest_main`: Bibliotecas proporcionadas por GoogleTest para ejecutar las pruebas. Estas fueron incluidas previamente usando FetchContent en el CMakeList principal.

```cmake
# Agregar las pruebas del lexer a CTest
add_test(
    NAME lexer_tests
    COMMAND lexer_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
```
- `add_test(...)`: Este comando registra un conjunto de pruebas en CTest.
    - `NAME`: Nombre descriptivo para las pruebas. Usa un nombre relacionado con el módulo que estás probando (por ejemplo, lexer_tests).
    - `COMMAND`: El ejecutable que se ejecutará para realizar las pruebas.
    - `WORKING_DIRECTORY`: Directorio de trabajo donde se ejecutará el comando. En este caso, `${CMAKE_BINARY_DIR}/bin` es el directorio donde se generan los ejecutables del proyecto.

```cmake
# Establecer el directorio de salida para el ejecutable
set_target_properties(lexer_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
```
- set_target_properties(...): Configura propiedades específicas del ejecutable.
    - `lexer_tests`: El ejecutable al que se aplicarán las propiedades.
    - `RUNTIME_OUTPUT_DIRECTORY`: Directorio donde se guardará el ejecutable generado. Esto garantiza que el ejecutable esté en un lugar específico `(${CMAKE_BINARY_DIR}/bin)`, facilitando la organización y ejecución de las pruebas.

## Pasos para Ejecutar las Pruebas

### 1. Crear un directorio de compilación
Desde la raíz del proyecto, crea una carpeta objetivo para la compilación, puedes usar:

```bash
mkdir build && cd build
```

### 2. Configurar el proyecto con CMake
Desde el directorio `build`, ejecuta:

```bash
cmake ..
```
Esto generará los archivos necesarios para compilar el proyecto y sus pruebas.

### 3. Compilar el proyecto
Compila el proyecto y las pruebas con:

```bash
cmake --build .
```

### 4. Ejecutar las pruebas
Ejecuta todas las pruebas configuradas con:

```bash
ctest
```
Esto ejecutará automáticamente los casos de prueba definidos en tu proyecto.

