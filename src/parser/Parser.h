#ifndef PARSER_H
#define PARSER_H

#include "../lexer/Lexer.h"
#include "../lexer/Tokens.h"
#include "../error/ErrorManager.h"
#include "../ast/nodes/ASTNode.h"
#include "../ast/nodes/include/program_node.h"
#include "../ast/nodes/include/function_definition_node.h"
#include "../ast/nodes/include/parameters_node.h"
#include "../ast/nodes/include/param_node.h"

#include "../ast/nodes/include/expression_node.h"
#include "../ast/nodes/include/statement_list_node.h"
#include "../ast/nodes/include/statement_node.h"
#include "../ast/nodes/include/if_statement_node.h"
#include "../ast/nodes/include/else_statement_node.h"
#include "../ast/nodes/include/expression_node.h"
#include "../ast/nodes/include/identifier_node.h"
#include "../ast/nodes/include/logical_and_expression_node.h"
#include "../ast/nodes/include/logical_or_expression_node.h"
#include "../ast/nodes/include/relational_expression_node.h"



#include <memory>
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

        std::unique_ptr<FunctionDefinitionNode> parseFunctionDefinition();
        std::vector<std::unique_ptr<ParamNode>> parseParameterList();
        //std::unique_ptr<ParamNode> parseParam();
        //std::unique_ptr<TypeNode> parseType();
        std::unique_ptr<StatementListNode> parseStatementList();
        std::unique_ptr<StatementNode> parseIfStatement();
        std::unique_ptr<StatementNode> parseVariableDeclaration();
        std::unique_ptr<StatementNode> parseStatement();
        std::unique_ptr<ExpressionNode> parseExpression();
       // std::unique_ptr<ExpressionNode> parseLogicalOrExpression();
        //std::unique_ptr<ExpressionNode> parseLogicalAndExpression();
        //std::unique_ptr<ExpressionNode> parseEqualityExpression();
        //std::unique_ptr<ExpressionNode> parseRelationalExpression();
        //std::unique_ptr<ExpressionNode> parseAdditiveExpression();
        //std::unique_ptr<ExpressionNode> parseMultiplicativeExpression();
       // std::unique_ptr<ExpressionNode> parseUnaryExpression();
       std::unique_ptr<ExpressionNode> parsePrimaryExpression();

        //void expectToken(TokenType expectedType);
        //void error(const std::string& message, int line, int column);
    };

} // namespace umbra

#endif // PARSER_H
