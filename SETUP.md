## Montando el proyecto

### Requisitos Previos

#### Instalación de LLVM

El proyecto requiere **LLVM** para su compilación. A continuación te mostramos cómo instalarlo. **Se recomienda instalar LLVM desde el código fuente** para obtener la versión más reciente. Puedes ver cómo hacerlo en este video:

[Instrucciones para instalar LLVM desde código fuente](https://www.youtube.com/watch?v=l0LI_7KeFtw&t=93s)

### Instalación de Dependencias
#### Linux

> Dependencias principales: LLVM 20, Boost Program Options, GoogleTest, CMake y un compilador C++17.

##### Distribuciones basadas en Debian (Ubuntu, etc.)

1) CMake, toolchain y Boost Program Options:
```bash
sudo apt update
sudo apt install -y cmake g++ build-essential libboost-program-options-dev
```

2) LLVM 20 (desde apt.llvm.org):
```bash
# Añade el repo oficial de LLVM y instala la versión 20
wget https://apt.llvm.org/llvm.sh -O /tmp/llvm.sh
chmod +x /tmp/llvm.sh
sudo /tmp/llvm.sh 20  # instala llvm-20, clang-20, lld-20, etc.
# Opcional: headers y cmake config
sudo apt install -y llvm-20-dev
```
Notas:
- Si prefieres paquetes de la distro, busca `llvm-X`, `clang-X`, `lld-X`. Asegúrate de que CMake encuentre LLVM 20.

3) GoogleTest:
```bash
sudo apt install -y libgtest-dev
# En algunas versiones es necesario compilar/instalar las libs del fuente del paquete
sudo cmake -S /usr/src/googletest -B /usr/src/googletest/build
sudo cmake --build /usr/src/googletest/build --config Release
sudo cmake --install /usr/src/googletest/build
```

##### Distribuciones basadas en Arch (Arch, Manjaro)

1) CMake, toolchain y Boost:
```bash
sudo pacman -Syu --needed cmake base-devel gcc boost
```

2) LLVM/Clang/Lld (Arch suele proveer la versión reciente):
```bash
sudo pacman -S --needed llvm clang lld
```

3) GoogleTest:
```bash
sudo pacman -S --needed gtest
```

> Si usas AUR helper (ej. paru): reemplaza `sudo pacman -S` por `paru -S` según tu preferencia.

### Clonar el Repositorio

Una vez que tengas las dependencias instaladas, clona el repositorio del proyecto:

```bash
git clone https://github.com/mordmora/Umbra.git
cd Umbra
```

### Configuración y Compilación
#### Creación del Directorio Build
A continuación, crea el directorio para los archivos de construcción y compila el proyecto con **CMake**:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

#### Configuración para Modo Debug o Release
Puedes elegir compilar el proyecto en modo **Debug** o **Release**. Para ello, especifica el tipo de construcción con el siguiente comando:
- Para **Debug:**

   ```bash
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   cmake --build .
   ```

- Para **Release:**

   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .
   ```

#### Integración de dependencias con CMake

- LLVM 20:
```cmake
# CMakeLists.txt
find_package(LLVM 20 REQUIRED CONFIG)
message(STATUS "Found LLVM: ${LLVM_PACKAGE_VERSION}")
# Usar variables que expone LLVM para includes/defines
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
```
Si CMake no encuentra LLVM automáticamente, pasa la ruta de configuración:
```bash
cmake -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm ..
# o donde esté instalado en tu sistema
```

- Boost Program Options:
```cmake
find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})
# target_link_libraries(tu_target PRIVATE Boost::program_options)
```

- GoogleTest (sistema):
```cmake
find_package(GTest REQUIRED)
# target_link_libraries(tu_test_target PRIVATE GTest::gtest GTest::gtest_main)
```

- GoogleTest (alternativa con FetchContent, sin paquetes del SO):
```cmake
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Para MSVC: deshabilita CRT warnings si aplica
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# target_link_libraries(tu_test_target PRIVATE gtest gtest_main)
```

### Verificar la Instalación
Para asegurarte de que todo se ha compilado correctamente, ejecuta las pruebas incluidas en el proyecto. Navega al directorio de salida y ejecuta el test del lexer:

```bash
cd bin
./lexer_test
```

#### Ejecutar pruebas con CTest
```bash
cd build
ctest --output-on-failure
```
