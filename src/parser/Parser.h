#ifndef PARSER_H
#define PARSER_H

#include "../lexer/Lexer.h"
#include "../ast/ASTNode.h"
#include "../ast/nodes/program_node.h"
#include "../ast/nodes/function_definition_node.h"
#include "../ast/nodes/function_definition_tail_node.h"
#include "../ast/nodes/parameters_node.h"
#include "../ast/nodes/param_node.h"
#include "../ast/nodes/type_node.h"
#include "../ast/nodes/expression_node.h"
#include "../ast/nodes/statement_list_node.h"
#include "../ast/nodes/statement_node.h"
#include "../ast/nodes/if_statement_node.h"
#include "../ast/nodes/else_if_statement_node.h"
#include "../ast/nodes/else_statement_node.h"
#include "../ast/nodes/expression_node.h"
#include "../ast/nodes/identifier_node.h"
#include "../ast/nodes/logical_and_expression_node.h"
#include "../ast/nodes/logical_or_expression_node.h"
#include "../ast/nodes/relational_expression_node.h"
#include "../ast/nodes/additive_expression_node.h"
#include "../ast/nodes/multiplicative_expression_node.h"
#include "../ast/nodes/unary_expression_node.h"
#include "../ast/nodes/primary_expression_node.h"

#include <memory>
#include <vector>

namespace umbra {

class Parser {
public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<ProgramNode> parseProgram();

private:
    Lexer& lexer;
    Lexer::Token currentToken;

    void advanceToken();

    std::unique_ptr<FunctionDefinitionNode> parseFunctionDefinition();
    std::unique_ptr<FunctionDefinitionTailNode> parseFunctionDefinitionTail();
    std::unique_ptr<ParametersNode> parseParameterList();
    std::unique_ptr<ParamNode> parseParam();
    std::unique_ptr<TypeNode> parseType();

    std::unique_ptr<StatementListNode> parseStatementList();
    std::unique_ptr<StatementNode> parseIfStatement();
    std::unique_ptr<StatementNode> parseStatement();

    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parseLogicalOrExpression();
    std::unique_ptr<ExpressionNode> parseLogicalAndExpression();
    std::unique_ptr<ExpressionNode> parseEqualityExpression();
    std::unique_ptr<ExpressionNode> parseRelationalExpression();
    std::unique_ptr<ExpressionNode> parseAdditiveExpression();
    std::unique_ptr<ExpressionNode> parseMultiplicativeExpression();
    std::unique_ptr<ExpressionNode> parseUnaryExpression();
    std::unique_ptr<ExpressionNode> parsePrimaryExpression();

    void expectToken(TokenType expectedType);
    void error();
};

} // namespace umbra

#endif // PARSER_H
