#ifndef PARSER_H
#define PARSER_H

#include "../lexer/Lexer.h"
#include "../lexer/Tokens.h"
#include "../error/ErrorManager.h"
#include "../ast/ASTNode.h"
#include "../ast/Nodes.h"


#include <vector>

namespace umbra {

    class Parser {
    public:

        Parser(const std::vector<Lexer::Token>& tokens);
        Parser(const std::vector<Lexer::Token>& tokens, ErrorManager& externalErrorManager);
        std::shared_ptr<ProgramNode> parseProgram();

    private:

        std::vector<Lexer::Token> tokens;
        std::vector<Lexer::Token>::const_iterator current;
        ErrorManager* errorManager;
        Lexer::Token previousToken;

        bool isTypeToken(const Lexer::Token& token);
        bool match(TokenType type);
        bool check(TokenType type) const;
        Lexer::Token advance();
        Lexer::Token previous() const;
        Lexer::Token peek() const;
        bool isAtEnd() const;
        Lexer::Token consume(TokenType type, const std::string& message);
        void synchronize(); // Error recovery

       // void advanceToken();

        std::unique_ptr<FunctionDefinition> parseFunctionDefinition();
        std::unique_ptr<Type> parseType();
        std::vector<std::unique_ptr<Statement>> parseStatementList();
        std::unique_ptr<Statement> parseStatement();
        std::unique_ptr<VariableDeclaration> parseVariableDeclaration();
        std::unique_ptr<Expression> parseExpression();
        std::unique_ptr<Expression> parseLogicalOr();
        std::unique_ptr<Expression> parseLogicalAnd();
        std::unique_ptr<Expression> parseEquality();
        std::unique_ptr<Expression> parseRelational();
        std::unique_ptr<Expression> parseAditive();
        std::unique_ptr<Expression> parseMultiplicative();
        std::unique_ptr<Expression> parseUnary();
        std::unique_ptr<Expression> parsePrimary();
        std::unique_ptr<Expression> parseIdentifier();
        std::unique_ptr<Statement> parseReturnStatement();
        

        //void expectToken(TokenType expectedType);
        void error(const std::string& message, int line, int column);
    };

} // namespace umbra

#endif // PARSER_H
