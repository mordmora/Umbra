/**
 * @file Lexer.h
 * @brief Implementación del analizador léxico para el lenguaje Umbra
 */
#include "umbra/lexer/Lexer.h"
#include "umbra/lexer/Tokens.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include "umbra/lexer/LookUpKeyword.h"

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
      errorManager(internalErrorManager.get()), current(0), line(1), column(1) {
        setupDispatch();
      }

/**
 * @brief Constructor con manejo de errores externo
 * @param source Código fuente a analizar
 * @param externalErrorManager Manejador de errores externo
 */
Lexer::Lexer(const std::string &source, ErrorManager &externalErrorManager)
    : source(source), errorManager(&externalErrorManager), current(0), line(1), column(1) {
        setupDispatch();
    }

/**
 * @brief Obtiene el código fuente actual
 * @return Cadena con el código fuente
 */
const std::string& Lexer::getSource() const { return source; }

/**
 * @brief Inicializa la tabla de despachadores
 *
 * Esta función asigna funciones de manejo a cada carácter en la tabla de despachadores.
 * Cada función maneja un tipo específico de token o símbolo.
 *
 * Ejemplo de asignación:
 * dispatchTable['+'] = &Lexer::handlePlus;
 * dispatchTable['-'] = &Lexer::handleMinus;
 */

void Lexer::setupDispatch(){
    dispatchTable[(unsigned char)'+'] = &Lexer::handlePlus;
    dispatchTable[(unsigned char)'-'] = &Lexer::handleMinus;
    dispatchTable[(unsigned char)'*'] = &Lexer::handleMultiply;
    dispatchTable[(unsigned char)'/'] = &Lexer::handleDivide;
    dispatchTable[(unsigned char)'='] = &Lexer::handleEqual;
    dispatchTable[(unsigned char)'('] = &Lexer::handleLeftParen;
    dispatchTable[(unsigned char)')'] = &Lexer::handleRightParen;
    dispatchTable[(unsigned char)'{'] = &Lexer::handleLeftBrace;
    dispatchTable[(unsigned char)'}'] = &Lexer::handleRightBrace;
    dispatchTable[(unsigned char)'['] = &Lexer::handleLeftBracket;
    dispatchTable[(unsigned char)']'] = &Lexer::handleRightBracket;
    dispatchTable[(unsigned char)','] = &Lexer::handleComma;
    dispatchTable[(unsigned char)'.'] = &Lexer::handleDot;
    dispatchTable[(unsigned char)'"'] = &Lexer::handleDoubleQuote;
    dispatchTable[(unsigned char)'\''] = &Lexer::handleSingleQuote;
    dispatchTable[(unsigned char)':'] = &Lexer::handleColon;

}

/**
 * @brief Maneja el token de suma o incremento
 *
 * Esta función verifica si el carácter actual es un signo de suma
 * y determina si es un operador de incremento o una suma normal.
 *
 */

void Lexer::handlePlus() {
    if (match('+')) {
        addToken(TokenType::TOK_INCREMENT);
    } else {
        addToken(TokenType::TOK_ADD);
    }
}

/**
 * @brief Maneja el token de resta o decremento
 *
 * Esta función verifica si el carácter actual es un signo de resta
 * y determina si es un operador de decremento, un operador de flecha o una resta normal.
 */

void Lexer::handleMinus() {
    if (match('-')) {
        addToken(TokenType::TOK_DECREMENT);
    } else if ( match('>')) {
        addToken(TokenType::TOK_ARROW);
    } else{
        addToken(TokenType::TOK_MINUS);
    }
}

/**
 * @brief Maneja el token de división
 *
 * Esta función verifica si el carácter actual es un signo de división
 * y determina si es un comentario o una división normal.
 */

void Lexer::handleDivide() {
    if (match('/')) {
        while (peek() != '\n' && !isAtEnd())
            advance();
    } else {
        addToken(TokenType::TOK_DIV);
    }
}

/**
 * @brief Maneja el token de igualdad o asignación
 *
 * Esta función verifica si el carácter actual es un signo de igualdad
 * y determina si es un operador de igualdad o una asignación.
 */

void Lexer::handleEqual() {
    if (match('=')) {
        addToken(TokenType::TOK_EQUAL);
    } else {
        addToken(TokenType::TOK_ASSIGN);
    }
}
/**
 * @brief Maneja el token de paréntesis izquierdo
 *
 * Esta función maneja el token de paréntesis izquierdo.
 */
void Lexer::handleLeftParen() {
    addToken(TokenType::TOK_LEFT_PAREN);
}
/**
 * @brief Maneja el token de paréntesis derecho
 *
 * Esta función maneja el token de paréntesis derecho.
 */
void Lexer::handleRightParen() {
    addToken(TokenType::TOK_RIGHT_PAREN);
}
/**
 * @brief Maneja el token de llave izquierda
 *
 * Esta función maneja el token de llave izquierda.
 */
void Lexer::handleLeftBrace() {
    addToken(TokenType::TOK_LEFT_BRACE);
}
/**
 * @brief Maneja el token de llave derecha
 *
 * Esta función maneja el token de llave derecha.
 */
