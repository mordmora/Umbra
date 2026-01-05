/**
 * @file Lexer.cpp
 * @brief Implementación del analizador léxico para el lenguaje Umbra
 * @author Umbra Team
 * @version 2.0
 * 
 * @details Analizador léxico optimizado que convierte código fuente en tokens.
 * Utiliza tabla de despacho para procesamiento O(1) de caracteres y
 * tablas de clasificación precalculadas para máximo rendimiento.
 */

#include "umbra/lexer/Lexer.h"
#include "umbra/lexer/Tokens.h"
#include "umbra/lexer/LookUpKeyword.h"
#include <cstring>

namespace umbra {

namespace {
    
/**
 * @brief Tabla de clasificación de caracteres precalculada
 * @details Bits: 0=dígito, 1=alfa, 2=alfanum, 3=whitespace, 4=operador
 */
constexpr uint8_t CHAR_DIGIT    = 0x01;
constexpr uint8_t CHAR_ALPHA    = 0x02;
constexpr uint8_t CHAR_ALNUM    = 0x04;
constexpr uint8_t CHAR_WSPACE   = 0x08;
constexpr uint8_t CHAR_OPERATOR = 0x10;

constexpr uint8_t buildCharClass(unsigned char c) {
    uint8_t r = 0;
    if (c >= '0' && c <= '9') r |= CHAR_DIGIT | CHAR_ALNUM;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') 
        r |= CHAR_ALPHA | CHAR_ALNUM;
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') r |= CHAR_WSPACE;
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || 
        c == '<' || c == '>' || c == '%' || c == '!') r |= CHAR_OPERATOR;
    return r;
}

constexpr auto buildCharTable() {
    struct { uint8_t data[256]; } t{};
    for (unsigned i = 0; i < 256; ++i) t.data[i] = buildCharClass(i);
    return t;
}

constexpr auto CHAR_TABLE = buildCharTable();

/// @brief Verifica si es dígito usando tabla
inline bool isDigit(unsigned char c) { return CHAR_TABLE.data[c] & CHAR_DIGIT; }

/// @brief Verifica si es alfabético usando tabla
inline bool isAlpha(unsigned char c) { return CHAR_TABLE.data[c] & CHAR_ALPHA; }

/// @brief Verifica si es alfanumérico usando tabla
inline bool isAlnum(unsigned char c) { return CHAR_TABLE.data[c] & CHAR_ALNUM; }

/// @brief Tabla de escape de caracteres (inicialización en tiempo de compilación)
constexpr char getEscapeChar(char c) {
    switch(c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '"';
        case '0': return '\0';
        default: return 0;
    }
}

} // namespace anónimo

//==============================================================================
// Constructores
//==============================================================================

/**
 * @brief Constructor con gestor de errores interno
 * @param source Código fuente a tokenizar
 */
Lexer::Lexer(const std::string &source)
    : source(source), 
      internalErrorManager(std::make_unique<ErrorManager>()),
      errorManager(internalErrorManager.get()) {
    setupDispatch();
}

/**
 * @brief Constructor con gestor de errores externo
 * @param source Código fuente a tokenizar
 * @param externalErrorManager Referencia al gestor externo
 */
Lexer::Lexer(const std::string &source, ErrorManager &externalErrorManager)
    : source(source), 
      errorManager(&externalErrorManager) {
    setupDispatch();
}

//==============================================================================
// Tabla de Despacho
//==============================================================================

/**
 * @brief Inicializa la tabla de despacho de caracteres
 * @details Asigna manejadores específicos para operadores y delimitadores.
 * Los caracteres no asignados usan el manejador por defecto.
 */
void Lexer::setupDispatch() {
    dispatchTable['+'] = &Lexer::handlePlus;
    dispatchTable['-'] = &Lexer::handleMinus;
    dispatchTable['*'] = &Lexer::handleMultiply;
    dispatchTable['/'] = &Lexer::handleDivide;
    dispatchTable['='] = &Lexer::handleEqual;
    dispatchTable['('] = &Lexer::handleLeftParen;
    dispatchTable[')'] = &Lexer::handleRightParen;
    dispatchTable['{'] = &Lexer::handleLeftBrace;
    dispatchTable['}'] = &Lexer::handleRightBrace;
    dispatchTable['['] = &Lexer::handleLeftBracket;
    dispatchTable[']'] = &Lexer::handleRightBracket;
    dispatchTable[','] = &Lexer::handleComma;
    dispatchTable['.'] = &Lexer::handleDot;
    dispatchTable['"'] = &Lexer::handleDoubleQuote;
    dispatchTable['\''] = &Lexer::handleSingleQuote;
    dispatchTable[':'] = &Lexer::handleColon;
    dispatchTable['<'] = &Lexer::handleLess;
    dispatchTable['>'] = &Lexer::handleGreater;
}

