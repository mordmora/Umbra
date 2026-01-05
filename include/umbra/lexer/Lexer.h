/**
 * @file Lexer.h
 * @brief Definición del analizador léxico para el lenguaje Umbra
 * @author Umbra Team
 * @version 2.0
 * 
 * @details Define la clase Lexer que convierte código fuente en tokens.
 * Implementa un analizador léxico optimizado con tabla de despacho O(1)
 * y clasificación de caracteres mediante tablas precalculadas.
 */

#ifndef LEXER_H
#define LEXER_H

#include "../error/ErrorManager.h"
#include "Tokens.h"
#include <string>
#include <vector>
#include <memory>

namespace umbra {

/**
 * @class Lexer
 * @brief Analizador léxico que convierte texto fuente en tokens
 * 
 * @details El Lexer procesa el código fuente carácter por carácter,
 * generando una secuencia de tokens que representan elementos léxicos
 * del lenguaje (números, identificadores, operadores, etc.).
 * 
 * Características:
 * - Tabla de despacho para procesamiento O(1) de operadores
 * - Tablas de clasificación de caracteres precalculadas
 * - Soporte para literales numéricos, cadenas y caracteres
 * - Manejo de secuencias de escape
 * - Reporte de errores con contexto visual
 */
class Lexer {
public:
    //==========================================================================
    // Estructuras Públicas
    //==========================================================================
    
    /**
     * @struct Token
     * @brief Representa un token léxico del código fuente
     */
    struct Token {
        TokenType type;      ///< Tipo del token
        std::string lexeme;  ///< Texto literal del token
        int line;            ///< Línea donde se encontró (1-indexed)
        int column;          ///< Columna donde se encontró (1-indexed)

        /// @brief Constructor por defecto
        Token() : type(TokenType::TOK_EOF), lexeme(""), line(0), column(0) {}

        /**
         * @brief Constructor de Token
         * @param type Tipo del token
         * @param start Puntero al inicio del lexema en el fuente
         * @param length Longitud del lexema
         * @param line Línea donde se encontró
         * @param column Columna donde se encontró
         */
        Token(TokenType type, const char* start, size_t length, int line, int column)
            : type(type), lexeme(start, length), line(line), column(column) {}

        /// @brief Obtiene el lexema del token
        inline const std::string& getLexeme() const { return lexeme; }
    };

    //==========================================================================
    // Constructores
    //==========================================================================
    
    /**
     * @brief Constructor con gestor de errores interno
     * @param source Código fuente a tokenizar
     */
    explicit Lexer(const std::string& source);

    /**
     * @brief Constructor con gestor de errores externo
     * @param source Código fuente a tokenizar
     * @param externalErrorManager Referencia al gestor de errores externo
     */
    Lexer(const std::string& source, ErrorManager& externalErrorManager);

    //==========================================================================
    // Interfaz Pública
    //==========================================================================
    
    /**
     * @brief Ejecuta el análisis léxico completo
     * @return Vector de tokens generados
     */
    std::vector<Token> tokenize();

    /**
     * @brief Obtiene el gestor de errores
     * @return Referencia constante al gestor de errores
     */
    const ErrorManager& getErrorManager() const { return *errorManager; }

    /**
     * @brief Reinicia el estado del lexer
     */
    void reset();

    /**
     * @brief Observa el siguiente token sin consumirlo
     * @return Copia del token actual
     */
    Token peekToken();

    /**
     * @brief Obtiene y consume el siguiente token
     * @return Token consumido
     */
    Token getNextToken();

    /**
     * @brief Obtiene el código fuente
     * @return Referencia al código fuente
     */
    const std::string& getSource() const;

private:
    //==========================================================================
    // Miembros de Datos
    //==========================================================================
    
    std::string source;                                 ///< Código fuente
    std::unique_ptr<ErrorManager> internalErrorManager; ///< Gestor interno
    ErrorManager* errorManager;                         ///< Gestor activo
    std::vector<Token> tokens;                          ///< Tokens generados
    
    /// @brief Tabla de despacho: mapea char → función manejadora
    void (Lexer::*dispatchTable[256])() = {};

    //==========================================================================
    // Estado del Escáner
    //==========================================================================
    
    /**
     * @enum State
     * @brief Estados del autómata para reconocimiento de números
     */
    enum class State { 
        Start,          ///< Estado inicial
        Integer,        ///< Parte entera
        Decimal,        ///< Parte decimal
        NotationNumber, ///< Notación científica
        Acceptance,     ///< Estado de aceptación
        Rejection       ///< Estado de rechazo
    };

