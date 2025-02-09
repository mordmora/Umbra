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