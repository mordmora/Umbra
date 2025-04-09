#ifndef LEXER_H
#define LEXER_H

#include "../error/ErrorManager.h"
#include "Tokens.h"
#include <string>
#include <vector>

namespace umbra {

/**
 * @brief Clase analizador léxico que convierte texto fuente en tokens
 * 
 * El Lexer es responsable de analizar el texto fuente carácter por carácter
 * y generar una secuencia de tokens que representan los elementos léxicos
 * del lenguaje (números, identificadores, operadores, etc.).
 */
class Lexer {
  public:
    /**
     * @brief Estructura que representa un token léxico
     */
    struct Token {
        TokenType type;      ///< Tipo del token
        const char* start;  ///< Texto literal del token
        std::size_t length; ///< Longitud del token
        int line;           ///< Línea donde se encontró el token
        int column;         ///< Columna donde se encontró el token

        /**
         * @brief Constructor de Token
         * @param t Tipo del token
         * @param l Lexema del token
         * @param ln Línea donde se encontró
         * @param col Columna donde se encontró
         */
        Token(TokenType type, const char* start, size_t length, int line, int column)
        : type(type), start(start), length(length), line(line), column(column) {}

        std::string lexeme() const {
            return std::string(start, length);
        }
    };

    /**
     * @brief Constructor que inicializa el Lexer con texto fuente
     * @param source Texto fuente a analizar
     */
    Lexer(const std::string &source);

    /**
     * @brief Constructor que inicializa el Lexer con texto fuente y un gestor de errores externo
     * @param source Texto fuente a analizar
     * @param externalErrorManager Gestor de errores externo
     */
    Lexer(const std::string &source, ErrorManager &externalErrorManager);

    /**
     * @brief Analiza el texto fuente y genera una lista de tokens
     * @return Vector de tokens generados
     */
    std::vector<Token> tokenize();

    /**
     * @brief Obtiene el gestor de errores actual
     * @return Referencia constante al gestor de errores
     */
    const ErrorManager &getErrorManager() const { return *errorManager; }

    /**
     * @brief Reinicia el estado del lexer
     */
    void reset();

    /**
     * @brief Observa el siguiente token sin consumirlo
     * @return Token siguiente en la secuencia
     */
    Token peekToken();

    /**
     * @brief Obtiene y consume el siguiente token
     * @return Token siguiente en la secuencia
     */
    Token getNextToken();

    /**
     * @brief Obtiene el texto fuente actual
     * @return String con el texto fuente
     */
    std::string getSource();
  private:
    std::string source;                              ///< Texto fuente a analizar
    std::unique_ptr<ErrorManager> internalErrorManager;  ///< Gestor de errores interno
    ErrorManager *errorManager;                      ///< Puntero al gestor de errores actual
    std::vector<Token> tokens;                       ///< Lista de tokens generados
    char lastChar = ' ';                            ///< Último carácter leído
    
    /**
     * @brief Estados del autómata para el reconocimiento de números
     */
    enum class State { 
        Start,          ///< Estado inicial
        Integer,        ///< Reconociendo parte entera
        Decimal,        ///< Reconociendo parte decimal
        NotationNumber, ///< Reconociendo notación científica
        Acceptance,     ///< Estado de aceptación
        Rejection       ///< Estado de rechazo
    };

    int current = 0;    ///< Posición actual en el texto fuente
    int line = 1;       ///< Línea actual
    int start = 0;      ///< Inicio del token actual
    int column = 1;     ///< Columna actual
    State state=State::Start;  ///< Estado actual del autómata

    /**
     * @brief Avanza al siguiente carácter del texto fuente
     * @return Carácter actual
     */
    char advance();

    /**
     * @brief Verifica si se llegó al final del texto fuente
     * @return true si no hay más caracteres que leer
     */
    bool isAtEnd() const;

    /**
     * @brief Observa el siguiente carácter sin consumirlo
     * @return Siguiente carácter o '\0' si es fin de archivo
     */
    char peek() const;

    /**
     * @brief Verifica si un carácter es un operador
     * @param c Carácter a verificar
     * @return true si es un operador
     */

    bool matchOperatorFromTable(const char c);

    /**
     * @brief Genera un mensaje de error léxico
     * @param msg Mensaje de error
     */

    void reportLexicalError(const std::string& msg, int offset=0);

    std::string  getLineContent(int line) const;

    /**
     * @brief Observa el carácter después del siguiente sin consumirlo
     * @return Carácter después del siguiente o '\0' si es fin de archivo
     */
    char peekNext() const;

    /**
     * @brief Intenta hacer match con un carácter esperado
     * @param expected Carácter esperado
     * @return true si hay match y se consumió el carácter
     */
    bool match(char expected);

    /**
     * @brief Agrega un nuevo token a la lista
     * @param type Tipo del token
     */
    void addToken(TokenType type);

    /**
     * @brief Agrega un nuevo token a la lista con un lexema específico
     * @param type Tipo del token
     * @param lexeme Lexema del token
     * @param line Línea donde se encontró el token
     * @param column Columna donde se encontró el token
     */
    void addToken(TokenType type, const char* lexeme, size_t length);

    /**
     * @brief Agrega un nuevo token a la lista con un lexema específico
     * @param type Tipo del token
     * @param lexeme Lexema del token
     */

    /**
     * @brief Procesa una cadena de texto
     */
    void string();

    /**
     * @brief Procesa un número (entero, decimal o notación científica)
     */
    void number();

    /**
     * @brief Procesa un carácter literal
     */
    void charliteral();

    /**
     * @brief Procesa un identificador
     */
    void identifier();

    /**
     * @brief Verifica si un carácter es una letra
     * @param c Carácter a verificar
     * @return true si es una letra
     */
    bool isAlpha(char c) const;

    /**
     * @brief Verifica si un carácter es alfanumérico
     * @param c Carácter a verificar
     * @return true si es alfanumérico
     */
    bool isAlphaNumeric(char c) const;

    /**
     * @brief Verifica si un carácter es un dígito
     * @param c Carácter a verificar
     * @return true si es un dígito
     */
    bool isDigit(char c) const;

    /**
     * @brief Verifica si un carácter es un signo de operación
     * @param c Carácter a verificar
     * @return true si es un signo de operación
     */
    bool isSing(char c) const;

    /**
     * @brief Verifica si un carácter es un espacio en blanco
     * @param c Carácter a verificar
     * @return true si es un espacio en blanco
     */
    bool isWhitespace(char c) const;

    /**
     * @brief Verifica si un carácter es un espacio en blanco (incluye tab)
     * @param c Carácter a verificar
     * @return true si es un espacio en blanco
     */
    bool isBlankSpace(char c) const;

    /**
     * @brief Verifica y procesa un número binario
     * @param c Carácter inicial
     * @return true si se procesó un número binario válido
     */
    bool isBinary(char c);

};

} // namespace umbra

#endif // LEXER_H