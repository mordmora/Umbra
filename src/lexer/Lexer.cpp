#include "Lexer.h"
#include "Tokens.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace umbra {

Lexer::Lexer(const std::string &source)
    : source(source), internalErrorManager(std::make_unique<ErrorManager>()),
      errorManager(internalErrorManager.get()), current(0), line(1), column(1) {}

Lexer::Lexer(const std::string &source, ErrorManager &externalErrorManager)
    : source(source), errorManager(&externalErrorManager), current(0), line(1), column(1) {}

std::vector<Lexer::Token> Lexer::tokenize() {
    tokens.clear();
    bool lastWasReturn = false;

    while (!isAtEnd()) {
        start = current;
        char c = advance();

        if (c == '\n') {
            if (!lastWasReturn) {
                addToken(TokenType::TOK_NEWLINE);
                lastWasReturn = true;
            }
            line++;
            column = 1;
            continue;
        }
        lastWasReturn = false;
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            // Ignorar espacios en blanco
            break;
        case '\'':
            charliteral();
            break;
        case '"':
            string();
            break;
        case '=':
            addToken(TokenType::TOK_ASSIGN);
            break;
        case '-':
            if (match('>'))
                addToken(TokenType::TOK_ARROW);
            else
                addToken(TokenType::TOK_MINUS); // Usar como operador de resta
            break;
        case '+':
            addToken(TokenType::TOK_ADD);
            break;
        case '*':
            addToken(TokenType::TOK_MULT);
            break;
        case '(':
            addToken(TokenType::TOK_LEFT_PAREN);
            break;
        case ')':
            addToken(TokenType::TOK_RIGHT_PAREN);
            break;
        case '{':
            addToken(TokenType::TOK_LEFT_BRACE);
            break;
        case '}':
            addToken(TokenType::TOK_RIGHT_BRACE);
            break;
        case '[':
            addToken(TokenType::TOK_LEFT_BRACKET);
            break;
        case ']':
            addToken(TokenType::TOK_RIGHT_BRACKET);
            break;
        case ',':
            addToken(TokenType::TOK_COMMA);
            break;
        case '.':
            addToken(TokenType::TOK_DOT);
            break;
        default:
            if (isDigit(c)) {
                number();
            } else if (isAlpha(c)) {
                identifier();
            } else {
                std::string errorMsg = "Unexpected character: " + std::string(1, c);
                errorManager->addError(
                    std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
            }
            break;
        }
    }
    tokens.emplace_back(TokenType::TOK_EOF, "", line, column);
    return tokens;
}

char Lexer::advance() {
    column++;
    return source[current++];
}

bool Lexer::isAtEnd() const {
    return static_cast<std::string::size_type>(current) >= source.length();
}

char Lexer::peek() const {
    if (isAtEnd())
        return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (static_cast<std::string::size_type>(current + 1) >= source.length())
        return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd())
        return false;
    if (source[current] != expected)
        return false;
    current++;
    column++;
    return true;
}

void Lexer::addToken(TokenType type) { addToken(type, source.substr(start, current - start)); }

void Lexer::addToken(TokenType type, const std::string &lexeme) {
    tokens.emplace_back(type, lexeme, line, column - lexeme.length());
}

void Lexer::charliteral() {
    while (peek() != '\'' && !isAtEnd()) {
        advance();
    }
    if (isAtEnd()) {
        std::string errorMsg = "Unterminated char starting at line " + std::to_string(line) +
                               ", column " + std::to_string(column);
        errorManager->addError(
            std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
        return;
    }
    // get closing \'
    advance();
    std::string value = source.substr(start + 1, current - start - 2); // clear \""
    std::cout << "----->" << value << "<--";

    addToken(TokenType::TOK_CHAR_LITERAL, value);
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isAtEnd()) {
        std::string errorMsg = "Unterminated string starting at line " + std::to_string(line) +
                               ", column " + std::to_string(column);
        errorManager->addError(
            std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
        return;
    }

    // get the closing ".
    advance();

    std::string value = source.substr(start + 1, current - start - 2); // clear \""
    addToken(TokenType::TOK_STRING_LITERAL, value);
}

void Lexer::number() {

    while (isDigit(peek()))
        advance();

    if (peek() == '.') {
        if (!isDigit(peekNext())) {
            std::string errorMsg = "Malformed number at line " + std::to_string(line) +
                                   ", column " + std::to_string(column) +
                                   ": expected digits after decimal point.";
            errorManager->addError(
                std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
            advance();
        } else {
            advance();
            while (isDigit(peek()))
                advance();
        }
    }
    addToken(TokenType::TOK_NUMBER, source.substr(start, current - start));
}

void Lexer::identifier() {
    while (isAlphaNumeric(peek()))
        advance();

    std::string text = source.substr(start, current - start);
    TokenType type = tokenManager.getKeywords().count(text) ? tokenManager.getKeywords().at(text)
                                                            : TokenType::TOK_IDENTIFIER;
    addToken(type, text);
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) const { return isAlpha(c) || isDigit(c); }

bool Lexer::isDigit(char c) const { return c >= '0' && c <= '9'; }

bool Lexer::isWhitespace(char c) const { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

void Lexer::reset() {
    current = 0;
    start = 0;
    line = 1;
    column = 1;
}

} // namespace umbra