
#ifndef FUCTION_DEFINITION_NODE_H
#define FUCTION_DEFINITION_NODE_H

#include "../ASTNode.h"
#include "../../../lexer/Tokens.h"
#include "param_node.h"
#include "statement_list_node.h"
#include <string>

namespace umbra {

    class FunctionDefinitionNode : public ASTNode {

    public:
        void accept(ASTVisitor &visitor) override {}
        FunctionDefinitionNode(std::string identifier, std::vector<std::unique_ptr<ParamNode>> params ,std::unique_ptr<StatementListNode> body, TokenType type)
            : identifier(identifier), parameters(std::move(params)) ,statements(std::move(body)), type(type) {}

    private:
        std::string identifier;
        std::vector<std::unique_ptr<ParamNode>> parameters;
        std::unique_ptr<StatementListNode> statements;
        TokenType type;

    };

};

#endif
