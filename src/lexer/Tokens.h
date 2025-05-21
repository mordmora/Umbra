// Tokens.h
#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <unordered_map>

namespace umbra {

    

enum class TokenType {
    // End of file token
    TOK_EOF,
    TOK_INVALID,
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
    TOK_INCREMENT, // '++'
    TOK_DECREMENT, // '--'
    TOK_COLON,

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



} // namespace umbra

#endif // TOKENS_H