void Lexer::handleRightBrace() {
    addToken(TokenType::TOK_RIGHT_BRACE);
}
/**
 * @brief Maneja el token de corchete izquierdo
 *
 * Esta función maneja el token de corchete izquierdo.
 */
void Lexer::handleLeftBracket() {
    addToken(TokenType::TOK_LEFT_BRACKET);
}
/**
 * @brief Maneja el token de corchete derecho
 *
 * Esta función maneja el token de corchete derecho.
 */
void Lexer::handleRightBracket() {
    addToken(TokenType::TOK_RIGHT_BRACKET);
}
/**
 * @brief Maneja el token de coma
 *
 * Esta función maneja el token de coma.
 */
void Lexer::handleComma() {
    addToken(TokenType::TOK_COMMA);
}
/**
 * @brief Maneja el token de punto
 *
 * Esta función verifica si el carácter actual es un punto seguido de un dígito
 * y determina si es un número decimal o un token de punto.
 */
void Lexer::handleDot() {
    if (std::isdigit(peek())) {
        state = State::Decimal;
        number();
    } else {
        addToken(TokenType::TOK_DOT);
    }
}
/**
 * @brief Maneja el token de comillas dobles
 *
 * Esta función maneja el token de comillas dobles.
 */
void Lexer::handleDoubleQuote() {
    string();
}
/**
 * @brief Maneja el token de comillas simples
 *
 * Esta función maneja el token de comillas simples.
 */
void Lexer::handleSingleQuote() {
    charliteral();
}
/**
 * @brief Maneja el token de dos puntos
 *
 * Esta función maneja el token de dos puntos.
 */
void Lexer::handleColon() {
    addToken(TokenType::TOK_COLON);
}

/**
 * @brief Maneja el token de multiplicación
 *
 * Esta función maneja el token de multiplicación.
 */
void Lexer::handleMultiply() {
    addToken(TokenType::TOK_MULT);
}

/**
 * @brief Maneja tokens por defecto
 * @param c Carácter actual
 *
 * Esta función maneja tokens que no son operadores ni delimitadores.
 * Se encarga de identificar literales, números y caracteres no válidos.
 *
 */

void Lexer::handleDefault(char c) {

    if (c == '0') { // Caso especial para prefijos como 0b, 0x
        if (peek() == 'b' || peek() == 'B') {

            isBinary(); // Renombrar o reestructurar isBinary
        } else if (peek() == 'x' || peek() == 'X') {
            // handleHex(); // Necesitarías una función similar
            number(); // O deja que number() maneje esto si lo modificas
        } else {
            number(); // Número octal (tradicionalmente) o solo '0'
        }
    } else if (std::isalpha(c) || c == '_') { // isAlpha ya incluye '_' en tu implementación
        identifier();
    } else if (std::isdigit(c)) {
        number();
    } else {
        std::string desc = std::isprint(c) ? std::string(1, c) : "<non-printable>";
        reportLexicalError("Unexpected character: '" + desc + "'");
    }
}


/**
 * @brief Realiza el análisis léxico del código fuente
 * @return Vector de tokens generados
 */
std::vector<Lexer::Token> Lexer::tokenize() {
    tokens.clear(); // Asegurarse de que esté limpio si se llama varias veces
    current = 0;
    line = 1;
    column = 1;
    start = 0;

    while (!isAtEnd()) {
        start = current;
        char c = advance();

        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                line++;
                column = 1;
                if (tokens.empty() || tokens.back().type != TokenType::TOK_NEWLINE) {
                    addToken(TokenType::TOK_NEWLINE);
                }

                break;
            default:
                if (dispatchTable[static_cast<unsigned char>(c)]) {
                    (this->*dispatchTable[static_cast<unsigned char>(c)])();
                } else {
                    handleDefault(c); // 'c' ya fue consumido por advance()
                }
                break;
        }
    }
    addToken(TokenType::TOK_EOF);
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
void Lexer::addToken(TokenType type) {
    addToken(type,&source[start], current - start);
}

/**
 * @brief Añade un nuevo token a la lista con un lexema específico
 * @param type Tipo de token
 * @param lexeme Lexema del token
 * @param line Línea donde se encontró el token
 * @param column Columna donde se encontró el token
 */

void Lexer::addToken(TokenType type, const char* lexeme, size_t length) {
    tokens.emplace_back(type, lexeme, length, line, column - length);
}

/**
 * @brief Añade un nuevo token con lexema específico
 * @param type Tipo de token
 * @param lexeme Lexema del token
 */


/**
 * @brief Procesa un literal de caracter
 */
