/**
 * @file Lexer.h
 * @brief Implementación del analizador léxico para el lenguaje Umbra
 */
#include "Lexer.h"
#include "Tokens.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include "LookUpKeyword.h"
#include "charTables.h"

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
std::string Lexer::getSource() { return source; }

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
    if (IS_DIGIT(peek())) {
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

    if (IS_ALPHA(c)) {

        identifier(); 
    } else if (IS_DIGIT(c)) {
        number(); 
    } else if (c == '"') {
        string(); 
    } else if (c == '\'') {
        charliteral(); 
    } else {

        std::string desc = isprint(c) ? std::string(1, c) : "<non-printable>";
        reportLexicalError("Unexpected character: '" + desc + "'");
    }
}


/**
 * @brief Realiza el análisis léxico del código fuente
 * @return Vector de tokens generados
 */
std::vector<Lexer::Token> Lexer::tokenize() {
    tokens.clear();

    bool lastWasReturn = false;

    while (!isAtEnd()) {
        start = current;
        lastChar = advance();
        if (IS_WHITESPACE(lastChar)) {
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

        void (Lexer::*handler)() = dispatchTable[(unsigned char)lastChar];
        if(handler){
            (this->*handler)();
        }else{
            handleDefault(lastChar);
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
 * @brief Verifica si un caracter es un operador
 * @param currentChar Caracter a verificar
 * @return true si es un operador, false en caso contrario
 */

bool Lexer::matchOperatorFromTable(const char currentChar) {
    for (const auto& rule : operatorRules) {
        if (rule.first == currentChar) {
            if (match(rule.second)) {
                addToken(rule.combined, &source[start], 2);
                return true;
            } else if (rule.single != TokenType::TOK_INVALID) {
                addToken(rule.single, &source[start], 1);
                return true;
            }
        }
    }
    return false;
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

        if (peek() == '\'' && IS_ALPHA(c)) {
            reportLexicalError("Character literal may contain only one character.");
            return;
        }
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
    std::cout << "Processing string literal" << std::endl;

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
        std::cout << "Reporting error" << std::endl;
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

    while (IS_DIGIT(peek()))
        advance();

    if (peek() == '.' && IS_DIGIT(peekNext())) {
        hasDot = true;
        advance(); // consume '.'
        while (IS_DIGIT(peek()))
            advance();
    }

    if (peek() == 'e' || peek() == 'E') {
        char next = peekNext();
        if (IS_DIGIT(next) || next == '+' || next == '-') {
            hasExp = true;
            advance(); // consume 'e'
            if (peek() == '+' || peek() == '-') advance();
            if (!IS_DIGIT(peek())) {
                reportLexicalError("Malformed number literal: expected digit after exponent.");
                return;
            }
            while (IS_DIGIT(peek()))
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
    while (IS_ALNUM(peek()))
        advance();

    const char* identStart = &source[start];
    size_t identLen = current - start;

    TokenType type = lookupKeyword(identStart, identLen);
    addToken(type, identStart, identLen);
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
Lexer::Token Lexer::peekToken() { return tokens[current]; }

/**
 * @brief Obtiene y consume el siguiente token
 * @return El siguiente token
 */
Lexer::Token Lexer::getNextToken() {
    if (static_cast<std::size_t>(current) < tokens.size()) {
        return tokens[current++];
    }
    return tokens.back();
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

    std::string underline(errorColumn + 2, ' ');
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
