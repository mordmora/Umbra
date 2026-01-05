/**
 * @file Parser.cpp
 * @brief Implementación del analizador sintáctico descendente recursivo
 * @author Umbra Team
 * @version 2.1
 * 
 * @details Parser LL(1) altamente optimizado con precedencia de operadores.
 * Construye AST desde tokens usando descenso recursivo predictivo.
 * 
 * @note Optimizaciones aplicadas:
 * - Tablas de lookup constexpr para mapeos O(1)
 * - string_view para evitar allocaciones en strings constantes
 * - Branch prediction hints [[likely]]/[[unlikely]]
 * - Funciones inline agresivas
 * - noexcept donde es seguro
 */

#include "umbra/parser/Parser.h"
#include "umbra/error/CompilerError.h"
#include "umbra/error/ErrorTypes.h"
#include <iostream>
#include <algorithm>
#include <string_view>
#include <array>

namespace umbra {

//==============================================================================
// Funciones Auxiliares Estáticas Optimizadas
//==============================================================================

namespace {

/**
 * @brief Verifica si TokenType es tipo básico (constexpr + inline)
 * @param t Tipo de token a verificar
 * @return true si es tipo básico
 */
[[nodiscard]] constexpr inline bool isBasicType(TokenType t) noexcept {
    // Optimizado: switch permite al compilador generar tabla de saltos
    switch (t) {
        case TokenType::TOK_INT:
        case TokenType::TOK_FLOAT:
        case TokenType::TOK_STRING:
        case TokenType::TOK_BOOL:
        case TokenType::TOK_VOID:
        case TokenType::TOK_CHAR:
        case TokenType::TOK_PTR:
        case TokenType::TOK_REF:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Verifica si es operador de comparación (optimizado)
 */
[[nodiscard]] constexpr inline bool isComparisonOp(TokenType t) noexcept {
    switch (t) {
        case TokenType::TOK_EQUAL:
        case TokenType::TOK_DIFFERENT:
        case TokenType::TOK_LESS:
        case TokenType::TOK_GREATER:
        case TokenType::TOK_LESS_EQ:
        case TokenType::TOK_GREATER_EQ:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Verifica si es operador aditivo
 */
[[nodiscard]] constexpr inline bool isAdditiveOp(TokenType t) noexcept {
    return t == TokenType::TOK_ADD || t == TokenType::TOK_MINUS;
}

/**
 * @brief Verifica si es operador multiplicativo
 */
[[nodiscard]] constexpr inline bool isMultiplicativeOp(TokenType t) noexcept {
    switch (t) {
        case TokenType::TOK_MULT:
        case TokenType::TOK_DIV:
        case TokenType::TOK_MOD:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Verifica si es operador unario prefijo
 */
[[nodiscard]] constexpr inline bool isUnaryPrefixOp(TokenType t) noexcept {
    switch (t) {
        case TokenType::TOK_MINUS:
        case TokenType::TOK_NOT:
        case TokenType::TOK_REF:
        case TokenType::TOK_ACCESS:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Mapea TokenType a BuiltinType (constexpr + noexcept)
 * @param t Tipo de token
 * @return BuiltinType correspondiente
 */
[[nodiscard]] constexpr BuiltinType tokenToBuiltinType(TokenType t) noexcept {
    switch (t) {
        case TokenType::TOK_INT:    return BuiltinType::Int;
        case TokenType::TOK_FLOAT:  return BuiltinType::Float;
        case TokenType::TOK_STRING: return BuiltinType::String;
        case TokenType::TOK_BOOL:   return BuiltinType::Bool;
        case TokenType::TOK_VOID:   return BuiltinType::Void;
        case TokenType::TOK_CHAR:   return BuiltinType::Char;
        case TokenType::TOK_PTR:    return BuiltinType::Ptr;
        case TokenType::TOK_REF:    return BuiltinType::Ref;
        default:                    return BuiltinType::Void;
    }
}

/**
 * @brief Tabla de lookup para operadores → string_view
 * @details Usa string_view para evitar allocaciones de memoria
 */
struct OperatorTable {
    /// @brief Obtiene string del operador (O(1) lookup)
    [[nodiscard]] static constexpr std::string_view get(TokenType t) noexcept {
        switch (t) {
            case TokenType::TOK_ADD:         return "+";
            case TokenType::TOK_MINUS:       return "-";
            case TokenType::TOK_MULT:        return "*";
            case TokenType::TOK_DIV:         return "/";
            case TokenType::TOK_MOD:         return "%";
            case TokenType::TOK_EQUAL:       return "==";
            case TokenType::TOK_DIFFERENT:   return "!=";
            case TokenType::TOK_LESS:        return "<";
            case TokenType::TOK_GREATER:     return ">";
            case TokenType::TOK_LESS_EQ:     return "<=";
            case TokenType::TOK_GREATER_EQ:  return ">=";
            case TokenType::TOK_AND:         return "and";
            case TokenType::TOK_OR:          return "or";
            case TokenType::TOK_NOT:         return "not";
            case TokenType::TOK_REF:         return "ref";
            case TokenType::TOK_ACCESS:      return "access";
            case TokenType::TOK_INCREMENT:   return "++";
            case TokenType::TOK_DECREMENT:   return "--";
            default:                         return "?";
        }
    }
};

/**
 * @brief Token EOF por defecto (evita construcciones repetidas)
 */
inline const Lexer::Token kEofToken{TokenType::TOK_EOF, "", 0, 0, 0};

} // namespace anónimo

//==============================================================================
// Constructores (con inicialización optimizada)
//==============================================================================

Parser::Parser(const std::vector<Lexer::Token>& tokens)
    : tokens(tokens)
    , current(this->tokens.cbegin())
    , errorManager(nullptr) 
{
    if (!tokens.empty()) [[likely]] {
        previousToken = tokens.front();
    }
}

Parser::Parser(const std::vector<Lexer::Token>& tokens, ErrorManager& errMgr)
    : tokens(tokens)
    , current(this->tokens.cbegin())
    , errorManager(&errMgr)
{
    if (!tokens.empty()) [[likely]] {
        previousToken = tokens.front();
    }
}

//==============================================================================
// Navegación de Tokens (funciones hot-path optimizadas)
//==============================================================================

Lexer::Token Parser::lookAhead(int dist) const noexcept {
    auto it = current;
    for (int i = 0; i < dist && it != tokens.cend(); ++i, ++it);
    return (it != tokens.cend()) ? *it : kEofToken;
}

bool Parser::check(TokenType t) const noexcept {
    return !isAtEnd() && current->type == t;
}

bool Parser::match(TokenType t) noexcept {
    if (check(t)) [[likely]] {
        advance();
        return true;
    }
    return false;
}

Lexer::Token Parser::advance() noexcept {
    previousToken = *current;
    if (!isAtEnd()) [[likely]] ++current;
    return previousToken;
}

Lexer::Token Parser::previous() const noexcept {
    return previousToken;
}

Lexer::Token Parser::peek() const noexcept {
    return (current != tokens.cend()) ? *current : kEofToken;
}

bool Parser::isAtEnd() const noexcept {
    return current == tokens.cend() || current->type == TokenType::TOK_EOF;
}

void Parser::skipNewLines() noexcept {
    while (check(TokenType::TOK_NEWLINE)) [[unlikely]] advance();
}

Lexer::Token Parser::consume(TokenType t, const char* msg) {
    if (check(t)) [[likely]] return advance();
    error(msg, peek().line, peek().column);
    return Lexer::Token{TokenType::TOK_INVALID, "", 0, peek().line, peek().column};
}

//==============================================================================
// Utilidades (optimizadas con branch hints)
//==============================================================================

bool Parser::isTypeToken(const Lexer::Token& tk) const noexcept {
    return isBasicType(tk.type) || tk.type == TokenType::TOK_IDENTIFIER;
}

bool Parser::isAssignmentAhead() const noexcept {
    if (peek().type != TokenType::TOK_IDENTIFIER) [[likely]] return false;
    
    int offset = 1;
    auto la = lookAhead(offset);
    
    while (la.type == TokenType::TOK_LEFT_BRACKET) [[unlikely]] {
        int brackets = 1;
        ++offset;
        while (brackets > 0) {
            la = lookAhead(offset++);
            if (la.type == TokenType::TOK_LEFT_BRACKET) ++brackets;
            else if (la.type == TokenType::TOK_RIGHT_BRACKET) --brackets;
            else if (la.type == TokenType::TOK_EOF) [[unlikely]] return false;
        }
        la = lookAhead(offset);
    }
    
    return la.type == TokenType::TOK_ASSIGN;
}

void Parser::synchronize() noexcept {
    advance();
    while (!isAtEnd()) [[likely]] {
        if (previous().type == TokenType::TOK_NEWLINE) return;
        
        switch (peek().type) {
            case TokenType::TOK_FUNC:
            case TokenType::TOK_IF:
            case TokenType::TOK_REPEAT:
            case TokenType::TOK_RETURN:
            case TokenType::TOK_INT:
            case TokenType::TOK_FLOAT:
            case TokenType::TOK_BOOL:
            case TokenType::TOK_STRING:
            case TokenType::TOK_VOID:
                return;
            default:
                break;
        }
        advance();
    }
}

void Parser::error(const char* msg, int ln, int col) {
    if (errorManager) {
        errorManager->addError(std::make_unique<CompilerError>(
            ErrorType::SYNTACTIC, std::string(msg), ln, col));
    } else {
        std::cerr << "[Parser Error] Línea " << ln << ", Col " << col 
                  << ": " << msg << std::endl;
    }
}

//==============================================================================
// Punto de Entrada
//==============================================================================

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    std::vector<std::unique_ptr<FunctionDefinition>> functions;
    functions.reserve(16);  // Pre-allocación típica
    
    skipNewLines();
    
    while (!isAtEnd()) [[likely]] {
        if (check(TokenType::TOK_FUNC)) [[likely]] {
            if (auto fn = parseFunctionDefinition()) {
                functions.push_back(std::move(fn));
            }
        } else [[unlikely]] {
            error("Se esperaba definición de función", peek().line, peek().column);
            synchronize();
        }
        skipNewLines();
    }
    
    return std::make_unique<ProgramNode>(std::move(functions));
}

//==============================================================================
// Parsing de Funciones
//==============================================================================

std::unique_ptr<FunctionDefinition> Parser::parseFunctionDefinition() {
    consume(TokenType::TOK_FUNC, "Se esperaba 'func'");
    skipNewLines();
    
    Lexer::Token nameToken = consume(TokenType::TOK_IDENTIFIER, "Se esperaba nombre de función");
    skipNewLines();
    
    consume(TokenType::TOK_LEFT_PAREN, "Se esperaba '(' después del nombre");
    skipNewLines();
    
    // Parsear parámetros como pares (tipo, nombre)
    std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> params;
    
    if (!check(TokenType::TOK_RIGHT_PAREN)) {
        do {
            skipNewLines();
            if (!isTypeToken(peek())) {
                error("Se esperaba tipo de parámetro", peek().line, peek().column);
                break;
            }
            
            auto paramType = parseType();
            Lexer::Token paramName = consume(TokenType::TOK_IDENTIFIER, "Se esperaba nombre de parámetro");
            
            params.emplace_back(
                std::move(paramType),
                std::make_unique<Identifier>(paramName.lexeme)
            );
            
            skipNewLines();
        } while (match(TokenType::TOK_COMMA));
    }
    
    consume(TokenType::TOK_RIGHT_PAREN, "Se esperaba ')' después de parámetros");
    skipNewLines();
    
    consume(TokenType::TOK_ARROW, "Se esperaba '->' para tipo de retorno");
    skipNewLines();
    
    auto returnType = parseType();
    skipNewLines();
    
    consume(TokenType::TOK_LEFT_BRACE, "Se esperaba '{' para cuerpo de función");
    skipNewLines();
    
    auto body = parseStatementList();
    skipNewLines();
    
    consume(TokenType::TOK_RIGHT_BRACE, "Se esperaba '}' al final de función");
    
    auto paramList = std::make_unique<ParameterList>(std::move(params));
    
    return std::make_unique<FunctionDefinition>(
        std::make_unique<Identifier>(nameToken.lexeme),
        std::move(paramList),
        std::move(returnType),
        std::move(body)
    );
}

//==============================================================================
// Parsing de Tipos
//==============================================================================

std::unique_ptr<Type> Parser::parseType() {
    bool isPointer = false;
    bool isReference = false;
    
    if (check(TokenType::TOK_PTR)) {
        advance();
        isPointer = true;
        skipNewLines();
    } else if (check(TokenType::TOK_REF)) {
        advance();
        isReference = true;
        skipNewLines();
    }
    
    if (!isTypeToken(peek())) {
        error("Se esperaba especificador de tipo", peek().line, peek().column);
        return std::make_unique<Type>(BuiltinType::Void);
    }
    
    Lexer::Token typeToken = advance();
    BuiltinType baseType = tokenToBuiltinType(typeToken.type);
    
    // Dimensiones de array con expresiones
    std::vector<std::unique_ptr<Expression>> arraySizes;
    int arrayDimensions = 0;
    
    while (check(TokenType::TOK_LEFT_BRACKET)) {
        advance();
        skipNewLines();
        
        if (check(TokenType::TOK_NUMBER)) {
            int size = std::stoi(advance().lexeme);
            arraySizes.push_back(std::make_unique<NumericLiteral>(
                static_cast<double>(size), BuiltinType::Int));
            ++arrayDimensions;
        } else {
            error("Se esperaba tamaño de array", peek().line, peek().column);
        }
        
        skipNewLines();
        consume(TokenType::TOK_RIGHT_BRACKET, "Se esperaba ']'");
    }
    
    if (isPointer || isReference) {
        auto innerType = std::make_unique<Type>(baseType, arrayDimensions, std::move(arraySizes));
        return std::make_unique<Type>(
            isPointer ? BuiltinType::Ptr : BuiltinType::Ref,
            std::move(innerType),
            isPointer,
            isReference
        );
    }
    
    return std::make_unique<Type>(baseType, arrayDimensions, std::move(arraySizes));
}

//==============================================================================
// Parsing de Statements
//==============================================================================

std::vector<std::unique_ptr<Statement>> Parser::parseStatementList() {
    std::vector<std::unique_ptr<Statement>> stmts;
    stmts.reserve(32);  // Pre-allocación para bodies típicos
    
    skipNewLines();
    
    while (!isAtEnd() && !check(TokenType::TOK_RIGHT_BRACE)) [[likely]] {
        if (auto stmt = parseStatement()) [[likely]] {
            stmts.push_back(std::move(stmt));
        }
        skipNewLines();
    }
    
    return stmts;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    skipNewLines();
    
    const TokenType t = peek().type;
    
    // Optimizado: switch para dispatch directo
    switch (t) {
        case TokenType::TOK_RETURN:
            return parseReturnExpression();
            
        case TokenType::TOK_IF:
            return parseIfStatement();
            
        case TokenType::TOK_REPEAT: {
            // Distinguir entre repeat (n) times y repeat if
            if (lookAhead(1).type == TokenType::TOK_IF) [[unlikely]] {
                advance(); // consumir 'repeat'
                return parseRepeatIfStatement();
            }
            return parseRepeatTimesStatement();
        }
        
        default:
            break;
    }
    
    // Declaración de variable (branch más frecuente después de control)
    if (isTypeToken(peek())) [[likely]] {
        const TokenType next = lookAhead(1).type;
        if (t == TokenType::TOK_PTR || t == TokenType::TOK_REF) {
            if (isBasicType(next) || next == TokenType::TOK_IDENTIFIER) {
                return parseVariableDeclaration();
            }
        } else if (next == TokenType::TOK_IDENTIFIER) {
            return parseVariableDeclaration();
        }
    }
    
    // Asignación
    if (isAssignmentAhead()) {
        return parseAssignmentStatement();
    }
    
    // Expresión como statement (fallback)
    auto expr = parseExpression();
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration() {
    auto type = parseType();
    Lexer::Token nameToken = consume(TokenType::TOK_IDENTIFIER, "Se esperaba nombre de variable");
    
    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::TOK_ASSIGN)) {
        skipNewLines();
        initializer = parseExpression();
    }
    
    return std::make_unique<VariableDeclaration>(
        std::move(type),
        std::make_unique<Identifier>(nameToken.lexeme),
        std::move(initializer)
    );
}

std::unique_ptr<AssignmentStatement> Parser::parseAssignmentStatement() {
    Lexer::Token nameToken = consume(TokenType::TOK_IDENTIFIER, "Se esperaba identificador");
    
    // Construir expresión target (puede ser acceso a array)
    std::unique_ptr<Expression> target = std::make_unique<Identifier>(nameToken.lexeme);
    
    while (check(TokenType::TOK_LEFT_BRACKET)) {
        advance();
        skipNewLines();
        auto index = parseExpression();
        skipNewLines();
        consume(TokenType::TOK_RIGHT_BRACKET, "Se esperaba ']'");
        
        target = std::make_unique<ArrayAccessExpression>(std::move(target), std::move(index));
    }
    
    consume(TokenType::TOK_ASSIGN, "Se esperaba '='");
    skipNewLines();
    
    auto value = parseExpression();
    
    return std::make_unique<AssignmentStatement>(std::move(target), std::move(value));
}

std::unique_ptr<ReturnExpression> Parser::parseReturnExpression() {
    consume(TokenType::TOK_RETURN, "Se esperaba 'return'");
    skipNewLines();
    
    std::unique_ptr<Expression> retVal = nullptr;
    if (!check(TokenType::TOK_NEWLINE) && !check(TokenType::TOK_RIGHT_BRACE) && !isAtEnd()) {
        retVal = parseExpression();
    }
    
    return std::make_unique<ReturnExpression>(std::move(retVal));
}

//==============================================================================
// Estructuras de Control
//==============================================================================

std::unique_ptr<IfStatement> Parser::parseIfStatement() {
    consume(TokenType::TOK_IF, "Se esperaba 'if'");
    skipNewLines();
    
    consume(TokenType::TOK_LEFT_PAREN, "Se esperaba '(' después de 'if'");
    skipNewLines();
    
    auto condition = parseExpression();
    skipNewLines();
    
    consume(TokenType::TOK_RIGHT_PAREN, "Se esperaba ')' después de condición");
    skipNewLines();
    
    consume(TokenType::TOK_LEFT_BRACE, "Se esperaba '{'");
    skipNewLines();
    
    auto thenBody = parseStatementList();
    skipNewLines();
    
    consume(TokenType::TOK_RIGHT_BRACE, "Se esperaba '}'");
    skipNewLines();
    
    // Construir branches
    std::vector<Branch> branches;
    Branch mainBranch;
    mainBranch.condition = std::move(condition);
    mainBranch.body = std::move(thenBody);
    branches.push_back(std::move(mainBranch));
    
    std::vector<std::unique_ptr<Statement>> elseBranch;
    if (match(TokenType::TOK_ELSE)) {
        skipNewLines();
        consume(TokenType::TOK_LEFT_BRACE, "Se esperaba '{' después de 'else'");
        skipNewLines();
        elseBranch = parseStatementList();
        skipNewLines();
        consume(TokenType::TOK_RIGHT_BRACE, "Se esperaba '}'");
    }
    
    return std::make_unique<IfStatement>(std::move(branches), std::move(elseBranch));
}

std::unique_ptr<RepeatTimesStatement> Parser::parseRepeatTimesStatement() {
    consume(TokenType::TOK_REPEAT, "Se esperaba 'repeat'");
    skipNewLines();
    
    
    consume(TokenType::TOK_LEFT_PAREN, "Se esperaba '('");
    skipNewLines();
    
    auto count = parseExpression();
    skipNewLines();
    
    consume(TokenType::TOK_RIGHT_PAREN, "Se esperaba ')'");
    skipNewLines();
    
    consume(TokenType::TOK_TIMES, "Se esperaba 'times'");
    skipNewLines();
    
    consume(TokenType::TOK_LEFT_BRACE, "Se esperaba '{'");
    skipNewLines();
    
    auto body = parseStatementList();
    skipNewLines();
    
    consume(TokenType::TOK_RIGHT_BRACE, "Se esperaba '}'");
    
    return std::make_unique<RepeatTimesStatement>(std::move(count), std::move(body));
}

std::unique_ptr<RepeatIfStatement> Parser::parseRepeatIfStatement() {
    consume(TokenType::TOK_IF, "Se esperaba 'if'");
    skipNewLines();
    
    consume(TokenType::TOK_LEFT_PAREN, "Se esperaba '('");
    skipNewLines();
    
    auto condition = parseExpression();
    skipNewLines();
    
    consume(TokenType::TOK_RIGHT_PAREN, "Se esperaba ')'");
    skipNewLines();
    
    consume(TokenType::TOK_LEFT_BRACE, "Se esperaba '{'");
    skipNewLines();
    
    auto body = parseStatementList();
    skipNewLines();
    
    consume(TokenType::TOK_RIGHT_BRACE, "Se esperaba '}'");
    
    return std::make_unique<RepeatIfStatement>(std::move(condition), std::move(body));
}

//==============================================================================
// Parsing de Expresiones (optimizado con predicados inline)
//==============================================================================

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseLogicalOr();
}

std::unique_ptr<Expression> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    
    while (check(TokenType::TOK_OR)) [[unlikely]] {
        advance();
        skipNewLines();
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryExpression>(
            std::string(OperatorTable::get(TokenType::TOK_OR)),
            std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseLogicalAnd() {
    auto left = parseEquality();
    
    while (check(TokenType::TOK_AND)) [[unlikely]] {
        advance();
        skipNewLines();
        auto right = parseEquality();
        left = std::make_unique<BinaryExpression>(
            std::string(OperatorTable::get(TokenType::TOK_AND)),
            std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseEquality() {
    auto left = parseRelational();
    
    TokenType t = peek().type;
    while (t == TokenType::TOK_EQUAL || t == TokenType::TOK_DIFFERENT) [[unlikely]] {
        std::string op(OperatorTable::get(advance().type));
        skipNewLines();
        auto right = parseRelational();
        left = std::make_unique<BinaryExpression>(std::move(op), std::move(left), std::move(right));
        t = peek().type;
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseRelational() {
    auto left = parseAdditive();
    
    // Optimizado: usa función helper en lugar de cadena de ||
    while (isComparisonOp(peek().type) && 
           peek().type != TokenType::TOK_EQUAL && 
           peek().type != TokenType::TOK_DIFFERENT) [[unlikely]] {
        std::string op(OperatorTable::get(advance().type));
        skipNewLines();
        auto right = parseAdditive();
        left = std::make_unique<BinaryExpression>(std::move(op), std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    
    // Optimizado: usa función helper inline
    while (isAdditiveOp(peek().type)) [[likely]] {
        std::string op(OperatorTable::get(advance().type));
        skipNewLines();
        auto right = parseMultiplicative();
        left = std::make_unique<BinaryExpression>(std::move(op), std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseMultiplicative() {
    auto left = parseUnary();
    
    // Optimizado: usa función helper inline
    while (isMultiplicativeOp(peek().type)) [[unlikely]] {
        std::string op(OperatorTable::get(advance().type));
        skipNewLines();
        auto right = parseUnary();
        left = std::make_unique<BinaryExpression>(std::move(op), std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseUnary() {
    const TokenType t = peek().type;
    
    // Operadores unarios prefijo (optimizado con helper)
    if (isUnaryPrefixOp(t)) [[unlikely]] {
        std::string op(OperatorTable::get(advance().type));
        skipNewLines();
        auto operand = parseUnary();
        return std::make_unique<UnaryExpression>(std::move(op), std::move(operand));
    }
    
    // Incremento prefijo
    if (t == TokenType::TOK_INCREMENT) [[unlikely]] {
        advance();
        skipNewLines();
        auto operand = parseUnary();
        return std::make_unique<IncrementExpression>(std::move(operand), true);
    }
    
    // Decremento prefijo
    if (t == TokenType::TOK_DECREMENT) [[unlikely]] {
        advance();
        skipNewLines();
        auto operand = parseUnary();
        return std::make_unique<DecrementExpression>(std::move(operand), true);
    }
    
    return parsePostfix();
}

std::unique_ptr<Expression> Parser::parsePostfix() {
    auto expr = parsePrimary();
    
    // Loop optimizado para postfix operations
    for (;;) {
        const TokenType t = peek().type;
        
        switch (t) {
            case TokenType::TOK_LEFT_BRACKET: {
                // Acceso a array
                advance();
                skipNewLines();
                auto index = parseExpression();
                skipNewLines();
                consume(TokenType::TOK_RIGHT_BRACKET, "Se esperaba ']'");
                expr = std::make_unique<ArrayAccessExpression>(std::move(expr), std::move(index));
                continue;
            }
            
            case TokenType::TOK_LEFT_PAREN: {
                // Llamada a función
                auto* id = dynamic_cast<Identifier*>(expr.get());
                if (id) [[likely]] {
                    std::string funcName = id->name;
                    advance();
                    skipNewLines();
                    
                    std::vector<std::unique_ptr<Expression>> args;
                    args.reserve(8);  // Pre-allocación para args típicos
                    
                    if (!check(TokenType::TOK_RIGHT_PAREN)) [[likely]] {
                        do {
                            skipNewLines();
                            args.push_back(parseExpression());
                            skipNewLines();
                        } while (match(TokenType::TOK_COMMA));
                    }
                    
                    consume(TokenType::TOK_RIGHT_PAREN, "Se esperaba ')'");
                    
                    expr = std::make_unique<FunctionCall>(
                        std::make_unique<Identifier>(std::move(funcName)),
                        std::move(args)
                    );
                    continue;
                }
                break;
            }
            
            case TokenType::TOK_INCREMENT:
                advance();
                expr = std::make_unique<IncrementExpression>(std::move(expr), false);
                continue;
                
            case TokenType::TOK_DECREMENT:
                advance();
                expr = std::make_unique<DecrementExpression>(std::move(expr), false);
                continue;
                
            default:
                break;
        }
        break;
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    const TokenType t = peek().type;
    
    // Optimizado: switch para dispatch directo O(1)
    switch (t) {
        // Literales numéricos (más frecuente)
        case TokenType::TOK_NUMBER: {
            const std::string& lexeme = peek().lexeme;
            double val = std::stod(lexeme);
            advance();
            // Detectar si es float por presencia de '.'
            BuiltinType type = (lexeme.find('.') != std::string::npos) 
                              ? BuiltinType::Float : BuiltinType::Int;
            return std::make_unique<NumericLiteral>(val, type);
        }
        
        // String literal
        case TokenType::TOK_STRING_LITERAL:
            return std::make_unique<StringLiteral>(advance().lexeme);
        
        // Boolean true
        case TokenType::TOK_TRUE:
            advance();
            return std::make_unique<BooleanLiteral>(true);
        
        // Boolean false
        case TokenType::TOK_FALSE:
            advance();
            return std::make_unique<BooleanLiteral>(false);
        
        // Char literal
        case TokenType::TOK_CHAR_LITERAL: {
            std::string val = advance().lexeme;
            char c = val.empty() ? '\0' : val[0];
            return std::make_unique<CharLiteral>(c);
        }
        
        // Identifier (muy frecuente)
        case TokenType::TOK_IDENTIFIER:
            return std::make_unique<Identifier>(advance().lexeme);
        
        // Expresión parentizada
        case TokenType::TOK_LEFT_PAREN: {
            advance();
            skipNewLines();
            auto expr = parseExpression();
            skipNewLines();
            consume(TokenType::TOK_RIGHT_PAREN, "Se esperaba ')'");
            return expr;
        }
        
        default:
            break;
    }
    
    // Error recovery
    error("Se esperaba expresión", peek().line, peek().column);
    advance();
    return std::make_unique<NumericLiteral>(0.0, BuiltinType::Int);
}

//==============================================================================
// Funciones Auxiliares (optimizadas)
//==============================================================================

std::unique_ptr<Expression> Parser::parseFunctionCall() {
    Lexer::Token nameToken = consume(TokenType::TOK_IDENTIFIER, "Se esperaba nombre de función");
    
    consume(TokenType::TOK_LEFT_PAREN, "Se esperaba '('");
    skipNewLines();
    
    std::vector<std::unique_ptr<Expression>> args;
    args.reserve(8);  // Pre-allocación típica
    
    if (!check(TokenType::TOK_RIGHT_PAREN)) [[likely]] {
        do {
            skipNewLines();
            args.push_back(parseExpression());
            skipNewLines();
        } while (match(TokenType::TOK_COMMA));
    }
    
    consume(TokenType::TOK_RIGHT_PAREN, "Se esperaba ')'");
    
    return std::make_unique<FunctionCall>(
        std::make_unique<Identifier>(nameToken.lexeme),
        std::move(args)
    );
}

std::unique_ptr<Identifier> Parser::parseIdentifier() {
    Lexer::Token tk = consume(TokenType::TOK_IDENTIFIER, "Se esperaba identificador");
    return std::make_unique<Identifier>(tk.lexeme);
}

std::unique_ptr<Literal> Parser::parseLiteral() {
    if (check(TokenType::TOK_NUMBER)) [[likely]] {
        const std::string& lexeme = peek().lexeme;
        double val = std::stod(lexeme);
        advance();
        // Detectar tipo por presencia de punto decimal
        BuiltinType type = (lexeme.find('.') != std::string::npos) 
                          ? BuiltinType::Float : BuiltinType::Int;
        return std::make_unique<NumericLiteral>(val, type);
    }
    
    error("Se esperaba literal numérico", peek().line, peek().column);
    return std::make_unique<NumericLiteral>(0.0, BuiltinType::Int);
}

} // namespace umbra
