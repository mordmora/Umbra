#include "Parser.h"
#include "../error/ErrorManager.h"
#include<vector>
#include <iostream>
#include <sstream>

#define UMBRA_PRINT(x) std::cout << x << std::endl;

namespace umbra {

    Parser::Parser(const std::vector<Lexer::Token>& tokens) : tokens(tokens), current(tokens.begin()), errorManager(nullptr),
        previousToken(*current) {
    }

    Parser::Parser(const std::vector<Lexer::Token>& tokens, ErrorManager& externalErrorManager) : tokens(tokens), current(tokens.begin()),
        errorManager(&externalErrorManager), previousToken(*current) {
    }

    bool Parser::match(TokenType type) {
        #ifdef UMBRA_DEBUG
        std::cout << "Matching token: " << static_cast<int>(current->type) << " with " << static_cast<int>(type) << std::endl;
        #endif
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool Parser::check(TokenType type) const {
        if (isAtEnd()) {
            #ifdef UMBRA_DEBUG
            std::stringstream ss;
            ss << "Check failed - at end of input" << std::endl;
            UMBRA_PRINT(ss.str());
            #endif
            return false;
        }
        #ifdef UMBRA_DEBUG
        std::stringstream ss;
        ss << "Checking token type: " << static_cast<int>(type) 
           << " against current: " << static_cast<int>(peek().type) << std::endl;
        UMBRA_PRINT(ss.str());
        #endif
        return peek().type == type;
    }

    Lexer::Token Parser::advance() {
        if (!isAtEnd()) {
            #ifdef UMBRA_DEBUG
            std::stringstream ss;
            ss << "Advancing token: " << static_cast<int>(current->type)
               << " at position: " << (current - tokens.begin()) << std::endl;
            ss << "Token text: \"" << current->lexeme << "\"" << std::endl;
            UMBRA_PRINT(ss.str());
            #endif
            previousToken = *current;
            return *current++;
        }
        #ifdef UMBRA_DEBUG
        std::stringstream ss;
        ss << "At end of input - returning previous token" << std::endl;
        UMBRA_PRINT(ss.str());
        #endif
        return previous();
    }

    Lexer::Token Parser::previous() const { 
        #ifdef UMBRA_DEBUG
        std::stringstream ss;
        ss << "Getting previous token: " << static_cast<int>(previousToken.type) << std::endl;
        UMBRA_PRINT(ss.str());
        #endif
        return previousToken; 
    }

    Lexer::Token Parser::peek() const { return *current; }

    bool Parser::isAtEnd() const { return peek().type == TokenType::TOK_EOF; }

    Lexer::Token Parser::consume(TokenType type, const std::string& message) {
        Lexer::Token currentToken = peek();
        #ifdef UMBRA_DEBUG
        std::cout << "Consuming token: " << static_cast<int>(currentToken.type) << " with " << static_cast<int>(type) << std::endl;
        #endif
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
        std::vector<std::unique_ptr<FunctionDefinition>> functionDefinitions;
        while (!isAtEnd()) {
            functionDefinitions.push_back(parseFunctionDefinition());
            if(match(TokenType::TOK_NEWLINE)) {
                continue;
            }

        }
        return std::make_shared<ProgramNode>(std::move(functionDefinitions));
    }
    //<function_definition> ::= "func" <identifier> "(" [ <parameter_list> ] ")" "->" <type> "{" <statement_list> "}" "\n"
    std::unique_ptr<FunctionDefinition> Parser::parseFunctionDefinition(){
        if(!match(TokenType::TOK_FUNC)){
            throw std::runtime_error("Expected 'func' keyword");
        }

        Lexer::Token name = consume(TokenType::TOK_IDENTIFIER, "Expected function name");

        consume(TokenType::TOK_LEFT_PAREN, "Expected '(' after function name");

        std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters;

        if(!check(TokenType::TOK_RIGHT_PAREN)){
            do{
                Lexer::Token paramName = consume(TokenType::TOK_IDENTIFIER, "Expected parameter name");
                consume(TokenType::TOK_COMMA, "Expected ',' after parameter name");
                auto paramType = parseType();
                auto paramId = std::make_unique<Identifier>(paramName.lexeme);
                parameters.push_back(std::make_pair(std::move(paramType), std::move(paramId)));
            } while(match(TokenType::TOK_COMMA));
        }

        consume(TokenType::TOK_RIGHT_PAREN, "Expected ')' after parameter list");
        consume(TokenType::TOK_ARROW, "Expected '->' after parameter list");

        auto returnType = parseType();

        consume(TokenType::TOK_LEFT_BRACE, "Expected '{' before function body");
        auto body = parseStatementList();
        consume(TokenType::TOK_RIGHT_BRACE, "Expected '}' after function body");

        if(match(TokenType::TOK_NEWLINE)) {
            return std::make_unique<FunctionDefinition>(std::make_unique<Identifier>(name.lexeme), 
                std::make_unique<ParameterList>(std::move(parameters)), std::move(returnType), std::move(body));
        }
        else {
            error("Expected newline after function body", name.line, name.column);
            return nullptr;
        }
    }



    void Parser::error(const std::string& message, int line, int column) {
        errorManager->addError(std::make_unique<CompilerError>(ErrorType::SYNTACTIC, message, line, column));
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
