#include "Parser.h"
#include "../ast/ProgramNode.h"
#include "../lexer/TokensUtils.h"
#include <iostream>
#include <stdexcept>

namespace umbra {

// Parser::Parser(Lexer &lexer) : lexer(lexer) {}

Parser::Parser(const std::vector<Lexer::Token> &tokens)
    : tokens(tokens), current(tokens.begin()), errorManager(new ErrorManager()),
      previousToken(*current) {}

// Constructor que usa un ErrorManager externo
Parser::Parser(const std::vector<Lexer::Token> &tokens, ErrorManager &externalErrorManager)
    : tokens(tokens), current(tokens.begin()), errorManager(&externalErrorManager),
      previousToken(*current) {}

std::unique_ptr<ASTNode> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> ast;

    while (!isAtEnd()) {
        try {
            ast.push_back(parseStatement());
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            // Skip to the next newline or EOF
            while (!isAtEnd() && !match(TokenType::TOK_NEWLINE)) {
                advance();
            }
        }
    }

    return std::make_unique<ProgramNode>(std::move(ast));
}

/*types */
// Centralize the handling of new types
bool Parser::isTypeSpecifier(TokenType type) const {
    switch (type) {
    case TokenType::TOK_INT:
    case TokenType::TOK_FLOAT:
    case TokenType::TOK_BOOL:
    case TokenType::TOK_CHAR:
    case TokenType::TOK_STRING:
        return true;
    default:
        return false;
    }
}

TokenType Parser::parseTypeSpecifier() {
    if (isTypeSpecifier(peek().type)) {
        TokenType type = peek().type;
        advance(); // Consumir el token de tipo
        return type;
    } else {
        throw std::runtime_error("Expected type specifier, but found: " +
                                 TokenManager::tokenTypeToString(peek().type));
    }
}

/*Statements*/
std::unique_ptr<ASTNode> Parser::parseStatement() {

    // some TOK_* type
    if (isTypeSpecifier(peek().type)) {
        return parseVariableDeclaration();
    }

    switch (peek().type) {
    case TokenType::TOK_IF:
        return nullptr; // parseIfStatement();
    // Otros casos de declaraciones, como bucles, llamadas a funciones, etc.
    default:
        throw std::runtime_error("Unexpected token: " +
                                 TokenManager::tokenTypeToString(peek().type));
    }
}

bool Parser::isTypeCompatible(TokenType declaredType, ExpressionNode *expr) {
    if (declaredType == TokenType::TOK_INT) {
        return dynamic_cast<ex_IntegerLiteralNode *>(expr) != nullptr;
    } else if (declaredType == TokenType::TOK_FLOAT) {
        return dynamic_cast<ex_FloatLiteralNode *>(expr) != nullptr ||
               dynamic_cast<ex_IntegerLiteralNode *>(expr) != nullptr;
    } else if (declaredType == TokenType::TOK_STRING) {
        return dynamic_cast<ex_StringLiteralNode *>(expr) != nullptr;
    }
    // Handle other types...
    return false;
}

std::string Parser::getExpressionTypeName(ExpressionNode *expr) {
    if (dynamic_cast<ex_IntegerLiteralNode *>(expr))
        return "int";
    if (dynamic_cast<ex_FloatLiteralNode *>(expr))
        return "float";
    if (dynamic_cast<ex_BoolLiteralNode *>(expr))
        return "bool";
    if (dynamic_cast<ex_StringLiteralNode *>(expr))
        return "string";
    return "unknown";
}

std::string Parser::getTypeName(TokenType type) {
    if (type == TokenType::TOK_INT)
        return "int";
    if (type == TokenType::TOK_FLOAT)
        return "float";
    if (type == TokenType::TOK_STRING)
        return "string";
    if (type == TokenType::TOK_BOOL)
        return "bool";

    return "unknown";
}

std::unique_ptr<ExpressionNode> Parser::parseNumber(const Lexer::Token &token) {
    /*
      TODO: not support to hexa, octal?
     */
    const std::string &lexeme = token.lexeme;
    try {
        // entero
        size_t pos;
        long long intValue = std::stoll(lexeme, &pos);
        if (pos == lexeme.length()) {
            return std::make_unique<ex_IntegerLiteralNode>(static_cast<int>(intValue));
        }
        double doubleValue = std::stod(lexeme);
        return std::make_unique<ex_FloatLiteralNode>(doubleValue);
    } catch (const std::out_of_range &e) {
        try {
            double doubleValue = std::stod(lexeme);
            return std::make_unique<ex_FloatLiteralNode>(doubleValue);
        } catch (const std::out_of_range &e) {
            throw std::runtime_error("Number out of range: " + lexeme);
        }
    } catch (const std::invalid_argument &e) {
        // not a number token!
        throw std::runtime_error("Invalid number format: " + lexeme);
    }
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    Lexer::Token currentToken = peek();

    switch (currentToken.type) {
    case TokenType::TOK_NUMBER: {
        advance();
        return parseNumber(currentToken);
    }

    case TokenType::TOK_STRING_LITERAL: {
        advance();
        return std::make_unique<ex_StringLiteralNode>(currentToken.lexeme);
    }

    case TokenType::TOK_BOOL: {
        advance();
        bool boolValue = (currentToken.lexeme == "true");
        return std::make_unique<ex_BoolLiteralNode>(boolValue);
    }

    case TokenType::TOK_IDENTIFIER: {
        advance();
        return std::make_unique<ex_IdentifierNode>(currentToken.lexeme);
    }

    case TokenType::TOK_CHAR_LITERAL: {
        advance();
        return std::make_unique<ex_CharLiteralNode>(currentToken.lexeme);
    }

    default:
        std::string errorMsg =
            "Unexpected token in expression: " + TokenManager::tokenTypeToString(currentToken.type);
        errorManager->addError(std::make_unique<CompilerError>(
            ErrorType::SYNTACTIC, errorMsg, currentToken.line, currentToken.column));
        // throw std::runtime_error(errorMsg);
        return nullptr;
    }

    return nullptr;
}