//==============================================================================
// Manejadores de Tokens Simples
//==============================================================================

/// @brief Procesa '+' o '++'
void Lexer::handlePlus() {
    addToken(match('+') ? TokenType::TOK_INCREMENT : TokenType::TOK_ADD);
}

/// @brief Procesa '-', '--' o '->'
void Lexer::handleMinus() {
    addToken(match('-') ? TokenType::TOK_DECREMENT : 
             match('>') ? TokenType::TOK_ARROW : TokenType::TOK_MINUS);
}

/// @brief Procesa '*'
void Lexer::handleMultiply() { addToken(TokenType::TOK_MULT); }

/// @brief Procesa '/' o comentario '//'
void Lexer::handleDivide() {
    if (match('/')) {
        // Comentario de línea: consumir hasta nueva línea
        while (peek() != '\n' && !isAtEnd()) advance();
    } else {
        addToken(TokenType::TOK_DIV);
    }
}

/// @brief Procesa '=' o '=='
void Lexer::handleEqual() {
    addToken(match('=') ? TokenType::TOK_EQUAL : TokenType::TOK_ASSIGN);
}

/// @brief Procesa '<' o '<<'
void Lexer::handleLess() {
    addToken(match('<') ? TokenType::TOK_LEFT_SHIFT : TokenType::TOK_LESS);
}

/// @brief Procesa '>' o '>>'
void Lexer::handleGreater() {
    addToken(match('>') ? TokenType::TOK_RIGHT_SHIFT : TokenType::TOK_GREATER);
}

/// @brief Procesa '('
void Lexer::handleLeftParen() { addToken(TokenType::TOK_LEFT_PAREN); }

/// @brief Procesa ')'
void Lexer::handleRightParen() { addToken(TokenType::TOK_RIGHT_PAREN); }

/// @brief Procesa '{'
void Lexer::handleLeftBrace() { addToken(TokenType::TOK_LEFT_BRACE); }

/// @brief Procesa '}'
void Lexer::handleRightBrace() { addToken(TokenType::TOK_RIGHT_BRACE); }

/// @brief Procesa '['
void Lexer::handleLeftBracket() { addToken(TokenType::TOK_LEFT_BRACKET); }

/// @brief Procesa ']'
void Lexer::handleRightBracket() { addToken(TokenType::TOK_RIGHT_BRACKET); }

/// @brief Procesa ','
void Lexer::handleComma() { addToken(TokenType::TOK_COMMA); }

/// @brief Procesa ':'
void Lexer::handleColon() { addToken(TokenType::TOK_COLON); }

/// @brief Procesa '.' o número decimal que empieza con punto
void Lexer::handleDot() {
    if (isDigit(peek())) {
        state = State::Decimal;
        number();
    } else {
        addToken(TokenType::TOK_DOT);
    }
}

/// @brief Procesa comillas dobles (cadenas)
void Lexer::handleDoubleQuote() { string(); }

/// @brief Procesa comillas simples (caracteres)
void Lexer::handleSingleQuote() { charliteral(); }

//==============================================================================
// Manejadores por Defecto
//==============================================================================

/**
 * @brief Procesa caracteres no asignados en la tabla de despacho
 * @param c Carácter a procesar
 * @details Maneja identificadores, números y caracteres inválidos
 */
void Lexer::handleDefault(char c) {
    if (c == '0') {
        char next = peek();
        if ((next | 0x20) == 'b') {
            isBinary();
        } else if ((next | 0x20) == 'x') {
            number(); // TODO: implementar hexadecimal
        } else {
            number();
        }
    } else if (isAlpha(c)) {
        identifier();
    } else if (isDigit(c)) {
        number();
    } else {
        char desc[2] = {c, '\0'};
        reportLexicalError(std::string("Carácter inesperado: '") + desc + "'");
    }
}

//==============================================================================
// Bucle Principal de Tokenización
//==============================================================================

/**
 * @brief Ejecuta el análisis léxico completo del código fuente
 * @return Vector de tokens generados
 * 
 * @details Procesa el código fuente carácter por carácter usando
 * tabla de despacho para operadores y clasificación optimizada
 * para identificadores y números.
 */
