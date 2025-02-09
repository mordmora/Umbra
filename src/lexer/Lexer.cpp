#include "Lexer.h"
#include "Tokens.h"
#include <bitset>
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


std::string Lexer::getSource() { return source; }

std::vector<Lexer::Token> Lexer::tokenize() {
    tokens.clear();
    std::cout << "Lexer: Comenzando tokenización..." << std::endl;
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
        if (lastChar == '0' && peek() == 'b') {
            if(isBinary(lastChar))
                continue;
        }
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
    std::cout << "Lexer: Tokenización completada. Tokens generados: " << tokens.size() << std::endl;
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

void Lexer::errorMessage(char lastChar) { // unexpected character error
    std::string errorMsg = "Unexpected character: " + std::string(1, lastChar);
    errorManager->addError(
        std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
}
bool Lexer::isBinary(char c) {
    // int binario = 0b010010
    c = advance();
    if(c != 'b') {
        return false;
    }
    c = advance();
    std::string binaryValue = "";
    while (c == '0' || c == '1') {
        binaryValue += c;
        advance();
    }
    if (!isWhitespace(c)) {
        std::string errorMsg = "Malformed binary number at line: " + std::to_string(line) +
                               ", column: " + std::to_string(column);
        errorManager->addError(
            std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
        return false;
    }else if(binaryValue.empty()) {
        std::string errorMsg = "Empty binary number at line: " + std::to_string(line) +
                               ", column: " + std::to_string(column);
        errorManager->addError(
            std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
        return false;
    }
    addToken(TokenType::TOK_BINARY, binaryValue);
    return true;
}

Lexer::Token Lexer::peekToken() { return tokens[current]; }

Lexer::Token Lexer::getNextToken() {
    if (current < tokens.size()) {
        return tokens[current++];
    }
    return tokens.back();
}

} // namespace umbra