std::unique_ptr<st_VariableDeclNode> Parser::parseVariableDeclaration() {
    /*
    <variable_declaration> ::= <type> [ "[" <array_size> "]" ] <identifier> ["=" <expression>]
    <newline> <array_size> ::= <const> | <identifier>
     */

    // type specifier
    TokenType type = parseTypeSpecifier();

    // array?
    std::unique_ptr<ExpressionNode> arraySize = nullptr;
    if (match(TokenType::TOK_LEFT_BRACKET)) {
        if (peek().type == TokenType::TOK_NUMBER || peek().type == TokenType::TOK_IDENTIFIER) {
            arraySize = parseExpression();
        } else {
            std::string errorMsg = "Expected constant or identifier for array size.";
            errorManager->addError(std::make_unique<CompilerError>(ErrorType::SYNTACTIC, errorMsg,
                                                                   peek().line, peek().column));
            throw std::runtime_error(errorMsg);
        }
        consume(TokenType::TOK_RIGHT_BRACKET, "Expected ']' after array size.");
    }

    // name (TOK_IDENTIFIER)
    auto token = consume(TokenType::TOK_IDENTIFIER, "Expected variable name");
    std::string name = token.lexeme;

    // initializer?
    std::unique_ptr<ExpressionNode> initializer = nullptr;
    if (match(TokenType::TOK_ASSIGN)) {
        initializer = parseExpression();
        if (initializer) {
            if (!isTypeCompatible(type, initializer.get())) {
                std::string errorMsg =
                    "Type mismatch in initialization of '" + name + "'. Cannot convert from " +
                    getExpressionTypeName(initializer.get()) + " to " + getTypeName(type);
                errorManager->addError(std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC, errorMsg, previous().line, previous().column));
            }
        } else {
            std::string errorMsg = "Expected expression after '=' in variable declaration.";
            errorManager->addError(std::make_unique<CompilerError>(ErrorType::SYNTACTIC, errorMsg,
                                                                   peek().line, peek().column));
        }
    }

    // TOK_NEWLINE
    if (!match(TokenType::TOK_NEWLINE)) {
        std::string errorMsg = "Expected newline after variable declaration.";
        errorManager->addError(std::make_unique<CompilerError>(ErrorType::SYNTACTIC, errorMsg,
                                                               peek().line, peek().column));
        throw std::runtime_error(errorMsg);
    }
    return std::make_unique<st_VariableDeclNode>(type, name, std::move(initializer),
                                                 std::move(arraySize));
}

/*
std::unique_ptr<FunctionDeclNode> Parser::parseFunctionDefinition() {
    consume(TokenType::TOK_FUNC, "Expected 'func' at the beginning of function definition");

    // Parse the function name (identifier)
    auto token = consume(TokenType::TOK_IDENTIFIER, "Expected function name");
    std::string functionName = token.lexeme;

    // Parse the parameter list
    consume(TokenType::TOK_LEFT_PAREN, "Expected '(' after function name");
    std::vector<std::unique_ptr<ParameterNode>> parameters = parseParameterList();
    consume(TokenType::TOK_RIGHT_PAREN, "Expected ')' after parameter list");

    // Parse the return type (after '->')
    consume(TokenType::TOK_ARROW, "Expected '->' for return type");
    TokenType returnType = parseTypeSpecifier();

    // Parse the function body (statements inside braces)
    consume(TokenType::TOK_LEFT_BRACE, "Expected '{' at the beginning of function body");

    std::vector<std::unique_ptr<ASTNode>> body;
    while (!check(TokenType::TOK_RIGHT_BRACE) && !isAtEnd()) {
        body.push_back(parseStatement());
    }

    consume(TokenType::TOK_RIGHT_BRACE, "Expected '}' at the end of function body");

    // Return statement if the function is not void
    std::unique_ptr<ReturnStatementNode> returnStmt = nullptr;
    if (returnType != TokenType::TOK_VOID && match(TokenType::TOK_RETURN)) {
        returnStmt = parseReturnStatement();
    }

    return std::make_unique<FunctionDeclNode>(functionName, std::move(parameters), returnType,
                                              std::move(body), std::move(returnStmt));
}
*/
/*Expression*/

/*Utils*/

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd())
        return false;
    return peek().type == type;
}

Lexer::Token Parser::advance() {
    if (!isAtEnd()) {
        previousToken = *current;
        ++current;
    }
    return previous();
}

Lexer::Token Parser::previous() const { return previousToken; }

Lexer::Token Parser::peek() const { return *current; }

bool Parser::isAtEnd() const { return peek().type == TokenType::TOK_EOF; }

Lexer::Token Parser::consume(TokenType type, const std::string &message) {
    Lexer::Token currentToken = peek();
    if (check(type)) {
        advance();
        return currentToken;
    }
    errorManager->addError(
        std::make_unique<CompilerError>(ErrorType::SYNTACTIC, message, peek().line, peek().column));
    throw std::runtime_error(message);
}

} // namespace umbra