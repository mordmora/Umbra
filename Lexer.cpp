#include "Lexer.h"
#include "Tokens.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <fstream>
#include <string>

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
        lastChar = advance();
        if (isBlankSpace(lastChar)) {
            column++;
            continue;
        }
        if (lastChar == '\n') {
            if (!lastWasReturn) {
                addToken(TokenType::TOK_NEWLINE);
                lastWasReturn = true;
            }
            line++;
            column = 1;
            continue;
        }
        lastWasReturn = false;
        switch (lastChar) {
        case '\'':
            charliteral();
            break;
        case '"':
            string();
            break;
        case '=':
            if (match('=')) {
                addToken(TokenType::TOK_EQUAL);
            } else {
                addToken(TokenType::TOK_ASSIGN);
            }
            break;
        case '!':
            if (match('=')) {
                addToken(TokenType::TOK_DIFFERENT);
            } else {
                errorMessage(lastChar); // esto puede cambiar en un futuro para if(!true)
            }
            break;
        case '<':
            if (match('<')) {
                addToken(TokenType::TOK_LEFT_SHIFT);
            } else if (match('=')) {
                addToken(TokenType::TOK_LESS_EQ);
            } else {
                addToken(TokenType::TOK_LESS);
            }
            break;
        case '>':
            if (match('>')) {
                addToken(TokenType::TOK_RIGHT_SHIFT);
            } else if (match('=')) {
                addToken(TokenType::TOK_GREATER_EQ);
            } else {
                addToken(TokenType::TOK_GREATER);
            }
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
            if (isDigit(peek())) {
                state = State::Decimal;
                number();
            } else {
                addToken(TokenType::TOK_DOT);
            }
            break;
        default:
            if (isDigit(lastChar)) {
                state = State::Integer;
                number();
            } else if (isAlpha(lastChar)) {
                identifier();
            } else {
                errorMessage(lastChar);
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
    while (state != State::Acceptance) {

        switch (state) { // 5555 5555h 555.5 555. 555e 555e 55.555
        case State::Integer:
            while (isDigit(peek()))
                advance();
            if (isWhitespace(peek()) || isSing(peek())) {
                state = State::Acceptance;
            } else if (peek() == '.') {
                state = State::Decimal;
                advance();
            } else if (peek() == 'e' || peek() == 'E') {
                state = State::NotationNumber;
                advance();
            } else {
                state = State::Rejection;
            }
            break;
        case State::Decimal:
            if (!isDigit(peek())) {
                std::string errorMsg = "Malformed number at line " + std::to_string(line) +
                                       ", column " + std::to_string(column) +
                                       ": expected digits after decimal point.";
                errorManager->addError(
                    std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
                state = State::Acceptance; // como se encontro error especifico acepta muestra error
            } else {
                while (isDigit(peek()))
                    advance();
                if (isWhitespace(peek()) || isSing(peek())) {
                    state = State::Acceptance;
                } else if (peek() == '.') {
                    std::string errorMsg = "Malformed number at line " + std::to_string(line) +
                                           ", column " + std::to_string(column);
                    errorManager->addError(std::make_unique<CompilerError>(ErrorType::LEXICAL,
                                                                           errorMsg, line, column));
                    state = State::Acceptance;
                    advance();
                } else if (peek() == 'e' || peek() == 'E') {
                    state = State::NotationNumber;
                    advance();
                } else {
                    state = State::Rejection;
                }
            }

            break;
        case State::NotationNumber:
            if (peek() == '-') {
                advance();
            }
            if (!isDigit(peek())) {
                std::string errorMsg = "Malformed number at line " + std::to_string(line) +
                                       ", column " + std::to_string(column) +
                                       ": expected digits after - or E";
                errorManager->addError(
                    std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
                state = State::Acceptance;
            }
            while (isDigit(peek()))
                advance();

            if (isWhitespace(peek()) || isSing(peek())) {
                state = State::Acceptance;
            } else {
                state = State::Rejection;
            }

            break;

        default:
            if (state == State::Rejection) {
                state = State::Acceptance;
                std::string errorMsg = "Malformed number at line " + std::to_string(line) +
                                       ", column " + std::to_string(column);
                errorManager->addError(
                    std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
            }
            state = State::Acceptance;
            break;
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

bool Lexer::isSing(char c) const {
    return c == '+' || c == '-' || c == '/' || c == '*' || c == '<' || c == '>' || c == '=' ||
           c == '%' || c == '!';
}

bool Lexer::isWhitespace(char c) const { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

bool Lexer::isBlankSpace(char c) const { return c == ' ' || c == '\t' || c == '\r'; }

void Lexer::reset() {
    current = 0;
    start = 0;
    line = 1;
    column = 1;
}

void Lexer::errorMessage(char lastChar) { 
    std::string errorMsg = "Unexpected character: " + std::string(1, lastChar);
    errorManager->addError(
        std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
}



string decimalToBinary(int decimal) {
  string binary = "";
  while (decimal > 0) {
    binary = to_string(decimal % 2) + binary;
    decimal /= 2;
  }
  return binary;
}

int binaryToDecimal(string binary) {
  int decimal = 0;
  int power = 0;
  for (int i = binary.length() - 1; i >= 0; i--) {
    if (binary[i] == '1') {
      decimal += pow(2, power);
    }
    power++;
  }
  return decimal;
}

void removeBOM(std::ifstream& inFile, std::ofstream& outFile) {
    if (!inFile.is_open() || !outFile.is_open()) {
        std::cerr << "Error: uno o ambos archivos no están abiertos." << std::endl;
        return;
    }

    char bom[3] = {0};
    inFile.read(bom, 3);

    if (bom[0] == static_cast<char>(0xEF) && 
        bom[1] == static_cast<char>(0xBB) && 
        bom[2] == static_cast<char>(0xBF)) {
        std::string content((std::istreambuf_iterator<char>(inFile)), 
                            std::istreambuf_iterator<char>());
        outFile << content; 
    } else {
        outFile.seekp(0);
        outFile.write(bom, inFile.gcount());
        outFile << std::string((std::istreambuf_iterator<char>(inFile)), 
                                std::istreambuf_iterator<char>());
    }
}


}
