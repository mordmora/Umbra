#ifndef PARSER_H
#define PARSER_H

#include "../ast/ASTNode.h"

#include "../lexer/Lexer.h"
#include "../ast/nodes/program_node.h"
#include "../ast/nodes/function_definition_node.h"
#include "../ast/nodes/function_definition_tail_node.h"
#include "../ast/nodes/parameters_node.h"
#include "../ast/nodes/param_node.h"
#include "../ast/nodes/type_node.h"
#include "../ast/nodes/expression_node.h"
#include <memory>
#include <vector>

namespace umbra {

    using Token = Lexer::Token;

    class Parser {
    public:
        explicit Parser(Lexer& lexer);

        std::unique_ptr<ProgramNode> parseProgram();

    private:
        Lexer& lexer;

        Token currentToken;

        void advanceToken();

        std::unique_ptr<FunctionDefinitionNode> parseFunctionDefinition();
        std::unique_ptr<FunctionDefinitionTailNode> parseFunctionDefinitionTail();
        std::unique_ptr<ParametersNode> parseParameterList();
        std::unique_ptr<ParamNode> parseParam();
        std::unique_ptr<TypeNode> parseType();

        std::unique_ptr<StatementListNode> parseStatementList();
        std::unique_ptr<StatementNode> parseStatement();

        std::unique_ptr<ExpressionNode> parseExpression();

        void expectToken(TokenType expectedType);
        void throwError(const std::string& message);


    };

};
#endif // PARSER_H
