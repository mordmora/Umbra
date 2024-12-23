#include "Parser.h"
#include "../error/ErrorManager.h"
#include "../ast/nodes/include/variable_declaration_node.h"
#include<vector>
#include <iostream>

namespace umbra {

    Parser::Parser(const std::vector<Lexer::Token>& tokens) : tokens(tokens), current(tokens.begin()), errorManager(nullptr),
        previousToken(*current) {
    }

    Parser::Parser(const std::vector<Lexer::Token>& tokens, ErrorManager& externalErrorManager) : tokens(tokens), current(tokens.begin()),
        errorManager(&externalErrorManager), previousToken(*current) {
    }

    bool Parser::match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool Parser::check(TokenType type) const {
        if (isAtEnd()) {
            return false;
        }
        return peek().type == type;
    }

    Lexer::Token Parser::advance() {
        if (!isAtEnd()) {
            previousToken = *current;
            return *current++;
        }
        return previous();
    }

    Lexer::Token Parser::previous() const { return previousToken; }

    Lexer::Token Parser::peek() const { return *current; }

    bool Parser::isAtEnd() const { return peek().type == TokenType::TOK_EOF; }

    Lexer::Token Parser::consume(TokenType type, const std::string& message) {
        Lexer::Token currentToken = peek();
        if (check(type)) {
            advance();
            return currentToken;
        }
        errorManager->addError(
            std::make_unique<CompilerError>(ErrorType::SYNTACTIC, message, peek().line, peek().column));
        throw std::runtime_error(message);
    }

    bool Parser::isTypeToken(const Lexer::Token& token) {
        return token.type == TokenType::TOK_INT || token.type == TokenType::TOK_FLOAT || token.type == TokenType::TOK_BOOL ||

            token.type == TokenType::TOK_CHAR || token.type == TokenType::TOK_STRING || token.type == TokenType::TOK_ARRAY || token.type == TokenType::TOK_VOID;
    }


    //<program> ::= { <function_definition> }
    std::shared_ptr<ProgramNode> Parser::parseProgram() {
        std::vector<std::unique_ptr<FunctionDefinitionNode>> functionDefinitions;
        while (!isAtEnd()) {
            functionDefinitions.push_back(parseFunctionDefinition());
            std::cout << "Function definition parsed" << std::endl;
        }
        return std::make_shared<ProgramNode>(std::move(functionDefinitions));
    }

    //<function_definition> ::= "func" <identifier> "(" [ <parameter_list> ] ")" "->" <type> "{" <statement_list> "}"
    std::unique_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition() {
        if (!match(TokenType::TOK_FUNC)) {
            errorManager->addError(
                std::make_unique<CompilerError>(ErrorType::SYNTACTIC, "Expected 'func' keyword", peek().line, peek().column));
            synchronize();
        }
        Lexer::Token identifier = consume(TokenType::TOK_IDENTIFIER, "Expected function name");
        consume(TokenType::TOK_LEFT_PAREN, "Expected '('");

        std::vector<std::unique_ptr<ParamNode>> parameters;
        if (!check(TokenType::TOK_RIGHT_PAREN)) {
            parameters = parseParameterList();
        }
        consume(TokenType::TOK_RIGHT_PAREN, "Expected ')'");
        consume(TokenType::TOK_ARROW, "Expected '->'");
        if (!isTypeToken(peek())) {
            std::cout << "Expected return type" << std::endl;
            errorManager->addError(
                std::make_unique<CompilerError>(ErrorType::SYNTACTIC, "Expected return type", peek().line, peek().column));
            synchronize();
        }
        Lexer::Token typeToken = consume(peek().type, "Expected return type");


        consume(TokenType::TOK_LEFT_BRACE, "Expected '{'");
        if (match(TokenType::TOK_NEWLINE)) {

        }
        auto statemets = parseStatementList();
        consume(TokenType::TOK_RIGHT_BRACE, "Expected '}'");
        return std::make_unique<FunctionDefinitionNode>(identifier.lexeme, std::move(parameters), std::move(statemets), typeToken.type);
    }

    //<param> ::= <type> <identifier>
    std::vector<std::unique_ptr<ParamNode>> Parser::parseParameterList() {
        std::vector<std::unique_ptr<ParamNode>> parameters;
        do {
            Lexer::Token identifier = consume(TokenType::TOK_IDENTIFIER, "Expected parameter name");
            if (!isTypeToken(peek())) {
                errorManager->addError(
                    std::make_unique<CompilerError>(ErrorType::SYNTACTIC, "Expected parameter type", peek().line, peek().column));
                synchronize();
            }
            Lexer::Token typeToken = consume(peek().type, "Expected parameter type");
            parameters.push_back(std::make_unique<ParamNode>(typeToken.type, identifier.lexeme));
        } while (match(TokenType::TOK_COMMA));
        return parameters;
    }

