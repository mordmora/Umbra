/**
 * @file Parser.h
 * @brief Analizador sintáctico descendente recursivo para el lenguaje Umbra
 * @author Umbra Team
 * @version 2.1
 * 
 * @details Implementa un parser LL(1) con lookahead extendido para manejar
 * gramáticas ambiguas. Genera un AST tipado desde tokens léxicos.
 * 
 * @note Optimizaciones:
 * - Funciones de navegación marcadas noexcept
 * - Métodos const donde es seguro
 * - [[nodiscard]] en funciones de consulta
 */

#ifndef PARSER_H
#define PARSER_H

#include "../lexer/Lexer.h"
#include "../lexer/Tokens.h"
#include "../error/ErrorManager.h"
#include "../ast/ASTNode.h"
#include "../ast/Nodes.h"
#include <vector>

namespace umbra {

/**
 * @class Parser
 * @brief Analizador sintáctico para construcción del AST
 * 
 * @details Convierte secuencia de tokens en árbol de sintaxis abstracta.
 * Usa descenso recursivo con precedencia de operadores integrada.
 */
class Parser {
public:
    //==========================================================================
    // Constructores
    //==========================================================================
    
    /// @brief Constructor básico sin gestor de errores externo
    explicit Parser(const std::vector<Lexer::Token>& tokens);
    
    /// @brief Constructor con gestor de errores externo
    Parser(const std::vector<Lexer::Token>& tokens, ErrorManager& errMgr);
    
    /// @brief Punto de entrada del análisis sintáctico
    [[nodiscard]] std::unique_ptr<ProgramNode> parseProgram();

private:
    //==========================================================================
    // Estado del Parser
    //==========================================================================
    
    std::vector<Lexer::Token> tokens;                    ///< Vector de tokens
    std::vector<Lexer::Token>::const_iterator current;   ///< Iterador actual
    ErrorManager* errorManager;                          ///< Gestor de errores
    Lexer::Token previousToken;                          ///< Token anterior

    //==========================================================================
    // Navegación de Tokens (hot-path, noexcept)
    //==========================================================================
    
    /// @brief Observa token a distancia sin consumir
    [[nodiscard]] Lexer::Token lookAhead(int dist) const noexcept;
    
    /// @brief Verifica tipo de token actual
    [[nodiscard]] bool check(TokenType t) const noexcept;
    
    /// @brief Consume si coincide el tipo
    bool match(TokenType t) noexcept;
    
    /// @brief Avanza y retorna token actual
    Lexer::Token advance() noexcept;
    
    /// @brief Retorna token previo
    [[nodiscard]] Lexer::Token previous() const noexcept;
    
    /// @brief Observa token actual
    [[nodiscard]] Lexer::Token peek() const noexcept;
    
    /// @brief Verifica fin de entrada
    [[nodiscard]] bool isAtEnd() const noexcept;
    
    /// @brief Consume token esperado o error
    Lexer::Token consume(TokenType t, const char* msg);
    
    /// @brief Salta tokens de nueva línea
    void skipNewLines() noexcept;

    //==========================================================================
    // Utilidades de Análisis
    //==========================================================================
    
    /// @brief Verifica si token es de tipo
    [[nodiscard]] bool isTypeToken(const Lexer::Token& tk) const noexcept;
    
    /// @brief Detecta asignación adelante
    [[nodiscard]] bool isAssignmentAhead() const noexcept;
    
    /// @brief Recuperación de errores
    void synchronize() noexcept;
    
    /// @brief Registra error sintáctico
    void error(const char* msg, int ln, int col);

    //==========================================================================
    // Reglas de Producción - Declaraciones
    //==========================================================================
    
    std::unique_ptr<FunctionDefinition> parseFunctionDefinition();
    std::unique_ptr<Type> parseType();
    std::vector<std::unique_ptr<Statement>> parseStatementList();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<VariableDeclaration> parseVariableDeclaration();
    std::unique_ptr<AssignmentStatement> parseAssignmentStatement();
    std::unique_ptr<ReturnExpression> parseReturnExpression();

    //==========================================================================
    // Reglas de Producción - Expresiones (Precedencia Ascendente)
    //==========================================================================
    
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseLogicalOr();
    std::unique_ptr<Expression> parseLogicalAnd();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseRelational();
    std::unique_ptr<Expression> parseAdditive();
    std::unique_ptr<Expression> parseMultiplicative();
    std::unique_ptr<Expression> parseUnary();
    std::unique_ptr<Expression> parsePostfix();
    std::unique_ptr<Expression> parsePrimary();

    //==========================================================================
    // Reglas de Producción - Estructuras de Control
    //==========================================================================
    
    std::unique_ptr<IfStatement> parseIfStatement();
    std::unique_ptr<RepeatTimesStatement> parseRepeatTimesStatement();
    std::unique_ptr<RepeatIfStatement> parseRepeatIfStatement();

    //==========================================================================
    // Reglas de Producción - Auxiliares
    //==========================================================================
    
    std::unique_ptr<Expression> parseFunctionCall();
    std::unique_ptr<Identifier> parseIdentifier();
    std::unique_ptr<Literal> parseLiteral();
};

} // namespace umbra

#endif // PARSER_H