std::vector<Lexer::Token> Lexer::tokenize() {
    tokens.clear();
    tokens.reserve(source.length() >> 2); // Heurística: ~4 chars por token
    
    current = 0;
    line = 1;
    column = 1;
    start = 0;

    while (!isAtEnd()) {
        start = current;
        char c = advance();

        // Espacios en blanco (optimizado para caso común)
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                continue;
            case '\n':
                ++line;
                column = 1;
                if (tokens.empty() || tokens.back().type != TokenType::TOK_NEWLINE) {
                    addToken(TokenType::TOK_NEWLINE);
                }
                continue;
            default:
                break;
        }

        // Tabla de despacho para operadores/delimitadores
        if (auto handler = dispatchTable[static_cast<unsigned char>(c)]) {
            (this->*handler)();
        } else {
            handleDefault(c);
        }
    }

    addToken(TokenType::TOK_EOF);
    return tokens;
}

//==============================================================================
// Funciones de Navegación
//==============================================================================

/**
 * @brief Avanza al siguiente carácter
 * @return Carácter consumido
 */
char Lexer::advance() {
    ++column;
    return source[current++];
}

/// @brief Verifica si se alcanzó el final del fuente
bool Lexer::isAtEnd() const {
    return static_cast<size_t>(current) >= source.length();
}

/// @brief Observa el carácter actual sin consumirlo
char Lexer::peek() const {
    return isAtEnd() ? '\0' : source[current];
}

/// @brief Observa el siguiente carácter sin consumirlo
char Lexer::peekNext() const {
    return (static_cast<size_t>(current + 1) >= source.length()) ? '\0' : source[current + 1];
}

/**
 * @brief Intenta consumir un carácter específico
 * @param expected Carácter esperado
 * @return true si se consumió el carácter
 */
bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    ++current;
    ++column;
    return true;
}

//==============================================================================
// Emisión de Tokens
//==============================================================================

/**
 * @brief Emite un token usando el lexema desde start hasta current
 * @param type Tipo de token a emitir
 */
void Lexer::addToken(TokenType type) {
    addToken(type, &source[start], current - start);
}

/**
 * @brief Emite un token con lexema específico
 * @param type Tipo de token
 * @param lexeme Puntero al inicio del lexema
 * @param length Longitud del lexema
 */
void Lexer::addToken(TokenType type, const char* lexeme, size_t length) {
    tokens.emplace_back(type, lexeme, length, line, column - length);
}

//==============================================================================
// Escaneo de Literales
//==============================================================================

/**
 * @brief Procesa un literal de cadena entre comillas dobles
 * @details Maneja secuencias de escape: \\n, \\t, \\r, \\\\, \\"
 */
void Lexer::string() {
    std::string value;
    value.reserve(64); // Pre-reservar para strings típicos

    while (!isAtEnd()) {
        char c = advance();
        
        if (c == '"') {
            addToken(TokenType::TOK_STRING_LITERAL, value.c_str(), value.length());
            return;
        }
        
        if (c == '\\') {
            if (isAtEnd()) break;
            char esc = advance();
            char mapped = getEscapeChar(esc);
            if (mapped || esc == '0') {
                value += mapped;
            } else {
                reportLexicalError("Secuencia de escape inválida en literal de cadena");
                value += esc;
            }
        } else {
            value += c;
        }
    }
    
    reportLexicalError("Literal de cadena sin terminar. ¿Olvidaste la comilla de cierre '\"'?");
}

/**
 * @brief Procesa un literal de carácter entre comillas simples
 * @details Maneja secuencias de escape estándar
 */
void Lexer::charliteral() {
    if (isAtEnd()) {
        reportLexicalError("Literal de carácter sin terminar");
        return;
    }

    char c = advance();
    char value;

    if (c == '\\') {
        if (isAtEnd()) {
            reportLexicalError("Secuencia de escape incompleta en literal de carácter");
            return;
        }
        char esc = advance();
        value = getEscapeChar(esc);
        if (!value && esc != '0') {
            reportLexicalError(std::string("Carácter de escape inválido: \\") + esc);
            return;
        }
    } else {
        value = c;
    }

    if (isAtEnd() || advance() != '\'') {
        reportLexicalError("Literal de carácter sin terminar. Se esperaba comilla de cierre '\\''");
        return;
    }

    std::string result(1, value);
    addToken(TokenType::TOK_CHAR_LITERAL, result.c_str(), 1);
}

/**
 * @brief Procesa un literal numérico
 * @details Soporta enteros, decimales y notación científica (e/E)
 */
void Lexer::number() {
    // Parte entera
    while (isDigit(peek())) advance();

    // Parte decimal
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // consume '.'
        while (isDigit(peek())) advance();
    }

    // Notación científica
    if ((peek() | 0x20) == 'e') {
        char next = peekNext();
        if (isDigit(next) || next == '+' || next == '-') {
            advance(); // consume 'e'
            if (peek() == '+' || peek() == '-') advance();
            if (!isDigit(peek())) {
                reportLexicalError("Número mal formado: se esperaba dígito después del exponente");
                return;
            }
            while (isDigit(peek())) advance();
        }
    }

    size_t len = current - start;
    addToken(TokenType::TOK_NUMBER, &source[start], len);
}

