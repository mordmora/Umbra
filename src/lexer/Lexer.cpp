/**
 * @file Lexer.h
 * @brief Implementación del analizador léxico para el lenguaje Umbra
 */
#include "Lexer.h"
#include "Tokens.h"
#include <cctype>
#include <iostream>
#include <unordered_map>

/**
 * @namespace umbra
 * @brief Espacio de nombres principal para el compilador Umbra
 */
namespace umbra {

/**
 * @brief Constructor con manejo de errores interno
 * @param source Código fuente a analizar
 */
Lexer::Lexer(const std::string &source)
    : source(source), internalErrorManager(std::make_unique<ErrorManager>()),
      errorManager(internalErrorManager.get()), current(0), line(1), column(1) {}

/**
 * @brief Constructor con manejo de errores externo
 * @param source Código fuente a analizar
 * @param externalErrorManager Manejador de errores externo
 */
Lexer::Lexer(const std::string &source, ErrorManager &externalErrorManager)
    : source(source), errorManager(&externalErrorManager), current(0), line(1), column(1) {}

/**
 * @brief Obtiene el código fuente actual
 * @return Cadena con el código fuente
 */
std::string Lexer::getSource() { return source; }

/**
 * @brief Realiza el análisis léxico del código fuente
 * @return Vector de tokens generados
 */
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
        case '/':
            if (match('/')) {
                std::cout << "comment found" << std::endl;
                while (peek() != '\n' && !isAtEnd())
                    advance();
            } else {
                addToken(TokenType::TOK_DIV);
            }
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

/**
 * @brief Avanza al siguiente caracter en el código fuente
 * @return El caracter actual
 */
char Lexer::advance() {
    column++;
    return source[current++];
}

/**
 * @brief Verifica si se ha llegado al final del código fuente
 * @return true si se llegó al final, false en caso contrario
 */
bool Lexer::isAtEnd() const {
    return static_cast<std::string::size_type>(current) >= source.length();
}

/**
 * @brief Obtiene el caracter actual sin avanzar
 * @return El caracter actual
 */
char Lexer::peek() const {
    if (isAtEnd())
        return '\0';
    return source[current];
}

/**
 * @brief Obtiene el siguiente caracter sin avanzar
 * @return El siguiente caracter
 */
char Lexer::peekNext() const {
    if (static_cast<std::string::size_type>(current + 1) >= source.length())
        return '\0';
    return source[current + 1];
}

/**
 * @brief Verifica y consume un caracter esperado
 * @param expected Caracter esperado
 * @return true si coincide, false en caso contrario
 */
bool Lexer::match(char expected) {
    if (isAtEnd())
        return false;
    if (source[current] != expected)
        return false;
    current++;
    column++;
    return true;
}

/**
 * @brief Añade un nuevo token a la lista
 * @param type Tipo de token
 */
void Lexer::addToken(TokenType type) { addToken(type, source.substr(start, current - start)); }

/**
 * @brief Añade un nuevo token con lexema específico
 * @param type Tipo de token
 * @param lexeme Lexema del token
 */
void Lexer::addToken(TokenType type, const std::string &lexeme) {
    tokens.emplace_back(type, lexeme, line, column - lexeme.length());
}

/**
 * @brief Procesa un literal de caracter
 */
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

/**
 * @brief Procesa un literal de cadena
 */
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

/**
 * @brief Procesa un número
 */
void Lexer::number() {
    while (state != State::Acceptance) {

        switch (state) {
        case State::Integer:
            while (isDigit(peek()))
                advance();
            if (isWhitespace(peek()) || isSing(peek()) || peek() == ')' || isAtEnd()) {
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
                if (isWhitespace(peek()) || isSing(peek()) || peek() == ')' || isAtEnd()) {
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

            if (isWhitespace(peek()) || isSing(peek()) || peek() == ')' || isAtEnd()) {
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

/**
 * @brief Procesa un identificador o palabra reservada
 */
void Lexer::identifier() {
    while (isAlphaNumeric(peek()))
        advance();

    std::string text = source.substr(start, current - start);
    TokenType type = tokenManager.getKeywords().count(text) ? tokenManager.getKeywords().at(text)
                                                            : TokenType::TOK_IDENTIFIER;
    addToken(type, text);
}

/**
 * @brief Verifica si un caracter es alfabético
 * @param c Caracter a verificar
 * @return true si es alfabético, false en caso contrario
 */
bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/**
 * @brief Verifica si un caracter es alfanumérico
 * @param c Caracter a verificar
 * @return true si es alfanumérico, false en caso contrario
 */
bool Lexer::isAlphaNumeric(char c) const { return isAlpha(c) || isDigit(c); }

/**
 * @brief Verifica si un caracter es un dígito
 * @param c Caracter a verificar
 * @return true si es un dígito, false en caso contrario
 */
bool Lexer::isDigit(char c) const { return c >= '0' && c <= '9'; }

/**
 * @brief Verifica si un caracter es un signo de operación
 * @param c Caracter a verificar
 * @return true si es un signo, false en caso contrario
 */
bool Lexer::isSing(char c) const {
    return c == '+' || c == '-' || c == '/' || c == '*' || c == '<' || c == '>' || c == '=' ||
           c == '%' || c == '!';
}

/**
 * @brief Verifica si un caracter es espacio en blanco
 * @param c Caracter a verificar
 * @return true si es espacio en blanco, false en caso contrario
 */
bool Lexer::isWhitespace(char c) const { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

/**
 * @brief Verifica si un caracter es espacio en blanco (excluyendo nueva línea)
 * @param c Caracter a verificar
 * @return true si es espacio en blanco, false en caso contrario
 */
bool Lexer::isBlankSpace(char c) const { return c == ' ' || c == '\t' || c == '\r'; }

/**
 * @brief Reinicia el estado del lexer
 */
void Lexer::reset() {
    current = 0;
    start = 0;
    line = 1;
    column = 1;
}

/**
 * @brief Maneja un caracter inesperado
 * @param lastChar Caracter inesperado
 */

void Lexer::errorMessage(char lastChar) { // unexpected character error
    std::string errorMsg = "Unexpected character: " + std::string(1, lastChar);
    errorManager->addError(
        std::make_unique<CompilerError>(ErrorType::LEXICAL, errorMsg, line, column));
}

/**
 * @brief Procesa un número binario
 * @param c Primer caracter del número binario
 * @return true si se procesó correctamente, false en caso contrario
 */
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

/**
 * @brief Obtiene el siguiente token sin consumirlo
 * @return El siguiente token
 */
Lexer::Token Lexer::peekToken() { return tokens[current]; }

/**
 * @brief Obtiene y consume el siguiente token
 * @return El siguiente token
 */
Lexer::Token Lexer::getNextToken() {
    if (current < tokens.size()) {
        return tokens[current++];
    }
    return tokens.back();
}

} // namespace umbra