    int current = 0;           ///< Posición actual en el fuente
    int line = 1;              ///< Línea actual
    int start = 0;             ///< Inicio del token actual
    int column = 1;            ///< Columna actual
    State state = State::Start;///< Estado del autómata
    size_t tokenIndex = 0;     ///< Índice para iteración de tokens

    //==========================================================================
    // Inicialización
    //==========================================================================
    
    /**
     * @brief Inicializa la tabla de despacho
     * @details Asigna manejadores a operadores y delimitadores
     */
    void setupDispatch();

    //==========================================================================
    // Manejadores de Tokens (Tabla de Despacho)
    //==========================================================================
    
    void handlePlus();        ///< Procesa '+' o '++'
    void handleMinus();       ///< Procesa '-', '--' o '->'
    void handleMultiply();    ///< Procesa '*'
    void handleDivide();      ///< Procesa '/' o comentario '//'
    void handleEqual();       ///< Procesa '=' o '=='
    void handleLeftParen();   ///< Procesa '('
    void handleRightParen();  ///< Procesa ')'
    void handleLeftBrace();   ///< Procesa '{'
    void handleRightBrace();  ///< Procesa '}'
    void handleLeftBracket(); ///< Procesa '['
    void handleRightBracket();///< Procesa ']'
    void handleComma();       ///< Procesa ','
    void handleDot();         ///< Procesa '.' o número decimal
    void handleDoubleQuote(); ///< Procesa '"' (cadena)
    void handleSingleQuote(); ///< Procesa '\'' (carácter)
    void handleColon();       ///< Procesa ':'
    void handleLess();        ///< Procesa '<' o '<<'
    void handleGreater();     ///< Procesa '>' o '>>'
    
    /**
     * @brief Manejador por defecto para caracteres no en tabla
     * @param c Carácter a procesar
     */
    void handleDefault(char c);

    //==========================================================================
    // Funciones de Navegación
    //==========================================================================
    
    /**
     * @brief Avanza al siguiente carácter
     * @return Carácter consumido
     */
    char advance();

    /**
     * @brief Verifica si se llegó al final del fuente
     * @return true si no hay más caracteres
     */
    bool isAtEnd() const;

    /**
     * @brief Observa el carácter actual sin consumirlo
     * @return Carácter actual o '\\0' si es fin
     */
    char peek() const;

    /**
     * @brief Observa el siguiente carácter sin consumirlo
     * @return Siguiente carácter o '\\0' si es fin
     */
    char peekNext() const;

    /**
     * @brief Intenta consumir un carácter esperado
     * @param expected Carácter a buscar
     * @return true si se consumió el carácter
     */
    bool match(char expected);

    //==========================================================================
    // Emisión de Tokens
    //==========================================================================
    
    /**
     * @brief Emite token con lexema desde start hasta current
     * @param type Tipo de token
     */
    void addToken(TokenType type);

    /**
     * @brief Emite token con lexema específico
     * @param type Tipo de token
     * @param lexeme Puntero al lexema
     * @param length Longitud del lexema
     */
    void addToken(TokenType type, const char* lexeme, size_t length);

    //==========================================================================
    // Escaneo de Literales
    //==========================================================================
    
    void string();      ///< Procesa literal de cadena
    void number();      ///< Procesa literal numérico
    void charliteral(); ///< Procesa literal de carácter
    void identifier();  ///< Procesa identificador o palabra clave
    bool isBinary();    ///< Procesa literal binario

    //==========================================================================
    // Funciones de Clasificación
    //==========================================================================
    
    /**
     * @brief Verifica si es operador
     * @param c Carácter a verificar
     * @return true si es operador
     */
    bool isSing(char c) const;

    /**
     * @brief Verifica si es espacio en blanco (incluye newline)
     * @param c Carácter a verificar
     * @return true si es espacio
     */
    bool isWhitespace(char c) const;

    /**
     * @brief Verifica si es espacio en blanco (sin newline)
     * @param c Carácter a verificar
     * @return true si es espacio
     */
    bool isBlankSpace(char c) const;

    //==========================================================================
    // Manejo de Errores
    //==========================================================================
    
    /**
     * @brief Reporta un error léxico con contexto visual
     * @param msg Mensaje de error
     * @param offset Desplazamiento para el indicador
     */
    void reportLexicalError(const std::string& msg, int offset = 0);

    /**
     * @brief Obtiene el contenido de una línea
     * @param lineNumber Número de línea (1-indexed)
     * @return Contenido de la línea
     */
    std::string getLineContent(int lineNumber) const;
};

} // namespace umbra

#endif // LEXER_H
