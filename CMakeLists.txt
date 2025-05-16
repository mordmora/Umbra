# Versión mínima requerida de CMake
cmake_minimum_required(VERSION 3.10)
enable_testing()
project(UmbraLang VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
 # add_definitions(-DUMBRA_DEBUG)
endif()

# Establecer el tipo de compilación predeterminado a Debug si no se especifica
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build: Debug or Release" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


# Flags del compilador para Debug y Release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Configuring Debug build")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Configuring Release build")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()


# Opciones de compilación según el compilador
if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra )
endif()

#paquete de boost
find_package(Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_definitions(-DBOOST_ALL_NO_LIB)
    
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
else()
    message(FATAL_ERROR "Boost not found")
endif()

# Paquete de LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


# Biblioteca del analizador léxico 
add_library(umbra_lexer STATIC
    src/lexer/Lexer.cpp
    src/lexer/Lexer.h
    src/error/CompilerError.cpp
    src/error/ErrorManager.cpp
    src/preprocessor/Preprocessor.cpp

)

add_library(umbra_nodes STATIC
    src/ast/ASTNode.h
    src/ast/Nodes.h
    src/ast/Nodes.cpp
    src/ast/Types.h
)

# Ejecutable del compilador Umbra
file(GLOB_RECURSE SOURCES
  "src/main.cpp"
  "src/utils/*.cpp"
  "src/parser/*.cpp"
  "src/lexer/*.cpp"
  "src/ast/*.cpp"
  "src/ast/visitor/*.cpp"
  "src/error/*.cpp"
  "src/semantic/*.cpp"
  "src/codegen/context/*.cpp"
  "src/codegen/ir/*.cpp"
  "src/codegen/types/*.cpp"
  "src/codegen/visitors/*.cpp"
  "src/compiler/*.cpp"
  "src/preprocessor/*.cpp")
add_executable(umbra ${SOURCES})
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(umbra ${llvm_libs} umbra_lexer)
target_link_libraries(umbra ${Boost_LIBRARIES})
set_target_properties(umbra
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


# Agregar pruebas
add_subdirectory(tests)
