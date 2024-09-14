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
    // TOK_ARRAY, wait for array definition

    // Keyword tokens for control structures
    TOK_IF,
    TOK_ELSE,
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
    TOK_DIFFERENT,  // 'different'
    TOK_LESS,       // 'less_than'
    TOK_GREATER,    // 'greater_than'
    TOK_LESS_EQ,    // 'less_or_equal'
    TOK_GREATER_EQ, // 'greater_or_equal'

    // Assignment token
    TOK_ASSIGN, // '='

    // Tokens for arithmetic operators
    TOK_MINUS, // '-' (subtraction)
    TOK_ADD,   // '+' (addition)
    TOK_MULT,  // '*' (multiplication)
    TOK_DIV,   // '/' (division)

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
    TOK_DOT

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
            {"different", TokenType::TOK_DIFFERENT},
            {"less_than", TokenType::TOK_LESS},
            {"greater_than", TokenType::TOK_GREATER},
            {"less_or_equal", TokenType::TOK_LESS_EQ},
            {"greater_or_equal", TokenType::TOK_GREATER_EQ}};
        return keywords;
    }

    // Convierte un TokenType a su cadena de representación
    static std::string tokenTypeToString(TokenType type) {
        static const std::unordered_map<TokenType, std::string> tokenTypeToStringMap = {
            {TokenType::TOK_EOF, "TOK_EOF"},
            {TokenType::TOK_NEWLINE, "TOK_NEWLINE"},
            {TokenType::TOK_INT, "TOK_INT"},
            {TokenType::TOK_FLOAT, "TOK_FLOAT"},
            {TokenType::TOK_BOOL, "TOK_BOOL"},
            {TokenType::TOK_CHAR, "TOK_CHAR"},
            {TokenType::TOK_STRING, "TOK_STRING"},
            {TokenType::TOK_IF, "TOK_IF"},
            {TokenType::TOK_ELSE, "TOK_ELSE"},
            {TokenType::TOK_REPEAT, "TOK_REPEAT"},
            {TokenType::TOK_TIMES, "TOK_TIMES"},
            {TokenType::TOK_FUNC, "TOK_FUNC"},
            {TokenType::TOK_RETURN, "TOK_RETURN"},
            {TokenType::TOK_NEW, "TOK_NEW"},
            {TokenType::TOK_DELETE, "TOK_DELETE"},
            {TokenType::TOK_AND, "TOK_AND"},
            {TokenType::TOK_OR, "TOK_OR"},
            {TokenType::TOK_EQUAL, "TOK_EQUAL"},
            {TokenType::TOK_DIFFERENT, "TOK_DIFFERENT"},
            {TokenType::TOK_LESS, "TOK_LESS"},
            {TokenType::TOK_GREATER, "TOK_GREATER"},
            {TokenType::TOK_LESS_EQ, "TOK_LESS_EQ"},
            {TokenType::TOK_GREATER_EQ, "TOK_GREATER_EQ"},
            {TokenType::TOK_ASSIGN, "TOK_ASSIGN"},
            {TokenType::TOK_MINUS, "TOK_MINUS"},
            {TokenType::TOK_ADD, "TOK_ADD"},
            {TokenType::TOK_MULT, "TOK_MULT"},
            {TokenType::TOK_DIV, "TOK_DIV"},
            {TokenType::TOK_ARROW, "TOK_ARROW"},
            {TokenType::TOK_IDENTIFIER, "TOK_IDENTIFIER"},
            {TokenType::TOK_NUMBER, "TOK_NUMBER"},
            {TokenType::TOK_STRING_LITERAL, "TOK_STRING_LITERAL"},
            {TokenType::TOK_CHAR_LITERAL, "TOK_CHAR_LITERAL"},
            {TokenType::TOK_LEFT_BRACE, "TOK_LEFT_BRACE"},
            {TokenType::TOK_RIGHT_BRACE, "TOK_RIGHT_BRACE"},
            {TokenType::TOK_LEFT_BRACKET, "TOK_LEFT_BRACKET"},
            {TokenType::TOK_RIGHT_BRACKET, "TOK_RIGHT_BRACKET"},
            {TokenType::TOK_LEFT_PAREN, "TOK_LEFT_PAREN"},
            {TokenType::TOK_RIGHT_PAREN, "TOK_RIGHT_PAREN"},
            {TokenType::TOK_COMMA, "TOK_COMMA"},
            {TokenType::TOK_DOT, "TOK_DOT"}};
        auto it = tokenTypeToStringMap.find(type);
        if (it != tokenTypeToStringMap.end()) {
            return it->second;
        }
        return "UNKNOWN_TOKEN";
    }
};

} // namespace umbra

#endif // TOKENS_H
