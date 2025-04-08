// Tokens.h
#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <unordered_map>

namespace umbra {

enum class TokenType {
    // End of file token
    TOK_EOF,
    TOK_NEWLINE,

    // Keyword tokens for data types
    TOK_INT,
    TOK_FLOAT,
    TOK_BOOL,
    TOK_CHAR,
    TOK_STRING,
    TOK_BINARY,
    TOK_ARRAY,
    TOK_VOID,
    // TOK_ARRAY, wait for array definition

    // Keyword tokens for control structures
    TOK_IF,
    TOK_ELSE,
    TOK_ELSEIF,
    TOK_REPEAT,
    TOK_TIMES,

    // Keyword tokens for functions and return
    TOK_FUNC,
    TOK_RETURN,

    // Tokens for memory management
    TOK_NEW,
    TOK_DELETE,

    // Tokens for logical operators
    TOK_AND, // 'and'
    TOK_OR,  // 'or'

    // Tokens for comparison operators
    TOK_EQUAL,      // 'equal'
    TOK_NOT,        // 'not'
    TOK_DIFFERENT,  // 'different'
    TOK_LESS,       // 'less_than'
    TOK_GREATER,    // 'greater_than'
    TOK_LESS_EQ,    // 'less_or_equal'
    TOK_GREATER_EQ, // 'greater_or_equal'
    TOK_TRUE,
    TOK_FALSE,

    // Assignment token
    TOK_ASSIGN, // '='

    // Tokens for arithmetic operators
    TOK_MINUS, // '-' (subtraction)
    TOK_ADD,   // '+' (addition)
    TOK_MULT,  // '*' (multiplication)
    TOK_DIV,   // '/' (division)
    TOK_MOD,   // '%' (modulo)

    // Token for memory managment
    TOK_PTR,
    TOK_ACCESS,
    TOK_REF,

    // Token for arrow operator (function definition)
    TOK_ARROW, // '->'

    // Tokens for identifiers and literals
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING_LITERAL,
    TOK_CHAR_LITERAL,

    // Symbols
    TOK_LEFT_BRACE,  // '{'
    TOK_RIGHT_BRACE, // '}'
    TOK_LEFT_BRACKET,
    TOK_RIGHT_BRACKET,
    TOK_LEFT_PAREN,
    TOK_RIGHT_PAREN,
    TOK_COMMA,
    TOK_DOT,

    //Bit shift operators
    TOK_LEFT_SHIFT,  //<<
    TOK_RIGHT_SHIFT  //>>

};

class TokenManager {
  public:
    // Devuelve el mapa de palabras clave
    const std::unordered_map<std::string, TokenType> &getKeywords() const {
        static const std::unordered_map<std::string, TokenType> keywords = {
            {"int", TokenType::TOK_INT},
            {"float", TokenType::TOK_FLOAT},
            {"bool", TokenType::TOK_BOOL},
            {"char", TokenType::TOK_CHAR},
            {"string", TokenType::TOK_STRING},
            {"array", TokenType::TOK_ARRAY},
            {"void", TokenType::TOK_VOID},
            {"if", TokenType::TOK_IF},
            {"else", TokenType::TOK_ELSE},
            {"repeat", TokenType::TOK_REPEAT},
            {"times", TokenType::TOK_TIMES},
            {"func", TokenType::TOK_FUNC},
            {"return", TokenType::TOK_RETURN},
            {"new", TokenType::TOK_NEW},
            {"delete", TokenType::TOK_DELETE},
            {"and", TokenType::TOK_AND},
            {"or", TokenType::TOK_OR},
            {"equal", TokenType::TOK_EQUAL},
        {"not", TokenType::TOK_NOT},
        {"true", TokenType::TOK_TRUE},
        {"false", TokenType::TOK_FALSE},
            {"different", TokenType::TOK_DIFFERENT},
            {"less_than", TokenType::TOK_LESS},
            {"greater_than", TokenType::TOK_GREATER},
            {"less_or_equal", TokenType::TOK_LESS_EQ},
            {"greater_or_equal", TokenType::TOK_GREATER_EQ}};
        return keywords;
    }

};

} // namespace umbra

#endif // TOKENS_H
