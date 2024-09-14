#include "../src/lexer/Lexer.h"
#include "../src/lexer/Tokens.h"
#include <gtest/gtest.h>
#include <vector>

namespace umbra {

namespace umbra {

// Prueba de tokenización básica para un programa simple
TEST(LexerTest, TokenizeSimpleAssignment) {
    std::string source = "int x = 10";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 5); // Deben ser 5 tokens (int, identifier, =, number, EOF)

    EXPECT_EQ(tokens[0].type, TokenType::TOK_INT);        // El primer token debe ser 'int'
    EXPECT_EQ(tokens[1].type, TokenType::TOK_IDENTIFIER); // Luego debe venir un identificador
    EXPECT_EQ(tokens[1].lexeme, "x");                     // El identificador debe ser 'x'
    EXPECT_EQ(tokens[2].type, TokenType::TOK_ASSIGN);     // Luego el símbolo '='
    EXPECT_EQ(tokens[3].type, TokenType::TOK_NUMBER);     // Luego un número
    EXPECT_EQ(tokens[3].lexeme, "10");                    // El número debe ser '10'
    EXPECT_EQ(tokens[4].type, TokenType::TOK_EOF);        // Finalmente el token EOF
}

// Prueba de operadores aritméticos
TEST(LexerTest, TokenizeArithmeticOperations_ValidateAllTokens) {
    std::string source = "float a = 5.0 float b = 3.0 float result = a + b * 2";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    // El número total de tokens debería ser 17
    ASSERT_EQ(tokens.size(), 17); // 17 tokens en total

    // Validación completa de cada token
    EXPECT_EQ(tokens[0].type, TokenType::TOK_FLOAT); // Token 'float'
    EXPECT_EQ(tokens[0].lexeme, "float");
    EXPECT_EQ(tokens[1].type, TokenType::TOK_IDENTIFIER); // Token 'a'
    EXPECT_EQ(tokens[1].lexeme, "a");
    EXPECT_EQ(tokens[2].type, TokenType::TOK_ASSIGN); // Token '='
    EXPECT_EQ(tokens[2].lexeme, "=");
    EXPECT_EQ(tokens[3].type, TokenType::TOK_NUMBER); // Token '5.0'
    EXPECT_EQ(tokens[3].lexeme, "5.0");
    EXPECT_EQ(tokens[4].type, TokenType::TOK_FLOAT); // Token 'float'
    EXPECT_EQ(tokens[4].lexeme, "float");
    EXPECT_EQ(tokens[5].type, TokenType::TOK_IDENTIFIER); // Token 'b'
    EXPECT_EQ(tokens[5].lexeme, "b");
    EXPECT_EQ(tokens[6].type, TokenType::TOK_ASSIGN); // Token '='
    EXPECT_EQ(tokens[6].lexeme, "=");
    EXPECT_EQ(tokens[7].type, TokenType::TOK_NUMBER); // Token '3.0'
    EXPECT_EQ(tokens[7].lexeme, "3.0");
    EXPECT_EQ(tokens[8].type, TokenType::TOK_FLOAT); // Token 'float'
    EXPECT_EQ(tokens[8].lexeme, "float");
    EXPECT_EQ(tokens[9].type, TokenType::TOK_IDENTIFIER); // Token 'result'
    EXPECT_EQ(tokens[9].lexeme, "result");
    EXPECT_EQ(tokens[10].type, TokenType::TOK_ASSIGN); // Token '='
    EXPECT_EQ(tokens[10].lexeme, "=");
    EXPECT_EQ(tokens[11].type, TokenType::TOK_IDENTIFIER); // Token 'a'
    EXPECT_EQ(tokens[11].lexeme, "a");
    EXPECT_EQ(tokens[12].type, TokenType::TOK_ADD); // Token '+'
    EXPECT_EQ(tokens[12].lexeme, "+");
    EXPECT_EQ(tokens[13].type, TokenType::TOK_IDENTIFIER); // Token 'b'
    EXPECT_EQ(tokens[13].lexeme, "b");
    EXPECT_EQ(tokens[14].type, TokenType::TOK_MULT); // Token '*'
    EXPECT_EQ(tokens[14].lexeme, "*");
    EXPECT_EQ(tokens[15].type, TokenType::TOK_NUMBER); // Token '2'
    EXPECT_EQ(tokens[15].lexeme, "2");
    EXPECT_EQ(tokens[16].type, TokenType::TOK_EOF); // Token 'EOF'
}

// Prueba de control de flujo con if/else
TEST(LexerTest, TokenizeIfElseControlFlow) {
    std::string source = "bool flag = true if flag { int x = 1 } else { int x = 2 }";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 20); // 20 tokens en total