/**
 * @brief Procesa un literal binario (0b...)
 * @return true si se procesó correctamente
 */
bool Lexer::isBinary() {
    advance(); // consume 'b' o 'B'
    
    size_t binStart = current;
    while (peek() == '0' || peek() == '1') advance();
    
    if (current == binStart) {
        reportLexicalError("Número binario mal formado");
        return false;
    }
    
    // Verificar que no hay caracteres alfanuméricos después
    if (isAlnum(peek())) {
        reportLexicalError("Número binario mal formado");
        return false;
    }
    
    addToken(TokenType::TOK_BINARY, &source[start], current - start);
    return true;
}

//==============================================================================
// Identificadores y Palabras Clave
//==============================================================================

/**
 * @brief Procesa un identificador o palabra reservada
 * @details Usa lookup hash constexpr para palabras clave en O(1) promedio
 */
void Lexer::identifier() {
    while (isAlnum(peek())) advance();
    
    const char* lexStart = &source[start];
    size_t lexLen = current - start;
    TokenType type = lookupKeyword(lexStart, lexLen);
    
    addToken(type, lexStart, lexLen);
}

//==============================================================================
// Funciones de Utilidad
//==============================================================================

/// @brief Obtiene el código fuente
const std::string& Lexer::getSource() const { return source; }

/// @brief Verifica si es signo de operación
bool Lexer::isSing(char c) const {
    return c == '+' || c == '-' || c == '/' || c == '*' || 
           c == '<' || c == '>' || c == '=' || c == '%' || c == '!';
}

/// @brief Verifica si es espacio en blanco (incluye newline)
bool Lexer::isWhitespace(char c) const { 
    return c == ' ' || c == '\t' || c == '\n' || c == '\r'; 
}

/// @brief Verifica si es espacio en blanco (sin newline)
bool Lexer::isBlankSpace(char c) const { 
    return c == ' ' || c == '\t' || c == '\r'; 
}

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
}

/**
 * @brief Observa el siguiente token sin consumirlo
 * @return Token en la posición actual
 */
Lexer::Token Lexer::peekToken() {
    if (tokenIndex < tokens.size()) return tokens[tokenIndex];
    if (!tokens.empty() && tokens.back().type == TokenType::TOK_EOF) return tokens.back();
    return Token(TokenType::TOK_EOF, "", 0, line, column);
}

/**
 * @brief Obtiene y consume el siguiente token
 * @return Siguiente token
 */
Lexer::Token Lexer::getNextToken() {
    if (tokenIndex < tokens.size()) {
        if (tokens[tokenIndex].type != TokenType::TOK_EOF) {
            return tokens[tokenIndex++];
        }
        return tokens[tokenIndex];
    }
    return Token(TokenType::TOK_EOF, "", 0, line, column);
}

/**
 * @brief Obtiene el contenido de una línea específica
 * @param lineNumber Número de línea (1-indexed)
 * @return Contenido de la línea
 */
std::string Lexer::getLineContent(int lineNumber) const {
    size_t pos = 0;
    int currentLine = 1;
    
    // Buscar inicio de la línea
    while (pos < source.length() && currentLine < lineNumber) {
        if (source[pos++] == '\n') ++currentLine;
    }
    
    if (currentLine != lineNumber) return "";
    
    // Buscar fin de la línea
    size_t lineStart = pos;
    while (pos < source.length() && source[pos] != '\n') ++pos;
    
    return std::string(source, lineStart, pos - lineStart);
}

//==============================================================================
// Manejo de Errores
//==============================================================================

/**
 * @brief Reporta un error léxico con contexto visual
 * @param msg Mensaje descriptivo del error
 * @param offset Desplazamiento adicional para el indicador
 */
void Lexer::reportLexicalError(const std::string& msg, int offset) {
    int tokenStartColumn = column - (current - start);
    int errorColumn = tokenStartColumn + offset;

    std::string lineContent = getLineContent(line);
    
    std::string underline(errorColumn > 0 ? errorColumn - 1 : 0, ' ');
    underline += '^';

    std::string fullMessage = "\n" + lineContent + "\n" + underline + "\n" + msg;

    errorManager->addError(std::make_unique<CompilerError>(
        ErrorType::LEXICAL, fullMessage, line, errorColumn));
}

} // namespace umbra
