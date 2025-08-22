cmake_minimum_required(VERSION 3.10)
project(UmbraLang VERSION 0.1.0 LANGUAGES C CXX)
enable_testing()

# Configuración base
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build: Debug or Release" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Flags por tipo de build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(UMBRA_DEBUG)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# Warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Boost
find_package(Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS})
    add_definitions(-DBOOST_ALL_NO_LIB)
else()
    message(FATAL_ERROR "Boost not found")
endif()

# LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Incluir headers desde include/
include_directories(${CMAKE_SOURCE_DIR}/include)

# Librerías internas por módulo

# === MÓDULOS COMO LIBRERÍAS INTERNAS ===

# AST
file(GLOB_RECURSE AST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/ast/*.cpp)
add_library(umbra_ast ${AST_SOURCES})
target_link_libraries(umbra_ast PUBLIC umbra_error)
target_include_directories(umbra_ast PUBLIC ${CMAKE_SOURCE_DIR}/include)

# ERROR
file(GLOB_RECURSE ERROR_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/error/*.cpp)
add_library(umbra_error ${ERROR_SOURCES})
target_include_directories(umbra_error PUBLIC ${CMAKE_SOURCE_DIR}/include)

# LEXER & PREPROCESSOR
file(GLOB_RECURSE LEXER_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/src/lexer/*.cpp
  ${CMAKE_SOURCE_DIR}/src/preprocessor/*.cpp)
add_library(umbra_lexer ${LEXER_SOURCES})
target_link_libraries(umbra_lexer PUBLIC umbra_error)
target_include_directories(umbra_lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)

# PARSER
file(GLOB_RECURSE PARSER_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/parser/*.cpp)
add_library(umbra_parser ${PARSER_SOURCES})
target_link_libraries(umbra_parser PUBLIC umbra_ast umbra_lexer umbra_error)
target_include_directories(umbra_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)

# SEMANTIC
file(GLOB_RECURSE SEMANTIC_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/semantic/*.cpp)
add_library(umbra_semantic ${SEMANTIC_SOURCES})
target_link_libraries(umbra_semantic PUBLIC umbra_parser umbra_error)
target_include_directories(umbra_semantic PUBLIC ${CMAKE_SOURCE_DIR}/include)

# CODEGEN
file(GLOB_RECURSE CODEGEN_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/src/codegen/context/*.cpp
  ${CMAKE_SOURCE_DIR}/src/codegen/ir/*.cpp
  ${CMAKE_SOURCE_DIR}/src/codegen/types/*.cpp
  ${CMAKE_SOURCE_DIR}/src/codegen/visitors/*.cpp)
add_library(umbra_codegen ${CODEGEN_SOURCES})
target_link_libraries(umbra_codegen PUBLIC umbra_ast LLVM)
target_include_directories(umbra_codegen PUBLIC ${CMAKE_SOURCE_DIR}/include)

# COMPILER
file(GLOB_RECURSE COMPILER_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/compiler/*.cpp)
add_library(umbra_compile ${COMPILER_SOURCES})
target_link_libraries(umbra_compile
  PUBLIC
    umbra_parser
    umbra_codegen
    umbra_ast
    umbra_lexer
    umbra_error
    umbra_semantic)
target_include_directories(umbra_compile PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(umbra src/main.cpp)

target_include_directories(umbra PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(umbra
  PRIVATE
    umbra_compile
    ${Boost_LIBRARIES}
)

set_target_properties(umbra PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Tests
add_subdirectory(tests)
