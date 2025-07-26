include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.14.0.zip
)
# Pruebas unitarias para lógica del Lexer
add_subdirectory(lexer)