void Lexer::charliteral() {
    if (isAtEnd()) {
        reportLexicalError("Unterminated character literal.");
        return;
    }

    char c = advance();
    char value;

    if (c == '\\') {
        if (isAtEnd()) {
            reportLexicalError("Unterminated escape sequence in character literal.");
            return;
        }

        char esc = advance();
        switch (esc) {
            case 'n': value = '\n'; break;
            case 't': value = '\t'; break;
            case 'r': value = '\r'; break;
            case '\\': value = '\\'; break;
            case '\'': value = '\''; break;
            default:
                reportLexicalError(std::string("Invalid escape character in character literal: \\") + esc);
                return;
        }
    } else {
        value = c;
    }

    if (isAtEnd() || advance() != '\'') {
        reportLexicalError("Unterminated character literal. Expected closing '\''.");
        return;
    }

    std::string result(1, value);
    addToken(TokenType::TOK_CHAR_LITERAL, result.c_str(), 1);
}


/**
 * @brief Procesa un literal de cadena
 */
void Lexer::string() {

    std::string value;
    bool unterminated = true;

    while (!isAtEnd()) {
        char c = advance();

        if (c == '"') {
            unterminated = false;
            break;
        }

        if (c == '\\') {
            if (isAtEnd()) break;
            char next = advance();

            switch (next) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default:
                    reportLexicalError("Invalid escape sequence in string literal");
                    value += next; // opcional: incluirlo de todos modos
                    break;
            }
        } else {
            value += c;
        }
    }

    if (unterminated) {

        reportLexicalError("Unterminated string literal. Did you forget a closing '\"'?");
        return;
    }
    addToken(TokenType::TOK_STRING_LITERAL, value.c_str(), value.length());
}

/**
 * @brief Procesa un número
 */

 void Lexer::number() {
    bool hasDot = false;
    bool hasExp = false;

    while (std::isdigit(peek()))
        advance();

    if (peek() == '.' && std::isdigit(peekNext())) {
        hasDot = true;
        advance(); // consume '.'
        while (std::isdigit(peek()))
            advance();
    }

    if (peek() == 'e' || peek() == 'E') {
        char next = peekNext();
        if (std::isdigit(next) || next == '+' || next == '-') {
            hasExp = true;
            advance(); // consume 'e'
            if (peek() == '+' || peek() == '-') advance();
            if (!std::isdigit(peek())) {
                reportLexicalError("Malformed number literal: expected digit after exponent.");
                return;
            }
            while (std::isdigit(peek()))
                advance();
        }
    }

    size_t len = current - start;
    addToken(TokenType::TOK_NUMBER, &source[start], len);
}


/**
 * @brief Procesa un identificador o palabra reservada
 */
void Lexer::identifier() {
    while (std::isalnum(peek()) || peek() == '_')
        advance();

    const char* identStart = &source[start];
    size_t identLen = current - start;

    TokenType type = lookupKeyword(identStart, identLen);
    addToken(type, identStart, identLen);
}


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
    tokenIndex = 0;
    state = State::Start;
    tokens.clear();
    // internalErrorManager->clear();
}


/**
 * @brief Procesa un número binario
 * @param c Primer caracter del número binario
 * @return true si se procesó correctamente, false en caso contrario
 */
bool Lexer::isBinary() {
    char c;
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
        reportLexicalError("Malformed binary number");
        return false;
    }else if(binaryValue.empty()) {
        reportLexicalError("Malformed binary number");
        return false;
    }
    addToken(TokenType::TOK_BINARY, binaryValue.c_str(), binaryValue.length());
    return true;
}

/**
 * @brief Obtiene el siguiente token sin consumirlo
 * @return El siguiente token
 */
Lexer::Token Lexer::peekToken() {
    if (tokenIndex < tokens.size()) {
        return tokens[tokenIndex];
    }

    if (!tokens.empty() && tokens.back().type == TokenType::TOK_EOF) {
        return tokens.back();
    }

}

/**
 * @brief Obtiene y consume el siguiente token
 * @return El siguiente token
 */
Lexer::Token Lexer::getNextToken() {
    if (tokenIndex < tokens.size()) {

        if (tokens[tokenIndex].type != TokenType::TOK_EOF) {
             return tokens[tokenIndex++];
        }
        return tokens[tokenIndex];
    }
}

/**
 * @brief Obtiene el contenido de una línea específica
 * @param lineNumber Número de línea
 * @return Contenido de la línea
 */

std::string Lexer::getLineContent(int lineNumber) const {
    std::istringstream iss(source);
    std::string line;
    int currentLine = 1;
    while (std::getline(iss, line)) {
        if (currentLine == lineNumber) {
            return line;
        }
        ++currentLine;
    }
    return "";
}

/**
 * @brief Genera un mensaje de error léxico
 * @param msg Mensaje de error
 * @param offset Desplazamiento para el subrayado
 */

void Lexer::reportLexicalError(const std::string& msg, int offset) {

    int tokenStartColumn = column - (current - start);
    int errorColumn = tokenStartColumn + offset;

    std::string lineContent = getLineContent(line);

    std::string underline(errorColumn > 0 ? errorColumn - 1 : 0, ' ');
    underline += "^";

    std::string fullMessage = "\n" + lineContent + "\n" +
                              underline + "\n" +
                              msg;

    errorManager->addError(std::make_unique<CompilerError>(
        ErrorType::LEXICAL,
        fullMessage,
        line,
        errorColumn
    ));
}


} // namespace umbra
