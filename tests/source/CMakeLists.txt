add_executable(source_tests source_manager_test.cpp)
target_link_libraries(source_tests PRIVATE umbra_lexer gtest gtest_main)
add_test(NAME source_tests COMMAND source_tests)
