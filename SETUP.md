### Montando el proyecto
Necesitamos instalar algunos paquetes antes de compilar el proyecto.

Primero debemos tener instalado LLVM, en este video se explica como hacerlo. Preferiblemente instalarlo desde el codigo fuente.
 https://www.youtube.com/watch?v=l0LI_7KeFtw&t=93s

#### Linux
- #### Distribuciones basadas en Debian
```sudo apt install cmake g++ build-essential``` 
- #### Distribuciones basadas en Arch
    Asumiendo que usas paru como AUR Helper

    ```paru -S cmake g++```

Ahora clona el repositorio

 ```
 git clone https://github.com/mordmora/Umbra.git
 cd Umbra   
 ```

 Luego copia estos comandos en tu terminal

 ```
    mkdir build
    cd build
    cmake ..
    cmake --build .

 ```

 Puedes usar configuraciones para compilar en modo debug o release
 ``` 
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake -DCMAKE_BUILD_TYPE=Release ..
 ```

 Para asegurarte que todo salió bien, ejecuta las pruebas.
 ```
 cd bin
./lexer_test

 ```
