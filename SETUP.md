## Montando el proyecto

### Requisitos Previos

#### Instalación de LLVM

El proyecto requiere **LLVM** para su compilación. A continuación te mostramos cómo instalarlo. **Se recomienda instalar LLVM desde el código fuente** para obtener la versión más reciente. Puedes ver cómo hacerlo en este video:

[Instrucciones para instalar LLVM desde código fuente](https://www.youtube.com/watch?v=l0LI_7KeFtw&t=93s)

### Instalación de Dependencias
#### Linux

#### Distribuciones basadas en Debian (como Ubuntu)

Primero instala las siguientes dependencias:

```bash
sudo apt install cmake g++ build-essential
``` 

#### Distribuciones basadas en Arch (como Manjaro)

Si usas paru como tu AUR Helper, puedes instalar los paquetes con el siguiente comando:

```bash
paru -S cmake g++
```

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

### Verificar la Instalación
Para asegurarte de que todo se ha compilado correctamente, ejecuta las pruebas incluidas en el proyecto. Navega al directorio de salida y ejecuta el test del lexer:

```bash
cd bin
./lexer_test
```
