#include "Parser.h"
#include "../error/ErrorManager.h"
#include <algorithm>
#include<vector>
#include <iostream>
#include <sstream>
#include <cctype>

#define UMBRA_PRINT(x) std::cout << x << std::endl;

namespace umbra {

    Parser::Parser(const std::vector<Lexer::Token>& tokens) 
    : tokens(tokens), current(tokens.begin()), errorManager(nullptr),
        previousToken(*current) {
    }

    Parser::Parser(const std::vector<Lexer::Token>& tokens, ErrorManager& externalErrorManager) 
    : tokens(tokens), current(tokens.begin()),
        errorManager(&externalErrorManager), 
        previousToken(*current) {
    }

    bool Parser::match(TokenType type) {
        #ifdef UMBRA_DEBUG
        std::cout << "Matching token: " 
                  << static_cast<int>(current->type) 
                  << " with " << static_cast<int>(type) 
                  << std::endl;
        #endif
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    std::string getTypeToString(Lexer::Token tk){
        switch(tk.type){
            case TokenType::TOK_INT:
                return tk.lexeme;
            case TokenType::TOK_FLOAT:
                return tk.lexeme;
            case TokenType::TOK_BOOL:
                return tk.lexeme;
            case TokenType::TOK_CHAR:
                return tk.lexeme;
            case TokenType::TOK_STRING:
                return tk.lexeme;
            default:
                return "";
        }
    }

    Lexer::Token Parser::lookAhead(int distance) {
        if (current + distance < tokens.end()) {
            return *(current + distance);
        }
        return tokens.back();
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
        ss << "Getting previous token: " 
           << static_cast<int>(previousToken.type) 
           << std::endl;
        UMBRA_PRINT(ss.str());
        #endif
        return previousToken; 
    }

    bool isNumber(const std::string &str) {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    }

    void Parser::skipNewLines(){
        while(match(TokenType::TOK_NEWLINE));
    }

    Lexer::Token Parser::peek() const { return *current; }

    bool Parser::isAtEnd() const { return peek().type == TokenType::TOK_EOF; }

    Lexer::Token Parser::consume(TokenType type, const std::string& message) {
        Lexer::Token currentToken = peek();
        #ifdef UMBRA_DEBUG
        std::cout << "Consuming token: " 
                  << static_cast<int>(currentToken.type) 
                  << " with " << static_cast<int>(type) 
                  << std::endl;
        #endif
        if (check(type)) {
            advance();
            return currentToken;
        }
        errorManager->addError(
            std::make_unique<CompilerError>(ErrorType::SYNTACTIC, 
            message, peek().line, peek().column));
        throw std::runtime_error(message);
    }

    bool Parser::isTypeToken(const Lexer::Token& token) {
        return token.type == TokenType::TOK_INT 
        || token.type == TokenType::TOK_FLOAT 
        || token.type == TokenType::TOK_BOOL 
        || token.type == TokenType::TOK_CHAR 
        || token.type == TokenType::TOK_STRING 
        || token.type == TokenType::TOK_ARRAY 
        || token.type == TokenType::TOK_VOID;
    }

    //<program> ::= { <function_definition> }
    std::shared_ptr<ProgramNode> Parser::parseProgram() {
        std::vector<std::unique_ptr<FunctionDefinition>> functionDefinitions;
        UMBRA_PRINT("Parsing program");
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
        UMBRA_PRINT("Parsing function definition");
        if(!match(TokenType::TOK_FUNC)){
            throw std::runtime_error("Expected 'func' keyword");
        }

        Lexer::Token name = consume(TokenType::TOK_IDENTIFIER,
        "Expected function name");

        consume(TokenType::TOK_LEFT_PAREN, 
        "Expected '(' after function name");

        std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters;

        if(!check(TokenType::TOK_RIGHT_PAREN)){ //Parse params
            do{
                UMBRA_PRINT("Parsing parameter");
                Lexer::Token paramName = consume(TokenType::TOK_IDENTIFIER,
                "Expected parameter name");
                auto paramType = parseType();
                auto paramId = std::make_unique<Identifier>(paramName.lexeme);
                parameters.push_back(std::make_pair(std::move(paramType), 
                std::move(paramId)));
            } while(match(TokenType::TOK_COMMA));
        }

        consume(TokenType::TOK_RIGHT_PAREN, "Expected ')' after parameter list");
        consume(TokenType::TOK_ARROW, "Expected '->' after parameter list");

        auto returnType = parseType();

        consume(TokenType::TOK_LEFT_BRACE, "Expected '{' before function body");
        if(check(TokenType::TOK_NEWLINE)){
            advance();
        }
        auto body = parseStatementList();
        UMBRA_PRINT("FUNC CURRENT TOKEN: "+current->lexeme);
        consume(TokenType::TOK_RIGHT_BRACE, "Expected '}' after function body");

        if(match(TokenType::TOK_NEWLINE)) {
            return std::make_unique<FunctionDefinition>(
                std::make_unique<Identifier>(name.lexeme
                ), 
                std::make_unique<ParameterList>(
                    std::move(parameters)
                ), std::move(returnType), 
                std::move(body));
        }
        else {
            error("Expected newline after function body", name.line, name.column);
            return nullptr;
        }
    }

    std::unique_ptr<Type> Parser::parseType(){
        UMBRA_PRINT("Parsing type");
        if(isTypeToken(peek()) == false){
            throw std::runtime_error("Expected type token");
        }
        auto typeStr = getTypeToString(peek());
        advance();
        return std::make_unique<Type>(typeStr);
    }

    std::vector<std::unique_ptr<Statement>> Parser::parseStatementList(){
        UMBRA_PRINT("Parsing statement list");
        std::vector<std::unique_ptr<Statement>> statements;
        while(!isAtEnd()){
            UMBRA_PRINT("STATEMENT LIST CURRENT TOKEN: "+peek().lexeme);
            auto statement = parseStatement();
            if(statement == nullptr){
                UMBRA_PRINT("PARSED STATEMENT IS NULL");
                return statements;
            }
            if(!match(TokenType::TOK_NEWLINE)){
                error("Expected newline after statement", peek().line, peek().column);
            }

            statements.push_back(std::move(statement));
        } 
        return statements;
    }

    std::unique_ptr<Statement> Parser::parseStatement(){
        UMBRA_PRINT("STATEMENT CURRENT TOKEN: "+peek().lexeme);
        if(isTypeToken(peek())){
            UMBRA_PRINT(peek().lexeme);
            return parseVariableDeclaration();
        }
        return nullptr;
    }

    std::unique_ptr<Statement> Parser::parseReturnStatement(){
        UMBRA_PRINT(peek().lexeme);
        consume(TokenType::TOK_RETURN, "Expected 'return' keyword");
        auto returnValue = parseExpression();
        return std::make_unique<ReturnStatement>(std::move(returnValue));
    }

    std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(){
        auto type = parseType();
        Lexer::Token name = consume(TokenType::TOK_IDENTIFIER, "Expected variable name");
        UMBRA_PRINT(peek().lexeme);
        if(match(TokenType::TOK_ASSIGN)){
            UMBRA_PRINT("Parsing initializer");
            
            auto initializer = parseExpression();
            if(!initializer){
                error("Expected expression after '='", peek().line, peek().column);
                synchronize();
                return nullptr;
            }
            UMBRA_PRINT("Parsed variable declaration with initializer");
            return std::make_unique<VariableDeclaration>(std::move(type), 
            std::make_unique<Identifier>(name.lexeme), std::move(initializer));
        }
        return std::make_unique<VariableDeclaration>(std::move(type), 
        std::make_unique<Identifier>(name.lexeme), nullptr);
    }

    std::unique_ptr<Expression> Parser::parseExpression(){
        return parseLogicalOr();
    }

    std::unique_ptr<Expression> Parser::parseLogicalOr(){
        UMBRA_PRINT("PARSE LOGICAL OR NODE WITH CURRENT TOKEN: "+ peek().lexeme);
        auto expr = parseLogicalAnd();
        while(match(TokenType::TOK_OR)){
            auto op = previous();
            auto right = parseLogicalAnd();
            expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parseLogicalAnd(){
        UMBRA_PRINT("PARSE LOGICAL AND NODE WITH CURRENT TOKEN: "+ peek().lexeme);
        auto expr = parseEquality();
        while(match(TokenType::TOK_AND)){
            auto op = previous();
            auto right = parseEquality();
            expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parseEquality(){
        UMBRA_PRINT("PARSE EQUALITY NODE WITH CURRENT TOKEN: "+ peek().lexeme);
        auto expr = parseRelational();
        while(match(TokenType::TOK_EQUAL)){
            auto op = previous();
            auto right = parseRelational();
            expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parseRelational(){
        UMBRA_PRINT("PARSE RELATIONAL NODE WITH CURRENT TOKEN: "+ peek().lexeme);
        auto expr = parseAditive();
        while(check(TokenType::TOK_LESS) || check(TokenType::TOK_GREATER) || check(TokenType::TOK_LESS_EQ) || check(TokenType::TOK_GREATER_EQ)){
            auto op = advance();
            auto right = parseAditive();
            expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parseAditive(){
        UMBRA_PRINT("PARSE ADITIVE NODE WITH CURRENT TOKEN: "+ peek().lexeme);
        auto expr = parseMultiplicative();
        while(check(TokenType::TOK_ADD) || check(TokenType::TOK_MINUS)){
            auto op = advance();
            auto right = parseMultiplicative();
            expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parseMultiplicative(){
        UMBRA_PRINT("PARSE MULTIPLICATIVE NODE WITH CURRENT TOKEN: "+ peek().lexeme);
        auto expr = parseUnary();
        while(match(TokenType::TOK_MULT) || match(TokenType::TOK_DIV) || match(TokenType::TOK_MOD)){
            auto op = previous();
            auto right = parseUnary();
            expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parseUnary(){
        UMBRA_PRINT("PARSE UNARY NODE WITH CURRENT TOKEN: "+ peek().lexeme);
        if(check(TokenType::TOK_PTR) || check(TokenType::TOK_REF) || check(TokenType::TOK_ACCESS)){
            auto op = advance();
            auto operand = parsePrimary();
            return std::make_unique<UnaryExpression>(op.lexeme, std::move(operand));
        }
        return parsePrimary();
    }

    std::unique_ptr<Expression> Parser::parsePrimary(){
        std::cout << "PARSE PRIMARY NODE WITH CURRENT TOKEN: " << static_cast<int>(peek().type) << std::endl;
        if(check(TokenType::TOK_IDENTIFIER)){
            return parseIdentifier();
        }if(check(TokenType::TOK_NUMBER) 
        || check(TokenType::TOK_CHAR) 
        || check(TokenType::TOK_STRING)
        || check(TokenType::TOK_TRUE)
        || check(TokenType::TOK_FALSE)){
            return parseLiteral();
        }if(check(TokenType::TOK_LEFT_PAREN)){
            advance();
            auto expr = parseExpression();
            consume(TokenType::TOK_RIGHT_PAREN, "Expected ')' after expression");
            return std::make_unique<PrimaryExpression>(std::move(expr));
        }
        return nullptr;
    }

    std::unique_ptr<Literal> Parser::parseLiteral(){
        UMBRA_PRINT("PARSE LITERAL NODE WITH CURRENT TOKEN: "+ peek().lexeme);
        auto literal = peek();
        advance();
        switch (literal.type){
            case TokenType::TOK_NUMBER:
                return std::make_unique<Literal>(literal.lexeme);
            case TokenType::TOK_CHAR:
                return std::make_unique<CharLiteral>(literal.lexeme[0]);
            case TokenType::TOK_STRING:
                return std::make_unique<StringLiteral>(literal.lexeme);
            default:
                if(literal.type == TokenType::TOK_TRUE || literal.type == TokenType::TOK_FALSE){
                    return std::make_unique<BooleanLiteral>(literal.type == TokenType::TOK_TRUE);
                }
                else{
                    error("Expected literal", literal.line, literal.column);
                    return nullptr;
                }
        }
    }

    std::unique_ptr<Expression> Parser::parseIdentifier(){
        UMBRA_PRINT(static_cast<int>(peek().type));
        auto id = consume(TokenType::TOK_IDENTIFIER, "Expected identifier");
        return std::make_unique<Identifier>(id.lexeme);
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