    std::unique_ptr<StatementListNode> Parser::parseStatementList() {
        std::vector<std::unique_ptr<StatementNode>> statements;
        while (!check(TokenType::TOK_RIGHT_BRACE) && !isAtEnd()) {
            statements.push_back(parseStatement());
        }
        return std::make_unique<StatementListNode>(std::move(statements));
    }

    // <statement> ::= <variable_declaration> 
    //           | <assignment_statement> 
    //           | <conditional> 
    //           | <loop> <newline>
    //           | <memory_management> 
    //           | <return_statement> 
    //           | <expression> 
    std::unique_ptr<StatementNode> Parser::parseStatement() {
        if (match(TokenType::TOK_IF)) {
            return parseIfStatement();
        }
        if (isTypeToken(peek())) {
            return parseVariableDeclaration();
        }
        return nullptr;
    }
    //<variable_declaration> ::= <type> <identifier> [ "=" <expression> ] <newline>
    std::unique_ptr<StatementNode> Parser::parseVariableDeclaration() {
        Lexer::Token typeToken = consume(peek().type, "Expected type");
        Lexer::Token identifier = consume(TokenType::TOK_IDENTIFIER, "Expected variable name");
        std::unique_ptr<ExpressionNode> expression;
        // if (match(TokenType::TOK_ASSIGN)) {
         //    expression = parseExpression();
         //}
        consume(TokenType::TOK_NEWLINE, "Expected newline");
        return std::make_unique<VariableDeclarationNode>(typeToken.type, identifier.lexeme);
    }


    //<conditional> ::= "if" <expression> "{" <statement_list> "}" { "elseif" <expression> "{" <statement_list> "}" }
    // [ "else" "{" <statement_list> "}" ]
    std::unique_ptr<StatementNode> Parser::parseIfStatement() {
        // Parse initial if condition
        std::unique_ptr<ExpressionNode> condition = parseExpression();
        if (condition == nullptr) {
            errorManager->addError(
                std::make_unique<CompilerError>(ErrorType::SYNTACTIC, "Expected expression", peek().line, peek().column));
            synchronize();
        }

        // Parse if body
        consume(TokenType::TOK_LEFT_BRACE, "Expected '{'");
        auto ifStatements = parseStatementList();
        consume(TokenType::TOK_RIGHT_BRACE, "Expected '}'");

        // Vector to store elseif conditions and their bodies
        std::vector<std::pair<std::unique_ptr<ExpressionNode>, std::unique_ptr<StatementListNode>>> elseIfBranches;

        // Parse elseif branches
        while (match(TokenType::TOK_ELSEIF)) {
            auto elseIfCondition = parseExpression();
            if (elseIfCondition == nullptr) {
                errorManager->addError(
                    std::make_unique<CompilerError>(ErrorType::SYNTACTIC, "Expected expression in elseif", peek().line, peek().column));
                synchronize();
            }

            consume(TokenType::TOK_LEFT_BRACE, "Expected '{' after elseif condition");
            auto elseIfStatements = parseStatementList();
            consume(TokenType::TOK_RIGHT_BRACE, "Expected '}'");

            elseIfBranches.push_back(std::make_pair(
                std::move(elseIfCondition),
                std::move(elseIfStatements)
            ));
        }

        // Parse optional else branch
        std::unique_ptr<StatementListNode> elseStatements;
        if (match(TokenType::TOK_ELSE)) {
            consume(TokenType::TOK_LEFT_BRACE, "Expected '{' after else");
            elseStatements = parseStatementList();
            consume(TokenType::TOK_RIGHT_BRACE, "Expected '}'");
        }

        // Create and return the complete if statement node
        return std::make_unique<IfStatementNode>(
            std::move(condition),
            std::move(ifStatements),
            std::move(elseIfBranches),
            std::move(elseStatements)
        );
    }

    //<primary_expression> ::= <identifier> 
    std::unique_ptr<ExpressionNode> Parser::parseExpression() {
        auto expression = std::make_unique<IdentifierNode>(peek().lexeme);
        advance();
        return expression;
    }

    void Parser::synchronize() {
        advance();
        while (!isAtEnd()) {
            if (previous().type == TokenType::TOK_NEWLINE) {
                return;
            }
            switch (peek().type) {
            case TokenType::TOK_FUNC:
            case TokenType::TOK_IF:
            case TokenType::TOK_ELSEIF:
            case TokenType::TOK_ELSE:
            case TokenType::TOK_REPEAT:
            case TokenType::TOK_RETURN:
                return;
            default:
                break;
            }
            advance();
        }
    }
}
