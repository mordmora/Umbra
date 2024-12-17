#include "Parser.h"
#include <stdexcept>

namespace umbra {

Parser::Parser(Lexer& lexer) : lexer(lexer), currentToken(lexer.getNextToken()) {
}

void Parser::advanceToken() {
    currentToken = lexer.getNextToken();
}

void Parser::expectToken(TokenType expectedType) {
    if (currentToken.type != expectedType) {
        error();
    }
}

void Parser::error() {
    // Manejo de errores
    exit(EXIT_FAILURE);
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto programNode = std::make_unique<ProgramNode>();
    while (currentToken.type != TokenType::TOK_EOF) {
        programNode->addFunction(parseFunctionDefinition());
    }
    return programNode;
}

std::unique_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition() {
    expectToken(TokenType::TOK_FUNC);
    advanceToken(); // Consumir 'func'

    expectToken(TokenType::TOK_IDENTIFIER);
    std::string funcName = currentToken.lexeme;
    advanceToken(); // Consumir el nombre de la función

    expectToken(TokenType::TOK_LEFT_PAREN);
    advanceToken(); // Consumir '('

    auto tail = parseFunctionDefinitionTail();
    return std::make_unique<FunctionDefinitionNode>(funcName, std::move(tail));
}

std::unique_ptr<FunctionDefinitionTailNode> Parser::parseFunctionDefinitionTail() {
    std::unique_ptr<ParametersNode> parameters = nullptr;

    if (currentToken.type != TokenType::TOK_RIGHT_PAREN) {
        parameters = parseParameterList();
    }

    expectToken(TokenType::TOK_RIGHT_PAREN);
    advanceToken(); // Consumir ')'

    expectToken(TokenType::TOK_ARROW);
    advanceToken(); // Consumir '->'

    auto returnType = parseType();

    expectToken(TokenType::TOK_LEFT_BRACE);
    advanceToken(); // Consumir '{'

    auto statements = parseStatementList();

    expectToken(TokenType::TOK_RIGHT_BRACE);
    advanceToken(); // Consumir '}'

    return std::make_unique<FunctionDefinitionTailNode>(
        std::move(returnType), std::move(statements), std::move(parameters));
}

std::unique_ptr<ParametersNode> Parser::parseParameterList() {
    auto parameters = std::make_unique<ParametersNode>(std::vector<std::unique_ptr<ParamNode>>{});
    parameters->addParameter(parseParam());

    while (currentToken.type == TokenType::TOK_COMMA) {
        advanceToken(); // Consumir ','
        parameters->addParameter(parseParam());
    }
    return parameters;
}

std::unique_ptr<ParamNode> Parser::parseParam() {
    auto type = parseType();

    expectToken(TokenType::TOK_IDENTIFIER);
    std::string paramName = currentToken.lexeme;
    advanceToken(); // Consumir el nombre del parámetro

    return std::make_unique<ParamNode>(std::move(type), paramName);
}

std::unique_ptr<TypeNode> Parser::parseType() {
    TypeNode::BaseType baseType;

    switch (currentToken.type) {
        case TokenType::TOK_INT:
            baseType = TypeNode::BaseType::INT;
            break;
        case TokenType::TOK_FLOAT:
            baseType = TypeNode::BaseType::FLOAT;
            break;
        case TokenType::TOK_BOOL:
            baseType = TypeNode::BaseType::BOOL;
            break;
        case TokenType::TOK_CHAR:
            baseType = TypeNode::BaseType::CHAR;
            break;
        case TokenType::TOK_STRING:
            baseType = TypeNode::BaseType::STRING;
            break;
        default:
            error();
    }
    advanceToken(); // Consumir el tipo

    std::unique_ptr<TypeNode> typeNode = std::make_unique<TypeNode>(baseType);

    while (currentToken.type == TokenType::TOK_LEFT_BRACKET) {
        advanceToken(); // Consumir '['
        expectToken(TokenType::TOK_RIGHT_BRACKET);
        advanceToken(); // Consumir ']'

        typeNode = std::make_unique<TypeNode>(TypeNode::BaseType::ARRAY, std::move(typeNode));
    }

    return typeNode;
}

std::unique_ptr<StatementListNode> Parser::parseStatementList() {
    auto statementList = std::make_unique<StatementListNode>();
    while (currentToken.type != TokenType::TOK_RIGHT_BRACE && currentToken.type != TokenType::TOK_END_OF_FILE) {
        statementList->addStatement(parseStatement());
    }
    return statementList;
}

std::unique_ptr<StatementNode> Parser::parseStatement() {
    switch (currentToken.type) {
        case TokenType::TOK_IF:
            return parseIfStatement();
        // Agregar casos para otros tipos de declaraciones
        default:
            error();
            return nullptr;
    }
}

std::unique_ptr<StatementNode> Parser::parseIfStatement() {
    advanceToken(); // Consumir 'if'

    auto condition = parseExpression();

    expectToken(TokenType::TOK_LEFT_BRACE);
    advanceToken(); // Consumir '{'

    auto thenStatements = parseStatementList();

    expectToken(TokenType::TOK_RIGHT_BRACE);
    advanceToken(); // Consumir '}'

    std::vector<std::unique_ptr<ElseIfStatementNode>> elseIfClauses;

    while (currentToken.type == TokenType::TOK_ELSEIF) {
        advanceToken(); // Consumir 'elseif'
        auto elseIfCondition = parseExpression();

        expectToken(TokenType::TOK_LEFT_BRACE);
        advanceToken(); // Consumir '{'

        auto elseIfStatements = parseStatementList();

        expectToken(TokenType::TOK_RIGHT_BRACE);
        advanceToken(); // Consumir '}'

        elseIfClauses.push_back(std::make_unique<ElseIfStatementNode>(
            std::move(elseIfCondition), std::move(elseIfStatements)));
    }

    std::unique_ptr<StatementListNode> elseStatements = nullptr;

    if (currentToken.type == TokenType::TOK_ELSE) {
        advanceToken(); // Consumir 'else'

        expectToken(TokenType::TOK_LEFT_BRACE);
        advanceToken(); // Consumir '{'

        elseStatements = parseStatementList();

        expectToken(TokenType::TOK_RIGHT_BRACE);
        advanceToken(); // Consumir '}'
    }

    return std::make_unique<IfStatementNode>(
        std::move(condition), std::move(thenStatements),
        std::move(elseIfClauses), std::move(elseStatements));
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    return parseLogicalOrExpression();
}

std::unique_ptr<ExpressionNode> Parser::parseLogicalOrExpression() {
    auto left = parseLogicalAndExpression();
    while (currentToken.type == TokenType::TOK_OR) {
        advanceToken(); // Consumir 'or'
        auto right = parseLogicalAndExpression();
        left = std::make_unique<LogicalOrExpressionNode>(std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseLogicalAndExpression() {
    auto left = parseEqualityExpression();
    while (currentToken.type == TokenType::TOK_AND) {
        advanceToken(); // Consumir 'and'
        auto right = parseEqualityExpression();
        left = std::make_unique<LogicalAndExpressionNode>(std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseEqualityExpression() {
    auto left = parseRelationalExpression();
    if (currentToken.type == TokenType::TOK_EQUAL_OP || currentToken.type == TokenType::TOK_NOT_EQUAL_OP) {
        auto op = currentToken.type;
        advanceToken(); // Consumir operador
        auto right = parseRelationalExpression();
        left = std::make_unique<EqualityExpressionNode>(
            std::move(left),
            (op == TokenType::TOK_EQUAL) ? EqualityExpressionNode::Operator::EQUAL : EqualityExpressionNode::Operator::NOT_EQUAL,
            std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseRelationalExpression() {
    auto left = parseAdditiveExpression();
    if (currentToken.type == TokenType::TOK_LESS || currentToken.type == TokenType::TOK_GREATER ||
        currentToken.type == TokenType::TOK_LESS_EQ || currentToken.type == TokenType::TOK_GREATER_EQ) {
        auto op = currentToken.type;
        advanceToken(); // Consumir operador
        auto right = parseAdditiveExpression();
        left = std::make_unique<RelationalExpressionNode>(
            std::move(left),
            static_cast<RelationalExpressionNode::Operator>(op),
            std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();
    while (currentToken.type == TokenType::TOK_ADD || currentToken.type == TokenType::TOK_MINUS) {
        auto op = currentToken.type;
        advanceToken(); // Consumir operador
        auto right = parseMultiplicativeExpression();
        left = std::make_unique<AdditiveExpressionNode>(
            std::move(left),
            (op == TokenType::TOK_ADD) ? AdditiveExpressionNode::Operator::ADD : AdditiveExpressionNode::Operator::SUBTRACT,
            std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseMultiplicativeExpression() {
    auto left = parseUnaryExpression();
    while (currentToken.type == TokenType::TOK_MULT || currentToken.type == TokenType::TOK_DIV || currentToken.type == TokenType::TOK_MOD) {
        auto op = currentToken.type;
        advanceToken(); // Consumir operador
        auto right = parseUnaryExpression();
        left = std::make_unique<MultiplicativeExpressionNode>(
            std::move(left),
            static_cast<MultiplicativeExpressionNode::Operator>(op),
            std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseUnaryExpression() {
    if (currentToken.type == TokenType::TOK_PTR || currentToken.type == TokenType::TOK_REF || currentToken.type == TokenType::TOK_ACCESS) {
        auto op = currentToken.type;
        advanceToken(); // Consumir operador
        auto operand = parsePrimaryExpression();
        return std::make_unique<UnaryExpressionNode>(
            static_cast<UnaryExpressionNode::Operator>(op), std::move(operand));
    }
    return parsePrimaryExpression();
}

std::unique_ptr<PrimaryExpressionNode> Parser::parsePrimaryExpression() {
    if (currentToken.type == TokenType::TOK_IDENTIFIER) {
        std::string idName = currentToken.lexeme;
        advanceToken(); // Consumir identificador
        return std::make_unique<PrimaryExpressionNode>(
            std::make_unique<IdentifierNode>(idName));
    }
    // Manejar literales, llamadas a funciones y expresiones entre paréntesis según sea necesario
    error();
    return nullptr;
}

} // namespace umbra