    EXPECT_EQ(tokens[0].type, TokenType::TOK_BOOL);       // 'bool'
    EXPECT_EQ(tokens[1].type, TokenType::TOK_IDENTIFIER); // Identificador 'flag'
    EXPECT_EQ(tokens[4].type, TokenType::TOK_IF);         // Palabra clave 'if'
    EXPECT_EQ(tokens[5].type, TokenType::TOK_IDENTIFIER); // Identificador 'flag'
    EXPECT_EQ(tokens[6].type, TokenType::TOK_LEFT_BRACE); // '{'
    EXPECT_EQ(tokens[7].type, TokenType::TOK_INT);        // 'int'
    EXPECT_EQ(tokens[12].type, TokenType::TOK_ELSE);      // Palabra clave 'else'
}

// Prueba de una función con retorno
TEST(LexerTest, TokenizeFunctionWithReturn) {
    std::string source = "func add(int a, int b) -> int { return a + b }";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 18); // 18 tokens en total

    EXPECT_EQ(tokens[0].type, TokenType::TOK_FUNC);       // 'func'
    EXPECT_EQ(tokens[1].type, TokenType::TOK_IDENTIFIER); // Nombre de la función 'add'
    EXPECT_EQ(tokens[9].type, TokenType::TOK_ARROW);      // Operador '->'
    EXPECT_EQ(tokens[12].type, TokenType::TOK_RETURN);    // Palabra clave 'return'
    EXPECT_EQ(tokens[14].type, TokenType::TOK_ADD);       // Operador '+'
}

// Prueba de un bucle repeat
TEST(LexerTest, TokenizeRepeatLoop) {
    std::string source = "int sum = 0 repeat 10 times { sum = sum + 1 }";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 15); // 16 tokens en total

    EXPECT_EQ(tokens[0].type, TokenType::TOK_INT);    // 'int'
    EXPECT_EQ(tokens[3].type, TokenType::TOK_NUMBER); // El número '0'
    EXPECT_EQ(tokens[4].type, TokenType::TOK_REPEAT); // 'repeat'
    EXPECT_EQ(tokens[6].type, TokenType::TOK_TIMES);  // 'times'
    EXPECT_EQ(tokens[11].type, TokenType::TOK_ADD);   // Operador '+'
}

// Prueba de comparación entre valores
TEST(LexerTest, TokenizeComparisonBetweenValues) {
    std::string source = "int a = 5 int b = 10 if a less_than b { return a } else { return b }";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 22); // 21 tokens en total

    EXPECT_EQ(tokens[10].type, TokenType::TOK_LESS);       // 'less_than'
    EXPECT_EQ(tokens[11].type, TokenType::TOK_IDENTIFIER); // Identificador 'b'
    EXPECT_EQ(tokens[13].type, TokenType::TOK_RETURN);     // Palabra clave 'return'
    EXPECT_EQ(tokens[18].type, TokenType::TOK_RETURN);     // Palabra clave 'return'
}

// Prueba de operadores lógicos
TEST(LexerTest, TokenizeLogicalOperators) {
    std::string source = "bool condition = (a greater_than b) and (b less_or_equal 10)";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 15); // 17 tokens en total

    EXPECT_EQ(tokens[0].type, TokenType::TOK_BOOL);     // 'bool'
    EXPECT_EQ(tokens[5].type, TokenType::TOK_GREATER);  // 'greater_than'
    EXPECT_EQ(tokens[8].type, TokenType::TOK_AND);      // 'and'
    EXPECT_EQ(tokens[11].type, TokenType::TOK_LESS_EQ); // 'less_or_equal'
}

// Prueba de operaciones sobre cadenas
TEST(LexerTest, TokenizeStringOperations) {
    std::string source = "string message = \"hello\" message = message + \" world\"";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 10); // 13 tokens en total

    EXPECT_EQ(tokens[0].type, TokenType::TOK_STRING);         // 'string'
    EXPECT_EQ(tokens[3].type, TokenType::TOK_STRING_LITERAL); // Literal "hello"
    EXPECT_EQ(tokens[3].lexeme, "hello");                     // Literal "hello"
    EXPECT_EQ(tokens[6].type, TokenType::TOK_IDENTIFIER);     // 'message'
    EXPECT_EQ(tokens[8].type, TokenType::TOK_STRING_LITERAL); // Literal " world"
    EXPECT_EQ(tokens[8].lexeme, " world");                    // Literal "hello"
}

// Prueba de una función con comparación y retorno
TEST(LexerTest, TokenizeFunctionWithComparison) {
    std::string source = "func compare(int a, int b) -> bool { return a equal b or a less_than b }";
    Lexer lexer(source);
    std::vector<Lexer::Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 22); // 21 tokens en total

    EXPECT_EQ(tokens[0].type, TokenType::TOK_FUNC);       // 'func'
    EXPECT_EQ(tokens[1].type, TokenType::TOK_IDENTIFIER); // Nombre de la función 'compare'
    EXPECT_EQ(tokens[12].type, TokenType::TOK_RETURN);    // 'return'
    EXPECT_EQ(tokens[14].type, TokenType::TOK_EQUAL);     // 'equal'
    EXPECT_EQ(tokens[16].type, TokenType::TOK_OR);        // 'or'
    EXPECT_EQ(tokens[18].type, TokenType::TOK_LESS);      // 'less_than'
}

} // namespace umbra

} // namespace umbra
